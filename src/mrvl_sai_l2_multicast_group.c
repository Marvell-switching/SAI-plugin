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
#define __MODULE__ SAI_L2_MC_GROUP


static void l2mc_group_id_key_to_str(_In_ sai_object_id_t sai_l2mc_group_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_l2mc_group_output_count_get(_In_ const sai_object_key_t   *key,
                                                         _Inout_ sai_attribute_value_t *value,
                                                         _In_ uint32_t                  attr_index,
                                                         _Inout_ vendor_cache_t        *cache,
                                                         void                          *arg);

static sai_status_t mrvl_sai_l2mc_group_member_list_get(_In_ const sai_object_key_t   *key,
                                                        _Inout_ sai_attribute_value_t *value,
                                                        _In_ uint32_t                  attr_index,
                                                        _Inout_ vendor_cache_t        *cache,
                                                        void                          *arg);
static sai_status_t mrvl_sai_l2mc_group_member_list_set(_In_ const sai_object_key_t      *key,
                                                        _In_ const sai_attribute_value_t *value,
                                                        void                             *arg);

static const sai_attribute_entry_t mrvl_sai_l2mc_group_attribs[] = {
    { SAI_L2MC_GROUP_ATTR_L2MC_OUTPUT_COUNT, false, false, false, true,
      "Number of L2MC output in the group", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_L2MC_GROUP_ATTR_L2MC_MEMBER_LIST, false, false, false, true,
      "L2MC member list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_l2mc_group_vendor_attribs[] = {
    { SAI_L2MC_GROUP_ATTR_L2MC_OUTPUT_COUNT,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_l2mc_group_output_count_get, NULL,
      NULL, NULL },
    { SAI_L2MC_GROUP_ATTR_L2MC_MEMBER_LIST,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_l2mc_group_member_list_get, NULL,
      mrvl_sai_l2mc_group_member_list_set, NULL },

    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static void l2mc_group_member_id_key_to_str(_In_ sai_object_id_t sai_l2mc_group_member_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_l2mc_group_member_l2mc_group_id_get(_In_ const sai_object_key_t   *key,
                                                                 _Inout_ sai_attribute_value_t *value,
                                                                 _In_ uint32_t                  attr_index,
                                                                 _Inout_ vendor_cache_t        *cache,
                                                                 void                          *arg);

static sai_status_t mrvl_sai_l2mc_group_member_l2mc_output_id_get(_In_ const sai_object_key_t   *key,
                                                                  _Inout_ sai_attribute_value_t *value,
                                                                  _In_ uint32_t                  attr_index,
                                                                  _Inout_ vendor_cache_t        *cache,
                                                                  void                          *arg);
static sai_status_t mrvl_sai_l2mc_group_member_l2mc_output_id_set(_In_ const sai_object_key_t      *key,
                                                                  _In_ const sai_attribute_value_t *value,
                                                                  void                             *arg);
    
static const sai_attribute_entry_t mrvl_sai_l2mc_group_member_attribs[] = {
    { SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID, false, false, false, true,
      "L2MC group id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID, false, false, false, true,
      "L2MC output id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_l2mc_group_member_vendor_attribs[] = {
    { SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_l2mc_group_member_l2mc_group_id_get, NULL,
      NULL, NULL },
    { SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_l2mc_group_member_l2mc_output_id_get, NULL,
      mrvl_sai_l2mc_group_member_l2mc_output_id_set, NULL },

    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static sai_status_t mrvl_sai_l2mc_group_output_count_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    sai_object_id_t l2mc_group_id   = key->key.object_id;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(l2mc_group_id, SAI_OBJECT_TYPE_L2MC_GROUP, &value->u32)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object L2 MC group\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_l2mc_group_member_list_get(_In_ const sai_object_key_t   *key,
                                                                 _Inout_ sai_attribute_value_t *value,
                                                                 _In_ uint32_t                  attr_index,
                                                                 _Inout_ vendor_cache_t        *cache,
                                                                 void                          *arg)
{
    sai_object_id_t data_obj;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER, 1, &data_obj)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object L2 MC group member\n");
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

static sai_status_t mrvl_sai_l2mc_group_member_list_set(_In_ const sai_object_key_t      *key,
                                                 _In_ const sai_attribute_value_t *value,
                                                 void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static sai_status_t mrvl_sai_l2mc_group_member_l2mc_group_id_get(_In_ const sai_object_key_t   *key,
                                                                 _Inout_ sai_attribute_value_t *value,
                                                                 _In_ uint32_t                  attr_index,
                                                                 _Inout_ vendor_cache_t        *cache,
                                                                 void                          *arg)
{
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_L2MC_GROUP, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object L2 MC group\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_l2mc_group_member_l2mc_output_id_get(_In_ const sai_object_key_t   *key,
                                                                  _Inout_ sai_attribute_value_t *value,
                                                                  _In_ uint32_t                  attr_index,
                                                                  _Inout_ vendor_cache_t        *cache,
                                                                  void                          *arg)
{
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_l2mc_group_member_l2mc_output_id_set(_In_ const sai_object_key_t      *key,
                                                                  _In_ const sai_attribute_value_t *value,
                                                                  void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void l2mc_group_id_key_to_str(_In_ sai_object_id_t sai_l2mc_group_id, _Out_ char *key_str)
{
    uint32_t     l2mc_group_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_l2mc_group_id, SAI_OBJECT_TYPE_L2MC_GROUP, &l2mc_group_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid L2 MC group id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "L2MC group id %u", l2mc_group_id);
    }
}

static void l2mc_group_member_id_key_to_str(_In_ sai_object_id_t sai_l2mc_group_member_id, _Out_ char *key_str)
{
    uint32_t     l2mc_group_member_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_l2mc_group_member_id, SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER, &l2mc_group_member_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid L2 MC group member id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "L2MC group member id %u", l2mc_group_member_id);
    }
}

/**
 * @brief Create L2MC group
 *
 * @param[out] l2mc_group_id L2MC group id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_l2mc_group(
        _Out_ sai_object_id_t *l2mc_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == l2mc_group_id) {
        MRVL_SAI_LOG_ERR("NULL L2 MC group id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_l2mc_group_attribs, mrvl_sai_l2mc_group_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_l2mc_group_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    /* create SAI L2MC GROUP object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_L2MC_GROUP, 1, l2mc_group_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove L2MC group
 *
 * @param[in] l2mc_group_id L2MC group id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_l2mc_group(
        _In_ sai_object_id_t l2mc_group_id)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set L2MC Group attribute
 *
 * @param[in] sai_object_id_t L2MC group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_set_l2mc_group_attribute(
        _In_ sai_object_id_t l2mc_group_id,
        _In_ const sai_attribute_t *attr)
{
	const sai_object_key_t key = { .key.object_id = l2mc_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    l2mc_group_member_id_key_to_str(l2mc_group_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_l2mc_group_attribs, mrvl_sai_l2mc_group_vendor_attribs, attr);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get L2MC Group attribute
 *
 * @param[in] sai_object_id_t L2MC group id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_get_l2mc_group_attribute(
        _In_ sai_object_id_t l2mc_group_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.object_id = l2mc_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == l2mc_group_id) {
        MRVL_SAI_LOG_ERR("NULL L2 MC group id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    l2mc_group_id_key_to_str(l2mc_group_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_l2mc_group_attribs, mrvl_sai_l2mc_group_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status); 
}

/**
 * @brief Create L2MC group member
 *
 * @param[out] l2mc_group_member_id L2MC group member id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_l2mc_group_member(
        _Out_ sai_object_id_t* l2mc_group_member_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == l2mc_group_member_id) {
        MRVL_SAI_LOG_ERR("NULL L2 MC group member id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_l2mc_group_member_attribs, mrvl_sai_l2mc_group_member_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_l2mc_group_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    /* create SAI L2MC GROUP MEMBER object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER, 1, l2mc_group_member_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove L2MC group member
 *
 * @param[in] l2mc_group_member_id L2MC group member id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_l2mc_group_member(
        _In_ sai_object_id_t l2mc_group_member_id)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set L2MC Group attribute
 *
 * @param[in] sai_object_id_t L2MC group member id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_set_l2mc_group_member_attribute(
        _In_ sai_object_id_t l2mc_group_member_id,
        _In_ const sai_attribute_t *attr)
{
	const sai_object_key_t key = { .key.object_id = l2mc_group_member_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    l2mc_group_member_id_key_to_str(l2mc_group_member_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_l2mc_group_member_attribs, mrvl_sai_l2mc_group_member_vendor_attribs, attr);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get L2MC Group attribute
 *
 * @param[in] sai_object_id_t L2MC group member id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_get_l2mc_group_member_attribute(
        _In_ sai_object_id_t l2mc_group_member_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.object_id = l2mc_group_member_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == l2mc_group_member_id) {
        MRVL_SAI_LOG_ERR("NULL L2 MC group member id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    l2mc_group_member_id_key_to_str(l2mc_group_member_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_l2mc_group_member_attribs, mrvl_sai_l2mc_group_member_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);    
}

/**
 * @brief L2MC group methods table retrieved with sai_api_query()
 */
const sai_l2mc_group_api_t l2mc_group_api=
{
    mrvl_create_l2mc_group,
    mrvl_remove_l2mc_group,
    mrvl_set_l2mc_group_attribute,
    mrvl_get_l2mc_group_attribute,
    mrvl_create_l2mc_group_member,
    mrvl_remove_l2mc_group_member,
    mrvl_set_l2mc_group_member_attribute,
    mrvl_get_l2mc_group_member_attribute
};
