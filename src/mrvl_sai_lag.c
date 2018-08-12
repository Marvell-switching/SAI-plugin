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

#undef  __MODULE__
#define __MODULE__ SAI_LAG

static mrvl_port_info_t mrvl_sai_lags_db[SAI_LAG_MAX_GROUPS_CNS] = {};

sai_status_t mrvl_sai_lag_member_object_create(_In_ uint32_t             port,
                                               _In_ uint32_t             lag_id,
                                               _Out_ sai_object_id_t     *lag_member_oid);

sai_status_t mrvl_sai_lag_member_to_port_lag(_Out_ uint32_t         *port,
                                             _Out_ uint32_t         *lag_id,
                                             _In_ sai_object_id_t   *lag_member_oid);

static const sai_attribute_entry_t  mrvl_sai_lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
      "LAG port list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_LAG_ATTR_INGRESS_ACL, false, true, true, true,
      "LAG bind point for ingress ACL object", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_LAG_ATTR_EGRESS_ACL, false, true, true, true,
      "LAG bind point for egress ACL object", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_LAG_ATTR_PORT_VLAN_ID, false, true, true, true,
      "LAG VLAN ID", SAI_ATTR_VALUE_TYPE_UINT16 },
    { SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY, false, true, true, true,
      "LAG default VLAN priority", SAI_ATTR_VALUE_TYPE_UINT8 },
    { SAI_LAG_ATTR_DROP_UNTAGGED, false, true, true, true,
      "LAG dropping of untagged frames (ingress)", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_LAG_ATTR_DROP_TAGGED, false, true, true, true,
      "LAG dropping of tagged frames (ingress)", SAI_ATTR_VALUE_TYPE_BOOL },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_lag_port_list_get_prv(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg);
static sai_status_t mrvl_lag_acl_binding_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       void                             *arg);
static sai_status_t mrvl_lag_acl_binding_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg);

static const sai_vendor_attribute_entry_t  mrvl_sai_lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_lag_port_list_get_prv, NULL,
      NULL, NULL },
    { SAI_LAG_ATTR_INGRESS_ACL,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_lag_acl_binding_get, (void*)SAI_LAG_ATTR_INGRESS_ACL,
      mrvl_lag_acl_binding_set, (void*)SAI_LAG_ATTR_INGRESS_ACL },
    { SAI_LAG_ATTR_EGRESS_ACL,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_lag_acl_binding_get, (void*)SAI_LAG_ATTR_EGRESS_ACL,
      mrvl_lag_acl_binding_set, (void*)SAI_LAG_ATTR_EGRESS_ACL},
    { SAI_LAG_ATTR_PORT_VLAN_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_port_lag_pvid_get, NULL,
      mrvl_port_lag_pvid_set, NULL },
    { SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_port_lag_default_vlan_prio_get, NULL,
      mrvl_port_lag_default_vlan_prio_set, NULL },
    { SAI_LAG_ATTR_DROP_UNTAGGED,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_port_lag_drop_tags_get, (void*)SAI_LAG_ATTR_DROP_UNTAGGED,
      mrvl_port_lag_drop_tags_set, (void*)SAI_LAG_ATTR_DROP_UNTAGGED },
    { SAI_LAG_ATTR_DROP_TAGGED,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_port_lag_drop_tags_get, (void*)SAI_LAG_ATTR_DROP_TAGGED,
      mrvl_port_lag_drop_tags_set, (void*)SAI_LAG_ATTR_DROP_TAGGED },
};

static const sai_attribute_entry_t  mrvl_sai_lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
      "LAG ID for LAG Member", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID for LAG Member", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
     { SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE, false, true, true, true,
      "LAG Member Egress Disable", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE, false, true, true, true,
      "LAG Member Ingress Disable", SAI_ATTR_VALUE_TYPE_BOOL },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_lag_member_lag_id_get_prv(_In_ const sai_object_key_t   *key,
                                                       _Inout_ sai_attribute_value_t *value,
                                                       _In_ uint32_t                  attr_index,
                                                       _Inout_ vendor_cache_t        *cache,
                                                       void                          *arg);
