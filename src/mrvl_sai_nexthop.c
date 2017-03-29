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
#define __MODULE__ SAI_NEXT_HOP

static mrvl_sai_nh_table_t mrvl_sai_nh_table[SAI_NEXTHOP_TABLE_SIZE_CNS] = {};

static const sai_attribute_entry_t mrvl_sai_next_hop_attribs[] = {
    { SAI_NEXT_HOP_ATTR_TYPE, true, true, false, true,
      "Next hop entry type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_NEXT_HOP_ATTR_IP, true, true, false, true,
      "Next hop entry IP address", SAI_ATTR_VAL_TYPE_IPADDR },
    { SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID, true, true, false, true,
      "Next hop entry router interface ID", SAI_ATTR_VAL_TYPE_U32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_next_hop_type_get_prv(_In_ const sai_object_key_t   *key,
                                    _Inout_ sai_attribute_value_t *value,
                                    _In_ uint32_t                  attr_index,
                                    _Inout_ vendor_cache_t        *cache,
                                    void                          *arg);
static sai_status_t mrvl_sai_next_hop_ip_get_prv(_In_ const sai_object_key_t   *key,
                                  _Inout_ sai_attribute_value_t *value,
                                  _In_ uint32_t                  attr_index,
                                  _Inout_ vendor_cache_t        *cache,
                                  void                          *arg);
static sai_status_t mrvl_sai_next_hop_rif_get_prv(_In_ const sai_object_key_t   *key,
                                   _Inout_ sai_attribute_value_t *value,
                                   _In_ uint32_t                  attr_index,
                                   _Inout_ vendor_cache_t        *cache,
                                   void                          *arg);

static const sai_vendor_attribute_entry_t mrvl_sai_next_hop_vendor_attribs[] = {
    { SAI_NEXT_HOP_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_next_hop_type_get_prv, NULL,
      NULL, NULL },
    { SAI_NEXT_HOP_ATTR_IP,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_next_hop_ip_get_prv, NULL,
      NULL, NULL },
    { SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_next_hop_rif_get_prv, NULL,
      NULL, NULL },
};
static void mrvl_sai_next_hop_key_to_str(_In_ sai_object_id_t next_hop_id, _Out_ char *key_str)
{
    uint32_t nh_idx;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "rif id invalid");
        return ;
    }
    snprintf(key_str, MAX_KEY_STR_LEN, "next hop id %u", nh_idx);
}

