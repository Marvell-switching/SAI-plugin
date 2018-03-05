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
#define __MODULE__ SAI_ROUTER_INTF

#define MRVL_SAI_FLOW_VLAN_MAX_NUMBER        4095
#define MRVL_SAI_VLAN_GET_RESERVED_VLAN_ID_MAC(port) \
            MRVL_SAI_FLOW_VLAN_MAX_NUMBER + 1 + port

/* Ip protocol types */
enum fpaIpProtocolType {
    FPA_IPv4_PROTOCOL   = 1 << 0,   /* Ip protocol type Ipv4 */
    FPA_IPv6_PROTOCOL   = 1 << 1    /* Ip protocol type Ipv6 */
};

static mrvl_sai_rif_table_t      mrvl_sai_rif_table[SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS] = {};
static mrvl_sai_rif_mtu_table_t  mrvl_sai_rif_mtu_table[SAI_RIF_MTU_PROFILES_CNS] = {};


static const sai_attribute_entry_t mrvl_sai_rif_attribs[] = {
    { SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID, true, true, false, true,
      "Router interface virtual router ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_ROUTER_INTERFACE_ATTR_TYPE, true, true, false, true,
      "Router interface type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_ROUTER_INTERFACE_ATTR_PORT_ID, false, true, false, true,
      "Router interface port ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, false, true, false, true,
      "Router interface vlan ID", SAI_ATTR_VAL_TYPE_U16 },
    { SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS, false, true, true, true,
      "Router interface source MAC address", SAI_ATTR_VAL_TYPE_MAC },
    { SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE, false, true, true, true,
      "Router interface admin v4 state", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE, false, true, true, true,
      "Router interface admin v6 state", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_ROUTER_INTERFACE_ATTR_MTU, false, true, true, true,
      "Router interface mtu", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION, false, true, true, true,
      "Router interface neighbor miss action", SAI_ATTR_VAL_TYPE_S32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, true,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_rif_attrib_get_prv(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg);
static sai_status_t mrvl_sai_rif_admin_get_prv(_In_ const sai_object_key_t   *key,
                                _Inout_ sai_attribute_value_t *value,
                                _In_ uint32_t                  attr_index,
                                _Inout_ vendor_cache_t        *cache,
                                void                          *arg);
static sai_status_t mrvl_sai_rif_attrib_set_prv(_In_ const sai_object_key_t      *key,
                                 _In_ const sai_attribute_value_t *value,
                                 void                             *arg);
static sai_status_t mrvl_sai_rif_admin_set_prv(_In_ const sai_object_key_t      *key,
                                _In_ const sai_attribute_value_t *value,
                                void                             *arg);

static const sai_vendor_attribute_entry_t mrvl_sai_rif_vendor_attribs[] = {
    { SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,
      NULL, NULL },
    { SAI_ROUTER_INTERFACE_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_TYPE,
      NULL, NULL },
    { SAI_ROUTER_INTERFACE_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_PORT_ID,
      NULL, NULL },
    { SAI_ROUTER_INTERFACE_ATTR_VLAN_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_VLAN_ID,
      NULL, NULL },
    { SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS,
      mrvl_sai_rif_attrib_set_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS },
    { SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_rif_admin_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE,
      mrvl_sai_rif_admin_set_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE },
    { SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_rif_admin_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE,
      mrvl_sai_rif_admin_set_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE },
    { SAI_ROUTER_INTERFACE_ATTR_MTU,
      { true, false, false/*true*/, true },
      { true, false, true, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_MTU,
      NULL/*mrvl_sai_rif_attrib_set_prv*/, (void*)SAI_ROUTER_INTERFACE_ATTR_MTU },
    { SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION,
      { true, false, false/*true*/, true },
      { true, false, true, true },
      mrvl_sai_rif_attrib_get_prv, (void*)SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION,
      NULL/*mrvl_sai_rif_attrib_set_prv*/, (void*)SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION }

};

static void mrvl_sai_rif_key_to_str(_In_ sai_object_id_t rif_id, _Out_ char *key_str)
{
    uint32_t rif;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "rif id invalid");
        return ;
    }
    snprintf(key_str, MAX_KEY_STR_LEN, "rif id %u ", rif);
}

sai_status_t mrvl_sai_rif_dump(void)
{
    uint32_t    idx, used;
    mrvl_sai_utl_dlist_elem_STC *dlist_elem;
    mrvl_sai_route_hash_data_t  *route_entry;
    
    printf("Mtu table:\n");
    printf("| idx | mtu size | ref cntr |\n");
    printf("-------------------------------\n");
    for (idx = 0; idx < SAI_RIF_MTU_PROFILES_CNS; idx++) {
        printf("| %3d | %8d | %8d |\n",idx, mrvl_sai_rif_mtu_table[idx].mtu_size, mrvl_sai_rif_mtu_table[idx].ref_cntr);
        
    }
    used = 0;
    printf("\nRouter interface table:\n");
    printf("| idx | mtu idx | type | value | vrf| nbr_v |  nbr idx | nbr act | nh_v |   nh idx |   mac address   | route list\n");
    printf("---------------------------------------------------------------------------------------------------------------------------\n");
    for (idx = 0; idx < SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS; idx++) {
        if (mrvl_sai_rif_table[idx].valid == true) {
            used++;
            printf("| %3d | %7d |   %s  | %5d |  %d |   %s   | %8x | %s |   %s  | %8x |%02x:%02x:%02x:%02x:%02x:%02x| ", idx, mrvl_sai_rif_table[idx].mtu_idx,
                   (mrvl_sai_rif_table[idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT)?"P":
                		   (mrvl_sai_rif_table[idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_VLAN) ? "V" : "L",
                   mrvl_sai_rif_table[idx].port_vlan_id,
                   mrvl_sai_rif_table[idx].vrf_id,
                   (mrvl_sai_rif_table[idx].nbr_valid == true)?"+":" ",
                   (mrvl_sai_rif_table[idx].nbr_valid == true)?mrvl_sai_rif_table[idx].first_nbr_idx:0xFFFFFFFF,
                   (mrvl_sai_rif_table[idx].nbr_miss_act == SAI_PACKET_ACTION_DROP)?"  DROP ":
                        ((mrvl_sai_rif_table[idx].nbr_miss_act == SAI_PACKET_ACTION_TRAP)?"  TRAP ":" OTHER "),
                   (mrvl_sai_rif_table[idx].nh_valid == true)?"+":" ",
                   (mrvl_sai_rif_table[idx].nh_valid == true)?mrvl_sai_rif_table[idx].first_nh_idx:0xFFFFFFFF,
                   mrvl_sai_rif_table[idx].src_mac[0],mrvl_sai_rif_table[idx].src_mac[1],mrvl_sai_rif_table[idx].src_mac[2],
                   mrvl_sai_rif_table[idx].src_mac[3],mrvl_sai_rif_table[idx].src_mac[4],mrvl_sai_rif_table[idx].src_mac[5]); 

            if (!mrvl_sai_utl_dlist_is_empty(&mrvl_sai_rif_table[idx].route_list_elem)) {
                dlist_elem = &mrvl_sai_rif_table[idx].route_list_elem;
                do {
                    dlist_elem = mrvl_sai_utl_dlist_get_next(dlist_elem); 
                    route_entry = (mrvl_sai_route_hash_data_t*)dlist_elem;
                    printf("0x%x, ",route_entry->cookie);

                } while (!mrvl_sai_utl_dlist_is_last(&mrvl_sai_rif_table[idx].route_list_elem, dlist_elem));
            } 
            printf("\n");
        }
    }
    printf("\nTotal used: %d\n\n",used);
    return SAI_STATUS_SUCCESS;
}

/**


 * @brief Remove router interface
 *
 * @param[in] rif_id Router interface id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_remove_router_interface(_In_ sai_object_id_t rif_id)
{
    char                        key_str[MAX_KEY_STR_LEN];
    uint32_t                    rif_idx, mtu_idx, i;
    uint64_t                    cookie;
    FPA_STATUS                  fpa_status;
    bool                        delete_control_pkt, delete_vr_id;    
    uint16_t                    vlan;
    sai_status_t 				status;
    
    MRVL_SAI_LOG_ENTER();
    mrvl_sai_rif_key_to_str(rif_id, key_str);
    MRVL_SAI_LOG_NTC("Remove rif %s\n", key_str);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", rif_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    if (mrvl_sai_rif_table[rif_idx].valid == false) {
        MRVL_SAI_LOG_ERR("router interface %d do not exist\n", rif_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }
    
    if ((mrvl_sai_rif_table[rif_idx].nbr_valid == true) ||
        (mrvl_sai_rif_table[rif_idx].nh_valid == true) ||
        (mrvl_sai_utl_dlist_is_empty(&mrvl_sai_rif_table[rif_idx].route_list_elem)== 0)){
        /*this rif include neighbor and can't be deleted*/
        MRVL_SAI_LOG_ERR("router interface %d include neighbor %d, nh or route and can't be deleted \n", rif_idx, mrvl_sai_rif_table[rif_idx].first_nbr_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    if (mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT ||
    	mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_VLAN	)
    {
        cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
                                                        mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                        FPA_IPv4_PROTOCOL,
                                                        mrvl_sai_rif_table[rif_idx].src_mac);
        fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, cookie);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to delete Ipv4 entry %llx from TERMINATION table status = %d\n", cookie, fpa_status);
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }
        cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
                                                        mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                        FPA_IPv6_PROTOCOL,
                                                        mrvl_sai_rif_table[rif_idx].src_mac);
        fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, cookie);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to delete Ipv6 entry %llx from TERMINATION table status = %d\n", cookie, fpa_status);
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }

        delete_vr_id = true;
        delete_control_pkt = true;
        /* check if another entry is using the same parameters*/
        for (i=0; i < SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS; i++) {
            if ((i == rif_idx) || (mrvl_sai_rif_table[i].valid == false)) {
                continue;
            }
            if ((mrvl_sai_rif_table[i].intf_type == mrvl_sai_rif_table[rif_idx].intf_type) &&
                (mrvl_sai_rif_table[i].port_vlan_id == mrvl_sai_rif_table[rif_idx].port_vlan_id)){
                delete_control_pkt = false;
                if (mrvl_sai_rif_table[i].vrf_id == mrvl_sai_rif_table[rif_idx].vrf_id) {
                    delete_vr_id = false;
                }
            }
            if ((delete_vr_id == false) && (delete_control_pkt == false)){
                break;
            }
        }
        if (delete_vr_id == true) {
            cookie = MRVL_SAI_ROUTER_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
                                            mrvl_sai_rif_table[rif_idx].port_vlan_id);
            fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VR_ID_E, cookie);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_ERR("Failed to delete entry %llx from VR_ID table status = %d\n", cookie, fpa_status);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }
        }


        if (delete_control_pkt == true) {
            cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(FPA_CONTROL_PKTS_TYPE_ARP_REQUEST_MESSAGES_E,
                                                       mrvl_sai_rif_table[rif_idx].intf_type,
                                                       mrvl_sai_rif_table[rif_idx].port_vlan_id, 0);
            fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, cookie);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_ERR("Failed to delete entry %llx from CONTROL_PKT table status = %d\n", cookie, fpa_status);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            if (SAI_ROUTER_INTERFACE_TYPE_VLAN == mrvl_sai_rif_table[rif_idx].intf_type) {
                vlan = mrvl_sai_rif_table[rif_idx].port_vlan_id;
            }
            else /* type port */
            {
                /* for port use reserved vlan */
                vlan = MRVL_SAI_VLAN_GET_RESERVED_VLAN_ID_MAC(mrvl_sai_rif_table[rif_idx].port_vlan_id);
            }

            cookie = MRVL_SAI_FDB_CREATE_COOKIE_MAC(vlan, mrvl_sai_rif_table[rif_idx].src_mac);
            fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, cookie);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_ERR("Failed to delete entry %llx from FDB table status = %d\n", cookie, fpa_status);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            if (mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
                mrvl_sai_utl_delete_l2_int_group(mrvl_sai_rif_table[rif_idx].port_vlan_id, 0);
            }

        } else {
            MRVL_SAI_LOG_DBG("We don't delete entry %llx from CONTROL_PKT table, used by rif %d\n", cookie, i);
        }
    }

    mtu_idx = mrvl_sai_rif_table[rif_idx].mtu_idx;
    if (mrvl_sai_rif_mtu_table[mtu_idx].ref_cntr == 0) {
        MRVL_SAI_LOG_ERR("mtu table index %d invalid ref_cntr\n", mtu_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    mrvl_sai_rif_mtu_table[mtu_idx].ref_cntr--;
    if (mrvl_sai_rif_mtu_table[mtu_idx].ref_cntr == 0) {
        mrvl_sai_rif_mtu_table[mtu_idx].mtu_size = 0;
    }
    mrvl_sai_virtual_router_update_referance_cntr(mrvl_sai_rif_table[rif_idx].vrf_id , false);

    memset(&mrvl_sai_rif_table[rif_idx], 0, sizeof(mrvl_sai_rif_table_t));
    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *    Remove all router interface - this function is not called from sai application
 *
 * Arguments:
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_rif_remove_all()
{
    uint32_t                    rif_idx;
    sai_object_id_t             rif_id;
    
    for (rif_idx = 0; rif_idx < SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS; rif_idx++) {
        if (mrvl_sai_rif_table[rif_idx].valid == false) 
            continue;
        
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
            return SAI_STATUS_FAILURE;
        }
        mrvl_sai_remove_router_interface(rif_id);
    }
    return SAI_STATUS_SUCCESS;
    
}

/**

 * @brief Create router interface.
 *
 * @param[out] rif_id Router interface id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error


 */

sai_status_t mrvl_sai_create_router_interface(_Out_ sai_object_id_t      *rif_id,
                                              _In_ sai_object_id_t        switch_id,
                                              _In_ uint32_t               attr_count,
                                              _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t  *type, *vrid, *port, *vlan, *mtu, *mac, *adminv4, *adminv6, *miss_act;
    uint32_t                     type_index, vrid_index, port_index, vlan_index, mtu_index, mac_index, adminv4_index, adminv6_index;
    uint32_t                     rif_idx, mtu_idx, i, vr_id, portVlan, mtu_val, miss_act_idx, miss_action;
    uint16_t                     fdb_vlan;
    FPA_STATUS                   fpa_status;
    uint64_t                     cookie;
    sai_mac_t                    mac_addr;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    bool                         ipv4_enable, ipv6_enable, found_empty, vr_is_valid;
    FPA_FLOW_TABLE_ENTRY_STC     flowEntry;
    uint32_t                     group;
    bool                         add_control_pkt, add_vr_id;
    uint32_t					 config_fpa=0;

    int mc_rc;
    unsigned char * mac_ptr;
    MRVL_SAI_LOG_ENTER();
    if (NULL == rif_id) {
        MRVL_SAI_LOG_ERR("NULL rif id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_rif_attribs, mrvl_sai_rif_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_rif_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create rif, %s\n", list_str);

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID, &vrid,
                               &vrid_index));
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vrid->oid, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &vr_id))) {
    	MRVL_SAI_API_RETURN(status);
    }
    status = mrvl_sai_virtual_router_is_valid(vr_id, &vr_is_valid);
    if ((status != SAI_STATUS_SUCCESS) || (vr_is_valid == false)) {
        MRVL_SAI_LOG_ERR("VIRTUAL_ROUTER %d is invalid\n", vr_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_TYPE, &type, &type_index));

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_MTU, &mtu, &mtu_index))) {
        mtu_val = mtu->u32;
    } else {
        mtu_val = SAI_DEFAULT_RIF_MTU_CNS;
    }
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION, &miss_act, &miss_act_idx))) {
        miss_action = miss_act->s32;
    } else {
        miss_action = SAI_DEFAULT_RIF_NBR_MISS_ACTION_CNS;
    }
    
    found_empty = false;
    mtu_idx = 0xFFFFFFFF;
    for (i=0; i < SAI_RIF_MTU_PROFILES_CNS; i++) {
        if (mrvl_sai_rif_mtu_table[i].mtu_size == mtu_val) {
            mtu_idx = i;
            break;/* mtu size was found */
        } else if ((found_empty == false) && (mrvl_sai_rif_mtu_table[i].mtu_size == 0)) {
            mtu_idx = i;
            found_empty = true;
        }
    }
    if (mtu_idx == 0xFFFFFFFF) { /*mtu size table is full */
        MRVL_SAI_LOG_ERR("rif mtu size table is full\n");
        return SAI_STATUS_TABLE_FULL;
    }
    rif_idx = 0xFFFFFFFF;
    for (i=0; i < SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS; i++) {
        if (mrvl_sai_rif_table[i].valid == false){
            rif_idx = i;
            break;
        }
    }
    if (rif_idx == 0xFFFFFFFF) { /*rif size table is full */
        MRVL_SAI_LOG_ERR("rif table is full\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }
    
    if (SAI_ROUTER_INTERFACE_TYPE_VLAN == type->s32) {
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, &vlan, &vlan_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute vlan id on create\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }
        if (SAI_STATUS_ITEM_NOT_FOUND !=
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_PORT_ID, &port, &port_index))) {
            MRVL_SAI_LOG_ERR("Invalid attribute port id for rif vlan on create\n");
            status = SAI_STATUS_INVALID_ATTRIBUTE_0 + port_index;
            MRVL_SAI_API_RETURN(status);
        }
        portVlan = vlan->u16;
        config_fpa = 1;

    } else if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32) {
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_PORT_ID, &port, &port_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute port id on create\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }
        if (SAI_STATUS_ITEM_NOT_FOUND !=
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, &vlan, &vlan_index))) {
            MRVL_SAI_LOG_ERR("Invalid attribute vlan id for rif port on create\n");
            status = SAI_STATUS_INVALID_ATTRIBUTE_0 + vlan_index;
            MRVL_SAI_API_RETURN(status);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port->oid, SAI_OBJECT_TYPE_PORT, &portVlan))) {
        	MRVL_SAI_API_RETURN(status);
        }
        config_fpa = 1;
    } else if (SAI_ROUTER_INTERFACE_TYPE_LOOPBACK == type->s32) {
        if (SAI_STATUS_ITEM_NOT_FOUND !=
            (status =
            		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_PORT_ID, &port, &port_index))) {
        	MRVL_SAI_LOG_ERR("Invalid attribute port id for loopback rif on create\n");
        	MRVL_SAI_LOG_EXIT();
            status = SAI_STATUS_INVALID_ATTRIBUTE_0 + port_index;
            MRVL_SAI_API_RETURN(status);
        }
        if (SAI_STATUS_ITEM_NOT_FOUND !=
            (status =
            		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_VLAN_ID, &vlan, &vlan_index))) {
        	MRVL_SAI_LOG_ERR("Invalid attribute vlan id for loopback rif on create\n");
        	MRVL_SAI_LOG_EXIT();
            status = SAI_STATUS_INVALID_ATTRIBUTE_0 + vlan_index;
            MRVL_SAI_API_RETURN(status);
        }
        portVlan =0;
    } else {
       MRVL_SAI_LOG_ERR("Invalid router interface type %d\n", type->s32);
        status = SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
        MRVL_SAI_API_RETURN(status);
    }


    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS, &mac, &mac_index))) {
    	if (SAI_ROUTER_INTERFACE_TYPE_LOOPBACK == type->s32) {
    		 status =  SAI_STATUS_INVALID_ATTRIBUTE_0 + mac_index;
    	     MRVL_SAI_LOG_ERR("Invalid attribute src mac for loopback rif on create\n");
    	     MRVL_SAI_LOG_EXIT();
    	     MRVL_SAI_API_RETURN(status);
    	}
        memcpy(mac_addr , mac->mac, 6);
    } else {
        status = mrvl_sai_virtual_router_get_mac(vr_id, mac_addr);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Failed to get default mac address\n");
            MRVL_SAI_API_RETURN(status);
        }
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE, &adminv4,
                                 &adminv4_index))) {
        ipv4_enable = adminv4->booldata;
    } else {
        ipv4_enable = true;
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE, &adminv6,
                                 &adminv6_index))) {
        ipv6_enable = adminv6->booldata;
    } else {
        ipv6_enable = true;
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, rif_id))) {
    	MRVL_SAI_API_RETURN(status);
    }


    if (config_fpa )
    {
    	if (ipv4_enable == true) {
			/* 1 - Add flow to TERMINATION table with mac and port/vlan*/
			/* 2 - Add flow to CONTROL_PKT table with  port/vlan*/
			fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to init TERMINATION entry status = %d\n", fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}

			cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(type->s32, portVlan, FPA_IPv4_PROTOCOL, mac_addr);
			flowEntry.cookie = cookie;
			memset(&flowEntry.data.termination.match.destMacMask.addr, 0xFF, 6);
			memcpy(&flowEntry.data.termination.match.destMac, mac_addr, 6);
			flowEntry.data.termination.match.etherType = 0x800;
			flowEntry.data.termination.match.etherTypeMask = 0xFFFF;
			flowEntry.data.termination.metadataMask = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;
			flowEntry.data.termination.metadataValue = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;

			if (SAI_ROUTER_INTERFACE_TYPE_VLAN == type->s32)
			{
				flowEntry.data.termination.match.vlanId = portVlan;
				flowEntry.data.termination.match.vlanIdMask = 0xFFFF;
			}
			else if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32)
			{
				flowEntry.data.termination.match.inPort = portVlan;
				flowEntry.data.termination.match.inPortMask = 0xFFFFFFFF;
			}
			fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to add entry Ipv4 %llx to TERMINATION table status = %d\n", cookie, fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}
    	}

        if (ipv6_enable == true) {
			/* 1 - Add flow to TERMINATION table with mac and port/vlan*/
			/* 2 - Add flow to CONTROL_PKT table with  port/vlan*/
			fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to init TERMINATION entry status = %d\n", fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}

			cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(type->s32, portVlan, FPA_IPv6_PROTOCOL, mac_addr);
			flowEntry.cookie = cookie;
			memset(&flowEntry.data.termination.match.destMacMask.addr, 0xFF, 6);
			memcpy(&flowEntry.data.termination.match.destMac, mac_addr, 6);
			flowEntry.data.termination.match.etherType = 0x86DD;
			flowEntry.data.termination.match.etherTypeMask = 0xFFFF;
			flowEntry.data.termination.metadataMask = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;
			flowEntry.data.termination.metadataValue = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;

			if (SAI_ROUTER_INTERFACE_TYPE_VLAN == type->s32)
			{
				flowEntry.data.termination.match.vlanId = portVlan;
				flowEntry.data.termination.match.vlanIdMask = 0xFFFF;
			}
			else if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32)
			{
				flowEntry.data.termination.match.inPort = portVlan;
				flowEntry.data.termination.match.inPortMask = 0xFFFFFFFF;
			}
			fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to add entry Ipv6 %llx to TERMINATION table status = %d\n", cookie, fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}
    	}
        /******** add virtual router *********/
		add_vr_id = true;
		add_control_pkt = true;
		/* check if another entry is using the same parameters*/
		for (i=0; i < SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS; i++) {
			if ((i == rif_idx) || (mrvl_sai_rif_table[i].valid == false)) {
				continue;
			}
			if ((mrvl_sai_rif_table[i].intf_type == mrvl_sai_rif_table[rif_idx].intf_type) &&
				(mrvl_sai_rif_table[i].port_vlan_id == portVlan)){
				add_control_pkt = false;
				if (mrvl_sai_rif_table[i].vrf_id == vr_id) {
					add_vr_id = false;
				}
			}
			if ((add_vr_id == false) && (add_control_pkt == false)){
				break;
			}
		}

		if (add_vr_id == true) {
			fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VR_ID_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to init CONTROL_PKT entry status = %d\n", fpa_status);
				mrvl_sai_remove_router_interface(*rif_id);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}
			cookie = MRVL_SAI_ROUTER_COOKIE_MAC(type->s32, portVlan);
			flowEntry.cookie = cookie;
			flowEntry.data.vr_id.assignVrId = vr_id;
			if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32){
				flowEntry.data.vr_id.inPort = portVlan;
				flowEntry.data.vr_id.inPortMask = 0xFFFFFFFF;
				/* update netdev mac address */
				mac_ptr =  (unsigned char *)mac->mac;
				 mc_rc = mrvl_sai_netdev_set_mac(host_fd[portVlan].name, mac_ptr);
				 MRVL_SAI_LOG_NTC("set netdev %s mac address, ret %d\n", host_fd[portVlan].name, mc_rc);

			} else {
				flowEntry.data.vr_id.vlanId = portVlan;
				flowEntry.data.vr_id.vlanIdMask = 0xFFFF;
			}
			fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VR_ID_E, &flowEntry);
			if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){
				mrvl_sai_remove_router_interface(*rif_id);
				MRVL_SAI_LOG_ERR("Failed to add entry %llx to VR_ID table status = %d\n", cookie, fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}
		}

		if (add_control_pkt == true) {
			/**** add control pkt entry ****/
			fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to init CONTROL_PKT entry status = %d\n", fpa_status);
				mrvl_sai_remove_router_interface(*rif_id);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}

			flowEntry.data.control_pkt.entry_type = FPA_CONTROL_PKTS_TYPE_ARP_REQUEST_MESSAGES_E;
			cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(FPA_CONTROL_PKTS_TYPE_ARP_REQUEST_MESSAGES_E, type->s32, portVlan, 0);
			flowEntry.cookie = cookie;
			memset(&flowEntry.data.control_pkt.match.dstMac, 0xFF, 6);
			memset(&flowEntry.data.control_pkt.match.dstMacMask, 0xFF, 6);
			if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32){
				flowEntry.data.control_pkt.match.inPort = portVlan;
				flowEntry.data.control_pkt.match.inPortMask = 0xFFFFFFFF;
				flowEntry.priority = 0xFFFFFFFF;

			} else {
				flowEntry.data.control_pkt.match.vlanId = portVlan;
				flowEntry.data.control_pkt.match.vlanIdMask = 0xFFFF;
			}
			flowEntry.data.control_pkt.match.etherType = 0x806;
			flowEntry.data.control_pkt.match.etherTypeMask = 0xFFFF;
			flowEntry.data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;
			fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
			if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){
				mrvl_sai_remove_router_interface(*rif_id);
				MRVL_SAI_LOG_ERR("Failed to add entry %llx to CONTROL_PKT table, for %s %d status = %d\n", cookie, 
                                 SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32 ? "port" : "vlan", portVlan, fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}

			/**** add FDB entry to TRAP ARP RESPONSE control packets ****/
			fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &flowEntry);
			if (fpa_status != FPA_OK) {
				MRVL_SAI_LOG_ERR("Failed to init FDB entry status = %d\n", fpa_status);
				mrvl_sai_remove_router_interface(*rif_id);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}

			if (SAI_ROUTER_INTERFACE_TYPE_VLAN == type->s32) {
				fdb_vlan = portVlan;
			}
			else /* type port */
			{
				/* for port use reserved vlan */
				fdb_vlan = MRVL_SAI_VLAN_GET_RESERVED_VLAN_ID_MAC(portVlan);
			}

			cookie = MRVL_SAI_FDB_CREATE_COOKIE_MAC(fdb_vlan, mac_addr);
			flowEntry.cookie = cookie;
			flowEntry.data.l2_bridging.match.vlanId     = fdb_vlan;
			flowEntry.data.l2_bridging.match.vlanIdMask = 0xFFFF;
			memcpy(flowEntry.data.l2_bridging.match.destMac.addr, mac_addr, FPA_MAC_ADDRESS_SIZE);
			memset(flowEntry.data.l2_bridging.match.destMacMask.addr, 0xFF, FPA_MAC_ADDRESS_SIZE);
			flowEntry.data.l2_bridging.outputPort = SAI_OUTPUT_CONTROLLER; /* TRAP */
			flowEntry.data.l2_bridging.groupId = -1;

			fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &flowEntry);
			if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){
				MRVL_SAI_LOG_ERR("Failed to add entry %llx to FDB table status = %d\n", cookie, fpa_status);
				status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
				MRVL_SAI_API_RETURN(status);
			}

            /**** add ipv6 neighbor solicitation control pkt entry ****/
            fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_ERR("Failed to init CONTROL_PKT entry status = %d\n", fpa_status);
                mrvl_sai_remove_router_interface(*rif_id);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            flowEntry.data.control_pkt.entry_type = FPA_CONTROL_PKTS_TYPE_IPV6_NEIGHBOR_SOLICITATION_E;
            cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(FPA_CONTROL_PKTS_TYPE_IPV6_NEIGHBOR_SOLICITATION_E, type->s32, portVlan, 0);
            flowEntry.cookie = cookie;
            memset(&flowEntry.data.control_pkt.match.dstIpv6, 0, 16);
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[0]  = 0xFF;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[1]  = 0x02;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[11] = 0x01;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[12] = 0xFF;
            memset(&flowEntry.data.control_pkt.match.dstIpv6Mask, 0, 16);
            memset(&flowEntry.data.control_pkt.match.dstIpv6Mask, 0xFF, 13);
            if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32){
                flowEntry.data.control_pkt.match.inPort = portVlan;
                flowEntry.data.control_pkt.match.inPortMask = 0xFFFFFFFF;
                flowEntry.priority = 0xFFFFFFFF;

            } else {
                flowEntry.data.control_pkt.match.vlanId = portVlan;
                flowEntry.data.control_pkt.match.vlanIdMask = 0xFFFF;
            }
            flowEntry.data.control_pkt.match.etherType = 0x86dd;
            flowEntry.data.control_pkt.match.etherTypeMask = 0xFFFF;
            flowEntry.data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;
            fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
            if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){
                mrvl_sai_remove_router_interface(*rif_id);
                MRVL_SAI_LOG_ERR("Failed to add entry %llx to CONTROL_PKT table, for %s %d status = %d\n", cookie, 
                                 SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32 ? "port" : "vlan", portVlan, fpa_status);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            /**** add ipv6 all_nodes control pkt entry ****/
            fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_ERR("Failed to init CONTROL_PKT entry status = %d\n", fpa_status);
                mrvl_sai_remove_router_interface(*rif_id);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            flowEntry.data.control_pkt.entry_type = FPA_CONTROL_PKTS_TYPE_IPV6_LINK_LOCAL_E;
            cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(FPA_CONTROL_PKTS_TYPE_IPV6_LINK_LOCAL_E, type->s32, portVlan, 0);
            flowEntry.cookie = cookie;
            memset(&flowEntry.data.control_pkt.match.dstIpv6, 0, 16);
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[0]  = 0xFF;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[1]  = 0x02;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[15] = 0x01;
            memset(&flowEntry.data.control_pkt.match.dstIpv6Mask, 0xFF, 16);
            if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32){
                flowEntry.data.control_pkt.match.inPort = portVlan;
                flowEntry.data.control_pkt.match.inPortMask = 0xFFFFFFFF;
                flowEntry.priority = 0xFFFFFFFF;

            } else {
                flowEntry.data.control_pkt.match.vlanId = portVlan;
                flowEntry.data.control_pkt.match.vlanIdMask = 0xFFFF;
            }
            flowEntry.data.control_pkt.match.etherType = 0x86dd;
            flowEntry.data.control_pkt.match.etherTypeMask = 0xFFFF;
            flowEntry.data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;
            fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
            if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){
                mrvl_sai_remove_router_interface(*rif_id);
                MRVL_SAI_LOG_ERR("Failed to add entry %llx to CONTROL_PKT table status = %d\n", cookie, fpa_status);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            /**** add ipv6 all_routers control pkt entry ****/
            fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_ERR("Failed to init CONTROL_PKT entry status = %d\n", fpa_status);
                mrvl_sai_remove_router_interface(*rif_id);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }

            flowEntry.data.control_pkt.entry_type = FPA_CONTROL_PKTS_TYPE_IPV6_LINK_LOCAL_E;
            cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(FPA_CONTROL_PKTS_TYPE_IPV6_LINK_LOCAL_E, type->s32, portVlan, 0);
            flowEntry.cookie = cookie;
            memset(&flowEntry.data.control_pkt.match.dstIpv6, 0, 16);
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[0]  = 0xFF;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[1]  = 0x02;
            flowEntry.data.control_pkt.match.dstIpv6.s6_addr[15] = 0x02;
            memset(&flowEntry.data.control_pkt.match.dstIpv6Mask, 0xFF, 16);
            if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32){
                flowEntry.data.control_pkt.match.inPort = portVlan;
                flowEntry.data.control_pkt.match.inPortMask = 0xFFFFFFFF;
                flowEntry.priority = 0xFFFFFFFF;

            } else {
                flowEntry.data.control_pkt.match.vlanId = portVlan;
                flowEntry.data.control_pkt.match.vlanIdMask = 0xFFFF;
            }
            flowEntry.data.control_pkt.match.etherType = 0x86dd;
            flowEntry.data.control_pkt.match.etherTypeMask = 0xFFFF;
            flowEntry.data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;
            fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
            if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){
                mrvl_sai_remove_router_interface(*rif_id);
                MRVL_SAI_LOG_ERR("Failed to add entry %llx to CONTROL_PKT table status = %d\n", cookie, fpa_status);
                status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                MRVL_SAI_API_RETURN(status);
            }
		}

		/*** add l2 interface group in case of rif on port ****/
		if (SAI_ROUTER_INTERFACE_TYPE_PORT == type->s32){
			mrvl_sai_utl_create_l2_int_group(portVlan, 0, SAI_VLAN_TAGGING_MODE_UNTAGGED, true, &group);
		}
    }

    /* update sai db */
    mrvl_sai_rif_mtu_table[mtu_idx].mtu_size = mtu_val;
    mrvl_sai_rif_mtu_table[mtu_idx].ref_cntr++;
    
    memset(&mrvl_sai_rif_table[rif_idx], 0, sizeof(mrvl_sai_rif_table_t));
    mrvl_sai_rif_table[rif_idx].valid = true;
    mrvl_sai_rif_table[rif_idx].mtu_idx = mtu_idx;
    memcpy(mrvl_sai_rif_table[rif_idx].src_mac,  mac_addr, 6);
    mrvl_sai_rif_table[rif_idx].intf_type = type->s32;
    mrvl_sai_rif_table[rif_idx].port_vlan_id = portVlan;
    mrvl_sai_rif_table[rif_idx].nbr_valid = false;   
    mrvl_sai_rif_table[rif_idx].nbr_miss_act = miss_action;
    mrvl_sai_rif_table[rif_idx].vrf_id = vr_id;
    mrvl_sai_utl_dlist_init(&mrvl_sai_rif_table[rif_idx].route_list_elem);

    mrvl_sai_virtual_router_update_referance_cntr(vr_id, true);
 
    mrvl_sai_rif_key_to_str(*rif_id, key_str);
    MRVL_SAI_LOG_NTC("Created rif %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**


 * @brief Set router interface attribute
 *
 * @param[in] rif_id Router interface id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_set_router_interface_attribute(_In_ sai_object_id_t rif_id,
                                                 _In_ const sai_attribute_t    *attr)
{
    const sai_object_key_t key = { .key.object_id = rif_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_rif_key_to_str(rif_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_rif_attribs, mrvl_sai_rif_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**



 * @brief Get router interface attribute
 *
 * @param[in] rif_id Router interface id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */
sai_status_t mrvl_sai_get_router_interface_attribute(_In_ sai_object_id_t rif_id,
                                                 _In_ uint32_t                  attr_count,
                                                 _Inout_ sai_attribute_t       *attr_list)
{
    const sai_object_key_t key = { .key.object_id = rif_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == rif_id) {
        MRVL_SAI_LOG_ERR("Invalid router interface id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    mrvl_sai_rif_key_to_str(rif_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_rif_attribs, mrvl_sai_rif_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/* MAC Address [sai_mac_t] */
/* MTU [uint32_t] */
static sai_status_t mrvl_sai_rif_attrib_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    uint32_t                    rif_idx;
    uint64_t                    cookie;
    FPA_FLOW_TABLE_ENTRY_STC    flowEntry;
    FPA_STATUS                  fpa_status;    
    
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", key);
        return SAI_STATUS_FAILURE;
    }

    if (mrvl_sai_rif_table[rif_idx].valid == false){
        MRVL_SAI_LOG_ERR("Rif index %d do not exist\n", rif_idx);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS != (PTR_TO_INT)arg) {
        MRVL_SAI_LOG_ERR("Arg %d not supported\n", arg);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS == (PTR_TO_INT)arg &&
    		mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK) {
    	MRVL_SAI_LOG_ERR("SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS not valid for SAI_ROUTER_INTERFACE_TYPE_LOOPBACK\n");
    	return SAI_STATUS_INVALID_PARAMETER;
    }

    /*TODO change mtu and nbr miss action is not supported*/
    
    if ( mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT ||
    	mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_VLAN )
    {
		cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
														mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                        FPA_IPv4_PROTOCOL,
														mrvl_sai_rif_table[rif_idx].src_mac);
		fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, cookie);
		if (fpa_status != FPA_OK) {
			MRVL_SAI_LOG_ERR("Failed to delete entry Ipv4 %llx from TERMINATION table status = %d\n", cookie, fpa_status);
			return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		}

        cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
														mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                        FPA_IPv6_PROTOCOL,
														mrvl_sai_rif_table[rif_idx].src_mac);
		fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, cookie);
		if (fpa_status != FPA_OK) {
			MRVL_SAI_LOG_ERR("Failed to delete entry Ipv6 %llx from TERMINATION table status = %d\n", cookie, fpa_status);
			return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		}

		memcpy(&mrvl_sai_rif_table[rif_idx].src_mac, value->mac, 6);
		cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
														mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                        FPA_IPv4_PROTOCOL,
														mrvl_sai_rif_table[rif_idx].src_mac);

		flowEntry.cookie = cookie;
		memset(&flowEntry.data.termination.match.destMacMask.addr, 0xFF, 6);
		memcpy(&flowEntry.data.termination.match.destMac, value->mac, 6);
		flowEntry.data.termination.match.etherType = 0x800;
		flowEntry.data.termination.match.etherTypeMask = 0xFFFF;
		flowEntry.data.termination.metadataValue = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;
		flowEntry.data.termination.metadataMask = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;

		fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
		if (fpa_status != FPA_OK) {
			MRVL_SAI_LOG_ERR("Failed to add entry %llx to TERMINATION table status = %d\n", cookie, fpa_status);
			return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		}
        
		cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
														mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                        FPA_IPv6_PROTOCOL,
														mrvl_sai_rif_table[rif_idx].src_mac);

		flowEntry.cookie = cookie;
		memset(&flowEntry.data.termination.match.destMacMask.addr, 0xFF, 6);
		memcpy(&flowEntry.data.termination.match.destMac, value->mac, 6);
		flowEntry.data.termination.match.etherType = 0x86DD;
		flowEntry.data.termination.match.etherTypeMask = 0xFFFF;
		flowEntry.data.termination.metadataValue = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;
		flowEntry.data.termination.metadataMask = FPA_FLOW_TABLE_METADATA_MAC2ME_BIT;

		fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
		if (fpa_status != FPA_OK) {
			MRVL_SAI_LOG_ERR("Failed to add entry %llx to TERMINATION table status = %d\n", cookie, fpa_status);
			return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		}
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Admin State V4, V6 [bool] */
static sai_status_t mrvl_sai_rif_admin_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    uint32_t rif_idx;    

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", key);
        return SAI_STATUS_FAILURE;
    }

    if (mrvl_sai_rif_table[rif_idx].valid == false){
        MRVL_SAI_LOG_ERR("Rif index %d do not exist\n", rif_idx);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }    

    if (value->booldata != true) {
        MRVL_SAI_LOG_ERR("we support only admin enable\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Virtual router id [sai_virtual_router_id_t] */
/* Type [sai_router_interface_type_t] */
/* Assosiated Port [sai_port_id_t] */
/* Assosiated Vlan [sai_vlan_id_t] */
/* MAC Address [sai_mac_t] */
/* MTU [uint32_t] */
static sai_status_t mrvl_sai_rif_attrib_get_prv(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg)
{
    uint32_t                    rif_idx;
/*    FPA_FLOW_TABLE_ENTRY_STC    flowEntry;
    uint64_t                    cookie;
    FPA_STATUS                  fpa_status;
*/
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", key);
        return SAI_STATUS_FAILURE;
    }

    if (mrvl_sai_rif_table[rif_idx].valid == false){
        MRVL_SAI_LOG_ERR("Rif index %d do not exist\n", rif_idx);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

/*
    cookie = MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(mrvl_sai_rif_table[rif_idx].intf_type,
                                                    mrvl_sai_rif_table[rif_idx].port_vlan_id,
                                                    mrvl_sai_rif_table[rif_idx].src_mac);
    
    flowEntry.cookie = cookie;
    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E, &flowEntry);
    if (fpa_status == FPA_NOT_FOUND) {
        MRVL_SAI_LOG_ERR("fpa TERMINATION table index %llx do not exist\n", cookie);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
*/

    switch ((PTR_TO_INT)arg) {
    case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &value->oid)) {
            return SAI_STATUS_FAILURE;
        }
        break;
        
    case SAI_ROUTER_INTERFACE_ATTR_TYPE:
        value->s32 = mrvl_sai_rif_table[rif_idx].intf_type;
        break;
        
    case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:     
        if ((SAI_ROUTER_INTERFACE_TYPE_PORT != mrvl_sai_rif_table[rif_idx].intf_type)/* ||
            (flowEntry.data.termination.match.inPortMask == 0)*/){
            MRVL_SAI_LOG_ERR("Can't get port id from interface whose type isn't port\n");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
        }

        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT,
        		/*flowEntry.data.termination.match.inPort*/
        		mrvl_sai_rif_table[rif_idx].port_vlan_id, &value->oid)) {
            return SAI_STATUS_FAILURE;
        }
        break;

    case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
        if ((SAI_ROUTER_INTERFACE_TYPE_VLAN != mrvl_sai_rif_table[rif_idx].intf_type) /* ||
            (flowEntry.data.termination.match.vlanIdMask == 0)*/ ){
            MRVL_SAI_LOG_ERR("Can't get vlan id from interface whose type isn't vlan\n");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
        }
        value->u16 = /*flowEntry.data.termination.match.vlanId*/ mrvl_sai_rif_table[rif_idx].port_vlan_id;
        break;

    case SAI_ROUTER_INTERFACE_ATTR_MTU:
        if (mrvl_sai_rif_mtu_table[mrvl_sai_rif_table[rif_idx].mtu_idx].mtu_size == 0) {
            MRVL_SAI_LOG_ERR("Can't get mtu  from interface\n");
            return SAI_STATUS_INVALID_ATTRIBUTE_0;
        }
        value->u32 = mrvl_sai_rif_mtu_table[mrvl_sai_rif_table[rif_idx].mtu_idx].mtu_size; 
        break;

    case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
    	 if (mrvl_sai_rif_table[rif_idx].intf_type == SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)
    	 {
    		 MRVL_SAI_LOG_ERR("SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS not relevent for SAI_ROUTER_INTERFACE_TYPE_LOOPBACK");
    		 return SAI_STATUS_INVALID_PARAMETER;
    	 }
        memcpy(value->mac, /*&flowEntry.data.termination.match.destMac*/ mrvl_sai_rif_table[rif_idx].src_mac, 6);
        break;
        
    case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
        value->u32 = mrvl_sai_rif_table[rif_idx].nbr_miss_act; 
        break;
        
    default:
        MRVL_SAI_LOG_ERR("Arg %d not supported\n", arg);
        return SAI_STATUS_NOT_SUPPORTED;

   }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Admin State V4, V6 [bool] */
