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
#define __MODULE__ SAI_HASH


static void hash_id_key_to_str(_In_ sai_object_id_t sai_hash_id, _Out_ char *key_str);

static sai_status_t mrvl_hash_native_field_list_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg);
static sai_status_t mrvl_hash_native_field_list_set(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg);
static sai_status_t mrvl_hash_udf_group_list_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg);
static sai_status_t mrvl_hash_udf_group_list_set(_In_ const sai_object_key_t      *key,
                                                 _In_ const sai_attribute_value_t *value,
                                                 void                             *arg);

static const sai_attribute_entry_t mrvl_sai_hash_attribs[] = {
    { SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST, false, false, true, true,
      "Hash native fields", SAI_ATTR_VALUE_TYPE_INT32_LIST},
    { SAI_HASH_ATTR_UDF_GROUP_LIST, false, false, true, true,
      "Hash UDF group", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_hash_vendor_attribs[] = {
    { SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_hash_native_field_list_get, NULL,
      mrvl_hash_native_field_list_set, NULL },
    { SAI_HASH_ATTR_UDF_GROUP_LIST,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_hash_udf_group_list_get, NULL,
      mrvl_hash_udf_group_list_set, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static sai_status_t mrvl_hash_native_field_list_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    uint32_t hash_data;
    sai_object_id_t hash_id   = key->key.object_id;
    sai_status_t     status;
    int32_t list_data;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(hash_id, SAI_OBJECT_TYPE_HASH, &hash_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object hash\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    list_data = (int32_t)hash_data;
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_s32list(&list_data, 1, &value->s32list)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill s32list\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_hash_native_field_list_set(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static sai_status_t mrvl_hash_udf_group_list_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg)
{
    sai_object_id_t data_obj;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_UDF_GROUP, 1, &data_obj)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object UDF group\n");
        status = mrvl_sai_utl_fill_objlist(&data_obj, 0, &value->objlist);
        MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_objlist(&data_obj, 1, &value->objlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill objlist\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_hash_udf_group_list_set(_In_ const sai_object_key_t      *key,
                                                 _In_ const sai_attribute_value_t *value,
                                                 void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void hash_id_key_to_str(_In_ sai_object_id_t sai_hash_id, _Out_ char *key_str)
{
    uint32_t     hash_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_hash_id, SAI_OBJECT_TYPE_HASH, &hash_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid hash id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "hash id %u", hash_id);
    }
}

/**
 * @brief Create hash
 *
 * @param[out] hash_id Hash id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
static sai_status_t mrvl_sai_create_hash(
        _Out_ sai_object_id_t *hash_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == hash_id) {
        MRVL_SAI_LOG_ERR("NULL hash id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_hash_attribs, mrvl_sai_hash_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_hash_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    /* create SAI HASH object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HASH, 1, hash_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove hash
 *
 * @param[in] hash_id Hash id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
static sai_status_t mrvl_sai_remove_hash(
        _In_ sai_object_id_t hash_id)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set hash attribute
 *
 * @param[in] hash_id Hash id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
static sai_status_t mrvl_sai_set_hash_attribute(
        _In_ sai_object_id_t hash_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = hash_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    hash_id_key_to_str(hash_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_hash_attribs, mrvl_sai_hash_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get hash attribute value
 *
 * @param[in] hash_id Hash id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
static sai_status_t mrvl_sai_get_hash_attribute(
        _In_ sai_object_id_t hash_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = hash_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == hash_id) {
        MRVL_SAI_LOG_ERR("NULL hash id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    hash_id_key_to_str(hash_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_hash_attribs, mrvl_sai_hash_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);    
}

/**
 * @brief Hash methods, retrieved via sai_api_query()
 */
const sai_hash_api_t hash_api = {
    mrvl_sai_create_hash,
    mrvl_sai_remove_hash,
    mrvl_sai_set_hash_attribute,
    mrvl_sai_get_hash_attribute

};

