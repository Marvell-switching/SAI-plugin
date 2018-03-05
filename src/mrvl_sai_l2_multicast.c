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
#define __MODULE__ SAI_L2_MC


static void l2mc_id_key_to_str(_In_ const sai_l2mc_entry_t *sai_l2mc_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_l2mc_entry_packet_action_get(_In_ const sai_object_key_t   *key,
                                                         _Inout_ sai_attribute_value_t *value,
                                                         _In_ uint32_t                  attr_index,
                                                         _Inout_ vendor_cache_t        *cache,
                                                         void                          *arg);

static sai_status_t mrvl_sai_l2mc_entry_output_group_get(_In_ const sai_object_key_t   *key,
                                                        _Inout_ sai_attribute_value_t *value,
                                                        _In_ uint32_t                  attr_index,
                                                        _Inout_ vendor_cache_t        *cache,
                                                        void                          *arg);
static sai_status_t mrvl_sai_l2mc_entry_output_group_set(_In_ const sai_object_key_t      *key,
                                                        _In_ const sai_attribute_value_t *value,
                                                        void                             *arg);

static const sai_attribute_entry_t mrvl_sai_l2mc_attribs[] = {
    { SAI_L2MC_ENTRY_ATTR_PACKET_ACTION, false, false, false, true,
      "L2MC entry type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID, false, false, false, true,
      "L2MC entry output group id", SAI_ATTR_VAL_TYPE_OID },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_l2mc_vendor_attribs[] = {
    { SAI_L2MC_ENTRY_ATTR_PACKET_ACTION,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_l2mc_entry_packet_action_get, NULL,
      NULL, NULL },
    { SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_l2mc_entry_output_group_get, NULL,
      mrvl_sai_l2mc_entry_output_group_set, NULL },

    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static sai_status_t mrvl_sai_l2mc_entry_packet_action_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    sai_object_id_t l2mc_entry_id   = key->key.object_id;
    uint32_t    l2mc_entry_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(l2mc_entry_id, SAI_OBJECT_TYPE_L2MC_ENTRY, &l2mc_entry_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object L2 MC entry\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->s32 = SAI_PACKET_ACTION_FORWARD;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


static sai_status_t mrvl_sai_l2mc_entry_output_group_get(_In_ const sai_object_key_t   *key,
                                                         _Inout_ sai_attribute_value_t *value,
                                                         _In_ uint32_t                  attr_index,
                                                         _Inout_ vendor_cache_t        *cache,
                                                         void                          *arg)
{
    sai_object_id_t l2mc_entry_id   = key->key.object_id;
    uint32_t    l2mc_entry_data;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(l2mc_entry_id, SAI_OBJECT_TYPE_L2MC_ENTRY, &l2mc_entry_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object L2 MC entry\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_L2MC_GROUP, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object L2 MC group member\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_l2mc_entry_output_group_set(_In_ const sai_object_key_t      *key,
                                                         _In_ const sai_attribute_value_t *value,
                                                         void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void l2mc_id_key_to_str(_In_ const sai_l2mc_entry_t *sai_l2mc_entry, _Out_ char *key_str)
{
    snprintf(key_str, MAX_KEY_STR_LEN, "l2mc entry vlan_id %u",
             sai_l2mc_entry->vlan_id);
             
}

/**
 * @brief Create L2MC entry
 *
 * @param[in] l2mc_entry L2MC entry
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_l2mc_entry(
    _In_ const sai_l2mc_entry_t *l2mc_entry,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == l2mc_entry) {
        MRVL_SAI_LOG_ERR("NULL l2 mc entry id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(l2mc_entry->switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_l2mc_attribs, mrvl_sai_l2mc_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_l2mc_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove L2MC entry
 *
 * @param[in] l2mc_entry L2MC entry
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_l2mc_entry(
    _In_ const sai_l2mc_entry_t *l2mc_entry)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set L2MC entry attribute value
 *
 * @param[in] l2mc_entry L2MC entry
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_set_l2mc_entry_attribute(
    _In_ const sai_l2mc_entry_t *l2mc_entry,
    _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.l2mc_entry = *l2mc_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == l2mc_entry) {
        MRVL_SAI_LOG_ERR("NULL l2mc entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    l2mc_id_key_to_str(l2mc_entry, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_l2mc_attribs, mrvl_sai_l2mc_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get L2MC entry attribute value
 *
 * @param[in] l2mc_entry L2MC entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_get_l2mc_entry_attribute(
    _In_ const sai_l2mc_entry_t *l2mc_entry,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.l2mc_entry = *l2mc_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == l2mc_entry) {
        MRVL_SAI_LOG_ERR("NULL l2mc entry param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    l2mc_id_key_to_str(l2mc_entry, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_l2mc_attribs, mrvl_sai_l2mc_vendor_attribs, attr_count, attr_list);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief L2MC method table retrieved with sai_api_query()
 */
const sai_l2mc_api_t l2mc_api=
{
    mrvl_create_l2mc_entry,
    mrvl_remove_l2mc_entry,
    mrvl_set_l2mc_entry_attribute,
    mrvl_get_l2mc_entry_attribute
};
