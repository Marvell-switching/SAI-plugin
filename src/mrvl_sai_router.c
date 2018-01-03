/*
 *  Copyright (C) 2014. Mellanox Technologies, Ltd. ALL RIGHTS RESERVED.
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
#define __MODULE__ SAI_ROUTER

typedef struct _mrvl_sai_vr_table_t {
    bool                    valid;
    sai_mac_t               vr_mac_addr;
    uint32_t                rif_ref_cntr;
} mrvl_sai_vr_table_t;

mrvl_sai_vr_table_t   mrvl_sai_vr_table[SAI_SWITCH_MAX_VR_CNS] ={};

static const sai_attribute_entry_t mrvl_sai_router_attribs[] = {
    { SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE, false, true, true, true,
      "Router admin V4 state", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE, false, true, true, true,
      "Router admin V6 state", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS, false, true, true, true,
      "Router source MAC address", SAI_ATTR_VAL_TYPE_MAC },
    { SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION, false, true, true, true,
      "Router action for TTL0/1", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION, false, true, true, true,
      "Router action for IP options", SAI_ATTR_VAL_TYPE_S32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_router_admin_get_prv(_In_ const sai_object_key_t   *key,
                                   _Inout_ sai_attribute_value_t *value,
                                   _In_ uint32_t                  attr_index,
                                   _Inout_ vendor_cache_t        *cache,
                                   void                          *arg);

/*static sai_status_t mrvl_sai_virtual_router_get_mac(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             void                          *arg);*/

static const sai_vendor_attribute_entry_t mrvl_sai_router_vendor_attribs[] = {
    { SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE,
      { true, false, false, true },
      { true, false, true, true },
      mrvl_sai_router_admin_get_prv, (void*)SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE,
      NULL, NULL },
    { SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE,
      { true, false, false, true },
      { true, false, true, true },
      mrvl_sai_router_admin_get_prv, (void*)SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE,
      NULL, NULL },
    { SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_TTL1_PACKET_ACTION,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_VIRTUAL_ROUTER_ATTR_VIOLATION_IP_OPTIONS_PACKET_ACTION,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};
static void mrvl_sai_router_key_to_str(_In_ sai_object_id_t vr_id, _Out_ char *key_str)
{
    uint32_t vr;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vr_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &vr)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "vr ID %u", vr);
        return ;
    }
}

