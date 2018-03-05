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
#define __MODULE__ SAI_QUEUE


static void queue_id_to_str(_In_ sai_object_id_t sai_queue_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_queue_config_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);
static sai_status_t mrvl_sai_queue_config_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
static sai_status_t mrvl_sai_queue_type_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg);
static sai_status_t mrvl_sai_queue_index_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg);
static sai_status_t mrvl_sai_queue_port_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg);
static sai_status_t mrvl_sai_queue_parent_sched_node_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sai_queue_parent_sched_node_set(_In_ const sai_object_key_t      *key,
                                                     _In_ const sai_attribute_value_t *value,
                                                     void                             *arg);

static const sai_attribute_entry_t mrvl_sai_queue_attribs[] = {
    { SAI_QUEUE_ATTR_TYPE, true, false, false, true,
      "Queue type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_QUEUE_ATTR_PORT, false, false, true, true,
      "Queue port ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_QUEUE_ATTR_INDEX, false, false, true, true,
      "Queue index", SAI_ATTR_VAL_TYPE_U8 },
    { SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE, false, false, true, true,
      "Queue parent scheduler node", SAI_ATTR_VAL_TYPE_OID },
    { SAI_QUEUE_ATTR_WRED_PROFILE_ID, false, false, true, true,
      "Queue WRED profile ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_QUEUE_ATTR_BUFFER_PROFILE_ID, false, false, true, true,
      "Queue buffer profile ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID, false, false, true, true,
      "Queue scheduler profile ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_QUEUE_ATTR_PAUSE_STATUS, false, false, false, true,
      "Queue pause status", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_QUEUE_ATTR_ENABLE_PFC_DLDR, false, false, false, true,
      "Queue enable PFC DLDR", SAI_ATTR_VAL_TYPE_BOOL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_queue_vendor_attribs[] = {
    { SAI_QUEUE_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_queue_type_get, NULL,
      NULL, NULL },
    { SAI_QUEUE_ATTR_PORT,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_queue_port_get, NULL,
      NULL, NULL },
    { SAI_QUEUE_ATTR_INDEX,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_queue_index_get, NULL,
      NULL, NULL },
    { SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_queue_parent_sched_node_get, NULL,
      mrvl_sai_queue_parent_sched_node_set, NULL },
    { SAI_QUEUE_ATTR_WRED_PROFILE_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_queue_config_get, (void*)SAI_QUEUE_ATTR_WRED_PROFILE_ID,
      mrvl_sai_queue_config_set, (void*)SAI_QUEUE_ATTR_WRED_PROFILE_ID },
    { SAI_QUEUE_ATTR_BUFFER_PROFILE_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_queue_config_get, (void*)SAI_QUEUE_ATTR_BUFFER_PROFILE_ID,
      mrvl_sai_queue_config_set, (void*)SAI_QUEUE_ATTR_BUFFER_PROFILE_ID },
    { SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_queue_config_get, (void*)SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID,
      mrvl_sai_queue_config_set, (void*)SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID },
    { SAI_QUEUE_ATTR_PAUSE_STATUS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_QUEUE_ATTR_ENABLE_PFC_DLDR,
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

static sai_status_t mrvl_sai_queue_config_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}
static sai_status_t mrvl_sai_queue_config_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    sai_object_id_t          queue_id                     = key->key.object_id;
    uint32_t                 port_num, queue_idx = 0;
    long                     attr      = (long)arg;
    sai_status_t             status;

    MRVL_SAI_LOG_ENTER();

    assert((SAI_QUEUE_ATTR_WRED_PROFILE_ID == attr) ||
           (SAI_QUEUE_ATTR_BUFFER_PROFILE_ID == attr) ||
           (SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID == attr));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(queue_id, SAI_OBJECT_TYPE_QUEUE, &port_num, &queue_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object queue\n");
        MRVL_SAI_API_RETURN(status);
    }

    switch (attr) {
    case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
        status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_WRED, 1, &value->oid);
        break;

    case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
        status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BUFFER_PROFILE, 1, &value->oid);
        break;

    case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
        status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER, 1, &value->oid);
        break;
    }

    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to create selected object\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

