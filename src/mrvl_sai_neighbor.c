/*
 *  Copyright (C) 2016. Marvell International Ltd. ALL RIGHTS RESERVED.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 *    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 *    See the Apache Version 2.0 License for specific language governing
 *    permissions and limitations under the License.
 *
 */
 
#include "sai.h"
#include "mrvl_sai.h"
#include "assert.h"

#undef  __MODULE__
#define __MODULE__ SAI_NEIGHBOR

#define MRVL_SAI_NBR_INVALID_ID_CNS 0xFFFFFFFF

static mrvl_sai_nbr_table_t mrvl_sai_nbr_table[SAI_NEIGHBOR_TABLE_SIZE_CNS] = {};

static const sai_attribute_entry_t mrvl_sai_neighbor_attribs[] = {
    { SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS, true, true, true, true,
      "Neighbor destination MAC", SAI_ATTR_VAL_TYPE_MAC },
    { SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION, false, true, true, true,
      "Neighbor L3 forwarding action", SAI_ATTR_VAL_TYPE_S32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_neighbor_mac_get_prv(_In_ const sai_object_key_t   *key,
                                   _Inout_ sai_attribute_value_t *value,
                                   _In_ uint32_t                  attr_index,
                                   _Inout_ vendor_cache_t        *cache,
                                   void                          *arg);
static sai_status_t mrvl_sai_neighbor_action_get_prv(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg);
static sai_status_t mrvl_sai_neighbor_mac_set_prv(_In_ const sai_object_key_t      *key,
                                   _In_ const sai_attribute_value_t *value,
                                   void                             *arg);
static sai_status_t mrvl_sai_neighbor_action_set_prv(_In_ const sai_object_key_t      *key,
                                      _In_ const sai_attribute_value_t *value,
                                      void                             *arg);

static const sai_vendor_attribute_entry_t mrvl_sai_neighbor_vendor_attribs[] = {
    { SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_neighbor_mac_get_prv, NULL,
      mrvl_sai_neighbor_mac_set_prv, NULL },
    { SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION,
      { true, false, false, true },
      { true, false, true, true },
      mrvl_sai_neighbor_action_get_prv, NULL,
      mrvl_sai_neighbor_action_set_prv, NULL },
};
static void mrvl_sai_neighbor_key_to_str(_In_ const sai_neighbor_entry_t* neighbor_entry, _Out_ char *key_str)
{
    int res1, res2, rif_idx;

    res1 = snprintf(key_str, MAX_KEY_STR_LEN, "neighbor ip ");
    mrvl_sai_utl_ipaddr_to_str(neighbor_entry->ip_address, MAX_KEY_STR_LEN - res1, key_str + res1, &res2);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(neighbor_entry->rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("invalid rif id\n");
    }
    snprintf(key_str + res1 + res2, MAX_KEY_STR_LEN - res1 - res2, " rif %u", rif_idx);
}

sai_status_t mrvl_sai_neighbor_dump(void)
{
    char         key_str[MAX_KEY_STR_LEN];
    uint32_t    idx, used = 0;
    int chars_written;
    printf("\nNeighbor table:\n");
    printf("| idx | next nbr id |  dst mac address  |   ip address    | next hop id |\n");
    printf("-------------------------------------------------------------------------\n");
    for (idx = 0; idx < SAI_NEIGHBOR_TABLE_SIZE_CNS; idx++) {
        if (mrvl_sai_nbr_table[idx].used == true) {
            mrvl_sai_utl_ipaddr_to_str(mrvl_sai_nbr_table[idx].inet_address, MAX_KEY_STR_LEN, key_str, &chars_written);
            printf("| %3d |    %8x | %02x:%02x:%02x:%02x:%02x:%02x | %15s | %11x |\n", idx, 
                   mrvl_sai_nbr_table[idx].next_nbr_id,
                   mrvl_sai_nbr_table[idx].dst_mac[0], mrvl_sai_nbr_table[idx].dst_mac[1], mrvl_sai_nbr_table[idx].dst_mac[2],
                   mrvl_sai_nbr_table[idx].dst_mac[3], mrvl_sai_nbr_table[idx].dst_mac[4], mrvl_sai_nbr_table[idx].dst_mac[5],
                   key_str, mrvl_sai_nbr_table[idx].next_hop_idx); 
            used++;
        }
    }
    printf("\nTotal used: %d\n\n",used);
    return SAI_STATUS_SUCCESS;
}



int mrvl_sai_add_route_direct(
		uint32_t                        group,
		uint32_t                        rif_idx,
		const sai_ip_address_t         *ip,
        uint64_t                        cookie
)
{
    FPA_FLOW_TABLE_ENTRY_STC        flow_entry;
    FPA_STATUS                      fpa_status;
    uint32_t                        vr_Id;
    sai_status_t                    sai_status;

    /* check if exist */
	flow_entry.cookie =  cookie;
    if (ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4)
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, &flow_entry);
    else
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_IPV6_E, &flow_entry);
    if (fpa_status==FPA_OK)
    {
    	/* already exist */
    	return 1;
    }
    sai_status = mrvl_sai_rif_get_vr_id(rif_idx, &vr_Id);
	if ( sai_status != SAI_STATUS_SUCCESS)
	{
		return 0;
	}

    if (ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, &flow_entry);
        flow_entry.data.l3_unicast.groupId = group;
        flow_entry.data.l3_unicast.match.vrfId = vr_Id;
        flow_entry.data.l3_unicast.match.dstIp4 = ip->addr.ip4;
        flow_entry.data.l3_unicast.match.dstIp4Mask = 0xffffffff;
        flow_entry.data.l3_unicast.match.etherType = 0x800;
        flow_entry.cookie =  cookie;
        fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, &flow_entry);
    }
    else
    {
        fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_IPV6_E, &flow_entry);
        flow_entry.data.l3_unicast.groupId = group;
        flow_entry.data.l3_unicast.match.vrfId = vr_Id;
        memcpy(&flow_entry.data.l3_unicast.match.dstIp6, &ip->addr.ip6, sizeof(sai_ip6_t));
        memset(&flow_entry.data.l3_unicast.match.dstIp6Mask, 0xff, sizeof(flow_entry.data.l3_unicast.match.dstIp6Mask));
        flow_entry.data.l3_unicast.match.etherType = 0x86dd;
        flow_entry.cookie =  cookie;
        fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_IPV6_E, &flow_entry);
    }
    return (fpa_status==FPA_OK);
}

