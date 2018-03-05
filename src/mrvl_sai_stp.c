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
#define __MODULE__ SAI_STP


static void stp_id_key_to_str(_In_ sai_object_id_t sai_stp_id, _Out_ char *key_str);
static void stp_port_id_key_to_str(_In_ sai_object_id_t sai_stp_port_id, _Out_ char *key_str);


static sai_status_t mrvl_stp_vlanlist_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          _In_ void                     *arg);
static sai_status_t mrvl_stp_ports_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       _In_ void                     *arg);
static sai_status_t mrvl_stp_bridge_id_get(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           _In_ void                     *arg);

static const sai_attribute_entry_t stp_attribs[] = {
    { SAI_STP_ATTR_VLAN_LIST, false, false, false, true,
      "VLANs attached to STP", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_STP_ATTR_PORT_LIST, false, false, false, true,
      "Port member list", SAI_ATTR_VAL_TYPE_OBJLIST },
    { SAI_STP_ATTR_BRIDGE_ID, false, false, false, true,
      "Bridge attached to STP", SAI_ATTR_VAL_TYPE_OID },

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, true,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t stp_vendor_attribs[] = {
    { SAI_STP_ATTR_VLAN_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_stp_vlanlist_get, NULL,
      NULL, NULL },
    { SAI_STP_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_stp_ports_get, NULL,
      NULL, NULL },
    { SAI_STP_ATTR_BRIDGE_ID,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_stp_bridge_id_get, NULL,
      NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static sai_status_t mrvl_stp_port_stp_id_get(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             _In_ void                     *arg);
static sai_status_t mrvl_stp_port_port_id_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              _In_ void                     *arg);
static sai_status_t mrvl_stp_port_state_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            _In_ void                     *arg);
static sai_status_t mrvl_stp_port_state_set(_In_ const sai_object_key_t      *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void                             *arg);

static const sai_attribute_entry_t stp_port_attribs[] = {
    { SAI_STP_PORT_ATTR_STP, true, false, false, true,
      "STP id", SAI_ATTR_VAL_TYPE_OID },
    { SAI_STP_PORT_ATTR_BRIDGE_PORT, true, false, false, true,
      "Bridge port id", SAI_ATTR_VAL_TYPE_OID },
    { SAI_STP_PORT_ATTR_STATE, true, false, true, true,
      "STP port state", SAI_ATTR_VAL_TYPE_S32 },

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, true,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t stp_port_vendor_attribs[] = {
    { SAI_STP_PORT_ATTR_STP,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_stp_port_stp_id_get, NULL,
      NULL, NULL },
    { SAI_STP_PORT_ATTR_BRIDGE_PORT,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_stp_port_port_id_get, NULL,
      NULL, NULL },
    { SAI_STP_PORT_ATTR_STATE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_stp_port_state_get, NULL,
      mrvl_stp_port_state_set, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static sai_status_t mrvl_stp_vlanlist_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          _In_ void                     *arg)
{
    sai_object_id_t data_obj;
    sai_status_t     status;
    sai_vlan_id_t   *vlan_data;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, 1, &data_obj)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object STP\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    vlan_data = (sai_vlan_id_t*)data_obj;
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_vlanlist(vlan_data, 1, &value->vlanlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill vlanlist\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_stp_ports_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       _In_ void                     *arg)
{
    const sai_object_id_t sai_stp_id  = key->key.object_id;
    sai_object_id_t stp_port_id;
    uint32_t        stp_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    /*if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(sai_stp_id, SAI_OBJECT_TYPE_STP, &stp_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert STP port object to type\n");
        MRVL_SAI_API_RETURN(status);
    }*/

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP_PORT, 1, &stp_port_id)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert STP port object to type\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_objlist(&stp_port_id, 1, &value->objlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill objlist\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_stp_bridge_id_get(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           _In_ void                     *arg)
{
    const sai_object_id_t sai_stp_id  = key->key.object_id;
    uint32_t        stp_idx;
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();

    /*if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(sai_stp_id, SAI_OBJECT_TYPE_STP, &stp_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert STP object to type\n");
        MRVL_SAI_API_RETURN(status);
    }*/
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_stp_port_stp_id_get(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             _In_ void                     *arg)
{
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object STP\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_stp_port_port_id_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              _In_ void                     *arg)
{
    const sai_object_id_t sai_stp_port_id = key->key.object_id;
    uint32_t    stp_port_idx;
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();

    /*if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(sai_stp_port_id, SAI_OBJECT_TYPE_STP_PORT, &stp_port_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert STP object to type\n");
        MRVL_SAI_API_RETURN(status);
    }*/
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object STP port\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_stp_port_state_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            _In_ void                     *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_STP_PORT_STATE_FORWARDING;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_stp_port_state_set(_In_ const sai_object_key_t      *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void stp_id_key_to_str(_In_ sai_object_id_t sai_stp_id, _Out_ char *key_str)
{
    uint32_t     stp_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_stp_id, SAI_OBJECT_TYPE_STP, &stp_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid STP id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "STP id %u", stp_id);
    }
}

static void stp_port_id_key_to_str(_In_ sai_object_id_t sai_stp_port_id, _Out_ char *key_str)
{
    uint32_t     stp_port_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_stp_port_id, SAI_OBJECT_TYPE_STP_PORT, &stp_port_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid STP port id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "STP port id %u", stp_port_id);
    }
}

/**
 * @brief Create STP instance with default port state as blocking.
 *
 * @param[out] stp_id STP instance id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 *
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
static sai_status_t mrvl_sai_create_stp(
        _Out_ sai_object_id_t *stp_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Remove STP instance.
 *
 * @param[in] stp_id STP instance id
 *
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 * error code is returned.
 */
static sai_status_t mrvl_sai_remove_stp(
        _In_ sai_object_id_t stp_id)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set the attribute of STP instance.
 *
 * @param[in] stp_id STP instance id
 * @param[in] attr Attribute value
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *    error code is returned.
 */
static sai_status_t mrvl_sai_set_stp_attribute(
        _In_ sai_object_id_t stp_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = stp_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    stp_port_id_key_to_str(stp_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, stp_attribs, stp_vendor_attribs, attr);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get the attribute of STP instance.
 *
 * @param[in] stp_id STP instance id
 * @param[in] attr_count Number of the attribute
 * @param[in] attr_list Attribute value
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *    error code is returned.
 */
static sai_status_t mrvl_sai_get_stp_attribute(
        _In_ sai_object_id_t stp_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = stp_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == stp_id) {
        MRVL_SAI_LOG_ERR("NULL STP id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    stp_id_key_to_str(stp_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for STP %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, stp_attribs, stp_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, stp_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Create STP port object
 *
 * @param[out] stp_port_id STP port id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Value of attributes
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *    error code is returned.
 */
static sai_status_t mrvl_sai_create_stp_port(
        _Out_ sai_object_id_t *stp_port_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}


/**
 * @brief Remove STP port object.
 *
 * @param[in] stp_port_id STP object id
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *    error code is returned.
 */
static sai_status_t mrvl_sai_remove_stp_port(
        _In_ sai_object_id_t stp_port_id)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set the attribute of STP port.
 *
 * @param[in] stp_port_id STP port id
 * @param[in] attr Attribute value
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *    error code is returned.
 */
static sai_status_t mrvl_sai_set_stp_port_attribute(
        _In_ sai_object_id_t stp_port_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = stp_port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    stp_port_id_key_to_str(stp_port_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, stp_port_attribs, stp_port_vendor_attribs, attr);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}


/**
 * @brief Get the attribute of STP port.
 *
 * @param[in] stp_port_id STP port id
 * @param[in] attr_count Number of the attribute
 * @param[in] attr_list Attribute value
 * @return #SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *    error code is returned.
 */
static sai_status_t mrvl_sai_get_stp_port_attribute(
        _In_ sai_object_id_t stp_port_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = stp_port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == stp_port_id) {
        MRVL_SAI_LOG_ERR("NULL STP port id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    stp_port_id_key_to_str(stp_port_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for STP port %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, stp_port_attribs, stp_port_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, stp_port_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Bulk stp ports creation.
 *
 * @param[in] switch_id SAI Switch object id
 * @param[in] object_count Number of objects to create
 * @param[in] attr_count List of attr_count. Caller passes the number
 *    of attribute for each object to create.
 * @param[in] attr_list List of attributes for every object.
 * @param[in] mode Bulk operation error handling mode.
 *
 * @param[out] object_id List of object ids returned
 * @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
 *
 * @return #SAI_STATUS_SUCCESS on success when all objects are created or #SAI_STATUS_FAILURE when
 * any of the objects fails to create. When there is failure, Caller is expected to go through the
 * list of returned statuses to find out which fails and which succeeds.
 */
static sai_status_t mrvl_sai_create_stp_ports(_In_ sai_object_id_t switch_id,
                                          _In_ uint32_t object_count,
                                          _In_ const uint32_t *attr_count,
                                          _In_ const sai_attribute_t **attr_list,
                                          _In_ sai_bulk_op_type_t type,
                                          _Out_ sai_object_id_t *object_id,
                                          _Out_ sai_status_t *object_statuses)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Bulk stp ports removal.
 *
 * @param[in] object_count Number of objects to create
 * @param[in] object_id List of object ids
 * @param[in] type bulk operation type.
 * @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
 *
 * @return #SAI_STATUS_SUCCESS on success when all objects are removed or #SAI_STATUS_FAILURE when
 * any of the objects fails to remove. When there is failure, Caller is expected to go through the
 * list of returned statuses to find out which fails and which succeeds.
 */
static sai_status_t mrvl_sai_remove_stp_ports(_In_ uint32_t object_count,
                                          _In_ const sai_object_id_t *object_id,
                                          _In_ sai_bulk_op_type_t type,
                                          _Out_ sai_status_t *object_statuses)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief STP method table retrieved with sai_api_query()
 */
const sai_stp_api_t stp_api = {
    mrvl_sai_create_stp,
    mrvl_sai_remove_stp,
    mrvl_sai_set_stp_attribute,
    mrvl_sai_get_stp_attribute,
    mrvl_sai_create_stp_port,
    mrvl_sai_remove_stp_port,
    mrvl_sai_set_stp_port_attribute,
    mrvl_sai_get_stp_port_attribute,
    mrvl_sai_create_stp_ports,
    mrvl_sai_remove_stp_ports,
};