static sai_status_t mrvl_sai_rif_admin_get_prv(_In_ const sai_object_key_t   *key,
                                _Inout_ sai_attribute_value_t *value,
                                _In_ uint32_t                  attr_index,
                                _Inout_ vendor_cache_t        *cache,
                                void                          *arg)
{
    uint32_t rif_idx;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router ROUTER_INTERFACE\n", key);
        return SAI_STATUS_FAILURE;
    }

    if (mrvl_sai_rif_table[rif_idx].valid == false){
        MRVL_SAI_LOG_ERR("Rif index %d do not exist\n", rif_idx);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    
    value->booldata = true;
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* check if rif exist and valid */
sai_status_t mrvl_sai_rif_is_exist(_In_ uint32_t   rif_idx,
                                   _In_ bool        *is_exist)
{
    if (rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) {
        return SAI_STATUS_FAILURE;
    }
    if (mrvl_sai_rif_table[rif_idx].valid == false) {
       *is_exist = false;
    } else {
        *is_exist = true;
    }
    return SAI_STATUS_SUCCESS;
}

/* get the first neighbor id that was crated on the rif*/
sai_status_t mrvl_sai_rif_get_first_nbr_id(_In_ uint32_t   rif_idx,
                                           _Out_ uint32_t   *first_nbr)
{
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        *first_nbr = MRVL_SAI_INVALID_ID_CNS;
        return SAI_STATUS_FAILURE;
    }
    if (mrvl_sai_rif_table[rif_idx].nbr_valid == false) {
       *first_nbr = MRVL_SAI_INVALID_ID_CNS;
       return SAI_STATUS_ITEM_NOT_FOUND;
    } else {
        *first_nbr = mrvl_sai_rif_table[rif_idx].first_nbr_idx;
        return SAI_STATUS_SUCCESS;
    }
}

