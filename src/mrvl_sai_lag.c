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
#define __MODULE__ SAI_LAG

static mrvl_sai_lag_group_table_t mrvl_sai_lag_group_table[SAI_LAG_MAX_GROUPS_CNS] = {};

static const sai_attribute_entry_t  mrvl_sai_lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
      "LAG port list", SAI_ATTR_VAL_TYPE_OBJLIST },
    { SAI_LAG_ATTR_INGRESS_ACL_LIST, false, false, false, false,
      "LAG bind point for ingress ACL objects", SAI_ATTR_VAL_TYPE_OBJLIST },
    { SAI_LAG_ATTR_EGRESS_ACL_LIST, false, false, false, false,
      "LAG bind point for egress ACL objects", SAI_ATTR_VAL_TYPE_OBJLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_lag_port_list_get_prv(_In_ const sai_object_key_t   *key,
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
    { SAI_LAG_ATTR_INGRESS_ACL_LIST,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_LAG_ATTR_EGRESS_ACL_LIST,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
};

static const sai_attribute_entry_t  mrvl_sai_lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
      "LAG ID for LAG Member", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID for LAG Member", SAI_ATTR_VAL_TYPE_OID },
     { SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE, false, true, true, true,
      "LAG Member Egress Disable", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE, false, true, true, true,
      "LAG Member Ingress Disable", SAI_ATTR_VAL_TYPE_BOOL },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
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
    sai_object_id_t  lag_oid; 

    lag_oid = key->object_id;        

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_oid %d\n",lag_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (mrvl_sai_lag_group_table[lag_id].used != true)
    {
        MRVL_SAI_LOG_ERR("Lag index %d not used\n", lag_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }

    value->objlist.count = mrvl_sai_lag_group_table[lag_id].group_member_counter;

    /* create the port list from members of the LAG */  
    status = mrvl_sai_utl_get_l2_lag_group_bucket_list(lag_id, &value->objlist);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't get lag group port list for lag index %d\n", lag_id);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
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
    sai_object_id_t  lag_member_oid;

    lag_member_oid = key->object_id;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_member_oid %d\n",lag_member_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* create SAI LAG object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_LAG, lag_id, &value->oid))) {
        return status;
    }    
    
    return SAI_STATUS_SUCCESS;
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
    sai_object_id_t  lag_member_oid;

    lag_member_oid = key->object_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_member_oid %d\n",lag_member_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    /* create SAI PORT object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_id, &value->oid))) {
        return status;
    }   

    return SAI_STATUS_SUCCESS;
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
    sai_object_id_t     lag_member_oid;

    lag_member_oid = key->object_id;

    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_member_oid %d\n",lag_member_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }    

    value->booldata = false;
    return SAI_STATUS_SUCCESS;
}

