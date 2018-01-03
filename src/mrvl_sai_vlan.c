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
#define __MODULE__ SAI_VLAN

#define MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id) ((vlan_id)>=1 && (vlan_id)<4095)


static const sai_attribute_entry_t mrvl_sai_vlan_attribs[] = {  
    {   SAI_VLAN_ATTR_VLAN_ID, true, true, false, true,
        "Vlan id", SAI_ATTR_VAL_TYPE_U16
    }, 
    {   SAI_VLAN_ATTR_MEMBER_LIST, false, false, false, true,
        "Vlan member list", SAI_ATTR_VAL_TYPE_OBJLIST
    },
    {   SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES, false, false, true, true,
        "Vlan Maximum number of learned MAC addresses", SAI_ATTR_VAL_TYPE_U32
    },
    {   SAI_VLAN_ATTR_STP_INSTANCE, false, false, true, true,
        "Vlan associated STP instance", SAI_ATTR_VAL_TYPE_OID
    },
    {   SAI_VLAN_ATTR_LEARN_DISABLE, false, false, true, true,
        "disable learning on a VLAN", SAI_ATTR_VAL_TYPE_BOOL
    },
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VAL_TYPE_UNDETERMINED
    }
};

static const sai_attribute_entry_t mrvl_sai_vlan_member_attribs[] = {   
    {   SAI_VLAN_MEMBER_ATTR_VLAN_ID, true, true, false, true,
        "Vlan id", SAI_ATTR_VAL_TYPE_U16
    },
    {   SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID, true, true, false, true,
        "Port id", SAI_ATTR_VAL_TYPE_OID
    },
    {   SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE, false, true, true, true,
        "Vlan tagging mode", SAI_ATTR_VAL_TYPE_S32
    },
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VAL_TYPE_UNDETERMINED
    }
};

static sai_status_t mrvl_sai_vlan_id_get_prv(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    uint32_t vlan_idx;
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
        return status;
    }
    value->u16 = vlan_idx;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_vlan_id_set_prv(_In_ const sai_object_key_t      *key,
                                             _In_ const sai_attribute_value_t *value,
                                              void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->u16 != 0) {
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Maximum number of learned MAC addresses [uint32_t]
 * zero means learning limit disable. (default to zero). */
static sai_status_t mrvl_sai_vlan_max_learned_addr_get_prv(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->u32 = 0;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Maximum number of learned MAC addresses [uint32_t]
 * zero means learning limit disable. (default to zero). */
static sai_status_t mrvl_sai_vlan_max_learned_addr_set_prv(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->u32 != 0) {
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* STP Instance that the VLAN is associated to [sai_object_id_t]
 * (default to default stp instance id)*/
static sai_status_t mrvl_sai_vlan_stp_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
            {
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP, 0, &value->oid))) {
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* STP Instance that the VLAN is associated to [sai_object_id_t]
 * (default to default stp instance id)*/
static sai_status_t mrvl_sai_vlan_stp_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_status_t status;
    uint32_t     stp_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_STP, &stp_id))) {
        return status;
    }
    if (stp_id != 0) { /* only default parameter is supported */
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT(); 
    return SAI_STATUS_SUCCESS;
}

    /** To disable learning on a VLAN. [bool] (CREATE_AND_SET)
      * (default set to false)
      * This should override port learn settings. If this is set to true on a vlan,
      * then the source mac learning is disabled for this vlan on a member port even
      * if learn is enable on the port(based on port learn attribute)
      */