sai_status_t mrvl_sai_virtual_router_dump(void)
{
    uint32_t    idx, used = 0;
    printf("\nVirtual router table:\n");
    printf("| vr id |   mac address   | rif cntr |\n");
    printf("--------------------------------------\n");
    for (idx = 0; idx < SAI_SWITCH_MAX_VR_CNS; idx++) {
        if (mrvl_sai_vr_table[idx].valid == true) {
            printf("| %5d |%02x:%02x:%02x:%02x:%02x:%02x| %8d |", idx, 
                   mrvl_sai_vr_table[idx].vr_mac_addr[0], mrvl_sai_vr_table[idx].vr_mac_addr[1], mrvl_sai_vr_table[idx].vr_mac_addr[2],
                   mrvl_sai_vr_table[idx].vr_mac_addr[3], mrvl_sai_vr_table[idx].vr_mac_addr[4], mrvl_sai_vr_table[idx].vr_mac_addr[5],
                   mrvl_sai_vr_table[idx].rif_ref_cntr); 
            used++;
            printf("\n");
        }
    }
    printf("\nTotal used: %d\n\n",used);
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_l3_dump(void)
{
    mrvl_sai_virtual_router_dump();
    mrvl_sai_rif_dump();
    mrvl_sai_neighbor_dump();
    mrvl_sai_next_hop_dump();
    mrvl_sai_route_dump();
    return SAI_STATUS_SUCCESS;
}

/**


 * @brief Set virtual router attribute Value
 *
 * @param[in] vr_id Virtual router id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_set_virtual_router_attribute(_In_ sai_object_id_t vr_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = vr_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_router_key_to_str(vr_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_router_attribs, mrvl_sai_router_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**


 * @brief Get virtual router attribute Value
 *
 * @param[in] vr_id Virtual router id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_get_virtual_router_attribute(_In_ sai_object_id_t     vr_id,
                                               _In_ uint32_t                attr_count,
                                               _Inout_ sai_attribute_t     *attr_list)
{
    const sai_object_key_t key = { .key.object_id = vr_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_router_key_to_str(vr_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_router_attribs, mrvl_sai_router_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/* Admin V4, V6 State [bool] */
static sai_status_t mrvl_sai_router_admin_get_prv(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg)
{
    uint32_t        vr_idx;

    MRVL_SAI_LOG_ENTER();

    assert((SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE == (PTR_TO_INT)arg));
    /*TODO add ipv6*/
    /* || (SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE == (PTR_TO_INT)arg));*/

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &vr_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router VIRTUAL_ROUTER\n", key);
        return SAI_STATUS_FAILURE;
    }
    if ((vr_idx >= SAI_SWITCH_MAX_VR_CNS) || (mrvl_sai_vr_table[vr_idx].valid == false)) {
        value->booldata = false; 
    }else {
        value->booldata = true; 
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/**

 * @brief Create virtual router
 *
 * @param[out] vr_id Virtual router id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success
 *    #SAI_STATUS_ADDR_NOT_FOUND if neither #SAI_SWITCH_ATTR_SRC_MAC_ADDRESS nor
 *    #SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS is set.

 */

sai_status_t mrvl_sai_create_virtual_router(_Out_ sai_object_id_t         *vr_id,
                                            _In_ sai_object_id_t           switch_id,
                                            _In_ uint32_t                  attr_count,
                                            _In_ const sai_attribute_t    *attr_list)
{
    sai_status_t                 status;
    uint32_t                     vr_idx, mac_mode;
    const sai_attribute_value_t *adminv4, *adminv6, *mac;
    uint32_t                     adminv4_index, adminv6_index, mac_index;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    bool                        ipv4_enable, ipv6_enable;
    FPA_STATUS                  fpa_status;
    
    MRVL_SAI_LOG_ENTER();

    if (NULL == vr_id) {
        MRVL_SAI_LOG_ERR("NULL vr_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_router_attribs, mrvl_sai_router_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_router_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create router, %s\n", list_str);

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE, &adminv4,
                                 &adminv4_index))) {
        ipv4_enable = adminv4->booldata;
        if (ipv4_enable == false) {
            MRVL_SAI_LOG_ERR("failed - ipv4 enable is false\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    } else {
        ipv4_enable = true;
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE, &adminv6,
                                 &adminv6_index))) {
    	ipv6_enable = adminv6->booldata;
        if (ipv6_enable == false) {
            MRVL_SAI_LOG_ERR("failed - ipv6 enable is false\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    } else {
        ipv6_enable = true;
    }

    for (vr_idx = 0; vr_idx < SAI_SWITCH_MAX_VR_CNS; vr_idx++) {
        if (mrvl_sai_vr_table[vr_idx].valid == false) {
            break;
        }
    }
    if (vr_idx >= SAI_SWITCH_MAX_VR_CNS){
        MRVL_SAI_LOG_ERR("vr table is full\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS, &mac, &mac_index))) {
        memcpy(mrvl_sai_vr_table[vr_idx].vr_mac_addr , mac->mac, 6);
    } else {
        /* Get default mac from switch object. Use switch first port, and zero down lower 6 bits port part (64 ports) */
        fpa_status = fpaLibSwitchSrcMacAddressGet(SAI_DEFAULT_ETH_SWID_CNS, &mac_mode, (FPA_MAC_ADDRESS_STC*)mrvl_sai_vr_table[vr_idx].vr_mac_addr); 
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to get mac address.\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }
    }
    mrvl_sai_vr_table[vr_idx].valid = true;
    mrvl_sai_vr_table[vr_idx].rif_ref_cntr = 0;
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, vr_idx, vr_id))) {
    	MRVL_SAI_API_RETURN(status);
    }
    
    mrvl_sai_router_key_to_str(*vr_id, key_str);
    MRVL_SAI_LOG_NTC("Created router %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**


 * @brief Remove virtual router
 *
 * @param[in] vr_id Virtual router id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_remove_virtual_router(_In_ sai_object_id_t vr_id)
{
    uint32_t        vr_idx;
    char            key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vr_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &vr_idx)) {
        MRVL_SAI_LOG_ERR("input param %llx is not router VIRTUAL_ROUTER\n", vr_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    if ((vr_idx >= SAI_SWITCH_MAX_VR_CNS) || (mrvl_sai_vr_table[vr_idx].valid == false) || (mrvl_sai_vr_table[vr_idx].rif_ref_cntr > 0)) {
    	MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    memset(&mrvl_sai_vr_table[vr_idx], 0, sizeof(mrvl_sai_vr_table_t));
    mrvl_sai_router_key_to_str(vr_id, key_str); 
    MRVL_SAI_LOG_NTC("Remove router %s\n", key_str);


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
/* check if rif exist and valid */
sai_status_t mrvl_sai_virtual_router_is_valid(_In_ uint32_t   vr_idx,
                                              _Out_ bool      *is_valid)
{
    if (vr_idx >= SAI_SWITCH_MAX_VR_CNS) {
        return SAI_STATUS_FAILURE;
    }
    if (mrvl_sai_vr_table[vr_idx].valid == false) {
       *is_valid = false;
    } else {
        *is_valid = true;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_virtual_router_get_mac(_In_ uint32_t   vr_idx, _Out_ sai_mac_t mac_address)
{
    MRVL_SAI_LOG_ENTER();

    if ((vr_idx >= SAI_SWITCH_MAX_VR_CNS) || (mrvl_sai_vr_table[vr_idx].valid == false)){
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    memcpy(mac_address, mrvl_sai_vr_table[vr_idx].vr_mac_addr, sizeof(sai_mac_t));
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* check if rif exist and valid */
sai_status_t mrvl_sai_virtual_router_update_referance_cntr(_In_ uint32_t   vr_idx,
                                                           _In_ bool      add)
{
    if ((vr_idx >= SAI_SWITCH_MAX_VR_CNS) || (mrvl_sai_vr_table[vr_idx].valid == false)) {
        return SAI_STATUS_FAILURE;
    }
    if (add) {
        mrvl_sai_vr_table[vr_idx].rif_ref_cntr++;
    } else {
        if (mrvl_sai_vr_table[vr_idx].rif_ref_cntr == 0) {
            return SAI_STATUS_FAILURE;
        }
        mrvl_sai_vr_table[vr_idx].rif_ref_cntr--; 
    }
    return SAI_STATUS_SUCCESS;
}

const sai_virtual_router_api_t virtual_router_api = {
    mrvl_sai_create_virtual_router,
    mrvl_sai_remove_virtual_router,
    mrvl_sai_set_virtual_router_attribute,
    mrvl_sai_get_virtual_router_attribute
};