static sai_status_t mrvl_sai_queue_type_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    /*sai_object_id_t queue_id   = key->key.object_id;
    sai_status_t     status;
    uint32_t port_num;
    uint32_t  queue_idx;*/

    MRVL_SAI_LOG_ENTER();
    

    /*if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(queue_id, SAI_OBJECT_TYPE_QUEUE, &port_num, &queue_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object queue\n");
        MRVL_SAI_API_RETURN(status);
    }*/
    
    value->s32 = SAI_QUEUE_TYPE_ALL;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_queue_index_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_object_id_t queue_id   = key->key.object_id;
    sai_status_t     status;
    uint32_t port_num;
    uint32_t  queue_idx;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(queue_id, SAI_OBJECT_TYPE_QUEUE, &port_num, &queue_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object queue\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->u8 = queue_idx;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_queue_port_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    sai_object_id_t queue_id   = key->key.object_id;
    sai_status_t     status;
    uint32_t port_num;
    uint32_t  queue_idx;

    MRVL_SAI_LOG_ENTER();

    

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(queue_id, SAI_OBJECT_TYPE_QUEUE, &port_num, &queue_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object queue\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->u8 = port_num;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_queue_parent_sched_node_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg)
{
    sai_object_id_t queue_id   = key->key.object_id;
    sai_status_t     status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER_GROUP, 1, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object scheduler group\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_queue_parent_sched_node_set(_In_ const sai_object_key_t      *key,
                                                     _In_ const sai_attribute_value_t *value,
                                                     void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void queue_id_to_str(_In_ sai_object_id_t sai_queue_id, _Out_ char *key_str)
{
    uint32_t port_num;
    uint32_t queue_idx;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_ext_type(sai_queue_id, SAI_OBJECT_TYPE_QUEUE, &port_num, &queue_idx)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid queue");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, " Queue:port %u:%u", queue_idx, port_num);
    }
}

/**
 * @brief Create queue
 *
 * @param[out] queue_id Queue id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_queue(
        _Out_ sai_object_id_t *queue_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}


/**
 * @brief Remove queue
 *
 * @param[in] queue_id Queue id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_queue(
        _In_ sai_object_id_t queue_id)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}


/**
 * @brief Set attribute to Queue
 * @param[in] queue_id queue id to set the attribute
 * @param[in] attr attribute to set
 *
 * @return  MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *           Failure status code on error
 */
sai_status_t mrvl_set_queue_attribute(
    _In_ sai_object_id_t queue_id,
    _In_ const sai_attribute_t *attr
)
{
	const sai_object_key_t key = { .key.object_id = queue_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == queue_id) {
        MRVL_SAI_LOG_ERR("NULL queue param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    queue_id_to_str(queue_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_queue_attribs, mrvl_sai_queue_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get attribute to Queue
 * @param[in] queue_id queue id to set the attribute
 * @param[in] attr_count number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return  MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *           Failure status code on error
 */
sai_status_t mrvl_get_queue_attribute(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t        attr_count,
    _Inout_ sai_attribute_t *attr_list
)
{
	const sai_object_key_t key = { .key.object_id = queue_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == queue_id) {
        MRVL_SAI_LOG_ERR("NULL queue param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    queue_id_to_str(queue_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_queue_attribs, mrvl_sai_queue_vendor_attribs, attr_count, attr_list);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}


/**
 * @brief   Get queue statistics counters.
 *
 * @param[in] queue_id Queue id
 * @param[in] counter_ids specifies the array of counter ids
 * @param[in] number_of_counters number of counters in the array
 * @param[out] counters array of resulting counter values.
 *
 * @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *         Failure status code on error
 */
sai_status_t mrvl_get_queue_stats(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_queue_stat_t *counter_ids,
    _Out_ uint64_t *counters
)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t status;
    uint32_t     ii, port, queue_idx;
    FPA_STATUS   fpa_status;
    FPA_QUEUE_STATISTICS_STC   queue_statistics;

	/*MRVL_SAI_LOG_ENTER();*/
	
    queue_id_to_str(queue_id, key_str);
    /*MRVL_SAI_LOG_NTC("Get queue stats %s\n", key_str);*/

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (NULL == counters) {
        MRVL_SAI_LOG_ERR("NULL counters array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (0 == number_of_counters) {
        MRVL_SAI_LOG_WRN("0 (ZERO) counters requested\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(queue_id, SAI_OBJECT_TYPE_QUEUE, &port, &queue_idx))) {
    	MRVL_SAI_API_RETURN(status);
    }

    fpa_status = fpaLibPortQueueStatisticsGet(SAI_DEFAULT_ETH_SWID_CNS, 
                                            port, queue_idx, 
                                            &queue_statistics);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get queue:port %u:%u counters\n", queue_idx, port);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    for (ii = 0; ii < number_of_counters; ii++) {
        switch (counter_ids[ii]) {
        case SAI_QUEUE_STAT_PACKETS:
            counters[ii] = queue_statistics.txPackets;
            break;
        case SAI_QUEUE_STAT_BYTES:
            counters[ii] = queue_statistics.txBytes;
            break;
        case SAI_QUEUE_STAT_DROPPED_PACKETS:
            counters[ii] = queue_statistics.txErrors;
            break;
        case SAI_QUEUE_STAT_DROPPED_BYTES:
        case SAI_QUEUE_STAT_GREEN_PACKETS:
        case SAI_QUEUE_STAT_GREEN_BYTES:
        case SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_GREEN_DROPPED_BYTES:
        case SAI_QUEUE_STAT_YELLOW_PACKETS:
        case SAI_QUEUE_STAT_YELLOW_BYTES:
        case SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES:
        case SAI_QUEUE_STAT_RED_PACKETS:
        case SAI_QUEUE_STAT_RED_BYTES:
        case SAI_QUEUE_STAT_RED_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_RED_DROPPED_BYTES:
        case SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_BYTES:
        case SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_BYTES:
        case SAI_QUEUE_STAT_RED_DISCARD_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_RED_DISCARD_DROPPED_BYTES:
        case SAI_QUEUE_STAT_DISCARD_DROPPED_PACKETS:
        case SAI_QUEUE_STAT_DISCARD_DROPPED_BYTES:
        case SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES:
        case SAI_QUEUE_STAT_WATERMARK_BYTES:
        case SAI_QUEUE_STAT_SHARED_CURR_OCCUPANCY_BYTES:
        case SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES:
            counters[ii] = 0;
            break;

        default:
            MRVL_SAI_LOG_ERR("Invalid queue counters %d\n", counter_ids[ii]);
            counters[ii] = 0;
        }
    }

    /*MRVL_SAI_LOG_EXIT();*/
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief   Clear queue statistics counters.
 *
 * @param[in] queue_id Queue id
 * @param[in] counter_ids specifies the array of counter ids
 * @param[in] number_of_counters number of counters in the array
 *
 * @MRVL_SAI_API_RETURN(SAI_STATUS_STUB) on success
 *         Failure status code on error
 */
sai_status_t mrvl_clear_queue_stats(
    _In_ sai_object_id_t queue_id,
    _In_ uint32_t number_of_counters,
    _In_ const sai_queue_stat_t *counter_ids
)
{
	sai_status_t status;
    uint32_t     port, queue_idx;
    char         key_str[MAX_KEY_STR_LEN];

    FPA_STATUS   fpa_status;

    MRVL_SAI_LOG_ENTER();

    queue_id_to_str(queue_id, key_str);
    MRVL_SAI_LOG_NTC("Clear %s stats\n", key_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(queue_id, SAI_OBJECT_TYPE_QUEUE, &port, &queue_idx))) {
    	MRVL_SAI_API_RETURN(status);
    }

    fpa_status = fpaLibPortQueueStatisticsClear(SAI_DEFAULT_ETH_SWID_CNS, port, queue_idx);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to clear queue:port %u:%u counters\n", queue_idx, port);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/**
 *  @brief Qos methods table retrieved with sai_api_query()
 */
const sai_queue_api_t queue_api=
{
    mrvl_create_queue,
    mrvl_remove_queue,
    mrvl_set_queue_attribute,
    mrvl_get_queue_attribute,
    mrvl_get_queue_stats,
    mrvl_clear_queue_stats
};