static sai_status_t mrvl_sai_vlan_disable_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{

    MRVL_SAI_LOG_ENTER();

    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

    /** To disable learning on a VLAN. [bool] (CREATE_AND_SET)
      * (default set to false)
      * This should override port learn settings. If this is set to true on a vlan,
      * then the source mac learning is disabled for this vlan on a member port even
      * if learn is enable on the port(based on port learn attribute)
      */
static sai_status_t mrvl_sai_vlan_disable_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    MRVL_SAI_LOG_ENTER();

    if (value->booldata != false) { /* only default parameter is supported */
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT(); 
    return SAI_STATUS_SUCCESS;
}


/* List of ports in a VLAN [sai_object_list_t]*/
static sai_status_t mrvl_sai_vlan_member_list_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    uint32_t        list_size, port, counter = 0, vlan_member, vlan;
    uint64_t        cookie;
    FPA_STATUS      fpa_status;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;
    
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan)) {
        return SAI_STATUS_FAILURE;
    }

    list_size = value->objlist.count;
    for (port = 0; port < SAI_MAX_NUM_OF_PORTS; port++) {
        cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
        fpa_flow_entry.cookie = cookie;
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, &fpa_flow_entry);
        if (fpa_status == FPA_NOT_FOUND) {
            continue;
        }
        vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
        if (counter < list_size) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &value->objlist.list[counter])) {
                return SAI_STATUS_FAILURE;
            }
        }
        counter++;
    }
    value->objlist.count = counter;
    
    MRVL_SAI_LOG_EXIT(); 
    if (counter >= list_size) 
        return SAI_STATUS_BUFFER_OVERFLOW;
    else
        return SAI_STATUS_SUCCESS;
}

/** VLAN ID [sai_vlan_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_vlan_member_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
     sai_object_id_t  vlan_member_id = key->key.object_id;
     uint32_t         vlan_member;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        return SAI_STATUS_FAILURE;
    }
    value->u16 = ((vlan_member >> 16) & 0xFFFF);
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/** logical port ID [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_vlan_member_port_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_object_id_t  vlan_member_id = key->key.object_id;
    uint32_t         vlan_member;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        return SAI_STATUS_FAILURE;
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, (vlan_member & 0xFFFF), &value->oid)) {
        return SAI_STATUS_FAILURE;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/** VLAN tagging mode [sai_vlan_tagging_mode_t] (CREATE_AND_SET)
 *     (default to SAI_VLAN_TAGGING_MODE_UNTAGGED) */
static sai_status_t mrvl_sai_vlan_member_tagging_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_object_id_t  vlan_member_id = key->key.object_id;
    uint32_t         vlan_member;
    sai_vlan_tagging_mode_t tag_mode;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        return SAI_STATUS_FAILURE;
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_l2_int_group_get_tagging_mode((vlan_member & 0xFFFF), ((vlan_member >> 16) & 0xFFFF), &tag_mode)){
        return SAI_STATUS_FAILURE;
    }
    value->s32 = tag_mode;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/** VLAN tagging mode [sai_vlan_tagging_mode_t] (CREATE_AND_SET)
 *     (default to SAI_VLAN_TAGGING_MODE_UNTAGGED) */