sai_status_t mrvl_sai_next_hop_dump(void)
{
    char         key_str[MAX_KEY_STR_LEN];
    uint32_t    idx, used = 0;
    int chars_written;
    mrvl_sai_utl_dlist_elem_STC *dlist_elem;
    mrvl_sai_route_hash_data_t  *route_entry;
    printf("\nNext hop table:\n");
    printf("| idx | next nh id | action | neighbor id | rif id | rout_v |   ip address   | route list\n");
    printf("-----------------------------------------------------------------------------------------------\n");
    for (idx = 0; idx < SAI_NEXTHOP_TABLE_SIZE_CNS; idx++) {
        if (mrvl_sai_nh_table[idx].used == true) {
            mrvl_sai_utl_ipaddr_to_str(mrvl_sai_nh_table[idx].inet_address, MAX_KEY_STR_LEN, key_str, &chars_written);
            printf("| %3d |   %8x | %s |    %8x | %6d |    %2d  |%15s |", idx, 
                   mrvl_sai_nh_table[idx].next_nh_id,
                   (mrvl_sai_nh_table[idx].nbr_id == MRVL_SAI_NEIGHBOR_TRAP_ID_CNS)?"  TRAP": 
                   ((mrvl_sai_nh_table[idx].nbr_id == MRVL_SAI_NEIGHBOR_DROP_ID_CNS)?"  DROP":"   FRW"),
                   mrvl_sai_nh_table[idx].nbr_id,
                   mrvl_sai_nh_table[idx].rif_id,
                   (mrvl_sai_utl_dlist_is_empty(&mrvl_sai_nh_table[idx].route_list_elem))?0:1,
                   key_str); 
            used++;
            if (!mrvl_sai_utl_dlist_is_empty(&mrvl_sai_nh_table[idx].route_list_elem)) {
                dlist_elem = &mrvl_sai_nh_table[idx].route_list_elem;
                do {
                    dlist_elem = mrvl_sai_utl_dlist_get_next(dlist_elem); 
                    route_entry = (mrvl_sai_route_hash_data_t*)dlist_elem;
                    printf("0x%x, ",route_entry->cookie);
                    
                } while (!mrvl_sai_utl_dlist_is_last(&mrvl_sai_nh_table[idx].route_list_elem, dlist_elem));
            } 
            printf("\n");
        }
    }
    printf("\nTotal used: %d\n\n",used);
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_next_hop_get_match_id(_In_  uint32_t          nh_idx ,
                                            _In_  const sai_ip_address_t  *ip_address, 
                                            _Out_ uint32_t          *match_id)
{
    MRVL_SAI_LOG_ENTER();

    *match_id = MRVL_SAI_INVALID_ID_CNS;
    do {
        if (mrvl_sai_nh_table[nh_idx].used == false) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        /* check for match */
        if (ip_address->addr_family == SAI_IP_ADDR_FAMILY_IPV4){
            if (memcmp(&mrvl_sai_nh_table[nh_idx].inet_address.addr.ip4, &ip_address->addr.ip4, sizeof(sai_ip4_t)) == 0) {
                break;
            }
        } else {
            if (memcmp(&mrvl_sai_nh_table[nh_idx].inet_address.addr.ip6, &ip_address->addr.ip6, sizeof(sai_ip6_t)) == 0) {
                break;
            }
        }
        nh_idx = mrvl_sai_nh_table[nh_idx].next_nh_id; 
    } while (nh_idx != MRVL_SAI_INVALID_ID_CNS);

    if (nh_idx == MRVL_SAI_INVALID_ID_CNS) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *match_id = nh_idx;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Create next hop
 *
 * Arguments:
 *    [out] next_hop_id - next hop id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
sai_status_t mrvl_sai_create_next_hop(_Out_ sai_object_id_t   * next_hop_id,
                                  _In_ uint32_t               attr_count,
                                  _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                status;
    const sai_attribute_value_t *type, *ip, *rif;
    uint32_t                    nh_idx, type_index, ip_index, rif_index, rif_idx, rif_first_nh, nbr_idx;
    char                        list_str[MAX_LIST_VALUE_STR_LEN];
    char                        key_str[MAX_KEY_STR_LEN];
    bool                        is_exist;
    sai_packet_action_t         nbr_miss_act;
    
    MRVL_SAI_LOG_ENTER();

    if (NULL == next_hop_id) {
        MRVL_SAI_LOG_ERR("NULL next hop id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_next_hop_attribs, mrvl_sai_next_hop_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_next_hop_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create next hop, %s\n", list_str);

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_ATTR_TYPE, &type, &type_index));
    if (SAI_NEXT_HOP_IP != type->s32) {
        MRVL_SAI_LOG_ERR("Invalid next hop type %d on create\n", type->s32);
        status =  SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
        MRVL_SAI_API_RETURN(status);
    }
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_ATTR_IP, &ip, &ip_index));
    
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID, &rif, &rif_index));
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(rif->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("invalid rif id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    status = mrvl_sai_rif_is_exist(rif_idx, &is_exist);
    if ((status != SAI_STATUS_SUCCESS) || (is_exist == false)) {
        MRVL_SAI_LOG_ERR("rif id %d does not exist\n", rif_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    status = mrvl_sai_rif_get_first_nh_id(rif_idx, &rif_first_nh);

    if (status == SAI_STATUS_SUCCESS) {
        status = mrvl_sai_next_hop_get_match_id(rif_first_nh ,&ip->ipaddr, &nbr_idx);
        if (status == SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't add  next hop - already exist %d\n", nbr_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }                                          
    }
    
    for (nh_idx = 0; nh_idx < SAI_NEXTHOP_TABLE_SIZE_CNS; nh_idx++) {
        if (mrvl_sai_nh_table[nh_idx].used == false) {
            break;
        }
    }
    if (nh_idx >= SAI_NEXTHOP_TABLE_SIZE_CNS){
        MRVL_SAI_LOG_ERR("Nexthop table is full\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }
    memset(&mrvl_sai_nh_table[nh_idx], 0, sizeof(mrvl_sai_nh_table_t));
    status = mrvl_sai_rif_add_first_nh_id(rif_idx, nh_idx, &rif_first_nh);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't add first nexthop\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    } else {
        mrvl_sai_nh_table[nh_idx].next_nh_id = rif_first_nh;
    }
    status = mrvl_sai_get_match_neighbor_id(rif_idx, &ip->ipaddr, &nbr_idx);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_DBG("Can't find neighbor get default\n");
        status = mrvl_sai_rif_get_nbr_miss_act(rif_idx, &nbr_miss_act);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get nbr_miss_act\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }
        if (nbr_miss_act == SAI_PACKET_ACTION_TRAP) {
            nbr_idx = MRVL_SAI_NEIGHBOR_TRAP_ID_CNS;
        } else {
            nbr_idx = MRVL_SAI_NEIGHBOR_DROP_ID_CNS;
        }
    }
    mrvl_sai_neighbor_set_next_hop(nbr_idx, nh_idx);
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, next_hop_id))) {
    	MRVL_SAI_API_RETURN(status);
    }
    
    mrvl_sai_nh_table[nh_idx].used = true;
    mrvl_sai_nh_table[nh_idx].rif_id = rif_idx;
    memcpy(&mrvl_sai_nh_table[nh_idx].inet_address, &ip->ipaddr, sizeof(sai_ip_address_t));
    mrvl_sai_nh_table[nh_idx].nbr_id = nbr_idx;
    mrvl_sai_utl_dlist_init(&mrvl_sai_nh_table[nh_idx].route_list_elem);

    mrvl_sai_next_hop_key_to_str(*next_hop_id, key_str);
    MRVL_SAI_LOG_NTC("Created next hop %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *  When neighbor change status (add or deleted) we need to update all routes that uses this neighbor.
 *  We start by checking all next hops to find which one uses this neighbor and then we go through the
 *  routers list (the routers that use the matched nexthop) and update their neghbor id.
 *  if the new neighbor id is "MRVL_SAI_INVALID_ID_CNS"  we use the rif default action which can betrap or drop.
 * 
 * Arguments:
 *    [in] rif_idx - router interface on which the neighbor changed
 *    [in] nbr_idx - the new neighbor id
 *    [in] inet_address - the changed(removed/added) neighbor ip address
 *  
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_next_hop_update_nbr_id(uint32_t rif_idx, uint32_t nbr_idx, sai_ip_address_t *inet_address, uint32_t *nh_idx)
{
    uint32_t            rif_first_nh, match_id;
    sai_status_t        status;
    sai_packet_action_t nbr_miss_act;
    mrvl_sai_utl_dlist_elem_STC *dlist_elem;
    mrvl_sai_route_hash_data_t  *route_entry;
    
    status = mrvl_sai_rif_get_first_nh_id(rif_idx, &rif_first_nh);
    if ((status != SAI_STATUS_SUCCESS) || (rif_first_nh == MRVL_SAI_INVALID_ID_CNS)) {
        MRVL_SAI_LOG_DBG("Can't get first nexthop\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }    
    status = mrvl_sai_next_hop_get_match_id(rif_first_nh ,inet_address, &match_id);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_DBG("Can't find match nexthop\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    } 
    *nh_idx = match_id;
    if (nbr_idx == MRVL_SAI_INVALID_ID_CNS) {
        /* neighbor was deleted - need to check default action */
        status = mrvl_sai_rif_get_nbr_miss_act(rif_idx, &nbr_miss_act);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get nbr_miss_act\n");
            return SAI_STATUS_FAILURE;
        }
        if (nbr_miss_act == SAI_PACKET_ACTION_TRAP) {
            nbr_idx = MRVL_SAI_NEIGHBOR_TRAP_ID_CNS;
            MRVL_SAI_LOG_DBG("Add trap neighbor\n");
        } else {
            nbr_idx = MRVL_SAI_NEIGHBOR_DROP_ID_CNS;
            MRVL_SAI_LOG_DBG("Add drop neighbor\n");
        }
    }
    mrvl_sai_nh_table[match_id].nbr_id = nbr_idx; 
    
    if (!mrvl_sai_utl_dlist_is_empty(&mrvl_sai_nh_table[match_id].route_list_elem)) {
        dlist_elem = &mrvl_sai_nh_table[match_id].route_list_elem;
        do {
            dlist_elem = mrvl_sai_utl_dlist_get_next(dlist_elem); 
            route_entry = (mrvl_sai_route_hash_data_t*)dlist_elem;
            /* update route table about the change in neighbor */
            mrvl_sai_route_update_nbr_id( route_entry,  nbr_idx);

        } while (!mrvl_sai_utl_dlist_is_last(&mrvl_sai_nh_table[match_id].route_list_elem, dlist_elem));
    } 
    
    return status; 
}

/*
 * Routine Description:
 *  remove next hop from list and return the new head of the list
 * 
 * Arguments:
 *    [in] first_nh - the first next hop id in the list
 *    [in] deletd_nh_idx - the next hop id to delete
 *    [out] new_first_idx - the new head of the list
 *  
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t mrvl_sai_next_hop_remove_entry_prv(uint32_t first_nh, uint32_t deletd_nh_idx, uint32_t *new_first_idx)
{
    uint32_t last_nh, nh;

    if (deletd_nh_idx == first_nh) {
        *new_first_idx = mrvl_sai_nh_table[deletd_nh_idx].next_nh_id;
        memset(&mrvl_sai_nh_table[deletd_nh_idx], 0, sizeof(mrvl_sai_nh_table_t));       
        return SAI_STATUS_SUCCESS;
    } else {
        *new_first_idx = first_nh;
    }
    
    nh = first_nh;
    while ((deletd_nh_idx != nh) && (nh != MRVL_SAI_INVALID_ID_CNS)) {
        last_nh = nh;
        nh = mrvl_sai_nh_table[nh].next_nh_id;
    }
    if (deletd_nh_idx == nh) {
        mrvl_sai_nh_table[last_nh].next_nh_id = mrvl_sai_nh_table[deletd_nh_idx].next_nh_id;
        memset(&mrvl_sai_nh_table[deletd_nh_idx], 0, sizeof(mrvl_sai_nh_table_t));       
        return SAI_STATUS_SUCCESS;
    } else {
        return SAI_STATUS_FAILURE;
    }
}

/*
 * Routine Description:
 *  Add route to the list in the next hop table.
 *  we add only routes that points directly to this next hop
 * 
 * Arguments:
 *    [in] nh_idx - next hop on which the route was created
 *    [in] route_list - route element to add to the list
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_next_hop_add_route(uint32_t nh_idx, mrvl_sai_utl_dlist_elem_STC *route_list)
{
    if (mrvl_sai_nh_table[nh_idx].used == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    mrvl_sai_utl_dlist_insert_to_tail (&mrvl_sai_nh_table[nh_idx].route_list_elem, route_list);
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Delete route from the list in the next hop table.
 * 
 * Arguments:
 *    [in] nh_idx - next hop on which the route was deleted
 *    [in] route_list - route element to delete from the list
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_next_hop_del_route(uint32_t nh_idx, mrvl_sai_utl_dlist_elem_STC *route_list)
{
    if (mrvl_sai_nh_table[nh_idx].used == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    mrvl_sai_utl_dlist_remove(route_list);
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  get the route list header.
 * 
 * Arguments:
 *    [in] nh_idx - next hop on which the route was deleted
 *    [in] route_list - route list header
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_next_hop_get_route_list(uint32_t nh_idx, mrvl_sai_utl_dlist_elem_STC **route_list)
{
    if (mrvl_sai_nh_table[nh_idx].used == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *route_list = &mrvl_sai_nh_table[nh_idx].route_list_elem;
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  get the neughbor id that is used by the next hop.
 * 
 * Arguments:
 *    [in] nh_idx - the requested next hop 
 *    [out] nbr_idx - the neighbor id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_next_hop_get_nbr_id(uint32_t nh_idx, uint32_t *nbr_idx)
{
    if (mrvl_sai_nh_table[nh_idx].used == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *nbr_idx = mrvl_sai_nh_table[nh_idx].nbr_id;
    return SAI_STATUS_SUCCESS;
}


sai_status_t mrvl_sai_next_hop_get_rif_id(uint32_t nh_idx, uint32_t *rif_idx)
{
    if (mrvl_sai_nh_table[nh_idx].used == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    *rif_idx = mrvl_sai_nh_table[nh_idx].rif_id;
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Remove next hop
 *
 * Arguments:
 *    [in] next_hop_id - next hop id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_next_hop(_In_ sai_object_id_t next_hop_id)
{
    char key_str[MAX_KEY_STR_LEN];
    uint32_t   nh_idx, rif_first_nh, rif_idx, new_first_idx;
    sai_status_t    status;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx)) {
        MRVL_SAI_LOG_ERR("invalid nh_idx\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if ((mrvl_sai_nh_table[nh_idx].used == 0) || 
        (mrvl_sai_utl_dlist_is_empty(&mrvl_sai_nh_table[nh_idx].route_list_elem)== 0)){
    	MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    /* remove nh from rif list */
    rif_idx = mrvl_sai_nh_table[nh_idx].rif_id;
    status = mrvl_sai_rif_get_first_nh_id(rif_idx, &rif_first_nh);
    if ((status != SAI_STATUS_SUCCESS) || (rif_first_nh == MRVL_SAI_INVALID_ID_CNS)) {
        MRVL_SAI_LOG_ERR("Can't get first nexthop\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    } 
     
    mrvl_sai_neighbor_set_next_hop(mrvl_sai_nh_table[nh_idx].nbr_id,0xFFFFFFFF);

    status = mrvl_sai_next_hop_remove_entry_prv(rif_first_nh, nh_idx, &new_first_idx);
    if (status == SAI_STATUS_SUCCESS) {
        if (new_first_idx != rif_first_nh) {
            status = mrvl_sai_rif_add_first_nh_id(rif_idx, new_first_idx, NULL);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("Can't add first nexthop\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }    
        }
    }
    mrvl_sai_next_hop_key_to_str(next_hop_id, key_str); 
    MRVL_SAI_LOG_NTC("Remove next hop %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *    Set Next Hop attribute
 *
 * Arguments:
 *    [in] sai_object_id_t - next_hop_id
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_set_next_hop_attribute(_In_ sai_object_id_t next_hop_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .object_id = next_hop_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_next_hop_key_to_str(next_hop_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_next_hop_attribs, mrvl_sai_next_hop_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}


/*
 * Routine Description:
 *    Get Next Hop attribute
 *
 * Arguments:
 *    [in] sai_object_id_t - next_hop_id
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_next_hop_attribute(_In_ sai_object_id_t   next_hop_id,
                                         _In_ uint32_t            attr_count,
                                         _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = next_hop_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_next_hop_key_to_str(next_hop_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_next_hop_attribs, mrvl_sai_next_hop_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/* Next hop entry type [sai_next_hop_type_t] */
static sai_status_t mrvl_sai_next_hop_type_get_prv(_In_ const sai_object_key_t   *key,
                                    _Inout_ sai_attribute_value_t *value,
                                    _In_ uint32_t                  attr_index,
                                    _Inout_ vendor_cache_t        *cache,
                                    void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_NEXT_HOP_IP;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Next hop entry ip address [sai_ip_address_t] */
static sai_status_t mrvl_sai_next_hop_ip_get_prv(_In_ const sai_object_key_t   *key,
                                  _Inout_ sai_attribute_value_t *value,
                                  _In_ uint32_t                  attr_index,
                                  _Inout_ vendor_cache_t        *cache,
                                  void                          *arg)
{
    uint32_t nh_idx;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->object_id, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", key);
        return SAI_STATUS_FAILURE;
    }
    if (mrvl_sai_nh_table[nh_idx].used == 0){
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
   
    memcpy(&value->ipaddr, &mrvl_sai_nh_table[nh_idx].inet_address, sizeof(sai_ip_address_t));

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Next hop entry router interface id [sai_router_interface_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_next_hop_rif_get_prv(_In_ const sai_object_key_t   *key,
                                   _Inout_ sai_attribute_value_t *value,
                                   _In_ uint32_t                  attr_index,
                                   _Inout_ vendor_cache_t        *cache,
                                   void                          *arg)
{
    uint32_t        nh_idx;
    sai_status_t    status;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->object_id, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", key);
        return SAI_STATUS_FAILURE;
    }
    if (mrvl_sai_nh_table[nh_idx].used == 0){
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, mrvl_sai_nh_table[nh_idx].rif_id, &value->oid))) {
        return status;
    }
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

const sai_next_hop_api_t nexthop_api = {
    mrvl_sai_create_next_hop,
    mrvl_sai_remove_next_hop,
    mrvl_sai_set_next_hop_attribute,
    mrvl_sai_get_next_hop_attribute
};