static sai_status_t mrvl_sai_lag_member_port_id_get_prv(_In_ const sai_object_key_t   *key,
                                                        _Inout_ sai_attribute_value_t *value,
                                                        _In_ uint32_t                  attr_index,
                                                        _Inout_ vendor_cache_t        *cache,
                                                        void                          *arg);
static sai_status_t mrvl_sai_lag_member_egress_disable_get_prv(_In_ const sai_object_key_t   *key,
                                                               _Inout_ sai_attribute_value_t *value,
                                                               _In_ uint32_t                  attr_index,
                                                               _Inout_ vendor_cache_t        *cache,
                                                               void                          *arg);
static sai_status_t mrvl_sai_lag_member_egress_disable_set_prv(_In_ const sai_object_key_t      *key,
                                                               _In_ const sai_attribute_value_t *value,
                                                               void                             *arg);
static sai_status_t mrvl_sai_lag_member_ingress_disable_get_prv(_In_ const sai_object_key_t   *key,
                                                                _Inout_ sai_attribute_value_t *value,
                                                                _In_ uint32_t                  attr_index,
                                                                _Inout_ vendor_cache_t        *cache,
                                                                void                          *arg);
static sai_status_t mrvl_sai_lag_member_ingress_disable_set_prv(_In_ const sai_object_key_t      *key,
                                                                _In_ const sai_attribute_value_t *value,
                                                                void                             *arg);

static const sai_vendor_attribute_entry_t  mrvl_sai_lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_lag_member_lag_id_get_prv, NULL,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_lag_member_port_id_get_prv, NULL,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_lag_member_egress_disable_get_prv, NULL,
      mrvl_sai_lag_member_egress_disable_set_prv, NULL },
    { SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_lag_member_ingress_disable_get_prv, NULL,
      mrvl_sai_lag_member_ingress_disable_set_prv, NULL },
};

static void lag_key_to_str(
        _In_  sai_object_id_t   lag_oid, 
        _Out_ char              *key_str
)
{
    uint32_t lagid;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lagid)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "Invalid LAG ID");
    } else {
        lagid = mrvl_lag_logical_to_real_index_MAC(lagid);
        snprintf(key_str, MAX_KEY_STR_LEN, "LAG ID %u", lagid);
    }
}

static void lag_member_key_to_str(_In_ sai_object_id_t lag_member_oid, _Out_ char *key_str)
{
    uint32_t lag_memberid;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &lag_memberid)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "Invalid LAG Member ID");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "LAG ID Member %u", lag_memberid);
    }
}


