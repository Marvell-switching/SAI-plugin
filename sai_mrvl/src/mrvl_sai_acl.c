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
#define __MODULE__ SAI_ACL




static const sai_attribute_entry_t acl_table_attribs[] = {   
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VAL_TYPE_UNDETERMINED
    }
};

static const sai_vendor_attribute_entry_t acl_table_vendor_attribs[] = {
};

static const sai_attribute_entry_t acl_entry_attribs[] = {   
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VAL_TYPE_UNDETERMINED
    }
};
static const sai_vendor_attribute_entry_t acl_entry_vendor_attribs[] = {
};

static const sai_attribute_entry_t acl_counter_attribs[] = {   
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VAL_TYPE_UNDETERMINED
    }
};
static const sai_vendor_attribute_entry_t acl_counter_vendor_attribs[] = {
};



static sai_status_t mrvl_parse_acl_match_fields_attr_prv(FPA_FLOW_TABLE_ENTRY_STC *fpa_flow_entry,
                                                         _In_ uint32_t            attr_count,
                                                         _Inout_ const sai_attribute_t  *attr_list)
{
/*    const sai_attribute_value_t *stage, *priority, *size, *group;
    uint32_t stage_index, priority_index, size_index, group_index;
    uint32_t table_size, group_id;
    sai_status_t  status;*/

  /*  table_size = 0;
    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_SIZE, &size, &size_index);
    if (status == SAI_STATUS_SUCCESS) {
        table_size = *size;
    }*/
    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *   @brief Set ACL table attribute
 *
 * Arguments:
 *    @param[in] acl_table_id - the acl table id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_set_acl_table_attribute(_In_ sai_object_id_t acl_table_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .object_id = acl_table_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    snprintf(key_str, MAX_KEY_STR_LEN, "acl_table %llx", (long long int)acl_table_id);
    return mrvl_sai_utl_set_attribute(&key, key_str, acl_table_attribs, acl_table_vendor_attribs, attr);
}


/**
 * Routine Description:
 *   @brief Get ACL table attribute
 *
 * Arguments:
 *    @param[in] acl_table_id - acl table id
 *    @param[in] attr_count - number of attributes
 *    @param[out] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_get_acl_table_attribute(_In_ sai_object_id_t acl_table_id,
                                         _In_ uint32_t            attr_count,
                                         _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = acl_table_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    snprintf(key_str, MAX_KEY_STR_LEN, "acl_table %llx", (long long int)acl_table_id);
    return mrvl_sai_utl_get_attributes(&key, key_str, acl_table_attribs, acl_table_vendor_attribs, attr_count, attr_list);
}

/**
 * Routine Description:
 *   @brief Set ACL table attribute
 *
 * Arguments:
 *    @param[in] acl_entry_id - the acl entry id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_set_acl_entry_attribute(_In_ sai_object_id_t acl_entry_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .object_id = acl_entry_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_entry %llx", (long long int)acl_entry_id);
    return mrvl_sai_utl_set_attribute(&key, key_str, acl_entry_attribs, acl_entry_vendor_attribs, attr);
}


/**
 * Routine Description:
 *   @brief Get ACL entry attribute
 *
 * Arguments:
 *    @param[in] acl_entry_id - acl entry id
 *    @param[in] attr_count - number of attributes
 *    @param[out] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_get_acl_entry_attribute(_In_ sai_object_id_t acl_entry_id,
                                         _In_ uint32_t            attr_count,
                                         _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = acl_entry_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_entry %llx", (long long int)acl_entry_id);
    return mrvl_sai_utl_get_attributes(&key, key_str, acl_entry_attribs, acl_entry_vendor_attribs, attr_count, attr_list);
}

/**
 * Routine Description:
 *   @brief Set ACL counter attribute
 *
 * Arguments:
 *    @param[in] acl_counter_id - the acl counter id
 *    @param[in] attr - attribute
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_set_acl_counter_attribute(_In_ sai_object_id_t acl_counter_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .object_id = acl_counter_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_counter %llx", (long long int)acl_counter_id);
    return mrvl_sai_utl_set_attribute(&key, key_str, acl_counter_attribs, acl_counter_vendor_attribs, attr);
}


/**
 * Routine Description:
 *   @brief Get ACL counter attribute
 *
 * Arguments:
 *    @param[in] acl_counter_id - acl counter id
 *    @param[in] attr_count - number of attributes
 *    @param[out] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_get_acl_counter_attribute(_In_ sai_object_id_t acl_counter_id,
                                         _In_ uint32_t            attr_count,
                                         _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = acl_counter_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_counter %llx", (long long int)acl_counter_id);
    return mrvl_sai_utl_get_attributes(&key, key_str, acl_counter_attribs, acl_counter_vendor_attribs, attr_count, attr_list);
}


/**
 *   Routine Description:
 *    @brief Create an ACL table
 * 
 *  Arguments:
 *  @param[out] acl_table_id - the the acl table id
 *  @param[in] attr_count - number of attributes
 *  @param[in] attr_list - array of attributes
 * 
 *  Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_create_acl_table(_Out_ sai_object_id_t* acl_table_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list)
{
    char    key_str[MAX_KEY_STR_LEN];
    char    list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *stage, *priority, *size, *group;
    uint32_t stage_index, priority_index, size_index, group_index;
    uint32_t group_id;
    sai_status_t  status;
    FPA_STATUS    fpa_status;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry;

    MRVL_SAI_LOG_ENTER();
    if (NULL == acl_table_id) {
        MRVL_SAI_LOG_ERR("NULL acl_table_id param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_table_attribs, acl_table_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        return status;
    }

    snprintf(key_str, MAX_KEY_STR_LEN, "acl_table %llx", (long long int)*acl_table_id);
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_INF("Create FDB entry %s\n", key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);

    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_STAGE, &stage, &stage_index));
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_PRIORITY, &priority, &priority_index));

    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_SIZE, &size, &size_index);

	group_id = -1; 
    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_GROUP_ID, &group, &group_index);
    if (status == SAI_STATUS_SUCCESS) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(group->oid, SAI_OBJECT_TYPE_ACL_TABLE_GROUP, &group_id))) {
            return status;
        }
    }
    switch (stage->u32) {
    case SAI_ACL_STAGE_INGRESS:
    case SAI_ACL_SUBSTAGE_INGRESS_PRE_L2:
        fpa_table = FPA_FLOW_TABLE_TYPE_PCL0_E;
        /*TODO - get pcl_id from group_id*/
        break;
    case SAI_ACL_STAGE_EGRESS:
        fpa_table = FPA_FLOW_TABLE_TYPE_EPCL_E;
        break;
    default:
        MRVL_SAI_LOG_ERR("Invalid acl stage %d\n", stage->u32);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    fpa_status = fpaLibFlowEntryInit(0, fpa_table, &fpa_flow_entry);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to init FDB entry status = %d\n", fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }
    status = mrvl_parse_acl_match_fields_attr_prv(&fpa_flow_entry, attr_count, (const sai_attribute_t  *)attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Failed to parse acl match fields status = %d\n", status);
        return status;
    }
    /*TODO*/
    MRVL_SAI_LOG_NTC("Create acl table %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/**
 *  Routine Description:
 *    @brief Delete an ACL table
 * 
 *  Arguments:
 *    @param[in] acl_table_id - the acl table id
 * 
 *  Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_delete_acl_table(_In_ sai_object_id_t acl_table_id)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_table %llx", (long long int)acl_table_id);
    MRVL_SAI_LOG_NTC("Delete  %s\n", key_str);
    /*TODO*/
    MRVL_SAI_LOG_EXIT();

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *   @brief Create an ACL entry
 *
 * Arguments:
 *   @param[out] acl_entry_id - the acl entry id
 *   @param[in] attr_count - number of attributes
 *   @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_create_acl_entry(_Out_ sai_object_id_t* acl_entry_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_entry %llx", (long long int)*acl_entry_id);
    /*TODO*/
    MRVL_SAI_LOG_NTC("Create  %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/**
 * Routine Description:
 *   @brief Delete an ACL entry
 *
 * Arguments:
 *  @param[in] acl_entry_id - the acl entry id
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_delete_acl_entry(_In_ sai_object_id_t acl_entry_id)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_entry %llx", (long long int)acl_entry_id);
    MRVL_SAI_LOG_NTC("Delete  %s\n", key_str);
    /*TODO*/
    MRVL_SAI_LOG_EXIT();

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *   @brief Create an ACL counter
 *
 * Arguments:
 *   @param[out] acl_counter_id - the acl counter id
 *   @param[in] attr_count - number of attributes
 *   @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_create_acl_counter(_Out_ sai_object_id_t* acl_counter_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_counter %llx", (long long int)*acl_counter_id);
    /*TODO*/
    MRVL_SAI_LOG_NTC("Create  %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/**
 * Routine Description:
 *   @brief Delete an ACL counter
 *
 * Arguments:
 *  @param[in] acl_counter_id - the acl counter id
 *
 * Return Values:
 *    @return  SAI_STATUS_SUCCESS on success
 *             Failure status code on error
 */
sai_status_t mrvl_delete_acl_counter(_In_ sai_object_id_t acl_counter_id)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_counter %llx", (long long int)acl_counter_id);
    MRVL_SAI_LOG_NTC("Delete  %s\n", key_str);
    /*TODO*/
    MRVL_SAI_LOG_EXIT();

    return SAI_STATUS_SUCCESS;
}
const sai_acl_api_t acl_api = {
    mrvl_create_acl_table,
    mrvl_delete_acl_table,
    mrvl_set_acl_table_attribute,
    mrvl_get_acl_table_attribute,
    mrvl_create_acl_entry,
    mrvl_delete_acl_entry,
    mrvl_set_acl_entry_attribute,
    mrvl_get_acl_entry_attribute,
    mrvl_create_acl_counter,
    mrvl_delete_acl_counter,
    mrvl_set_acl_counter_attribute,
    mrvl_get_acl_counter_attribute
};


