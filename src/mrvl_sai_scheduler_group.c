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
#define __MODULE__ SAI_SCHEDULER_GROUPS

#define MAX_SCHED_LEVELS 2
#define MAX_SCHED_CHILD_GROUPS 8

#define level_max_groups(level) (uint32_t)MAX_SCHED_CHILD_GROUPS

#define level_max_childs(level) \
    (((level) == MAX_SCHED_LEVELS - 1) ? MAX_QUEUES : MAX_SCHED_CHILD_GROUPS)

static void sched_group_key_to_str(_In_ sai_object_id_t sched_group_id, _Out_ char *key_str);

static sai_status_t mrvl_sched_group_child_count_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sched_group_child_list_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg);
static sai_status_t mrvl_sched_group_port_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
static sai_status_t mrvl_sched_group_level_get(_In_ const sai_object_key_t   *key,
                                               _Inout_ sai_attribute_value_t *value,
                                               _In_ uint32_t                  attr_index,
                                               _Inout_ vendor_cache_t        *cache,
                                               void                          *arg);
static sai_status_t mrvl_sched_group_max_childs_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg);
static sai_status_t mrvl_sched_group_profile_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg);
static sai_status_t mrvl_sched_group_profile_set(_In_ const sai_object_key_t      *key,
                                                 _In_ const sai_attribute_value_t *value,
                                                 void                             *arg);
static sai_status_t mrvl_sched_group_parent_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg);

static sai_status_t mrvl_sched_group_parent_set(_In_ const sai_object_key_t      *key,
                                                _In_ const sai_attribute_value_t *value,
                                                void                             *arg);