static sai_status_t mrvl_sai_vlan_member_tagging_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_object_id_t  vlan_member_id = key->key.object_id;
    uint32_t         vlan_member;
    sai_vlan_tagging_mode_t tag_mode;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        return SAI_STATUS_FAILURE;
    }
    tag_mode = value->s32;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_l2_int_group_set_tagging_mode((vlan_member & 0xFFFF), ((vlan_member >> 16) & 0xFFFF), tag_mode)){
        return SAI_STATUS_FAILURE;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


static const sai_vendor_attribute_entry_t mrvl_sai_vlan_vendor_attribs[] = {
    {   SAI_VLAN_ATTR_VLAN_ID,
        { true, true, true, true },
        { true, true, true, true },
        mrvl_sai_vlan_id_get_prv, NULL,
        mrvl_sai_vlan_id_set_prv, NULL
    },
    {   SAI_VLAN_ATTR_MEMBER_LIST,
        { true, true, true, true },
        { true, true, true, true },
        mrvl_sai_vlan_member_list_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_max_learned_addr_get_prv, NULL,
        mrvl_sai_vlan_max_learned_addr_set_prv, NULL
    },
    {   SAI_VLAN_ATTR_STP_INSTANCE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_stp_get_prv, NULL,
        mrvl_sai_vlan_stp_set_prv, NULL
    },
    {   SAI_VLAN_ATTR_LEARN_DISABLE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_disable_get_prv, NULL,
        mrvl_sai_vlan_disable_set_prv, NULL
    },
};
static const sai_vendor_attribute_entry_t mrvl_sai_vlan_member_vendor_attribs[] = {
    {   SAI_VLAN_MEMBER_ATTR_VLAN_ID,
        { true, false, false, true },
        { true, false, false, true },
        mrvl_sai_vlan_member_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID,
        { true, false, false, true },
        { true, false, false, true },
        mrvl_sai_vlan_member_port_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE,
        { true, false, true, true },
        { true, false, true, true },
        mrvl_sai_vlan_member_tagging_get_prv, NULL,
        mrvl_sai_vlan_member_tagging_set_prv, NULL
    }
};


static void mrvl_sai_vlan_key_to_str(_In_ sai_vlan_id_t vlan_id, _Out_ char *key_str)
{
    snprintf(key_str, MAX_KEY_STR_LEN, "vlan %u", vlan_id);
}

static void mrvl_sai_vlan_member_key_to_str(_In_ sai_object_id_t vlan_member_id, _Out_ char *key_str)
{
    uint32_t vlan_member;

    if (SAI_STATUS_SUCCESS !=  mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        return;
    }
    snprintf(key_str, MAX_KEY_STR_LEN, "vlan %u port %d", ((vlan_member >> 16) & 0xFFFF), (vlan_member & 0xFFFF));
}

/**
 * @brief Set VLAN Attribute
 *
 * @param[in] vlan_id VLAN ID
 * @param[in] attr Attribute structure containing ID and value
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error





 */

sai_status_t mrvl_sai_set_vlan_attribute(_In_ sai_object_id_t vlan_id,
                                         _In_ const sai_attribute_t *attr)

{
    const sai_object_key_t key = { .key.object_id = vlan_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get VLAN Attribute
 *
 * @param[in] vlan_id VLAN ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list List of attribute structures containing ID and value
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error





 */

sai_status_t mrvl_sai_get_vlan_attribute(_In_ sai_object_id_t vlan_id,
                                         _In_ const uint32_t attr_count,
                                         _Inout_ sai_attribute_t *attr_list)

{
    const sai_object_key_t key = { .key.object_id = vlan_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}


/*
 * Routine Description:
 *    Remove VLAN configuration (remove all VLANs).
 *
 * Arguments:
 *    None
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_all_vlans(void)
{
    MRVL_SAI_LOG_NTC("Remove all vlan\n");

    return SAI_STATUS_SUCCESS;
}

/**

 * @brief Create a VLAN
 *
 * @param[out] vlan_id VLAN ID
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributessrc/mrvl_sai_vlan.c:519:105: error: 'list_str' undeclared (first use in this function)

 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_vlan(_Out_ sai_object_id_t *vlan_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list)
{
    char key_str[MAX_KEY_STR_LEN];
    uint32_t vlan_idx = 0;
    sai_status_t status;
    const sai_attribute_value_t *attr_val       = NULL;
    uint32_t                    attr_idx;

    MRVL_SAI_LOG_ENTER();

    if (NULL == vlan_id) {
        MRVL_SAI_LOG_ERR("NULL vlan_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_vlan_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_NTC("Create vlan_id, %s\n", key_str);

    /* check mandatory attribute SAI_VLAN_ATTR_VLAN_ID */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_ATTR_VLAN_ID, &attr_val, &attr_idx));
    vlan_idx = attr_val->u16;

    /* create SAI VLAN object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan_idx, vlan_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    /* make sure the given vlan_id satisfies the spec*/
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }

    mrvl_sai_vlan_key_to_str(vlan_idx, key_str);
    MRVL_SAI_LOG_NTC("Create vlan %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove VLAN
 *
 * @param[in] vlan_id VLAN member ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error


 */

sai_status_t mrvl_sai_remove_vlan(_In_ sai_object_id_t vlan_id)
{
    char key_str[MAX_KEY_STR_LEN];
    uint32_t vlan_idx = 0;

    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Remove vlan %s\n", key_str);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx)) {
        MRVL_SAI_LOG_ERR("invalid vlan_idx %d\n",vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }


    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_idx <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    
    MRVL_SAI_LOG_EXIT();

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
 
/**
 * @brief Create VLAN Member
 *
 * @param[out] vlan_member_id VLAN member ID
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
 
sai_status_t mrvl_sai_create_vlan_member(_Out_ sai_object_id_t *vlan_member_id,
                                         _In_ sai_object_id_t switch_id,
                                         _In_ uint32_t attr_count,
                                         _In_ const sai_attribute_t *attr_list)
{
    char                        list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *vlan_val, *port_val, *tagging_val;
    uint32_t                    vlan_index, port_index, tagging_index;
    uint32_t                    port_idx;
    uint32_t                    vlan_idx;
    sai_status_t                status;
    sai_vlan_tagging_mode_t     tagging_mode;
    uint32_t                    group, vlan_member;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Create vlan member\n");
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_vlan_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);
    if (NULL == vlan_member_id) {
        MRVL_SAI_LOG_ERR("NULL vlan_member_id entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_vlan_member_attribs, mrvl_sai_vlan_member_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_VLAN_ID, &vlan_val, &vlan_index));
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_val->oid, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
        MRVL_SAI_API_RETURN(status);
    }

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID, &port_val, &port_index));
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_val->oid, SAI_OBJECT_TYPE_PORT, &port_idx))) {
    	MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE, &tagging_val, &tagging_index))) {
        tagging_mode = tagging_val->s32;
    } else {
        tagging_mode = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    }
    if (tagging_mode >= SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED){
        MRVL_SAI_LOG_ERR("Vlan port priority tagged %d not supported\n", tagging_mode);
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
    }

    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    
    /* skip CPU port, which doesn't need to be added/removed to vlan */
    if (SAI_CPU_PORT_CNS == port_idx) {
        MRVL_SAI_LOG_NTC("add port %d not supported\n", port_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PORT_NUMBER);
    }

    /* add group L2 interface, if group exist update tagging mode */
    status = mrvl_sai_utl_create_l2_int_group(port_idx, vlan_idx, tagging_mode, true, &group);
    if (status != SAI_STATUS_SUCCESS)
    	MRVL_SAI_API_RETURN(status);
    
    fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, &fpa_flow_entry);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to init VLAN entry status = %d\n", fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }
        
    /* set vlan entry */
    cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_idx, port_idx);
    fpa_flow_entry.cookie = cookie; 
    fpa_flow_entry.data.vlan.vlanId     = vlan_idx;
    fpa_flow_entry.data.vlan.vlanIdMask = FPA_FLOW_VLAN_MASK_TAG;
    fpa_flow_entry.data.vlan.inPort = port_idx;
    fpa_flow_entry.data.vlan.newTagVid = FPA_FLOW_VLAN_IGNORE_VAL;
    fpa_flow_entry.data.vlan.newTagPcp = FPA_FLOW_VLAN_IGNORE_VAL;

    fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, &fpa_flow_entry);
    if (status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to add VLAN entry %llx to fpa flow table status = %d\n", cookie, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    } 
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_idx, port_idx);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, vlan_member_id))) {
    	MRVL_SAI_API_RETURN(status);
    }
       
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove VLAN Member
 *
 * @param[in] vlan_member_id VLAN member ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error

 */

