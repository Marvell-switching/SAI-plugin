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
#define __MODULE__ SAI_UDF


static void udf_id_key_to_str(_In_ sai_object_id_t sai_udf_id, _Out_ char *key_str);
static void udf_match_id_key_to_str(_In_ sai_object_id_t sai_udf_match_id, _Out_ char *key_str);
static void udf_group_id_key_to_str(_In_ sai_object_id_t sai_udf_group_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_udf_group_list_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg);
static sai_status_t mrvl_sai_udf_group_list_set(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg);

static const sai_attribute_entry_t mrvl_sai_udf_attribs[] = {
    { SAI_UDF_ATTR_MATCH_ID, false, false, true, true,
      "UDF match ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_UDF_ATTR_GROUP_ID, false, false, true, true,
      "UDF group ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_UDF_ATTR_BASE, false, false, true, true,
      "UDF base", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_UDF_ATTR_OFFSET, false, false, true, true,
      "UDF byte offset", SAI_ATTR_VAL_TYPE_U16 },
    { SAI_UDF_ATTR_HASH_MASK, false, false, true, true,
      "UDF hash mask", SAI_ATTR_VAL_TYPE_U8LIST },
    
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_udf_vendor_attribs[] = {
    { SAI_UDF_ATTR_MATCH_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_ATTR_GROUP_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_ATTR_BASE,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_ATTR_OFFSET,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_ATTR_HASH_MASK,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static const sai_attribute_entry_t mrvl_sai_udf_match_attribs[] = {
    { SAI_UDF_MATCH_ATTR_L2_TYPE, false, false, true, true,
      "UDF L2 match rule", SAI_ATTR_VAL_TYPE_U16 },
    { SAI_UDF_MATCH_ATTR_L3_TYPE, false, false, true, true,
      "UDF L3 match rule", SAI_ATTR_VAL_TYPE_U8 },
    { SAI_UDF_MATCH_ATTR_GRE_TYPE, false, false, true, true,
      "UDF GRE match rule", SAI_ATTR_VAL_TYPE_U16 },
    { SAI_UDF_MATCH_ATTR_PRIORITY, false, false, true, true,
      "UDF match priority", SAI_ATTR_VAL_TYPE_U8 },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_udf_match_vendor_attribs[] = {
    { SAI_UDF_MATCH_ATTR_L2_TYPE,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_MATCH_ATTR_L3_TYPE,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_MATCH_ATTR_GRE_TYPE,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_MATCH_ATTR_PRIORITY,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};


static const sai_attribute_entry_t mrvl_sai_udf_group_attribs[] = {
    { SAI_UDF_GROUP_ATTR_UDF_LIST, false, false, true, true,
      "UDF list", SAI_ATTR_VAL_TYPE_OBJLIST },
    { SAI_UDF_GROUP_ATTR_TYPE, false, false, true, true,
      "UDF group type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_UDF_GROUP_ATTR_LENGTH, false, false, true, true,
      "UDF byte length", SAI_ATTR_VAL_TYPE_U16 },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_udf_group_vendor_attribs[] = {
    { SAI_UDF_GROUP_ATTR_UDF_LIST,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_udf_group_list_get, NULL,
      mrvl_sai_udf_group_list_set, NULL },
    { SAI_UDF_GROUP_ATTR_TYPE,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_UDF_GROUP_ATTR_LENGTH,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};


static sai_status_t mrvl_sai_udf_group_list_get(_In_ const sai_object_key_t   *key,
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

static sai_status_t mrvl_sai_udf_group_list_set(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void udf_id_key_to_str(_In_ sai_object_id_t sai_udf_id, _Out_ char *key_str)
{
    uint32_t     udf_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_udf_id, SAI_OBJECT_TYPE_UDF, &udf_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid udf id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "udf id %u", udf_id);
    }
}

static void udf_match_id_key_to_str(_In_ sai_object_id_t sai_udf_match_id, _Out_ char *key_str)
{
    uint32_t     udf_match_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_udf_match_id, SAI_OBJECT_TYPE_UDF_MATCH, &udf_match_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid udf match id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "udf match id %u", udf_match_id);
    }
}

static void udf_group_id_key_to_str(_In_ sai_object_id_t sai_udf_group_id, _Out_ char *key_str)
{
    uint32_t     udf_group_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_udf_group_id, SAI_OBJECT_TYPE_UDF_GROUP, &udf_group_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid udf group id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "udf group id %u", udf_group_id);
    }
}
#if 0
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
    char                         key_str[MAX_KEY_STR_LEN];
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
#endif
/**
 * Routine Description:
 *    @brief Create UDF
 *
 * Arguments:
 *    @param[out] udf_id - UDF id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 *
 */
sai_status_t mrvl_create_udf(
    _Out_ sai_object_id_t* udf_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * Routine Description:
 *    @brief Remove UDF
 *
 * Arguments:
 *    @param[in] udf_id - UDF id
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_remove_udf(
    _In_ sai_object_id_t udf_id
)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * Routine Description:
 *    @brief Set UDF attribute
 *
 * Arguments:
 *    @param[in] udf_id - UDF id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_set_udf_attribute(
    _In_ sai_object_id_t udf_id,
    _In_ const sai_attribute_t *attr
)
{
	const sai_object_key_t key = { .key.object_id = udf_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    udf_id_key_to_str(udf_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_udf_attribs, mrvl_sai_udf_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * Routine Description:
 *    @brief Get UDF attribute value
 *
 * Arguments:
 *    @param[in] udf_id - UDF id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_get_udf_attribute(
    _In_ sai_object_id_t udf_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
)
{
	const sai_object_key_t key = { .key.object_id = udf_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == udf_id) {
        MRVL_SAI_LOG_ERR("NULL udf id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    udf_id_key_to_str(udf_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_udf_attribs, mrvl_sai_udf_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);    
}

/**
 * Routine Description:
 *    @brief Create UDF match
 *
 * Arguments:
 *    @param[out] udf_match_id - UDF match id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 *
 */
sai_status_t mrvl_create_udf_match(
    _Out_ sai_object_id_t* udf_match_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * Routine Description:
 *    @brief Remove UDF match
 *
 * Arguments:
 *    @param[in] udf_match_id - UDF match id
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_remove_udf_match(
    _In_ sai_object_id_t udf_match_id
)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * Routine Description:
 *    @brief Set UDF match attribute
 *
 * Arguments:
 *    @param[in] udf_match_id - UDF match id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_set_udf_match_attribute(
    _In_ sai_object_id_t udf_match_id,
    _In_ const sai_attribute_t *attr
)
{
	const sai_object_key_t key = { .key.object_id = udf_match_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    udf_match_id_key_to_str(udf_match_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_udf_match_attribs, mrvl_sai_udf_match_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * Routine Description:
 *    @brief Get UDF match attribute value
 *
 * Arguments:
 *    @param[in] udf_match_id - UDF match id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_get_udf_match_attribute(
    _In_ sai_object_id_t udf_match_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
)
{
	const sai_object_key_t key = { .key.object_id = udf_match_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == udf_match_id) {
        MRVL_SAI_LOG_ERR("NULL udf match id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    udf_match_id_key_to_str(udf_match_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_udf_match_attribs, mrvl_sai_udf_match_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);    
}

/**
 * Routine Description:
 *    @brief Create UDF group
 *
 * Arguments:
 *    @param[out] udf_group_id - UDF group id
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 *
 */
sai_status_t mrvl_create_udf_group(
    _Out_ sai_object_id_t* udf_group_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * Routine Description:
 *    @brief Remove UDF group
 *
 * Arguments:
 *    @param[in] udf_group_id - UDF group id
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_remove_udf_group(
    _In_ sai_object_id_t udf_group_id
)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * Routine Description:
 *    @brief Set UDF group attribute
 *
 * Arguments:
 *    @param[in] udf_group_id - UDF group id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_set_udf_group_attribute(
    _In_ sai_object_id_t udf_group_id,
    _In_ const sai_attribute_t *attr
)
{
	const sai_object_key_t key = { .key.object_id = udf_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    udf_group_id_key_to_str(udf_group_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_udf_group_attribs, mrvl_sai_udf_group_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * Routine Description:
 *    @brief Get UDF group attribute value
 *
 * Arguments:
 *    @param[in] udf_group_id - UDF group id
 *    @param[in] attr_count - number of attributes
 *    @param[inout] attrs - array of attributes
 *
 * Return Values:
 *    @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *            Failure status code on error
 */
sai_status_t mrvl_get_udf_group_attribute(
    _In_ sai_object_id_t udf_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
)
{
	const sai_object_key_t key = { .key.object_id = udf_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == udf_group_id) {
        MRVL_SAI_LOG_ERR("NULL udf group id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    udf_group_id_key_to_str(udf_group_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_udf_group_attribs, mrvl_sai_udf_group_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);    
}

/**
 *  @brief UDF methods, retrieved via sai_api_query()
 */
const sai_udf_api_t udf_api=
{
    mrvl_create_udf,
    mrvl_remove_udf,
    mrvl_set_udf_attribute,
    mrvl_get_udf_attribute,
    mrvl_create_udf_match,
    mrvl_remove_udf_match,
    mrvl_set_udf_match_attribute,
    mrvl_get_udf_match_attribute,
    mrvl_create_udf_group,
    mrvl_remove_udf_group,
    mrvl_set_udf_group_attribute,
    mrvl_get_udf_group_attribute
} ;