int mrvl_sai_del_route_direct(
		const sai_ip_address_t         *ip,
		uint32_t                        rif_idx,
        uint64_t                        cookie
)
{
    FPA_STATUS                     fpa_status;
    if (ip->addr_family == SAI_IP_ADDR_FAMILY_IPV4)
        fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, cookie);
    else
        fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_IPV6_E, cookie);

    return  (fpa_status==FPA_OK);
}

/**


 * @brief Create neighbor entry
 *
 * Note: IP address expected in Network Byte Order.
 *
 * @param[in] neighbor_entry Neighbor entry
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_create_neighbor_entry(_In_ const sai_neighbor_entry_t *neighbor_entry,
                                            _In_ uint32_t                    attr_count,
                                            _In_ const sai_attribute_t      *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *mac, *action, *value;
    uint32_t                     mac_index, action_index, rif_idx, nbr_idx, rif_first_nbr, group;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    bool                        is_exist;
    uint32_t                    temp, nh_idx,i;
    sai_packet_action_t         act;
    char                        debug_str[52];
    uint8_t                    *ptr; 

    int route_status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == neighbor_entry) {
        MRVL_SAI_LOG_ERR("NULL neighbor entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_neighbor_attribs, mrvl_sai_neighbor_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_neighbor_key_to_str(neighbor_entry, key_str);
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_neighbor_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create neighbor entry %s\n", key_str);
    MRVL_SAI_LOG_NTC("Attribs %s\n", list_str);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(neighbor_entry->rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("invalid rif id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    status = mrvl_sai_rif_is_exist(rif_idx, &is_exist);
    if ((status != SAI_STATUS_SUCCESS) || (is_exist == false)) {
        MRVL_SAI_LOG_ERR("rif id %d does not exist\n", rif_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS, &mac, &mac_index));
    
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION, &action, &action_index))) {
        if (SAI_PACKET_ACTION_FORWARD != action->s32){
            MRVL_SAI_LOG_ERR("Action not supported %d\n", action->s32);
            MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
        }
        act = action->s32;
    } else {
        act = SAI_PACKET_ACTION_FORWARD;
    }
    
    for (nbr_idx = 0; nbr_idx < SAI_NEIGHBOR_TABLE_SIZE_CNS; nbr_idx++) {
        if (mrvl_sai_nbr_table[nbr_idx].used == false) {
            break;
        }
    }
    if (nbr_idx >= SAI_NEIGHBOR_TABLE_SIZE_CNS){
        MRVL_SAI_LOG_ERR("Neighbor table is full\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }
    /* check if this ip was already inserted */
    status = mrvl_sai_get_match_neighbor_id(rif_idx , &neighbor_entry->ip_address, &temp);
    if (status == SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't add  neighbor - already exist %d\n", temp);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }                                          

    /* add the entry to FPA l3 group table */
    memcpy(mrvl_sai_nbr_table[nbr_idx].dst_mac , mac->mac, 6);
    status = mrvl_sai_utl_create_l3_unicast_group(nbr_idx, mrvl_sai_nbr_table[nbr_idx].dst_mac, rif_idx, act, &group);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't create group %d\n", nbr_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    /* add the entry to SAI neighbor table */
    mrvl_sai_nbr_table[nbr_idx].used = true;
    mrvl_sai_nbr_table[nbr_idx].next_hop_idx =  0xFFFFFFFF;
    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4) { 
        mrvl_sai_nbr_table[nbr_idx].inet_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        mrvl_sai_nbr_table[nbr_idx].inet_address.addr.ip4 = /*ntohl*/(neighbor_entry->ip_address.addr.ip4);        
    }
    else {
        mrvl_sai_nbr_table[nbr_idx].inet_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        for (i=0; i<16; i++) {
            mrvl_sai_nbr_table[nbr_idx].inet_address.addr.ip6[i] = /*ntohl*/(neighbor_entry->ip_address.addr.ip6[i]);
        }        
    }

    status = mrvl_sai_rif_add_first_nbr_id(rif_idx, nbr_idx, &rif_first_nbr);
    if (status != SAI_STATUS_SUCCESS) {
        mrvl_sai_utl_delete_l3_unicast_group(nbr_idx);
        MRVL_SAI_LOG_ERR("Can't add first neighbor\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    } else {
        mrvl_sai_nbr_table[nbr_idx].next_nbr_id = rif_first_nbr;
    }

    /* look in the next hop table for entry with ip match to update its nbr_id */
    status = mrvl_sai_next_hop_update_nbr_id(rif_idx, nbr_idx, &mrvl_sai_nbr_table[nbr_idx].inet_address, &nh_idx);
    if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_NOT_FOUND)) {
        MRVL_SAI_LOG_ERR("Can't update neighbor\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    mrvl_sai_nbr_table[nbr_idx].next_hop_idx = nh_idx;

    /* look in the rif table for entry with match rif_id or nh_id */
    status = mrvl_sai_rif_update_nbr_id(rif_idx, nbr_idx, &mrvl_sai_nbr_table[nbr_idx].inet_address);
    if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_NOT_FOUND)) {
        MRVL_SAI_LOG_ERR("Can't update rif\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE, &value, &action_index)))
    {
        route_status = mrvl_sai_add_route_direct(group, rif_idx, &neighbor_entry->ip_address, (uint64_t)&mrvl_sai_nbr_table[nbr_idx]);
        if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
        {
            ptr = (uint8_t *)&neighbor_entry->ip_address.addr.ip4;
            sprintf(debug_str, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
	        MRVL_SAI_LOG_NTC("ADD direct route to neighbor group %d, rif_idx %d, ipv4 %s, status %d\n", group, rif_idx, debug_str, route_status);
        }
        else
        {
            ptr = (uint8_t *)&neighbor_entry->ip_address.addr.ip6;
            sprintf(debug_str, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
                                ptr[0], ptr[1], ptr[2], ptr[3],
                                ptr[4], ptr[5], ptr[6], ptr[7],
                                ptr[8], ptr[9], ptr[10], ptr[11],
                                ptr[12], ptr[13], ptr[14], ptr[15]);
            MRVL_SAI_LOG_NTC("ADD direct route to neighbor group %d, rif_idx %d, ipv6 %s, status %d\n", group, rif_idx, debug_str, route_status);
        }
    }

    MRVL_SAI_LOG_EXIT();

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


static sai_status_t mrvl_sai_neighbor_remove_entry_prv(uint32_t first_nbr, uint32_t deleted_nbr_idx, uint32_t *new_first_idx)
{
    uint32_t last_nbr, nbr;
    sai_status_t    status;
        
    status = mrvl_sai_utl_delete_l3_unicast_group(deleted_nbr_idx);  
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    
    if (deleted_nbr_idx == first_nbr) {
        *new_first_idx = mrvl_sai_nbr_table[deleted_nbr_idx].next_nbr_id;
        memset(&mrvl_sai_nbr_table[deleted_nbr_idx], 0, sizeof(mrvl_sai_nbr_table_t)); 
        return SAI_STATUS_SUCCESS;
    } else {
        *new_first_idx = first_nbr;
    }
    nbr = first_nbr;
    while ((deleted_nbr_idx != nbr) && (nbr != MRVL_SAI_INVALID_ID_CNS)) {
        last_nbr = nbr;
        nbr = mrvl_sai_nbr_table[nbr].next_nbr_id;
    }
    if (deleted_nbr_idx == nbr) {
        mrvl_sai_nbr_table[last_nbr].next_nbr_id = mrvl_sai_nbr_table[deleted_nbr_idx].next_nbr_id;
        memset(&mrvl_sai_nbr_table[deleted_nbr_idx], 0, sizeof(mrvl_sai_nbr_table_t));       
        return SAI_STATUS_SUCCESS;
    } else {
        return SAI_STATUS_FAILURE;
    }
}


/**

 * @brief Remove neighbor entry
 *
 * Note: IP address expected in Network Byte Order.
 *
 * @param[in] neighbor_entry Neighbor entry
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_remove_neighbor_entry(_In_ const sai_neighbor_entry_t *neighbor_entry)
{
    sai_status_t    status;
    char            key_str[MAX_KEY_STR_LEN];
    uint32_t        rif_idx, nbr_idx, rif_first_nbr, new_first_idx, nh_idx;
    int             route_status;
    char            debug_str[52];
    uint8_t        *ptr; 

    MRVL_SAI_LOG_ENTER();

    if (NULL == neighbor_entry) {
        MRVL_SAI_LOG_ERR("NULL neighbor entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(neighbor_entry->rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("invalid rif id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* find current neighbor id */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_match_neighbor_id(rif_idx, &neighbor_entry->ip_address, &nbr_idx))) {
    	MRVL_SAI_API_RETURN(status);
    }

    route_status = mrvl_sai_del_route_direct(&neighbor_entry->ip_address, rif_idx, (uint64_t)&mrvl_sai_nbr_table[nbr_idx]);
    if ( neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        ptr = (uint8_t *)&neighbor_entry->ip_address.addr.ip4;
        sprintf(debug_str, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
    	MRVL_SAI_LOG_NTC("REMOVE direct route to neighbor rif_idx %d, ipv4 %s, status %d\n", rif_idx, debug_str, route_status);
    }
    else
    {
        ptr = (uint8_t *)&neighbor_entry->ip_address.addr.ip6;
        sprintf(debug_str, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
                            ptr[0], ptr[1], ptr[2], ptr[3],
                            ptr[4], ptr[5], ptr[6], ptr[7],
                            ptr[8], ptr[9], ptr[10], ptr[11],
                            ptr[12], ptr[13], ptr[14], ptr[15]);
        MRVL_SAI_LOG_NTC("REMOVE direct route to neighbor rif_idx %d, ipv6 %s, status %d\n", rif_idx, debug_str, route_status);
    }

    status = mrvl_sai_rif_get_first_nbr_id(rif_idx, &rif_first_nbr);
    if ((status != SAI_STATUS_SUCCESS) || (rif_first_nbr == MRVL_SAI_INVALID_ID_CNS)) {
        MRVL_SAI_LOG_ERR("Can't get first neighbor\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }    

    status = mrvl_sai_next_hop_update_nbr_id(rif_idx, MRVL_SAI_INVALID_ID_CNS, &mrvl_sai_nbr_table[nbr_idx].inet_address, &nh_idx);
    if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_NOT_FOUND)) {
        MRVL_SAI_LOG_ERR("Can't update neighbor\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    /* look in the rif table for entry with match rif_id or nh_id */
    status = mrvl_sai_rif_update_nbr_id(rif_idx, MRVL_SAI_INVALID_ID_CNS, &mrvl_sai_nbr_table[nbr_idx].inet_address);
    if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_NOT_FOUND)) {
        MRVL_SAI_LOG_ERR("Can't update rif\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    
    /* remove nbr from list */
    status = mrvl_sai_neighbor_remove_entry_prv(rif_first_nbr, nbr_idx, &new_first_idx);
    if (status == SAI_STATUS_SUCCESS) {
        if (new_first_idx != rif_first_nbr) {
            status = mrvl_sai_rif_add_first_nbr_id(rif_idx, new_first_idx, NULL);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("Can't add first nexthop\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }    
        }
    }

    //mrvl_sai_nbr_table[nbr_idx].used = false;

    mrvl_sai_neighbor_key_to_str(neighbor_entry, key_str);
    MRVL_SAI_LOG_NTC("Remove neighbor entry %s\n", key_str);


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**


 * @brief Set neighbor entry attribute value
 *
 * @param[in] neighbor_entry Neighbor entry
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_set_neighbor_entry_attribute(_In_ const sai_neighbor_entry_t *neighbor_entry,
                                                   _In_ const sai_attribute_t      *attr)
{
    const sai_object_key_t key = { .key.neighbor_entry = neighbor_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;
    
    MRVL_SAI_LOG_ENTER();

    if (NULL == neighbor_entry) {
        MRVL_SAI_LOG_ERR("NULL neighbor entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_neighbor_key_to_str(neighbor_entry, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_neighbor_attribs, mrvl_sai_neighbor_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get neighbor entry attribute value
 *
 * @param[in] neighbor_entry Neighbor entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_get_neighbor_entry_attribute(_In_ const sai_neighbor_entry_t *neighbor_entry,
                                                   _In_ uint32_t                    attr_count,
                                                   _Inout_ sai_attribute_t         *attr_list)
{
    const sai_object_key_t key = { .key.neighbor_entry = neighbor_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == neighbor_entry) {
        MRVL_SAI_LOG_ERR("NULL neighbor entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_neighbor_key_to_str(neighbor_entry, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_neighbor_attribs, mrvl_sai_neighbor_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_neighbor_set_next_hop(_In_  uint32_t  nbr_idx ,
                                            _In_  uint32_t  nh_idx)
{
    if ((nbr_idx < SAI_NEIGHBOR_TABLE_SIZE_CNS) && (mrvl_sai_nbr_table[nbr_idx].used == true)) {
        mrvl_sai_nbr_table[nbr_idx].next_hop_idx = nh_idx;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_neighbor_get_next_hop(_In_  uint32_t  nbr_idx ,
                                            _Out_  uint32_t  *nh_idx)
{
    if ((nbr_idx < SAI_NEIGHBOR_TABLE_SIZE_CNS) && (mrvl_sai_nbr_table[nbr_idx].used == true)) {
        *nh_idx = mrvl_sai_nbr_table[nbr_idx].next_hop_idx;
        return SAI_STATUS_SUCCESS;
    }
    else
        return  SAI_STATUS_FAILURE;
}


/*
 * Routine Description:
 *    find neighbor id in DB
 *    Get the first nbr in the rif list and go through the list maching the ip address until a match is found
 *
 * Arguments:
 *    [in] rif_idx - router interface on which the neighbor was created
 *    [in] ip_address - the requested neighbor ip address
 *    [inout] nbr_id - the neighbor id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_match_neighbor_id(_In_  uint32_t          rif_idx ,
                                            _In_  const sai_ip_address_t  *ip_address, 
                                            _Out_ uint32_t          *nbr_id)
{
    uint32_t            nbr_idx;
    sai_status_t        status;

    MRVL_SAI_LOG_ENTER();

    *nbr_id = MRVL_SAI_INVALID_ID_CNS;
    status = mrvl_sai_rif_get_first_nbr_id(rif_idx, &nbr_idx);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    do {
        if (mrvl_sai_nbr_table[nbr_idx].used == false) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        /* check for match */
        if (ip_address->addr_family == SAI_IP_ADDR_FAMILY_IPV4){
            if (memcmp(&mrvl_sai_nbr_table[nbr_idx].inet_address.addr.ip4, &ip_address->addr.ip4, sizeof(sai_ip4_t)) == 0) {
                break;
            }
        } else {
            if (memcmp(&mrvl_sai_nbr_table[nbr_idx].inet_address.addr.ip6, &ip_address->addr.ip6, sizeof(sai_ip6_t)) == 0) {
                break;
            }
        }
        nbr_idx = mrvl_sai_nbr_table[nbr_idx].next_nbr_id; 
    } while (nbr_idx != MRVL_SAI_INVALID_ID_CNS);

    if (nbr_idx == MRVL_SAI_INVALID_ID_CNS) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *nbr_id = nbr_idx;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Destination mac address for the neighbor [sai_mac_t] */
static sai_status_t mrvl_sai_neighbor_mac_get_prv(_In_ const sai_object_key_t   *key,
                                   _Inout_ sai_attribute_value_t *value,
                                   _In_ uint32_t                  attr_index,
                                   _Inout_ vendor_cache_t        *cache,
                                   void                          *arg)
{
    sai_status_t                status;
    uint32_t                    nbr_id, rif_idx;
    const sai_neighbor_entry_t *neighbor_entry = &key->key.neighbor_entry;

    
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(neighbor_entry->rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("invalid rif id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_match_neighbor_id(rif_idx, &neighbor_entry->ip_address, &nbr_id))) {
        return status;
    }

    memcpy(value->mac, &mrvl_sai_nbr_table[nbr_id].dst_mac, sizeof(value->mac));

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* L3 forwarding action for this neighbor [sai_packet_action_t] */
static sai_status_t mrvl_sai_neighbor_action_get_prv(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_PACKET_ACTION_FORWARD;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_modify_neighbor_entry_prv(_In_ uint32_t nbr_idx,
                                                       _In_ sai_mac_t mac)
{
    sai_status_t    status;

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_modify_l3_unicast_group(nbr_idx, mac);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    memcpy(&mrvl_sai_nbr_table[nbr_idx].dst_mac, mac, 6); 
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Destination mac address for the neighbor [sai_mac_t] */
static sai_status_t mrvl_sai_neighbor_mac_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value,
                                   void *arg)
{
    sai_status_t                status;
    uint32_t                    nbr_id, rif_idx;
    sai_mac_t                   mac;
    const sai_neighbor_entry_t *neighbor_entry = &key->key.neighbor_entry;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(neighbor_entry->rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("invalid rif id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_match_neighbor_id(rif_idx, &neighbor_entry->ip_address, &nbr_id))) {
        return status;
    }

    memcpy(mac, value->mac, 6); 

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_modify_neighbor_entry_prv(nbr_id, mac))) {
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* L3 forwarding action for this neighbor [sai_packet_action_t] */
static sai_status_t mrvl_sai_neighbor_action_set_prv(_In_ const sai_object_key_t      *key,
                                      _In_ const sai_attribute_value_t *value,
                                      void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    if (value->s32 != SAI_PACKET_ACTION_FORWARD) {
        MRVL_SAI_LOG_ERR("we support only SAI_PACKET_ACTION_FORWARD\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/*
 * Routine Description:
 *    Remove all neighbor entries
 *
 * Arguments:
 *    None
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_all_neighbor_entries(void)
{
    sai_status_t        status;
    uint32_t            rif_idx, nbr_idx, delete_idx, nh_idx;
    int                 route_status;
    char                debug_str[52];
    uint8_t            *ptr; 

    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_NTC("Remove all neighbor entries\n");

    for (rif_idx = 0; rif_idx < SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS; rif_idx++) {
        status =  mrvl_sai_rif_get_first_nbr_id(rif_idx, &nbr_idx);
        if (status != SAI_STATUS_SUCCESS) {
            continue;
        }        
        status = mrvl_sai_rif_add_first_nbr_id(rif_idx, MRVL_SAI_INVALID_ID_CNS, NULL);
        if (status != SAI_STATUS_SUCCESS) {
            continue;
        }
        do {

            route_status = mrvl_sai_del_route_direct(&mrvl_sai_nbr_table[nbr_idx].inet_address, rif_idx,  (uint64_t)&mrvl_sai_nbr_table[nbr_idx]);
            if ( mrvl_sai_nbr_table[nbr_idx].inet_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
            {
                ptr = (uint8_t *)&mrvl_sai_nbr_table[nbr_idx].inet_address.addr.ip4;
                sprintf(debug_str, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
            	MRVL_SAI_LOG_NTC("REMOVE direct route to neighbor rif_idx %d, ipv4 %s, status %d\n", rif_idx, debug_str, route_status);
            }
            else
            {
                ptr = (uint8_t *)&mrvl_sai_nbr_table[nbr_idx].inet_address.addr.ip6;
                sprintf(debug_str, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
                                    ptr[0], ptr[1], ptr[2], ptr[3],
                                    ptr[4], ptr[5], ptr[6], ptr[7],
                                    ptr[8], ptr[9], ptr[10], ptr[11],
                                    ptr[12], ptr[13], ptr[14], ptr[15]);
                MRVL_SAI_LOG_NTC("REMOVE direct route to neighbor rif_idx %d, ipv6 %s, status %d\n", rif_idx, debug_str, route_status);
            }

            status = mrvl_sai_next_hop_update_nbr_id(rif_idx, MRVL_SAI_INVALID_ID_CNS, &mrvl_sai_nbr_table[nbr_idx].inet_address, &nh_idx);
            status = mrvl_sai_rif_update_nbr_id(rif_idx, MRVL_SAI_INVALID_ID_CNS, &mrvl_sai_nbr_table[nbr_idx].inet_address);
            delete_idx = nbr_idx;
            nbr_idx = mrvl_sai_nbr_table[nbr_idx].next_nbr_id;
            memset(&mrvl_sai_nbr_table[delete_idx], 0, sizeof(mrvl_sai_nbr_table_t));       
            status = mrvl_sai_utl_delete_l3_unicast_group(delete_idx);  
            if (status != SAI_STATUS_SUCCESS) {
            	MRVL_SAI_API_RETURN(status);
            }

        } while (nbr_idx != MRVL_SAI_INVALID_ID_CNS);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

const sai_neighbor_api_t neighbor_api = {
    mrvl_sai_create_neighbor_entry,
    mrvl_sai_remove_neighbor_entry,
    mrvl_sai_set_neighbor_entry_attribute,
    mrvl_sai_get_neighbor_entry_attribute,
    mrvl_sai_remove_all_neighbor_entries
};