/* get the first next hop id that was crated on the rif*/
sai_status_t mrvl_sai_rif_get_first_nh_id(_In_  uint32_t   rif_idx,
                                          _Out_ uint32_t   *first_nh)
{
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        *first_nh = MRVL_SAI_INVALID_ID_CNS;
        return SAI_STATUS_FAILURE;
    }
    if (mrvl_sai_rif_table[rif_idx].nh_valid == false) {
        *first_nh = MRVL_SAI_INVALID_ID_CNS;
       return SAI_STATUS_ITEM_NOT_FOUND;
    } else {
        *first_nh = mrvl_sai_rif_table[rif_idx].first_nh_idx;
        return SAI_STATUS_SUCCESS;
    }
}

/* get rif default action when there is no neighbor */
sai_status_t mrvl_sai_rif_get_nbr_miss_act(_In_ uint32_t               rif_idx,
                                           _Out_ sai_packet_action_t  *nbr_miss_act)
{
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        return SAI_STATUS_FAILURE;
    }
    *nbr_miss_act = mrvl_sai_rif_table[rif_idx].nbr_miss_act;
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_rif_get_entry(_In_ uint32_t               rif_idx,
                                    _Out_ mrvl_sai_rif_table_t  *entry)
{
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        return SAI_STATUS_FAILURE;
    }
    memcpy(entry, &mrvl_sai_rif_table[rif_idx], sizeof(mrvl_sai_rif_table_t));
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_rif_get_vr_id(_In_ uint32_t    rif_idx,
                                    _Out_ uint32_t  *vr_idx)
{
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        return SAI_STATUS_FAILURE;
    }
    *vr_idx = mrvl_sai_rif_table[rif_idx].vrf_id;
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_rif_get_mtu_size(_In_ uint32_t   rif_idx,
                                       _Out_ uint32_t   *mtu_size)
{
    uint32_t mtu_idx;
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || 
        (mrvl_sai_rif_table[rif_idx].valid == false)){
        return SAI_STATUS_FAILURE;
    }
    mtu_idx = mrvl_sai_rif_table[rif_idx].mtu_idx;
    *mtu_size = mrvl_sai_rif_mtu_table[mtu_idx].mtu_size;
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Add new neighbor id the the head of the list in the rif table.
 *  if the new nbr id is "MRVL_SAI_INVALID_ID_CNS" the list is empty
 *     
 * Arguments:
 *    [in] rif_idx - router interface on which the neighbor was created
 *    [in] new_first_nbr - the new neighbor ip to add to the list
 *    [out] old_nbr - the old neighbor id head
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_rif_add_first_nbr_id(_In_  uint32_t  rif_idx,
                                           _In_  uint32_t  new_first_nbr,
                                           _Out_ uint32_t  *old_nbr)
{
    uint32_t    last_nbr_head;
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        return SAI_STATUS_FAILURE;
    }
    last_nbr_head = MRVL_SAI_INVALID_ID_CNS;
    if (new_first_nbr == MRVL_SAI_INVALID_ID_CNS) {
        mrvl_sai_rif_table[rif_idx].nbr_valid = false;
    } else if (mrvl_sai_rif_table[rif_idx].nbr_valid == false) {
       mrvl_sai_rif_table[rif_idx].nbr_valid = true;
    } else {        
        last_nbr_head = mrvl_sai_rif_table[rif_idx].first_nbr_idx;
    }
    if (old_nbr != NULL) {
        *old_nbr = last_nbr_head;
    }
    mrvl_sai_rif_table[rif_idx].first_nbr_idx = new_first_nbr;
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Add new next hop id the the head of the list in the rif table.
 *  if the new nh id is "MRVL_SAI_INVALID_ID_CNS" the list is empty
 *     
 * Arguments:
 *    [in] rif_idx - router interface on which the next hop was created
 *    [in] new_first_nh - the new neighbor ip to add to the list
 *    [out] old_nh - the old next hop id head
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_rif_add_first_nh_id(_In_  uint32_t  rif_idx,
                                          _In_  uint32_t  new_first_nh,
                                          _Out_ uint32_t  *old_nh)
{
    uint32_t    last_nh_head;
    if ((rif_idx >= SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS) || (mrvl_sai_rif_table[rif_idx].valid == false)){
        return SAI_STATUS_FAILURE;
    }
    last_nh_head = MRVL_SAI_INVALID_ID_CNS;
    if (new_first_nh == MRVL_SAI_INVALID_ID_CNS) {
        mrvl_sai_rif_table[rif_idx].nh_valid = false;
    } else if (mrvl_sai_rif_table[rif_idx].nh_valid == false) {
      mrvl_sai_rif_table[rif_idx].nh_valid = true;
    } else {        
        last_nh_head = mrvl_sai_rif_table[rif_idx].first_nh_idx;
    }
    if (old_nh != NULL) {
        *old_nh = last_nh_head;
    }
    mrvl_sai_rif_table[rif_idx].first_nh_idx = new_first_nh; 
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Add route to the list in the rif table.
 *  we add only routes that points directly to this rif
 * 
 * Arguments:
 *    [in] rif_idx - router interface on which the route was created
 *    [in] route_list - route element to add to the list
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_rif_add_route(uint32_t rif_idx, mrvl_sai_utl_dlist_elem_STC *route_list)
{
    if (mrvl_sai_rif_table[rif_idx].valid == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    mrvl_sai_utl_dlist_insert_to_tail (&mrvl_sai_rif_table[rif_idx].route_list_elem, route_list);
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Delete route from the list in the rif table.
 * 
 * Arguments:
 *    [in] rif_idx - router interface on which the route was deleted
 *    [in] route_list - route element to delete from the list
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_rif_del_route(uint32_t rif_idx, mrvl_sai_utl_dlist_elem_STC *route_list)
{
    if (mrvl_sai_rif_table[rif_idx].valid == false) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    mrvl_sai_utl_dlist_remove(route_list);
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Delete route from the list in the rif table.
 * 
 * Arguments:
 *    [in] rif_idx - router interface on which the route was deleted
 *    [in] route_list - route element to delete from the list
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_rif_update_nbr_id(uint32_t rif_idx, uint32_t nbr_idx, sai_ip_address_t *inet_address)
{
    sai_status_t        status;
    sai_packet_action_t nbr_miss_act;
    mrvl_sai_utl_dlist_elem_STC *dlist_elem;
    mrvl_sai_route_hash_data_t  *route_entry;
    
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
    if (!mrvl_sai_utl_dlist_is_empty(&mrvl_sai_rif_table[rif_idx].route_list_elem)) {
        dlist_elem = &mrvl_sai_rif_table[rif_idx].route_list_elem;
        do {
            dlist_elem = mrvl_sai_utl_dlist_get_next(dlist_elem); 
            route_entry = (mrvl_sai_route_hash_data_t*)dlist_elem;
            /* update route table about the change in neighbor */
            mrvl_sai_route_update_nbr_id_if_match( route_entry, inet_address, nbr_idx);

        } while (!mrvl_sai_utl_dlist_is_last(&mrvl_sai_rif_table[rif_idx].route_list_elem, dlist_elem));
    } 
    
    return status; 
}

const sai_router_interface_api_t router_interface_api = {
    mrvl_sai_create_router_interface,
    mrvl_sai_remove_router_interface,
    mrvl_sai_set_router_interface_attribute,
    mrvl_sai_get_router_interface_attribute,
};