/** SAI port list [sai_object_list_t] */
static sai_status_t mrvl_sai_lag_port_list_get_prv(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
    sai_status_t     status;
    uint32_t         lag_id;   
    sai_object_id_t  lag_oid = key->key.object_id; 

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_id))) {
        MRVL_SAI_LOG_ERR("Failed to convert lag oid %" PRIx64 "\n", lag_oid);
        MRVL_SAI_API_RETURN(status);
    }
    lag_id = mrvl_lag_logical_to_real_index_MAC(lag_id);
    if (true != is_lag_used(lag_id))
    {
        MRVL_SAI_LOG_ERR("Lag %d has not been created\n", lag_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }

    if (true != is_lag_empty(lag_id))
    {
        value->objlist.count = mrvl_sai_lags_db[lag_id].group_member_counter;

        /* create the port list from members of the LAG */  
        status = mrvl_sai_utl_get_l2_lag_group_bucket_list(lag_id, &value->objlist);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get lag group port list for lag %d\n", lag_id);
            MRVL_SAI_API_RETURN(status);
        }
    }

    MRVL_SAI_LOG_NTC("LAG %d has %d members\n", lag_id, mrvl_sai_lags_db[lag_id].group_member_counter);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/* LAG ID [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_lag_member_lag_id_get_prv(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_status_t     status;
    uint32_t         port_id, lag_id;
    sai_object_id_t  lag_member_oid = key->key.object_id;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    /* create SAI LAG object */
    lag_id = mrvl_lag_real_to_logical_index_MAC(lag_id);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_LAG, lag_id, &value->oid))) {
        MRVL_SAI_LOG_ERR("Failed to create lag oid from lag %d\n", lag_id);
        MRVL_SAI_API_RETURN(status);
    }    
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* logical port ID [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_lag_member_port_id_get_prv(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_status_t     status;
    uint32_t         port_id, lag_id;
    sai_object_id_t  lag_member_oid = key->key.object_id;;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    /* create SAI PORT object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_id, &value->oid))) {
        MRVL_SAI_LOG_ERR("Failed to create port oid from port %d\n", port_id);
        MRVL_SAI_API_RETURN(status);
    }   

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Disable traffic distribution to this port as part of LAG. [bool] (CREATE_AND_SET) default to FALSE */
static sai_status_t mrvl_sai_lag_member_egress_disable_get_prv(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{    
    uint32_t            port_id, lag_id;
    sai_object_id_t     lag_member_oid = key->key.object_id;

    MRVL_SAI_LOG_ENTER();
    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    value->booldata = false;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Disable traffic distribution to this port as part of LAG. [bool] (CREATE_AND_SET) default to FALSE */
static sai_status_t mrvl_sai_lag_member_egress_disable_set_prv(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{    
    uint32_t         port_id, lag_id;
    sai_object_id_t  lag_member_oid = key->key.object_id;

    MRVL_SAI_LOG_ENTER();
    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (value->booldata != false ) {
        MRVL_SAI_LOG_WRN("invalid lag_member_oid %" PRIx64 " egress disable attribute %d\n", lag_member_oid, value->booldata);        
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Disable traffic collection from this port as part of LAG. [bool] (CREATE_AND_SET) default to FALSE. */
static sai_status_t mrvl_sai_lag_member_ingress_disable_get_prv(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{    
    uint32_t            port_id, lag_id;
    sai_object_id_t     lag_member_oid = key->key.object_id;

    MRVL_SAI_LOG_ENTER();
    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    value->booldata = false;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Disable traffic collection from this port as part of LAG. [bool] (CREATE_AND_SET) default to FALSE. */
static sai_status_t mrvl_sai_lag_member_ingress_disable_set_prv(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{    
    uint32_t         port_id, lag_id;
    sai_object_id_t  lag_member_oid = key->key.object_id;

    MRVL_SAI_LOG_ENTER();
    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (value->booldata != false ) {
        MRVL_SAI_LOG_WRN("invalid lag_member_oid %" PRIx64 " ingress disable attribute %d\n", lag_member_oid, value->booldata);        
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_lag_acl_binding_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       _In_ void                        *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     lag;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_LAG, &lag))) {
        MRVL_SAI_API_RETURN(status);
    }

    lag = mrvl_lag_logical_to_real_index_MAC(lag);
    if (lag > SAI_LAG_MAX_GROUPS_CNS){
        MRVL_SAI_LOG_ERR("Invalid lag %d\n", lag);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (value->oid == SAI_NULL_OBJECT_ID){
    	/* unbind action */
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_unbind_from_lag(arg, lag))){
            MRVL_SAI_LOG_ERR("Unable to unbind lag %d from ACL TABLE\n", lag);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    	}
    }
    else {
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_bind_to_lag(arg, value->oid, lag))){
            MRVL_SAI_LOG_ERR("Unable to unbind lag %d from ACL TABLES\n", lag);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    	}
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

static sai_status_t mrvl_lag_acl_binding_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg){
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     lag;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_LAG, &lag))) {
        MRVL_SAI_API_RETURN(status);
    }

    lag = mrvl_lag_logical_to_real_index_MAC(lag);
    if (lag > SAI_LAG_MAX_GROUPS_CNS){
        MRVL_SAI_LOG_ERR("Invalid lag %d\n", lag);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_get_table_id_per_lag(arg, lag, value))){
        MRVL_SAI_LOG_ERR("Unable to get assigned ACL table per lag %d\n", lag);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}

bool is_lag_interface(_In_ uint32_t  lag_idx)
{
    return (lag_idx >= SAI_FIRST_LAG_INDEX_CNS);
}

bool is_lag_used(_In_ uint32_t  lag_idx)
{
    return (true == mrvl_sai_lags_db[lag_idx].is_present);
}

bool is_lag_empty(_In_ uint32_t  lag_idx)
{
    return (0 == mrvl_sai_lags_db[lag_idx].group_member_counter);
}

uint32_t lag_members_count(_In_ uint32_t  lag_idx)
{
    return mrvl_sai_lags_db[lag_idx].group_member_counter;
}

void lag_vlan_info_set(_In_ uint32_t lag_idx, _In_ sai_vlan_id_t vlan_id, _In_ sai_vlan_tagging_mode_t tagging_mode)
{
    mrvl_sai_lags_db[lag_idx].vlan_id = vlan_id;
    mrvl_sai_lags_db[lag_idx].tagging_mode = tagging_mode;
}

void lag_vlan_info_reset(_In_ uint32_t lag_idx)
{
    mrvl_sai_lags_db[lag_idx].vlan_id = 0;
}

sai_vlan_id_t lag_vlan_info_get(_In_ uint32_t lag_idx, _Out_ sai_vlan_tagging_mode_t *tagging_mode)
{
    *tagging_mode = mrvl_sai_lags_db[lag_idx].tagging_mode; 
    return mrvl_sai_lags_db[lag_idx].vlan_id;
}
sai_status_t mrvl_sai_lag_member_object_create(_In_ uint32_t             port,
                                               _In_ uint32_t             lag_id,
                                               _Out_ sai_object_id_t     *lag_member_oid)
{
    sai_status_t status;
    uint8_t     ext_data[RESERVED_DATA_LENGTH_CNS];

    MRVL_SAI_LOG_ENTER();
    assert(lag_member_oid);

    memset(ext_data, 0, RESERVED_DATA_LENGTH_CNS);
    ext_data[0] = (uint8_t)(lag_id & 0xFF);
    ext_data[1] = (uint8_t)((lag_id << 8) & 0xFF);
    status = mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_LAG_MEMBER, port, ext_data, lag_member_oid);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed to create queue object with port %d, lag index %d\n", port, lag_id);
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_lag_member_to_port_lag(_Out_ uint32_t         *port,
                                             _Out_ uint32_t         *lag_id,
                                             _In_ sai_object_id_t   *lag_member_oid)
{
    sai_status_t status;
    uint8_t     ext_data[RESERVED_DATA_LENGTH_CNS];

    MRVL_SAI_LOG_ENTER();
    assert(lag_member_oid);

    memset(ext_data, 0, RESERVED_DATA_LENGTH_CNS);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(*lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, port, ext_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object lag member\n");
        MRVL_SAI_API_RETURN(status);
    }
    *lag_id = (uint32_t)(ext_data[1] >> 8 | ext_data[0]);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Create LAG Member
 *
 * @param[out] lag_member_id LAG Member id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_create_lag_member(
    _Out_ sai_object_id_t          *lag_member_oid,
    _In_ sai_object_id_t            switch_id,
    _In_ uint32_t                   attr_count,
    _In_ const sai_attribute_t      *attr_list)
{
    sai_status_t                 status;
    uint32_t                     port_id, real_lag_id, lag_id, changed;
    const sai_attribute_value_t  *attr_lag_id, *attr_port_id, *attr_egress_disable, *attr_ingress_disable;
    sai_object_id_t              lag_oid, port_oid;
    uint32_t                     index;
    uint8_t                      data_ext[2];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    bool                         ingress_disable, egress_disable;
    MRVL_SAI_LOG_ENTER(); 

    if (NULL == lag_member_oid) {
        MRVL_SAI_LOG_ERR("NULL lag member id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_lag_member_attribs, mrvl_sai_lag_member_vendor_attribs,
                                                 SAI_COMMON_API_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* Trace: print input parametrs */ 
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create lag member, %s\n", list_str);

    /* Get LAG member attributes: LAG ID, port, egress & ingress modes */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &attr_lag_id, &index));    
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &attr_port_id, &index));        

    /* get LAG member egress mode. Default - egress enabled */    
    if (SAI_STATUS_SUCCESS == 
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE, &attr_egress_disable, &index)))
    {
        egress_disable = attr_egress_disable->booldata;
    }
    else
    {
        egress_disable = true;
    }
    /* get LAG member ingress mode. Default - ingress enabled */    
    if (SAI_STATUS_SUCCESS == 
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE, &attr_ingress_disable, &index)))
    {
        ingress_disable = attr_ingress_disable->booldata;
    }
    else
    {
        ingress_disable = true;
    }

    lag_oid  = attr_lag_id->oid;
    port_oid = attr_port_id->oid;

    /* Get port & LAG numbers */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_oid_to_lag_port(lag_oid, &lag_id))) {
        MRVL_SAI_LOG_ERR("Failed to convert LAG oid %" PRIx64 "\n", lag_oid);
        MRVL_SAI_API_RETURN(status);
    }
    real_lag_id = mrvl_lag_logical_to_real_index_MAC(lag_id);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_oid_to_lag_port(port_oid, &port_id))) {
        MRVL_SAI_LOG_ERR("Failed to convert port oid %" PRIx64 "\n", port_oid);
        MRVL_SAI_API_RETURN(status);
    }

    if (true != is_lag_used(real_lag_id))
    {
        MRVL_SAI_LOG_ERR("Lag %d has not been created\n", real_lag_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }

    if (true == mrvl_sai_is_port_lag_member(port_id))
    {
        MRVL_SAI_LOG_NTC("Port %d is already a member in LAG\n", port_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
    }
    /* Add port as a member to lag */
    status = mrvl_sai_utl_update_l2_lag_group_bucket(real_lag_id, port_oid, MRVL_SAI_UTL_ADD, &changed);
    if (SAI_STATUS_SUCCESS != status) { /* lag group was deleted */
        mrvl_sai_lags_db[real_lag_id].group_member_counter = 0;
        mrvl_sai_lags_db[real_lag_id].is_present = false;
        MRVL_SAI_LOG_ERR("Failed to add L2 LAG group bucket: lag %d, port %d\n", real_lag_id, port_id);
        MRVL_SAI_API_RETURN(status);
    }else {
        mrvl_sai_lags_db[real_lag_id].group_member_counter += changed;
    }
    status = mrvl_sai_port_set_port_in_lag(port_id, lag_id, true);
    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to add port %d to lag %d\n", port_id, lag_id);
        MRVL_SAI_API_RETURN(status);    
    }
                                                         
    /* create SAI LAG member object */       
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_lag_member_object_create(port_id, real_lag_id, lag_member_oid))) {
        MRVL_SAI_LOG_ERR("Failed to create lag member oid from port %d, lag %d\n", port_id, real_lag_id);
        MRVL_SAI_API_RETURN(status);
    }

    lag_member_key_to_str(*lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove LAG Member
 *
 * @param[in] lag_member_id LAG Member id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_remove_lag_member(
    _In_ sai_object_id_t    lag_member_oid
)
{
    sai_status_t        status;
    uint32_t            port_id, lag_id, logic_lag_id, changed;     
    char                key_str[MAX_KEY_STR_LEN];  
    sai_object_id_t     port_oid;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_to_port_lag(&port_id, &lag_id, &lag_member_oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert lag member oid %" PRIx64 " to port and lag\n", lag_member_oid);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    /* create SAI PORT object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_id, &port_oid))) {
        MRVL_SAI_API_RETURN(status);
    }

    /* Remove port from the LAG */
    status = mrvl_sai_utl_update_l2_lag_group_bucket(lag_id, port_oid, MRVL_SAI_UTL_DEL, &changed);
    if (SAI_STATUS_SUCCESS != status) { /* lag group was deleted */
        mrvl_sai_lags_db[lag_id].group_member_counter = 0;
        mrvl_sai_lags_db[lag_id].is_present = false;
    }else {
        mrvl_sai_lags_db[lag_id].group_member_counter -= changed;
    }
    logic_lag_id = mrvl_lag_real_to_logical_index_MAC(lag_id);
    status = mrvl_sai_port_set_port_in_lag(port_id, logic_lag_id, false);
    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to remove port %d from lag %d\n", port_id, lag_id);
        MRVL_SAI_API_RETURN(status);    
    }                                   
            
    lag_member_key_to_str(lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Removed %s\n", key_str);
      
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}


/**
 * @brief Set LAG Member Attribute
 *
 * @param[in] lag_member_id LAG Member id
 * @param[in] attr Structure containing ID and value to be set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error

 */

sai_status_t mrvl_sai_set_lag_member_attribute(
    _In_ sai_object_id_t        lag_member_oid, 
    _In_ const sai_attribute_t  *attr)
{
    const sai_object_key_t key = { .key.object_id = lag_member_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    lag_member_key_to_str(lag_member_oid, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_lag_member_attribs, mrvl_sai_lag_member_vendor_attribs, attr);

    MRVL_SAI_LOG_NTC("Set %s\n", key_str);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get LAG Member Attribute
 *
 * @param[in] lag_member_id LAG Member id
 * @param[in] attr_count Number of attributes to be get
 * @param[inout] attr_list List of structures containing ID and value to be get
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_get_lag_member_attribute(
    _In_ sai_object_id_t     lag_member_oid,
    _In_ uint32_t            attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = lag_member_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == lag_member_oid) {
        MRVL_SAI_LOG_ERR("NULL lag member id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }
    lag_member_key_to_str(lag_member_oid, key_str);
    status =  mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_lag_member_attribs, mrvl_sai_lag_member_vendor_attribs,
                                        attr_count, attr_list);

    lag_member_key_to_str(lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Get %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Create LAG
 *
 * @param[out] lag_id LAG id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_create_lag( 
    _Out_ sai_object_id_t       *lag_oid,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t               attr_count,
    _In_ const sai_attribute_t  *attr_list)
{
    sai_status_t                 status;
    uint32_t                     lag_idx, group, index;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    const sai_attribute_value_t *attr_ing_acl = NULL, *attr_eg_acl = NULL, *attr_vlan = NULL, *attr_vlan_prio = NULL, *attr_drop_tagged = NULL, *attr_drop_untagged = NULL;   

    MRVL_SAI_LOG_ENTER();

    if (NULL == lag_oid) {
        MRVL_SAI_LOG_ERR("NULL lag id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }
    status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_lag_attribs, mrvl_sai_lag_vendor_attribs,
                                                 SAI_COMMON_API_CREATE);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);    
    MRVL_SAI_LOG_NTC("Create lag, %s\n", list_str);    

    /* find new lag id - LAG '0' is not valid in CPSS. LAG group range 1..SAI_LAG_MAX_GROUPS_CNS */
    for (lag_idx = 1; lag_idx < SAI_LAG_MAX_GROUPS_CNS; lag_idx++) {
        if (true != is_lag_used(lag_idx)) {
            break;
        }
    }
    if (lag_idx >= SAI_LAG_MAX_GROUPS_CNS) { 
        MRVL_SAI_LOG_ERR("Failed to allocate LAG id in SAI DB\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }

    /* check attr SAI_LAG_ATTR_INGRESS_ACL  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_INGRESS_ACL, &attr_ing_acl, &index))){
        if (attr_ing_acl->oid != SAI_NULL_OBJECT_ID) {
        	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_bind_to_lag((void*)SAI_LAG_ATTR_INGRESS_ACL, attr_ing_acl->oid, lag_idx))){
                MRVL_SAI_LOG_ERR("Unable to bind lag %d to ACL TABLE %" PRIx64 "\n", attr_ing_acl->oid);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        	}
        }
    }
    /* check attr SAI_LAG_ATTR_EGRESS_ACL  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_EGRESS_ACL, &attr_eg_acl, &index))){
        if (attr_eg_acl->oid != SAI_NULL_OBJECT_ID) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_bind_to_lag((void*)SAI_LAG_ATTR_EGRESS_ACL, attr_eg_acl->oid, lag_idx))){
                MRVL_SAI_LOG_ERR("Unable to bind lag %d to ACL TABLE %" PRIx64 "\n", attr_eg_acl->oid);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        	}
        }
    }

    mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_PORT_VLAN_ID, &attr_vlan, &index);
    mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY, &attr_vlan_prio, &index);
    mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_DROP_UNTAGGED, &attr_drop_untagged, &index);
    mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_DROP_TAGGED, &attr_drop_tagged, &index);
    
    /* set LAG pvid */
    if (attr_vlan) 
    {}

    /* set default VLAN priority */
    if (attr_vlan_prio)
    {}

    /* set LAG's accepted frame types */
    if (attr_drop_tagged || attr_drop_untagged) 
    {}

    mrvl_sai_lags_db[lag_idx].index = mrvl_lag_real_to_logical_index_MAC(lag_idx);
    /* create SAI LAG object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_LAG, mrvl_sai_lags_db[lag_idx].index, lag_oid))) {
        MRVL_SAI_API_RETURN(status);
    }
  
    status = mrvl_sai_utl_create_l2_lag_group(lag_idx, &group);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't create lag group %d\n", lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    mrvl_sai_lags_db[lag_idx].is_present = true;
    mrvl_sai_lags_db[lag_idx].group_member_counter = 0; /* empty lag group */

    lag_key_to_str(*lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);    
}


/**
 * @brief Remove LAG
 *
 * @param[in] lag_id LAG id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_remove_lag(
    _In_ sai_object_id_t    lag_oid
)
{
    sai_status_t        status;    
    uint32_t            lag_idx, port_idx, i; 
    uint8_t             data_ext[RESERVED_DATA_LENGTH_CNS];      
    sai_object_id_t     port_oid;
    sai_object_id_t     lag_member_oid;
    char                key_str[MAX_KEY_STR_LEN];
    sai_object_list_t   port_objlist;
    sai_object_id_t     port_list[SAI_MAX_NUM_OF_PORTS];
    sai_attribute_t     acl_table;

    MRVL_SAI_LOG_ENTER();        

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_idx))) {
        MRVL_SAI_LOG_ERR("Failed to convert lag oid %" PRIx64 "\n", lag_oid);
        MRVL_SAI_API_RETURN(status);
    }
    
    lag_idx = mrvl_lag_logical_to_real_index_MAC(lag_idx);
    if (true != is_lag_used(lag_idx))
    {
        MRVL_SAI_LOG_ERR("Lag index %d not used\n", lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }
    if (SAI_STATUS_SUCCESS == mrvl_sai_acl_get_table_id_per_lag((void*)SAI_LAG_ATTR_INGRESS_ACL, lag_idx, &acl_table.value)) {
        if (acl_table.value.oid != SAI_NULL_OBJECT_ID){
            MRVL_SAI_LOG_ERR("Unable to remove lag_idx %d - bound to ingress ACL table\n",lag_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    } 
    if (SAI_STATUS_SUCCESS == mrvl_sai_acl_get_table_id_per_lag((void*)SAI_LAG_ATTR_EGRESS_ACL, lag_idx, &acl_table.value)) {
        if (acl_table.value.oid != SAI_NULL_OBJECT_ID){
            MRVL_SAI_LOG_ERR("Unable to remove lag_idx %d - bound to egress ACL table\n",lag_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    } 

    /*  Check if the LAG is empty */
    if (true != is_lag_empty(lag_idx))
    {
        port_objlist.count = mrvl_sai_lags_db[lag_idx].group_member_counter; 
        port_objlist.list = port_list;
       
        /* create the port list from members of the LAG */  
        status = mrvl_sai_utl_get_l2_lag_group_bucket_list(lag_idx, &port_objlist);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get lag group port list for lag index %d\n", lag_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }    

        /*  Create and remove LAG members */
        for (i = 0; i < port_objlist.count; i++) {
            port_oid = port_objlist.list[i];
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_oid_to_lag_port(port_oid, &port_idx)) {
                MRVL_SAI_LOG_ERR("Failed to convert port oid %" PRIx64 " to index\n", port_oid);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
            }
                    
            if (SAI_STATUS_SUCCESS != mrvl_sai_lag_member_object_create(port_idx, lag_idx, &lag_member_oid)) {
                MRVL_SAI_LOG_ERR("Failed to create LAG member oid from port %d, lag\n", port_idx, lag_idx);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            if (SAI_STATUS_SUCCESS != mrvl_sai_remove_lag_member(lag_member_oid)) {
                MRVL_SAI_LOG_ERR("failed to remove lag member, lag_member_oid %d\n",lag_member_oid);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }            
        }
    }
    
    
    /* Remove the LAG from system */
    status = mrvl_sai_utl_delete_l2_lag_group(lag_idx);    
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't remove lag group %d\n", lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    
    /* clear entry in DB */
    memset(&mrvl_sai_lags_db[lag_idx], 0, sizeof(mrvl_port_info_t));

    lag_key_to_str(lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Removed %s and %d LAG members\n", key_str, port_objlist.count);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Set LAG Attribute
 *
 * @param[in] lag_id LAG id
 * @param[in] attr Structure containing ID and value to be set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_set_lag_attribute(
        _In_ sai_object_id_t        lag_oid, 
        _In_ const sai_attribute_t  *attr)
{
    const sai_object_key_t key = { .key.object_id = lag_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;    

    MRVL_SAI_LOG_ENTER(); 

    lag_key_to_str(lag_oid, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_lag_attribs, mrvl_sai_lag_vendor_attribs, attr);

    MRVL_SAI_LOG_NTC("Set %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get LAG Attribute
 *
 * @param[in] lag_id LAG id
 * @param[in] attr_count Number of attributes to be get
 * @param[inout] attr_list List of structures containing ID and value to be get
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_get_lag_attribute(
    _In_ sai_object_id_t        lag_oid,
    _In_ uint32_t               attr_count,
    _Inout_ sai_attribute_t     *attr_list
)
{
    const sai_object_key_t key = { .key.object_id = lag_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;    

    MRVL_SAI_LOG_ENTER();
    
    if (SAI_NULL_OBJECT_ID == lag_oid) {
        MRVL_SAI_LOG_ERR("NULL lag id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_lag_attribs, mrvl_sai_lag_vendor_attribs, attr_count, attr_list);

    lag_key_to_str(lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Get %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
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

sai_status_t mrvl_sai_create_lag_members(
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t object_count,
        _In_ const uint32_t *attr_count,
        _In_ const sai_attribute_t **attr_list,
        _In_ sai_bulk_op_error_mode_t mode,
        _Out_ sai_object_id_t *object_id,
        _Out_ sai_status_t *object_statuses)
{

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
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

sai_status_t mrvl_sai_remove_lag_members(
        _In_ uint32_t object_count,
        _In_ const sai_object_id_t *object_id,
        _In_ sai_bulk_op_error_mode_t mode,
        _Out_ sai_status_t *object_statuses)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

sai_status_t mrvl_sai_get_lag_port_list(
    _In_ const uint32_t          lag_id,
    _Out_ sai_object_list_t     *portobjlist)
{
    sai_status_t     status;    
    MRVL_SAI_LOG_ENTER();
    if (lag_id < 1 || lag_id > SAI_LAG_MAX_GROUPS_CNS) { 
        MRVL_SAI_LOG_ERR("invalid lag %d\n",lag_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (true != is_lag_used(lag_id))
    {
        MRVL_SAI_LOG_ERR("Lag index %d not used\n", lag_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }

    /*  Check if the LAG is empty */
    if (true != is_lag_empty(lag_id))
    {
        portobjlist->count = mrvl_sai_lags_db[lag_id].group_member_counter;

        /* create the port list from members of the LAG */  
        status = mrvl_sai_utl_get_l2_lag_group_bucket_list(lag_id, portobjlist);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get lag group port list for lag %d\n", lag_id);
            MRVL_SAI_API_RETURN(status);
        }
    }

    MRVL_SAI_LOG_NTC("LAG %d has %d members\n", lag_id, mrvl_sai_lags_db[lag_id].group_member_counter);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


const sai_lag_api_t lag_api = {
    mrvl_sai_create_lag,
    mrvl_sai_remove_lag,
    mrvl_sai_set_lag_attribute,
    mrvl_sai_get_lag_attribute,
    mrvl_sai_create_lag_member,
    mrvl_sai_remove_lag_member,
    mrvl_sai_set_lag_member_attribute,
    mrvl_sai_get_lag_member_attribute,
    mrvl_sai_create_lag_members,
    mrvl_sai_remove_lag_members
};