/* Disable traffic distribution to this port as part of LAG. [bool] (CREATE_AND_SET) default to FALSE */
static sai_status_t mrvl_sai_lag_member_egress_disable_set_prv(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{    
    uint32_t         port_id, lag_id;
    sai_object_id_t  lag_member_oid;

    lag_member_oid = key->object_id;

    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_member_oid %d\n",lag_member_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }        

    if (value->booldata != false ) {
        MRVL_SAI_LOG_WRN("invalid lag_member_oid %d egress disable attribute %d\n",lag_member_oid,value->booldata);        
    }

    return SAI_STATUS_SUCCESS;
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
    sai_object_id_t     lag_member_oid;

    lag_member_oid = key->object_id;

    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_member_oid %d\n",lag_member_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }   

    value->booldata = false;
    return SAI_STATUS_SUCCESS;
}

/* Disable traffic collection from this port as part of LAG. [bool] (CREATE_AND_SET) default to FALSE. */
static sai_status_t mrvl_sai_lag_member_ingress_disable_set_prv(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{    
    uint32_t         port_id, lag_id;
    sai_object_id_t  lag_member_oid;

    lag_member_oid = key->object_id;

    /* to check data is valid */
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_member_oid %d\n",lag_member_oid);
        return SAI_STATUS_INVALID_PARAMETER;
    }   

    if (value->booldata != false ) {
        MRVL_SAI_LOG_WRN("invalid lag_member_oid %d egress disable attribute %d\n",lag_member_oid,value->booldata);        
    }

    return SAI_STATUS_SUCCESS;
}

/*
    \brief Create LAG Member
    \param[out] lag_member_id LAG Member id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_create_lag_member(
    _Out_ sai_object_id_t          *lag_member_oid,
    _In_ uint32_t                   attr_count,
    _In_ const sai_attribute_t      *attr_list)
{
    sai_status_t                 status;
    uint32_t                     port_id, lag_id, changed;
    const sai_attribute_value_t  *attr_lag_id, *attr_port_id, *attr_egress_disable, *attr_ingress_disable;
    sai_object_id_t              lag_oid, port_oid;
    uint32_t                     index;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    
    MRVL_SAI_LOG_ENTER(); 

    if (NULL == lag_member_oid) {
        MRVL_SAI_LOG_ERR("NULL lag member id param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_lag_member_attribs, mrvl_sai_lag_member_vendor_attribs,
                                                 SAI_COMMON_API_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        return status;
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
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE, &attr_egress_disable, &index));

    /* get LAG member ingress mode. Default - ingress enabled */    
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE, &attr_ingress_disable, &index));

    lag_oid  = attr_lag_id->oid;
    port_oid = attr_port_id->oid;

    /* Get port & LAG numbers */
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid lag_id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_oid, SAI_OBJECT_TYPE_PORT, &port_id)) {
        MRVL_SAI_LOG_ERR("invalid port_id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* Add port as a member to lag */
    status = mrvl_sai_utl_update_l2_lag_group_bucket(lag_id, port_oid, MRVL_SAI_UTL_ADD, &changed);
    if (SAI_STATUS_SUCCESS != status) { /* lag group was deleted */
        mrvl_sai_lag_group_table[lag_id].group_member_counter = 0;
        mrvl_sai_lag_group_table[lag_id].used = false;
    }else {
        mrvl_sai_lag_group_table[lag_id].group_member_counter += changed;
    }                                                     
    
    /* create SAI LAG member object */       
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_LAG_MEMBER, port_id, lag_id, lag_member_oid))) {
        MRVL_SAI_API_RETURN(status);
    }

    lag_member_key_to_str(*lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
    \brief Remove LAG Member
    \param[in] lag_member_id LAG Member id
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_remove_lag_member(
    _In_ sai_object_id_t    lag_member_oid
)
{
    sai_status_t        status;
    uint32_t            port_id, lag_id, changed;      
    char                key_str[MAX_KEY_STR_LEN];  
    sai_object_id_t     port_oid;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(lag_member_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &port_id, &lag_id)) {
        MRVL_SAI_LOG_ERR("invalid port_id %d lag_id %d\n",port_id,lag_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* create SAI PORT object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_id, &port_oid))) {
        MRVL_SAI_API_RETURN(status);
    }

    /* Remove port from the LAG */
    status = mrvl_sai_utl_update_l2_lag_group_bucket(lag_id, port_oid, MRVL_SAI_UTL_DEL, &changed);
    if (SAI_STATUS_SUCCESS != status) { /* lag group was deleted */
        mrvl_sai_lag_group_table[lag_id].group_member_counter = 0;
        mrvl_sai_lag_group_table[lag_id].used = false;
    }else {
        mrvl_sai_lag_group_table[lag_id].group_member_counter -= changed;
    }      
        
    lag_member_key_to_str(lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Removed %s\n", key_str);
      
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/*
    \brief Set LAG Member Attribute
    \param[in] lag_member_id LAG Member id
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_set_lag_member_attribute(
    _In_ sai_object_id_t        lag_member_oid, 
    _In_ const sai_attribute_t  *attr)
{
    const sai_object_key_t key = { .object_id = lag_member_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    lag_member_key_to_str(lag_member_oid, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_lag_member_attribs, mrvl_sai_lag_member_vendor_attribs, attr);

    lag_member_key_to_str(lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Set %s\n", key_str);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/*
    \brief Get LAG Member Attribute
    \param[in] lag_member_id LAG Member id
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_get_lag_member_attribute(
    _In_ sai_object_id_t     lag_member_oid,
    _In_ uint32_t            attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = lag_member_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    lag_member_key_to_str(lag_member_oid, key_str);
    status =  mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_lag_member_attribs, mrvl_sai_lag_member_vendor_attribs,
                                        attr_count, attr_list);

    lag_member_key_to_str(lag_member_oid, key_str);
    MRVL_SAI_LOG_NTC("Get %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}


/*
    \brief Create LAG
    \param[out] lag_id LAG id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_create_lag( 
    _Out_ sai_object_id_t       *lag_oid,
    _In_ uint32_t               attr_count,
    _In_ const sai_attribute_t  *attr_list)
{
    sai_status_t                 status;
    uint32_t                     lag_idx, group, index;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    const sai_attribute_value_t *attr_ing_acl_list = NULL;
    const sai_attribute_value_t *attr_egr_acl_list = NULL;   

    MRVL_SAI_LOG_ENTER();

    if (NULL == lag_oid) {
        MRVL_SAI_LOG_ERR("NULL lag id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_lag_attribs, mrvl_sai_lag_vendor_attribs,
                                                 SAI_COMMON_API_CREATE);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* lag attributes SAI_LAG_ATTR_INGRESS_ACL_LIST and SAI_LAG_ATTR_EGRESS_ACL_LIST are not supported */
    assert(SAI_STATUS_SUCCESS !=
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_INGRESS_ACL_LIST, &attr_ing_acl_list, &index));
    assert(SAI_STATUS_SUCCESS !=
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_EGRESS_ACL_LIST, &attr_egr_acl_list, &index));

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);    
    MRVL_SAI_LOG_NTC("Create lag, %s\n", list_str);    

    /* find new lag id - LAG '0' is not valid in CPSS. LAG group range 1..SAI_LAG_MAX_GROUPS_CNS */
    for (lag_idx = 1; lag_idx < SAI_LAG_MAX_GROUPS_CNS; lag_idx++) {
        if (mrvl_sai_lag_group_table[lag_idx].used == false){
            break;
        }
    }
    if (lag_idx >= SAI_LAG_MAX_GROUPS_CNS) { 
        MRVL_SAI_LOG_ERR("Failed to allocate LAG id in SAI DB\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }

    /* create SAI LAG object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_LAG, lag_idx, lag_oid))) {
        MRVL_SAI_API_RETURN(status);
    }
  
    status = mrvl_sai_utl_create_l2_lag_group(lag_idx, &group);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't create lag group %d\n", lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
        
    mrvl_sai_lag_group_table[lag_idx].used = true;
    mrvl_sai_lag_group_table[lag_idx].group_member_counter = 0; /* empty lag group */
    lag_key_to_str(*lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);    
}

/*
    \brief Remove LAG
    \param[in] lag_id LAG id
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_remove_lag(
    _In_ sai_object_id_t    lag_oid
)
{
    sai_status_t        status;    
    uint32_t            lag_idx, port_idx, portList_count, i;       
    sai_object_id_t     port_oid;
    sai_object_id_t     lag_member_oid;
    char                key_str[MAX_KEY_STR_LEN];
    sai_object_list_t   port_objlist;
    sai_object_id_t     port_list[SAI_MAX_NUM_OF_PORTS];

    MRVL_SAI_LOG_ENTER();        

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_idx)) {
        MRVL_SAI_LOG_ERR("invalid lag_idx %d\n",lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    if (mrvl_sai_lag_group_table[lag_idx].used != true)
    {
        MRVL_SAI_LOG_ERR("Lag index %d not used\n", lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
    }

    portList_count = mrvl_sai_lag_group_table[lag_idx].group_member_counter;
    port_objlist.count = portList_count;
    port_objlist.list = port_list;

    if (portList_count > 0) {        
        /* create the port list from members of the LAG */  
        status = mrvl_sai_utl_get_l2_lag_group_bucket_list(lag_idx, &port_objlist);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get lag group port list for lag index %d\n", lag_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }    

        /*  Check if the LAG is empty */
        for (i = 0; i <portList_count; i++) {
            port_oid = port_objlist.list[i];
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_object_type(port_oid, SAI_OBJECT_TYPE_PORT)) {
                MRVL_SAI_LOG_ERR("invalid port oid in list %d at index %d\n",port_oid,i);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_oid, SAI_OBJECT_TYPE_PORT, &port_idx)) {
                MRVL_SAI_LOG_ERR("invalid port_idx %d\n",port_idx);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }        
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_LAG_MEMBER, port_idx, lag_idx, &lag_member_oid)) {
                MRVL_SAI_LOG_ERR("invalid lag_member_oid %d from port_id %d lag_id %d\n",lag_member_oid, port_idx, lag_idx);
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
    
    mrvl_sai_lag_group_table[lag_idx].used = false;
    mrvl_sai_lag_group_table[lag_idx].group_member_counter = 0;

    lag_key_to_str(lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Removed %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
    \brief Set LAG Attribute
    \param[in] lag_id LAG id
    \param[in] attr Structure containing ID and value to be set
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_set_lag_attribute(
        _In_ sai_object_id_t        lag_oid, 
        _In_ const sai_attribute_t  *attr)
{
    const sai_object_key_t key = { .object_id = lag_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;    

    MRVL_SAI_LOG_ENTER(); 
    
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_lag_attribs, mrvl_sai_lag_vendor_attribs, attr);

    lag_key_to_str(lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Set %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/*
    \brief Get LAG Attribute
    \param[in] lag_id LAG id
    \param[in] attr_count Number of attributes to be get
    \param[in,out] attr_list List of structures containing ID and value to be get
    \return Success: SAI_STATUS_SUCCESS
            Failure: Failure status code on error
*/
sai_status_t mrvl_sai_get_lag_attribute(
    _In_ sai_object_id_t        lag_oid,
    _In_ uint32_t               attr_count,
    _Inout_ sai_attribute_t     *attr_list
)
{
    const sai_object_key_t key = { .object_id = lag_oid };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;    

    MRVL_SAI_LOG_ENTER();
    
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_lag_attribs, mrvl_sai_lag_vendor_attribs, attr_count, attr_list);

    lag_key_to_str(lag_oid, key_str);
    MRVL_SAI_LOG_NTC("Get %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

const sai_lag_api_t lag_api = {
    mrvl_sai_create_lag,
    mrvl_sai_remove_lag,
    mrvl_sai_set_lag_attribute,
    mrvl_sai_get_lag_attribute,
    mrvl_sai_create_lag_member,
    mrvl_sai_remove_lag_member,
    mrvl_sai_set_lag_member_attribute,
    mrvl_sai_get_lag_member_attribute
};