sai_status_t mrvl_sai_remove_vlan_member(_In_ sai_object_id_t  vlan_member_id)
{
    char                        key_str[MAX_KEY_STR_LEN];
    uint32_t                    vlan_id, port;
    sai_status_t                status;
    uint32_t                    group, vlan_member;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;
    
    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_member_key_to_str(vlan_member_id, key_str);
    MRVL_SAI_LOG_NTC("Remove vlan member %s\n", key_str);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))) {
    	MRVL_SAI_API_RETURN(status);
    }
    vlan_id = ((vlan_member >> 16) & 0xFFFF);
    port = (vlan_member & 0xFFFF);
    
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    
    /* skip CPU port, which doesn't need to be added/removed to vlan */
    if (SAI_CPU_PORT_CNS == port) {
        MRVL_SAI_LOG_NTC("add port %d not supported\n", port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PORT_NUMBER);
    }
         
    /* delete vlan entry */
    memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
    cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_id, port);
    fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, cookie);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to delete entry %llx from VLAN table status = %d\n", cookie, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }
    parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
    parsed_group_identifier.portNum = port;
    parsed_group_identifier.vlanId = vlan_id;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &group);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", vlan_id, port);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    fpa_status = fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, group);
    if (fpa_status == FPA_NOT_READY){
        MRVL_SAI_LOG_ERR("Failed to delete group vlan %d port %d (used by other tables)\n", vlan_id, port);
    }
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Set VLAN Member Attribute
 *
 * @param[in] vlan_member_id VLAN member ID
 * @param[in] attr Attribute structure containing ID and value
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_set_vlan_member_attribute(_In_ sai_object_id_t vlan_member_id,
                                                _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = vlan_member_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_member_key_to_str(vlan_member_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_vlan_member_attribs, mrvl_sai_vlan_member_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get VLAN Member Attribute
 *
 * @param[in] vlan_member_id VLAN member ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list List of attribute structures containing ID and value
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_get_vlan_member_attribute(_In_ sai_object_id_t vlan_member_id,
                                                _In_ const uint32_t attr_count,
                                                _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = vlan_member_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_member_key_to_str(vlan_member_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_vlan_member_attribs, mrvl_sai_vlan_member_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Bulk objects creation.
 *
 * @param[in] switch_id SAI Switch object id
 * @param[in] object_count Number of objects to create
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to create.
 * @param[in] attr_list List of attributes for every object.
 * @param[in] type Bulk operation type.
 *
 * @param[out] object_id List of object ids returned
 * @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
 *
 * @return #SAI_STATUS_SUCCESS on success when all objects are created or #SAI_STATUS_FAILURE when
 * any of the objects fails to create. When there is failure, Caller is expected to go through the
 * list of returned statuses to find out which fails and which succeeds.
 */
sai_status_t mrvl_create_vlan_members (
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t object_count,
        _In_ const uint32_t *attr_count,
        _In_ const sai_attribute_t **attr_list,
        _In_ sai_bulk_op_type_t type,
        _Out_ sai_object_id_t *object_id,
        _Out_ sai_status_t *object_statuses)
{

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Create vlan members\n");

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}
 

/**
 * @brief Bulk objects removal.
 *
 * @param[in] object_count Number of objects to create
 * @param[in] object_id List of object ids
 * @param[in] type Bulk operation type.
 * @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
 *
 * @return #SAI_STATUS_SUCCESS on success when all objects are removed or #SAI_STATUS_FAILURE when
 * any of the objects fails to remove. When there is failure, Caller is expected to go through the
 * list of returned statuses to find out which fails and which succeeds.
 */

sai_status_t mrvl_remove_vlan_members(
        _In_ uint32_t object_count,
        _In_ const sai_object_id_t *object_id,
        _In_ sai_bulk_op_type_t type,
        _Out_ sai_status_t *object_statuses)
{

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Remove vlan members\n");

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}
 
/**


 * @brief Get vlan statistics counters.
 *
 * @param[in] vlan_id VLAN id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_get_vlan_stats(_In_ sai_object_id_t vlan_id,
                                     _In_ uint32_t number_of_counters,
                                     _In_ const sai_vlan_stat_t *counter_ids,
                                     _Out_ uint64_t *counters)
{
    uint32_t ii;

    UNREFERENCED_PARAMETER(vlan_id);
    UNREFERENCED_PARAMETER(number_of_counters);

    MRVL_SAI_LOG_ENTER();

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (NULL == counters) {
        MRVL_SAI_LOG_ERR("NULL counters array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    for (ii = 0; ii < number_of_counters; ii++) {
        counters[ii] = 0;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**

 * @brief Clear vlan statistics counters.
 *
 * @param[in] vlan_id Vlan id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_clear_vlan_stats(_In_ sai_object_id_t vlan_id,
                                       _In_ uint32_t number_of_counters,
                                       _In_ const sai_vlan_stat_t *counter_ids)
{

    UNREFERENCED_PARAMETER(vlan_id);
    UNREFERENCED_PARAMETER(number_of_counters);

    MRVL_SAI_LOG_ENTER();

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}


const sai_vlan_api_t vlan_api = {
    mrvl_sai_create_vlan,
    mrvl_sai_remove_vlan,
    mrvl_sai_set_vlan_attribute,
    mrvl_sai_get_vlan_attribute,
    mrvl_sai_create_vlan_member,
    mrvl_sai_remove_vlan_member,
    mrvl_sai_set_vlan_member_attribute,
    mrvl_sai_get_vlan_member_attribute,
    mrvl_create_vlan_members,
    mrvl_remove_vlan_members, 
    mrvl_sai_get_vlan_stats,
    mrvl_sai_clear_vlan_stats
};