static const sai_attribute_entry_t        sched_group_attribs[] = {
    { SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT, false, false, false, true,
      "Number of scheduler groups/queues childs added to", SAI_ATTR_VALUE_TYPE_UINT32 },
	{ SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST, false, false, false, true,
	  "Scheduler group child list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
	{ SAI_SCHEDULER_GROUP_ATTR_PORT_ID, true, false, false, true,
	  "Scheduler group on port", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_SCHEDULER_GROUP_ATTR_LEVEL, false, false, false, true,
	  "Scheduler group level", SAI_ATTR_VALUE_TYPE_UINT8 },
	{ SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, true, false, false, true,
	  "Max number of childs on scheduler group", SAI_ATTR_VALUE_TYPE_UINT8},
	{ SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, false, false, false, true,
	  "Scheduler profile id", SAI_ATTR_VALUE_TYPE_OBJECT_ID},
	{ SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, true, false, false, true,
      "Scheduler group parent node", SAI_ATTR_VALUE_TYPE_OBJECT_ID},

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t sched_group_vendor_attribs[] = {
    /* READ-ONLY */
    { SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sched_group_child_count_get, NULL,
      NULL, NULL },
    { SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sched_group_child_list_get, NULL,
      NULL, NULL },
    /* CREATE-ONLY */
    { SAI_SCHEDULER_GROUP_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sched_group_port_get, NULL,
      NULL, NULL },
    { SAI_SCHEDULER_GROUP_ATTR_LEVEL,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sched_group_level_get, NULL,
      NULL, NULL },
    { SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sched_group_max_childs_get, NULL,
      NULL, NULL },
    /* READ, WRITE, CREATE */
    { SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sched_group_profile_get, NULL,
      mrvl_sched_group_profile_set, NULL},
    { SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sched_group_parent_get, NULL,
      mrvl_sched_group_parent_set, NULL},
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

/** Number of queues/groups childs added to
 * scheduler group [uint32_t] */
static sai_status_t mrvl_sched_group_child_count_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 1;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/** Scheduler Group child obejct id List [sai_object_list_t] */
static sai_status_t mrvl_sched_group_child_list_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    sai_object_id_t data_obj;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER_GROUP, 1, &data_obj)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object scheduler group\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_objlist(&data_obj, 1, &value->objlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill objlist group child list\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** Scheduler group on port [sai_object_id_t]
 *  MANDATORY_ON_CREATE,  CREATE_ONLY */
static sai_status_t mrvl_sched_group_port_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object port\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** Scheduler group level [sai_uint8_t]
*  MANDATORY_ON_CREATE,  CREATE_ONLY */
static sai_status_t mrvl_sched_group_level_get(_In_ const sai_object_key_t   *key,
                                               _Inout_ sai_attribute_value_t *value,
                                               _In_ uint32_t                  attr_index,
                                               _Inout_ vendor_cache_t        *cache,
                                               void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->u8 = 1;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** Maximum Number of childs on group [uint8_t]
 * MANDATORY_ON_CREATE,  CREATE_ONLY */
static sai_status_t mrvl_sched_group_max_childs_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->u8 = 5;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** Scheduler ID [sai_object_id_t] */
static sai_status_t mrvl_sched_group_profile_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg)
{
    sai_status_t    status;
    uint32_t        sched_idx;
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_SCHEDULER_GROUP, &sched_idx))) {
        MRVL_SAI_LOG_ERR("Error converting scheduler group object to type\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER, 1, &value->oid))) {
        MRVL_SAI_LOG_ERR("Error creating scheduler object\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** Scheduler ID [sai_object_id_t] */
static sai_status_t mrvl_sched_group_profile_set(_In_ const sai_object_key_t      *key,
                                                 _In_ const sai_attribute_value_t *value,
                                                 void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/* It is used also by mrvl_sai_queue.c */
sai_status_t mrvl_sched_group_parent_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    uint32_t port;
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();

    if (sai_object_type_query(key->key.object_id) == SAI_OBJECT_TYPE_QUEUE) {
        status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_QUEUE, &port);
    } else if (sai_object_type_query(key->key.object_id) == SAI_OBJECT_TYPE_SCHEDULER_GROUP) {
        status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_SCHEDULER_GROUP, &port);
    } else {
        status = SAI_STATUS_INVALID_OBJECT_TYPE;
        MRVL_SAI_LOG_ERR("Expected queue or scheduler group object type, type is %d\n", sai_object_type_query(key->key.object_id));
    }
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(status);
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER_GROUP, port, &value->oid))) {
        MRVL_SAI_LOG_ERR("Error creating scheduler object\n");
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* It is used also by mrvl_sai_queue.c */
sai_status_t mrvl_sched_group_parent_set(_In_ const sai_object_key_t      *key,
                                         _In_ const sai_attribute_value_t *value,
                                         void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief  Create Scheduler group
 *
 * @param[out] scheduler_group_id Scheudler group id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
static sai_status_t mrvl_create_scheduler_group(_Out_ sai_object_id_t      *scheduler_group_id,
                                                _In_ sai_object_id_t        switch_id,
                                                _In_ uint32_t               attr_count,
                                                _In_ const sai_attribute_t *attr_list)
{
    sai_status_t  status; 
    char          list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *attr_val       = NULL;
    uint32_t                    attr_idx, port_id;
    uint8_t                     level;
    uint8_t                      max_child_count;
    uint8_t ext_data[2] = {0};
    /*sai_object_id_t              parent_group_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t              scheduler_id    = SAI_NULL_OBJECT_ID;*/
    sai_object_id_t              port_oid;

    MRVL_SAI_LOG_ENTER();

    if (NULL == scheduler_group_id) {
        MRVL_SAI_LOG_ERR("NULL scheduler group id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, sched_group_attribs, sched_group_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, sched_group_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create scheduler group: %s\n", list_str);

    
    /* check mandatory attribute SAI_SCHEDULER_GROUP_ATTR_PORT_ID */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SCHEDULER_GROUP_ATTR_PORT_ID, &attr_val, &attr_idx));
    status = mrvl_sai_utl_object_to_type(attr_val->oid, SAI_OBJECT_TYPE_PORT, &port_id);

    /*parent_group_id = attr_val->oid;*/

    /* Handle SAI_SCHEDULER_GROUP_ATTR_LEVEL */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SCHEDULER_GROUP_ATTR_LEVEL, &attr_val, &attr_idx));
    
    level = attr_val->u8;
    if (level >= MAX_SCHED_LEVELS) {
        status = SAI_STATUS_INVALID_PARAMETER;
        MRVL_SAI_LOG_ERR("Invalid scheduler group level %u, (min, max) allowed are (%u, %u)\n",
                   level, 0, MAX_SCHED_LEVELS - 1);
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(status);
    }

    if (level > 0) {
        /* Handle SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE */
        assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE, &attr_val, &attr_idx));
        
        /*parent_group_id = attr_val->oid;*/
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_id, &port_oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object port\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* Handle SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, &attr_val, &attr_idx));
    
    /* Handle SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS, &attr_val, &attr_idx));

    max_child_count = attr_val->u8;
    if (max_child_count > level_max_childs(level)) {
        status = SAI_STATUS_INVALID_PARAMETER;
        MRVL_SAI_LOG_ERR("Invalid scheduler group max childs %u, maximum allowed is %u\n",
                   max_child_count, level_max_childs(level));
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(status);
    }

    

    ext_data[0] = level;
    ext_data[1] = 1; /*index */
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER_GROUP, level, scheduler_group_id)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object scheduler group\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    
    MRVL_SAI_LOG_NTC("Created scheduler group %x at port %x level %u index %u\n",
               *scheduler_group_id, port_id, level, ext_data[1]);

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief  Remove Scheduler group
 *
 * @param[in] scheduler_group_id Scheudler group id
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
static sai_status_t mrvl_remove_scheduler_group(_In_ sai_object_id_t scheduler_group_id)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief  Set Scheduler group Attribute
 *
 * @param[in] scheduler_group_id Scheudler group id
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
static sai_status_t mrvl_set_scheduler_group_attribute(_In_ sai_object_id_t        scheduler_group_id,
                                                       _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = scheduler_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    sched_group_key_to_str(scheduler_group_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, sched_group_attribs, sched_group_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief  Get Scheduler Group attribute
 *
 * @param[in] scheduler_group_id - scheduler group id
 * @param[in] attr_count - number of attributes
 * @param[inout] attr_list - array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */

static sai_status_t mrvl_get_scheduler_group_attribute(_In_ sai_object_id_t     scheduler_group_id,
                                                       _In_ uint32_t            attr_count,
                                                       _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = scheduler_group_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == scheduler_group_id) {
        MRVL_SAI_LOG_ERR("Invalid scheduler group id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    sched_group_key_to_str(scheduler_group_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, sched_group_attribs, sched_group_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

static void sched_group_key_to_str(_In_ sai_object_id_t sched_group_id, _Out_ char *key_str)
{
    uint32_t sched_group;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sched_group_id, SAI_OBJECT_TYPE_SCHEDULER_GROUP, &sched_group)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid scheduler group");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "scheduler group %x", sched_group);
    }
}

/**
 * @brief  Scheduler Group methods table retrieved with sai_api_query()
 */
const sai_scheduler_group_api_t scheduler_group_api=
{
    mrvl_create_scheduler_group,
    mrvl_remove_scheduler_group,
    mrvl_set_scheduler_group_attribute,
    mrvl_get_scheduler_group_attribute
};

