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

extern uint32_t mrvl_sai_switch_ecmp_hash_algorithm;
static const sai_attribute_entry_t mrvl_sai_next_hop_group_attribs[] = {
    { SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT, false, false, false, true,
      "Next hop group entries count", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_NEXT_HOP_GROUP_ATTR_TYPE, true, true, false, true,
      "Next hop group type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST, true, true, true, true,
      "Next hop group hop list", SAI_ATTR_VAL_TYPE_OBJLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static mrvl_sai_next_hop_group_table_t mrvl_sai_next_hop_group_table[SAI_ECMP_MAX_GROUPS_CNS] = {};

static sai_status_t mrvl_sai_next_hop_group_count_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg);
static sai_status_t mrvl_sai_next_hop_group_type_get_prv(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
static sai_status_t mrvl_sai_next_hop_group_hop_list_get_prv(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
static sai_status_t mrvl_sai_next_hop_group_hop_list_set_prv(_In_ const sai_object_key_t      *key,
                                              _In_ const sai_attribute_value_t *value,
                                              void                             *arg);

static const sai_vendor_attribute_entry_t mrvl_sai_next_hop_group_vendor_attribs[] = {
    { SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_next_hop_group_count_get_prv, NULL,
      NULL, NULL },
    { SAI_NEXT_HOP_GROUP_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_next_hop_group_type_get_prv, NULL,
      NULL, NULL },
    { SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_next_hop_group_hop_list_get_prv, NULL,
      mrvl_sai_next_hop_group_hop_list_set_prv, NULL },
};



/* State DB *************/
/*typedef struct _mrvl_sai_next_hop_group_t {
    uint32_t          next_hop_count;
    sai_next_hop_id_t next_hop_list[ECMP_MAX_PATHS];
    bool              is_valid;
} mrvl_sai_next_hop_group_t;
*/
/*static mrvl_sai_next_hop_group_t next_hop_group_db[SAI_ECMP_MAX_GROUPS_CNS];

void db_init_next_hop_group()
{
    memset(next_hop_group_db, 0, sizeof(next_hop_group_db));
}

sai_status_t db_get_next_hop_group(_In_ sai_next_hop_group_id_t next_hop_group_id,
                                   _Out_ sai_next_hop_list_t   *next_hop_list)
{
    if (NULL == next_hop_list) {
        MRVL_SAI_LOG_ERR("NULL next hop list param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((next_hop_group_id >= SAI_ECMP_MAX_GROUPS_CNS) ||
        (!next_hop_group_db[next_hop_group_id].is_valid)) {
        MRVL_SAI_LOG_ERR("Invalid next hop group ID %u\n", next_hop_group_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    next_hop_list->next_hop_count = next_hop_group_db[next_hop_group_id].next_hop_count;
    next_hop_list->next_hop_list = next_hop_group_db[next_hop_group_id].next_hop_list;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t db_find_free_index(_Out_ uint32_t *free_index)
{
    uint32_t ii;

    for (ii = 0; ii < SAI_ECMP_MAX_GROUPS_CNS; ii++) {
        if (false == next_hop_group_db[ii].is_valid) {
            *free_index = ii;
            return SAI_STATUS_SUCCESS;
        }
    }

    MRVL_SAI_LOG_ERR("Next hop group table full\n");
    return SAI_STATUS_TABLE_FULL;
}

static sai_status_t db_create_next_hop_group(_Out_ sai_next_hop_group_id_t  *next_hop_group_id,
                                             _In_ const sai_next_hop_list_t *next_hop_list,
                                             _In_ uint32_t                   param_index)
{
    uint32_t     group_index;
    sai_status_t status;

    if (NULL == next_hop_group_id) {
        MRVL_SAI_LOG_ERR("NULL next hop group id param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == next_hop_list) {
        MRVL_SAI_LOG_ERR("NULL next hop list param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (next_hop_list->next_hop_count > ECMP_MAX_PATHS) {
        MRVL_SAI_LOG_ERR("Next hop count %u bigger than maximum %u\n", next_hop_list->next_hop_count, ECMP_MAX_PATHS);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + param_index;
    }

    if (SAI_STATUS_SUCCESS !=
        (status = db_find_free_index(&group_index))) {
        return status;
    }

    next_hop_group_db[group_index].next_hop_count = next_hop_list->next_hop_count;
    memcpy(next_hop_group_db[group_index].next_hop_list,
           next_hop_list->next_hop_list,
           sizeof(sai_next_hop_id_t) * next_hop_list->next_hop_count);
    next_hop_group_db[group_index].is_valid = true;
    *next_hop_group_id = group_index;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t db_remove_next_hop_group(_In_ sai_next_hop_group_id_t next_hop_group_id)
{
    if ((next_hop_group_id >= SAI_ECMP_MAX_GROUPS_CNS) ||
        (!next_hop_group_db[next_hop_group_id].is_valid)) {
        MRVL_SAI_LOG_ERR("Invalid next hop group ID %u\n", next_hop_group_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    next_hop_group_db[next_hop_group_id].is_valid = false;

    return SAI_STATUS_SUCCESS;
}

sai_status_t db_update_next_hop_group_list(_In_ sai_next_hop_group_id_t next_hop_group_id,
                                           _In_ sai_next_hop_list_t     next_hop_list)
{
    if ((next_hop_group_id >= SAI_ECMP_MAX_GROUPS_CNS) ||
        (!next_hop_group_db[next_hop_group_id].is_valid)) {
        MRVL_SAI_LOG_ERR("Invalid next hop group ID %u\n", next_hop_group_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (next_hop_list.next_hop_count > ECMP_MAX_PATHS) {
        MRVL_SAI_LOG_ERR("Next hop count %u bigger than maximum %u\n", next_hop_list.next_hop_count, ECMP_MAX_PATHS);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    next_hop_group_db[next_hop_group_id].next_hop_count = next_hop_list.next_hop_count;
    memcpy(next_hop_group_db[next_hop_group_id].next_hop_list,
           next_hop_list.next_hop_list,
           sizeof(sai_next_hop_id_t) * next_hop_list.next_hop_count);

    return SAI_STATUS_SUCCESS;
}

sai_status_t db_add_members_next_hop_group_list(_In_ sai_next_hop_group_id_t  next_hop_group_id,
                                                _In_ uint32_t                 next_hop_count,
                                                _In_ const sai_next_hop_id_t* nexthops)
{
    mrvl_sai_next_hop_group_t *group;

    if ((next_hop_group_id >= SAI_ECMP_MAX_GROUPS_CNS) ||
        (!next_hop_group_db[next_hop_group_id].is_valid)) {
        MRVL_SAI_LOG_ERR("Invalid next hop group ID %u\n", next_hop_group_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    group = &next_hop_group_db[next_hop_group_id];

    if (next_hop_count + group->next_hop_count > ECMP_MAX_PATHS) {
        MRVL_SAI_LOG_ERR("Next hop count %u bigger than maximum %u\n",
                   next_hop_count + group->next_hop_count, ECMP_MAX_PATHS);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    memcpy(&group->next_hop_list[group->next_hop_count],
           nexthops,
           sizeof(sai_next_hop_id_t) * next_hop_count);
    group->next_hop_count += next_hop_count;

    return SAI_STATUS_SUCCESS;
}

bool next_hop_in_list(sai_next_hop_id_t nexthop, _In_ uint32_t next_hop_count, _In_ const sai_next_hop_id_t* nexthops)
{
    uint32_t ii;

    for (ii = 0; ii < next_hop_count; ii++) {
        if (nexthop == nexthops[ii]) {
            return true;
        }
    }

    return false;
}

sai_status_t db_remove_members_next_hop_group_list(_In_ sai_next_hop_group_id_t  next_hop_group_id,
                                                   _In_ uint32_t                 next_hop_count,
                                                   _In_ const sai_next_hop_id_t* nexthops)
{
    mrvl_sai_next_hop_group_t *group;
    uint32_t               ii = 0;

    if ((next_hop_group_id >= SAI_ECMP_MAX_GROUPS_CNS) ||
        (!next_hop_group_db[next_hop_group_id].is_valid)) {
        MRVL_SAI_LOG_ERR("Invalid next hop group ID %u\n", next_hop_group_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    group = &next_hop_group_db[next_hop_group_id];

    while (ii < group->next_hop_count) {
        if (next_hop_in_list(group->next_hop_list[ii], next_hop_count, nexthops)) {
            group->next_hop_count--;
            group->next_hop_list[ii] = group->next_hop_list[group->next_hop_count];
            continue;
        }
        ii++;
    }

    return SAI_STATUS_SUCCESS;
}
*/
/*************************/

static void mrvl_sai_next_hop_group_key_to_str(_In_ sai_object_id_t next_hop_group_id, _Out_ char *key_str)
{
    uint32_t nhg;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "next_hop_group_id invalid");
        return ;
    }
    snprintf(key_str, MAX_KEY_STR_LEN, "next hop group id %u", nhg);
}

/*
 * Routine Description:
 *    Create next hop group
 *
 * Arguments:
 *    [out] next_hop_group_id - next hop group id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_create_next_hop_group(_Out_ sai_object_id_t* next_hop_group_id,
                                        _In_ uint32_t                  attr_count,
                                        _In_ const sai_attribute_t    *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *type, *hop_list;
    uint32_t                     type_index, hop_list_index, nhg_count, nhg_idx, group;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == next_hop_group_id) {
        MRVL_SAI_LOG_ERR("NULL next hop group id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_next_hop_group_attribs, mrvl_sai_next_hop_group_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_next_hop_group_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create next hop group, %s\n", list_str);

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_NEXT_HOP_GROUP_ATTR_TYPE, &type, &type_index));
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count,
                               attr_list,
                               SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST,
                               &hop_list,
                               &hop_list_index));

    if (SAI_NEXT_HOP_GROUP_ECMP != type->s32) {
        MRVL_SAI_LOG_ERR("Invalid next hop group type %d on create\n", type->s32);
        status = SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
        MRVL_SAI_API_RETURN(status);
    }
    nhg_count = hop_list->objlist.count;
    if ((nhg_count < 1) || (nhg_count > SAI_ECMP_MAX_MEMBERS_IN_GROUP_CNS )) {
        MRVL_SAI_LOG_ERR("Invalid next hop list length %d on create\n", nhg_count);
        status = SAI_STATUS_INVALID_ATTR_VALUE_0 + hop_list_index;
        MRVL_SAI_API_RETURN(status);
    }
    /* find new nhg_id */
    for (nhg_idx = 0; nhg_idx < SAI_ECMP_MAX_GROUPS_CNS; nhg_idx++) {
        if (mrvl_sai_next_hop_group_table[nhg_idx].used == false){
            break;
        }
    }
    if (nhg_idx > SAI_ECMP_MAX_GROUPS_CNS) { /*next hop group table is full */
        MRVL_SAI_LOG_ERR("next hop group table is full\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }

    status = mrvl_sai_utl_create_l3_ecmp_group(nhg_idx, mrvl_sai_switch_ecmp_hash_algorithm, &hop_list->objlist, &group);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't create group %d\n", nhg_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
        
        
/*    if (SAI_STATUS_SUCCESS !=
        (status = db_create_next_hop_group(next_hop_group_id, &(hop_list->objlist), hop_list_index))) {
        return status;
    }*/
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, next_hop_group_id))) {
    	MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_next_hop_group_table[nhg_idx].used = true;
    mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter = nhg_count;
    mrvl_sai_next_hop_group_key_to_str(*next_hop_group_id, key_str);
    MRVL_SAI_LOG_NTC("Created next hop group %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *    Remove next hop group
 *
 * Arguments:
 *    [in] next_hop_group_id - next hop group id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_next_hop_group(_In_ sai_object_id_t next_hop_group_id)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t status;
    uint32_t     nhg_idx;
    MRVL_SAI_LOG_ENTER();

    mrvl_sai_next_hop_group_key_to_str(next_hop_group_id, key_str);
    MRVL_SAI_LOG_NTC("Remove next hop group %s\n", key_str);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg_idx)) {
        MRVL_SAI_LOG_ERR("invalid nhg_idx\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    mrvl_sai_next_hop_group_table[nhg_idx].used = false;
    mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter = 0;

    status = mrvl_sai_utl_delete_l3_ecmp_group(nhg_idx);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't delete group %d\n", nhg_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
 /*   if (SAI_STATUS_SUCCESS !=
        (status = db_remove_next_hop_group(next_hop_group_id))) {
        return status;
    }
*/
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *    Set Next Hop Group attribute
 *
 * Arguments:
 *    [in] sai_object_id_t - next_hop_group_id
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_set_next_hop_group_attribute(_In_ sai_object_id_t next_hop_group_id,
                                               _In_ const sai_attribute_t  *attr)
{
    const sai_object_key_t key = { .object_id = next_hop_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_next_hop_group_key_to_str(next_hop_group_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_next_hop_group_attribs, mrvl_sai_next_hop_group_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/*
 * Routine Description:
 *    Get Next Hop Group attribute
 *
 * Arguments:
 *    [in] sai_object_id_t - next_hop_group_id
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_next_hop_group_attribute(_In_ sai_object_id_t next_hop_group_id,
                                               _In_ uint32_t                attr_count,
                                               _Inout_ sai_attribute_t     *attr_list)
{
    const sai_object_key_t key = { .object_id = next_hop_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_next_hop_group_key_to_str(next_hop_group_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key,
                              key_str,
                              mrvl_sai_next_hop_group_attribs,
                              mrvl_sai_next_hop_group_vendor_attribs,
                              attr_count,
                              attr_list);
    MRVL_SAI_API_RETURN(status);
}

/* Next hop group type [sai_next_hop_group_type_t] */
static sai_status_t mrvl_sai_next_hop_group_type_get_prv(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_NEXT_HOP_GROUP_ECMP;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Number of next hops in the group [uint32_t] */
static sai_status_t mrvl_sai_next_hop_group_count_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_object_id_t     next_hop_group_id = key->object_id;
    uint32_t            nhg_idx;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg_idx)) {
        MRVL_SAI_LOG_ERR("invalid nhg_idx\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    value->u32 = mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter;
    
/*    if (SAI_STATUS_SUCCESS !=
        (status = db_get_next_hop_group(next_hop_group_id, &hop_list))) {
        return status;
    }
*/
  /*  value->u32 = hop_list.next_hop_count;*/

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Next hop list [sai_next_hop_list_t] */
static sai_status_t mrvl_sai_next_hop_group_hop_list_get_prv(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_status_t        status;
    sai_object_id_t     next_hop_group_id = key->object_id;
    uint32_t            nhg_idx;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg_idx)) {
        MRVL_SAI_LOG_ERR("invalid nhg_idx\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    status = mrvl_sai_utl_get_l3_ecmp_group_bucket_list(nhg_idx, &value->objlist);
    /*   if (SAI_STATUS_SUCCESS !=
(status = db_get_next_hop_group(next_hop_group_id, &hop_list))) {
return status;
}
*/
    /* Not enough space allocated by caller */
 /*   if (value->nhlist.next_hop_count < hop_list.next_hop_count) {
        MRVL_SAI_LOG_ERR("Insufficient hop list buffer size. Allocated %u needed %u\n",
                   value->nhlist.next_hop_count, hop_list.next_hop_count);
        value->nhlist.next_hop_count = hop_list.next_hop_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    value->nhlist.next_hop_count = hop_list.next_hop_count;
    memcpy(value->nhlist.next_hop_list, hop_list.next_hop_list, sizeof(sai_next_hop_id_t) * hop_list.next_hop_count);
*/
    MRVL_SAI_LOG_EXIT();
    return status;
}

/* Next hop list [sai_next_hop_list_t] */
static sai_status_t mrvl_sai_next_hop_group_hop_list_set_prv(_In_ const sai_object_key_t      *key,
                                              _In_ const sai_attribute_value_t *value,
                                              void                             *arg)
{
    sai_status_t    status;
    sai_object_id_t next_hop_group_id = key->object_id;
    uint32_t        nhg_idx, i, numChanged;
    sai_object_list_t objlist;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg_idx)) {
        MRVL_SAI_LOG_ERR("invalid nhg_idx\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    objlist.count = value->objlist.count;
    objlist.list = value->objlist.list;
    for (i = 0; i <objlist.count; i++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_object_type(objlist.list[i], SAI_OBJECT_TYPE_NEXT_HOP)) {
            MRVL_SAI_LOG_ERR("invalid next hop id in list %d\n",i);
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    status = mrvl_sai_utl_update_l3_ecmp_group_bucket_list(nhg_idx, &objlist, MRVL_SAI_UTL_MODIFY, &numChanged);
    if (SAI_STATUS_SUCCESS != status) {
        mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter = 0;
        mrvl_sai_next_hop_group_table[nhg_idx].used = false;
    } else {
        mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter = objlist.count;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

/*
 * Routine Description:
 *    Add next hop to a group
 *
 * Arguments:
 *    [in] next_hop_group_id - next hop group id
 *    [in] next_hop_count - number of next hops
 *    [in] nexthops - array of next hops
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_add_next_hop_to_group(_In_ sai_object_id_t        next_hop_group_id,
                                            _In_ uint32_t               next_hop_count,
                                            _In_ const sai_object_id_t* nexthops)
{
    sai_status_t      status;
    uint32_t          nhg_idx, i, changed;
    sai_object_list_t objlist;
    MRVL_SAI_LOG_ENTER();

    if (NULL == nexthops) {
        MRVL_SAI_LOG_ERR("NULL nexthops param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (next_hop_count != 1) {
        MRVL_SAI_LOG_ERR("Can add only one nexthop\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg_idx)) {
        MRVL_SAI_LOG_ERR("invalid nhg_idx\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    for (i = 0; i <next_hop_count; i++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_object_type(nexthops[i], SAI_OBJECT_TYPE_NEXT_HOP)) {
            MRVL_SAI_LOG_ERR("invalid next hop id in list %d\n",i);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    objlist.count = next_hop_count;
    objlist.list = (sai_object_id_t*)nexthops;
    status = mrvl_sai_utl_update_l3_ecmp_group_bucket_list(nhg_idx, &objlist, MRVL_SAI_UTL_ADD, &changed);

    if (SAI_STATUS_SUCCESS != status) {
        mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter = 0;
        mrvl_sai_next_hop_group_table[nhg_idx].used = false;
    }else {
        mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter += changed;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/*
 * Routine Description:
 *    Remove next hop from a group
 *
 * Arguments:
 *    [in] next_hop_group_id - next hop group id
 *    [in] next_hop_count - number of next hops
 *    [in] nexthops - array of next hops
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_next_hop_from_group(_In_ sai_object_id_t  next_hop_group_id,
                                             _In_ uint32_t                 next_hop_count,
                                             _In_ const sai_object_id_t* nexthops)
{
    sai_status_t      status;
    uint32_t          nhg_idx, i, changed;
    sai_object_list_t objlist;
    MRVL_SAI_LOG_ENTER();

    if (NULL == nexthops) {
        MRVL_SAI_LOG_ERR("NULL nexthops param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    if (next_hop_count != 1) {
        MRVL_SAI_LOG_ERR("Can remove only one nexthop\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &nhg_idx)) {
        MRVL_SAI_LOG_ERR("invalid nhg_idx\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    for (i = 0; i <next_hop_count; i++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_object_type(nexthops[i], SAI_OBJECT_TYPE_NEXT_HOP)) {
            MRVL_SAI_LOG_ERR("invalid next hop id in list %d\n",i);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    objlist.count = next_hop_count;
    objlist.list = (sai_object_id_t*)nexthops;
    status = mrvl_sai_utl_update_l3_ecmp_group_bucket_list(nhg_idx, &objlist, MRVL_SAI_UTL_DEL, &changed);
    if (SAI_STATUS_SUCCESS != status) {
        mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter = 0;
        mrvl_sai_next_hop_group_table[nhg_idx].used = false;
    }else {
        mrvl_sai_next_hop_group_table[nhg_idx].group_member_counter -= changed;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

const sai_next_hop_group_api_t next_hop_group_api = {
    mrvl_sai_create_next_hop_group,
    mrvl_sai_remove_next_hop_group,
    mrvl_sai_set_next_hop_group_attribute,
    mrvl_sai_get_next_hop_group_attribute,
    mrvl_sai_add_next_hop_to_group,
    mrvl_sai_remove_next_hop_from_group
};
