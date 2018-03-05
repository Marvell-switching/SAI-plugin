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
#include <pthread.h>

#undef  __MODULE__
#define __MODULE__ SAI_SWITCH

uint32_t mrvl_sai_switch_init_first_time=1;

sai_switch_notification_t     mrvl_sai_notification_callbacks;

bool                      mrvl_switch_is_created = false;
uint32_t                  mrvl_profile_id = 0;
uint32_t                  mrvl_sai_switch_aging_time = SAI_DEFAULT_FDB_AGING_TIME_CNS;
uint32_t                  mrvl_sai_switch_ecmp_hash_algorithm = SAI_ECMP_DEFAULT_HASH_ALGORITHM_CNS;

pthread_t mrvl_sai_au_thread;

uint32_t SAI_SYS_PORT_MAPPING[SAI_MAX_NUM_OF_PORTS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
   48,49,50,51,52,53 /*,54,55,56,57,58,59,64,65,66,67,68,69,70,71,80,81,82,83*/
};


extern sai_status_t mrvl_sai_create_virtual_router(_Out_ sai_object_id_t      *vr_id,
                                                   _In_ sai_object_id_t        switch_id,
                                                   _In_ uint32_t               attr_count,
                                                   _In_ const sai_attribute_t *attr_list);


/* TODO - find another solution for this WA */
uint32_t  *saiSysPortMappingPtr=SAI_SYS_PORT_MAPPING;

extern uint32_t fpaSysBridgingTblSize;

static sai_status_t mrvl_initialize_switch(_In_ sai_object_id_t switch_id);


/* private functions implementation */
static void mrvl_sai_switch_object_id_to_str(_In_ sai_object_type_t type, _In_ sai_object_id_t id, _Out_ char *object_id_str)
{
    uint32_t switch_idx;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(id, type, &switch_idx)) {
        snprintf(object_id_str, MAX_KEY_STR_LEN, "invalid switch");
    } else {
        snprintf(object_id_str, MAX_KEY_STR_LEN, "switch %u", switch_idx);
    }
}

static sai_status_t mrvl_sai_switch_notify_fn_set_prv(_In_ const sai_object_key_t      *key,
                                               _In_ const sai_attribute_value_t *value,
                                               void                             *arg)
{
    long attr_id = (long)arg;

    MRVL_SAI_LOG_ENTER();

    switch (attr_id) {
    case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
        mrvl_sai_notification_callbacks.on_switch_state_change = (sai_switch_state_change_notification_fn)value->ptr;
        break;

    case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
        mrvl_sai_notification_callbacks.on_switch_shutdown_request = (sai_switch_shutdown_request_notification_fn)value->ptr;
        break;

    case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
        mrvl_sai_notification_callbacks.on_fdb_event = (sai_fdb_event_notification_fn)value->ptr;
        break;

    case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
        mrvl_sai_notification_callbacks.on_port_state_change = (sai_port_state_change_notification_fn)value->ptr;
        break;

    case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
        mrvl_sai_notification_callbacks.on_packet_event = (sai_packet_event_notification_fn)value->ptr;
        break;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_switch_notify_fn_get_prv(_In_ const sai_object_key_t   *key,
                                               _Inout_ sai_attribute_value_t *value,
                                               _In_ uint32_t                  attr_index,
                                               _Inout_ vendor_cache_t        *cache,
                                               void                          *arg)
{
    long attr_id = (long)arg;

    MRVL_SAI_LOG_ENTER();

    switch (attr_id) {
    case SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY:
        value->ptr = mrvl_sai_notification_callbacks.on_switch_state_change;
        break;

    case SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY:
        value->ptr = mrvl_sai_notification_callbacks.on_switch_shutdown_request;
        break;

    case SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY:
        value->ptr = mrvl_sai_notification_callbacks.on_fdb_event;
        break;

    case SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY:
        value->ptr = mrvl_sai_notification_callbacks.on_port_state_change;
        break;

    case SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY:
        value->ptr = mrvl_sai_notification_callbacks.on_packet_event;
        break;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* set Switch init[sai_bool_t]
 */
static sai_status_t mrvl_sai_switch_init_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg)
{

    MRVL_SAI_LOG_ENTER();

    if (value->booldata == true){
        if (true == mrvl_switch_is_created)
        {
            MRVL_SAI_LOG_ERR("Switch already created - call remove_switch first\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_ALREADY_EXISTS);
        }
        mrvl_initialize_switch(key->key.object_id);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
static sai_status_t mrvl_sai_switch_init_get_prv(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{

    MRVL_SAI_LOG_ENTER();

    value->booldata = mrvl_switch_is_created;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* set profile id [sai_u32_t]
 */
static sai_status_t mrvl_sai_switch_profile_id_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg)
{

    MRVL_SAI_LOG_ENTER();

    mrvl_profile_id = value->u32;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
static sai_status_t mrvl_sai_switch_profile_id_get_prv(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{

    MRVL_SAI_LOG_ENTER();

    value->u32 = mrvl_profile_id;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Switching mode [sai_switch_switching_mode_t]
 *  (default to SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD) */
static sai_status_t mrvl_sai_switch_mode_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    MRVL_SAI_LOG_ENTER();

    switch (value->s32) {
    case SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD:
        break;

    case SAI_SWITCH_SWITCHING_MODE_CUT_THROUGH:
    default:
         MRVL_SAI_LOG_ERR("Invalid switching mode value %d\n", value->s32);
         MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/* Dynamic FDB entry aging time in seconds [uint32_t]
 *   Zero means aging is disabled.
 *  (default to zero)
 */
static sai_status_t mrvl_sai_switch_aging_time_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg)
{
    FPA_STATUS  fpa_status;
    MRVL_SAI_LOG_ENTER();
    fpa_status = fpaLibSwitchAgingTimeoutSet(SAI_DEFAULT_ETH_SWID_CNS, value->u32);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_DBG("fpaLibSwitchAgingTimeoutSet failed, timeout %d, status %d\n", value->u32, fpa_status);
        MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_FAILURE;
    }
    mrvl_sai_switch_aging_time =  value->u32;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/* The SDK can
 * 1 - Read the counters directly from HW (or)
 * 2 - Cache the counters in SW. Caching is typically done if
 * retrieval of counters directly from HW for each counter
 * read is CPU intensive
 * This setting can be used to
 * 1 - Move from HW based to SW based or Vice versa
 * 2 - Configure the SW counter cache refresh rate
 * Setting a value of 0 enables direct HW based counter read. A
 * non zero value enables the SW cache based and the counter
 * refresh rate.
 * A NPU may support both or one of the option. It would return
 * error for unsupported options. [uint32_t]
 */
static sai_status_t mrvl_sai_switch_counter_refresh_set_prv(_In_ const sai_object_key_t      *key,
                                             _In_ const sai_attribute_value_t *value,
                                             void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}


/* set Switch source MAC address [sai_mac_t]
 */
static sai_status_t mrvl_sai_switch_src_mac_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg)
{
    FPA_MAC_ADDRESS_STC src_mac;
    FPA_STATUS          fpa_status;
    char           value_str[MAX_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    memcpy(src_mac.addr, value->mac, FPA_MAC_ADDRESS_SIZE);
    fpa_status = fpaLibSwitchSrcMacAddressSet(SAI_DEFAULT_ETH_SWID_CNS, SAI_SWITCH_DEFAULT_MAC_MODE_CNS, &src_mac);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_DBG("fpaLibSwitchSrcMacAddressSet failed, mode %d, status %d\n", SAI_SWITCH_DEFAULT_MAC_MODE_CNS, fpa_status);
    	MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_FAILURE;
    }
    mrvl_sai_utl_value_to_str(*value, SAI_ATTR_VAL_TYPE_MAC, MAX_VALUE_STR_LEN, value_str);
    MRVL_SAI_LOG_DBG("mrvl_sai_switch_src_mac_set_prv: Set SAI_SWITCH_ATTR_SRC_MAC_ADDRESS: %s\n", value_str); 
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* The number of ports on the switch [uint32_t] */
static sai_status_t mrvl_sai_switch_port_number_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_MAX_NUM_OF_PORTS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get the port list [sai_object_list_t] */
static sai_status_t mrvl_sai_switch_port_list_get_prv(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{
    sai_status_t status;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    if (value->objlist.count < SAI_MAX_NUM_OF_PORTS) {
        value->objlist.count = SAI_MAX_NUM_OF_PORTS;
        MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    value->objlist.count = SAI_MAX_NUM_OF_PORTS;
    for (i=0; i < SAI_MAX_NUM_OF_PORTS; i++) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, saiSysPortMappingPtr[i], &value->objlist.list[i])))
        {
            MRVL_SAI_LOG_DBG("Failed to create port list, for port %d\n", saiSysPortMappingPtr[i]);
        	MRVL_SAI_LOG_EXIT();
        	return status;
        }
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get the CPU Port [sai_object_id_t] */
static sai_status_t mrvl_sai_switch_cpu_port_get_prv(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, SAI_CPU_PORT_CNS, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Max number of virtual routers supported [uint32_t] */
static sai_status_t mrvl_sai_switch_max_vr_get_prv(_In_ const sai_object_key_t   *key,
                                    _Inout_ sai_attribute_value_t *value,
                                    _In_ uint32_t                  attr_index,
                                    _Inout_ vendor_cache_t        *cache,
                                    void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_SWITCH_MAX_VR_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* The size of the FDB Table in bytes [uint32_t] */
static sai_status_t mrvl_sai_switch_fdb_size_get_prv(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = fpaSysBridgingTblSize;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Default SAI VLAN ID [sai_object_id_t] */
static sai_status_t mrvl_sai_switch_default_vlan_id_get_prv(_In_ const sai_object_key_t   *key,
                                                             _Inout_ sai_attribute_value_t *value,
                                                             _In_ uint32_t                  attr_index,
                                                             _Inout_ vendor_cache_t        *cache,
                                                             void                          *arg)
{
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, 1, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/* Default SAI STP instance ID [sai_object_id_t] */
static sai_status_t mrvl_sai_switch_default_stp_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP, 1, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Default SAI Virtual Router ID [sai_object_id_t]
 * Must return SAI_STATUS_OBJECT_IN_USE when try to delete this VR ID.
 */
static sai_status_t mrvl_sai_switch_default_vr_id_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/* SAI_SWITCH_ATTR_INGRESS_ACL and SAI_SWITCH_ATTR_EGRESS_ACL [sai_object_id_t]
 * 
 */

sai_status_t mrvl_sai_switch_acl_set_prv(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       _In_ void                        *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
     uint32_t switch_idx;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_SWITCH, &switch_idx))) {
        return status;
    }

    if (switch_idx != SAI_DEFAULT_ETH_SWID_CNS){
        MRVL_SAI_LOG_ERR("Invalid switch %d\n", switch_idx);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (value->oid == SAI_NULL_OBJECT_ID){
    	/* unbind action */
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_unbind_from_switch(arg, switch_idx))){
            MRVL_SAI_LOG_ERR("Unable to unbind switch %d from ACL TABLE\n", switch_idx);
            return SAI_STATUS_INVALID_PARAMETER;
    	}
    }
    else {
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_bind_to_switch(arg, value->oid, switch_idx))){
            MRVL_SAI_LOG_ERR("Unable to bind switch %d to ACL TABLE\n", switch_idx);
            return SAI_STATUS_INVALID_PARAMETER;
    	}
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}


static sai_status_t mrvl_sai_switch_acl_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t status;
    uint32_t switch_idx;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_SWITCH, &switch_idx))) {
        return status;
    }

    if (switch_idx != SAI_DEFAULT_ETH_SWID_CNS){
        MRVL_SAI_LOG_ERR("Invalid switch %d\n", switch_idx);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_get_table_id_per_switch(arg, switch_idx, value))){
        MRVL_SAI_LOG_ERR("Unable to get assigned ACL table per switch %d\n", switch_idx);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/*
 *   Local subnet routing supported [bool]
 *   Routes with next hop set to "on-link"
 */
static sai_status_t mrvl_sai_switch_on_link_get_prv(_In_ const sai_object_key_t   *key,
                                     _Inout_ sai_attribute_value_t *value,
                                     _In_ uint32_t                  attr_index,
                                     _Inout_ vendor_cache_t        *cache,
                                     void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->booldata = true;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Oper state [sai_switch_oper_status_t] */
static sai_status_t mrvl_sai_switch_oper_status_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->s32 = SAI_SWITCH_OPER_STATUS_UP;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* minimum priority for ACL table [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_table_min_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ACL_MIN_PRIORITY_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* maximum priority for ACL table [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_table_max_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ACL_MAX_PRIORITY_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* minimum priority for ACL entry [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_entry_min_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ACL_MIN_PRIORITY_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* maximum priority for ACL entry [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_entry_max_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ACL_MAX_PRIORITY_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Switching mode [sai_switch_switching_mode_t]
 *  (default to SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD) */
static sai_status_t mrvl_sai_switch_mode_get_prv(_In_ const sai_object_key_t   *key,
                                  _Inout_ sai_attribute_value_t *value,
                                  _In_ uint32_t                  attr_index,
                                  _Inout_ vendor_cache_t        *cache,
                                  void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->s32 = SAI_SWITCH_SWITCHING_MODE_STORE_AND_FORWARD;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Default switch MAC Address [sai_mac_t] */
static sai_status_t mrvl_sai_switch_src_mac_get_prv(_In_ const sai_object_key_t   *key,
                                     _Inout_ sai_attribute_value_t *value,
                                     _In_ uint32_t                  attr_index,
                                     _Inout_ vendor_cache_t        *cache,
                                     void                          *arg)
{
    uint32_t            mode;
    FPA_MAC_ADDRESS_STC src_mac;
    FPA_STATUS          fpa_status;

    MRVL_SAI_LOG_ENTER();
    fpa_status = fpaLibSwitchSrcMacAddressGet(SAI_DEFAULT_ETH_SWID_CNS, &mode, &src_mac);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_DBG("fpaLibSwitchSrcMacAddressGet failed, status %d\n", fpa_status);
    	MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_FAILURE;
    }
    memcpy(value->mac, src_mac.addr, FPA_MAC_ADDRESS_SIZE);
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Dynamic FDB entry aging time in seconds [uint32_t]
 *   Zero means aging is disabled.
 *  (default to zero)
 */
static sai_status_t mrvl_sai_switch_aging_time_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    FPA_STATUS          fpa_status;
    MRVL_SAI_LOG_ENTER();
    fpa_status = fpaLibSwitchAgingTimeoutGet(SAI_DEFAULT_ETH_SWID_CNS, &value->u32);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_DBG("fpaLibSwitchAgingTimeoutGet failed, status %d\n", fpa_status);
    	MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_FAILURE;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* The SDK can
 * 1 - Read the counters directly from HW (or)
 * 2 - Cache the counters in SW. Caching is typically done if
 * retrieval of counters directly from HW for each counter
 * read is CPU intensive
 * This setting can be used to
 * 1 - Move from HW based to SW based or Vice versa
 * 2 - Configure the SW counter cache refresh rate
 * Setting a value of 0 enables direct HW based counter read. A
 * non zero value enables the SW cache based and the counter
 * refresh rate.
 * A NPU may support both or one of the option. It would return
 * error for unsupported options. [uint32_t]
 */
static sai_status_t mrvl_sai_switch_counter_refresh_get_prv(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             void                          *arg)
{
#ifndef DEMO
	MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
#endif
    return SAI_STATUS_NOT_IMPLEMENTED;
}

/* Get the Max MTU in bytes [uint32_t]
 */
static sai_status_t mrvl_sai_switch_max_mtu_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_MAX_MTU_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The L3 Host Table size [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_l3_neighbor_size_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_NEIGHBOR_TABLE_SIZE_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The L3 route Table size [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_l3_route_table_size_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ROUTE_TABLE_SIZE_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/* Get The Number of ports that can be part of a LAG [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_lag_member_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_LAG_MAX_MEMBERS_IN_GROUP_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Number of LAGs that can be created [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_num_of_lags_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_LAG_MAX_GROUPS_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The ECMP number of members per group [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_ecmp_members_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ECMP_MAX_MEMBERS_IN_GROUP_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/* Get The ECMP number of group [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_num_of_ecmp_groups_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_ECMP_MAX_GROUPS_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch number of Unicast Queues per port [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_num_of_unicast_queues_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch number of Multicast Queues per port [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_num_of_multicast_queues_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch total number of Queues per port [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_num_of_queues_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_QUEUES_PER_PORT_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch number of CPU Queues [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_num_of_cpu_queues_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_QUEUES_PER_PORT_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch Maximum traffic classes limit [sai_uint8_t]
 */
static sai_status_t mrvl_sai_switch_max_tc_classes_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u8 = SAI_QUEUES_PER_PORT_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch Maximum Number of Hierarchy scheduler [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_max_num_sched_group_hierarchy_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch Maximum number of scheduler groups on each Hierarchy [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_max_num_sched_group_per_hierarchy_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch Maximum number of childs supported per scheudler group [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_max_num_childs_per_sched_group_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch total buffer size in KB [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_total_buf_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = SAI_TOTAL_BUFFER_SIZE_KB_CNS;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch number of ingress buffer pool [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_ingress_buf_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch number of egress buffer pool [sai_uint32_t]
 */
static sai_status_t mrvl_sai_switch_egress_buf_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    /*TODO*/
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get The Switch Default trap group [sai_object_id_t]
 */
static sai_status_t mrvl_sai_switch_default_trap_group_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP, 0, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/*  Get the Switch hash object for packets going through ECMP[sai_object_id_t] */
static sai_status_t mrvl_sai_switch_ecmp_hash_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HASH, 1, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/*  Get the Switch hash object for packets going through LAG [sai_object_id_t] */
static sai_status_t mrvl_sai_switch_lag_hash_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HASH, 1, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/** SAI ECMP default hash algorithm [sai_hash_algorithm_t]
 *  (default to SAI_HASH_ALGORITHM_CRC) */
static sai_status_t mrvl_sai_switch_ecmp_hash_algorithm_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->s32 = mrvl_sai_switch_ecmp_hash_algorithm;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/** SAI ECMP default hash algorithm [sai_hash_algorithm_t]
 *  (default to SAI_HASH_ALGORITHM_CRC) */
static sai_status_t mrvl_sai_switch_ecmp_hash_algorithm_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg)
{

    MRVL_SAI_LOG_ENTER();
    if ((value->s32 != SAI_HASH_ALGORITHM_CRC) &&
        (value->s32 != SAI_HASH_ALGORITHM_XOR) &&
        (value->s32 != SAI_HASH_ALGORITHM_RANDOM)){
    	MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_FAILURE;
    }
    mrvl_sai_switch_ecmp_hash_algorithm = value->s32;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

    /** SAI ECMP default hash seed [sai_uint32_t] (default to 0) */
sai_status_t mrvl_sai_switch_ecmp_hash_seed_get_prv(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* ECMP hashing seed  [uint32_t] */
sai_status_t mrvl_sai_switch_ecmp_hash_seed_set_prv(_In_ const sai_object_key_t      *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

    /** SAI ECMP default symmetric hash [bool] (default to false)
    *   When set, the hash calculation will result in the same value as when the source and
    *   destination addresses (L2 src/dst mac,L3 src/dst ip,L4 src/dst port) were swapped,
    *   ensuring the same conversation will result in the same hash value.
    */
sai_status_t mrvl_sai_switch_ecmp_hash_symmetric_get_prv(_In_ const sai_object_key_t   *key,
                                                         _Inout_ sai_attribute_value_t *value,
                                                         _In_ uint32_t                  attr_index,
                                                         _Inout_ vendor_cache_t        *cache,
                                                         void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = false;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

    /** SAI ECMP default symmetric hash [bool] (default to false)
    *   When set, the hash calculation will result in the same value as when the source and
    *   destination addresses (L2 src/dst mac,L3 src/dst ip,L4 src/dst port) were swapped,
    *   ensuring the same conversation will result in the same hash value.
    */
sai_status_t mrvl_sai_switch_ecmp_hash_symmetric_set_prv(_In_ const sai_object_key_t      *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

    /** SAI LAG default hash seed [sai_uint32_t] (default to 0) */
sai_status_t mrvl_sai_switch_lag_hash_seed_get_prv(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u32 = 0;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/* LAG hashing seed  [uint32_t] */
sai_status_t mrvl_sai_switch_lag_hash_seed_set_prv(_In_ const sai_object_key_t      *key,
                                                   _In_ const sai_attribute_value_t *value,
                                                   void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}
sai_status_t mrvl_sai_switch_qos_map_id_get_prv(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{
    sai_qos_map_type_t qos_map_type = (sai_qos_map_type_t)arg;
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();
    /*assert(qos_map_type < SAI_QOS_MAP_TYPES_MAX);*/
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_QOS_MAP, 1, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
static sai_status_t mrvl_sai_switch_default_1q_bridge_id_get_prv(_In_ const sai_object_key_t   *key,
                                                                 _Inout_ sai_attribute_value_t *value,
                                                                 _In_ uint32_t                  attr_index,
                                                                 _Inout_ vendor_cache_t        *cache,
                                                                 void                          *arg)
{
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, 1, &value->oid))) {
    	MRVL_SAI_LOG_EXIT();
        return status;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/****************************************************************/
static const sai_attribute_entry_t        switch_attribs[] = {
    /****** read only ******/
    { SAI_SWITCH_ATTR_PORT_NUMBER, false, false, false, true,
      "Switch ports number", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_PORT_LIST, false, false, false, true,
      "Switch ports list", SAI_ATTR_VAL_TYPE_OBJLIST },
    { SAI_SWITCH_ATTR_PORT_MAX_MTU, false, false, false, true,
      "Switch port max mtu", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_CPU_PORT, false, false, false, true,
      "Switch CPU port", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS, false, false, false, true,
      "Switch max virtual routers", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_FDB_TABLE_SIZE, false, false, false, true,
      "Switch FDB table size", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_L3_NEIGHBOR_TABLE_SIZE, false, false, false, true,
      "Switch L3 Host Table size", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_L3_ROUTE_TABLE_SIZE, false, false, false, true,
      "Switch L3 route Table size", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_LAG_MEMBERS, false, false, false, true,
      "Switch Number of ports that can be part of a LAG", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_NUMBER_OF_LAGS, false, false, false, true,
      "Switch Number of LAGs that can be created", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ECMP_MEMBERS, false, false, false, true,
      "Switch ECMP number of members per group", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS, false, false, false, true,
      "Switch ECMP number of group", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES, false, false, false, true,
      "Switch number of Unicast Queues per port", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES, false, false, false, true,
      "Switch number of Multicast Queues per port", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_NUMBER_OF_QUEUES, false, false, false, true,
      "Switch total number of Queues per port", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES, false, false, false, true,
      "Switch number of CPU Queues", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED, false, false, false, true,
      "Switch on link route supported", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_OPER_STATUS, false, false, false, true,
      "Switch operational status", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_MAX_TEMP, false, false, false, true,
      "Switch maximum temperature", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY, false, false, false, true,
      "Switch ACL table min priority", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY, false, false, false, true,
      "Switch ACL table max priority", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY, false, false, false, true,
      "Switch ACL entry min priority", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY, false, false, false, true,
      "Switch ACL entry max priority", SAI_ATTR_VAL_TYPE_U32 },

    /*METADATA not supported */
    { SAI_SWITCH_ATTR_DEFAULT_VLAN_ID, false, false, false, true,
      "Switch Default VLAN ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID, false, false, false, true,
      "Switch Default SAI STP instance ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID, false, false, false, true,
      "Switch Default SAI Virtual Router ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID, false, false, false, true,
      "Switch Default .1Q bridge ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_INGRESS_ACL, false, false, false, true,
      "Switch bind to ingress acl", SAI_ATTR_VAL_TYPE_OID },
     { SAI_SWITCH_ATTR_EGRESS_ACL, false, true, true, true,
      "Switch bind to egress acl", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES, false, false, false, true,
      "Switch Maximum traffic classes limit", SAI_ATTR_VAL_TYPE_U8 },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS, false, false, false, true,
      "Switch Maximum Number of Hierarchy scheduler", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL, false, false, false, true,
      "Switch Maximum number of scheduler groups on each Hierarchy", SAI_ATTR_VAL_TYPE_U32LIST },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP, false, false, false, true,
      "Switch Maximum number of childs supported per scheudler group", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE, false, false, false, true,
      "Switch total buffer size in KB", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM, false, false, false, true,
      "Switch number of ingress buffer pool", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM, false, false, false, true,
      "Switch number of egress buffer pool", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP, false, false, false, true,
      "Switch Default trap group", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_ECMP_HASH, false, false, false, true,
      "Switch hash object for packets going through ECMP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_LAG_HASH, false, false, false, true,
      "Switch hash object for packets going through LAG", SAI_ATTR_VAL_TYPE_OID },

    /****** read write ******/
    { SAI_SWITCH_ATTR_SWITCHING_MODE, false, false, true, true,
      "Switch switching mode", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE, false, false, true, true,
      "Switch broadcast flood control to cpu", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE, false, false, true, true,
      "Switch multicast flood control to cpu", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_SRC_MAC_ADDRESS, false, false, true, true,
      "Switch source MAC address", SAI_ATTR_VAL_TYPE_MAC },
    { SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES, false, false, true, true,
      "Switch maximum number of learned MAC addresses", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_FDB_AGING_TIME, false, false, true, true,
      "Switch FDB aging time", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION, false, false, true, true,
      "Switch flood control for unknown unicast address", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION, false, false, true, true,
      "Switch flood control for unknown broadcast address", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION, false, false, true, true,
      "Switch flood control for unknown multicast address", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM, false, false, true, true,
      "Switch ECMP default hash algorithm", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED, false, false, true, true,
      "Switch ECMP default hash seed", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH, false, false, true, true,
      "Switch ECMP default symmetric hash", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_ECMP_HASH_IPV4, false, false, true, true,
      "Switch hash object for IPv4 packets going through ECMP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4, false, false, true, true,
      "Switch hash object for IPv4 in IPv4 packets going through ECMP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_ECMP_HASH_IPV6, false, false, true, true,
      "Switch hash object for IPv6 packets going through ECMP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM, false, false, true, true,
      "Switch LAG default hash algorithm", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED, false, false, true, true,
      "Switch LAG default hash seed", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH, false, false, true, true,
      "Switch LAG default symmetric hash", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV4, false, false, true, true,
      "Switch hash object for IPv4 packets going through LAG", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4, false, false, true, true,
      "Switch hash object for IPv4 in IPv4 packets going through LAG", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV6, false, false, true, true,
      "Switch hash object for IPv4 packets going through LAG", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL, false, false, true, true,
      "Switch counter refresh interval", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_QOS_DEFAULT_TC, false, false, true, true,
      "Switch Default Traffic class value", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP, false, false, true, true,
      "Switch Enable DOT1P -> TC MAP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP, false, false, true, true,
      "Switch Enable DOT1P -> COLOR MAP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP, false, false, true, true,
      "Switch Enable DSCP -> TC MAP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP, false, false, true, true,
      "Switch Enable DSCP -> COLOR MAP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP, false, false, true, true,
      "Switch Enable TC -> Queue MAP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP, false, false, true, true,
      "Switch Enable TC + COLOR -> DOT1P MAP", SAI_ATTR_VAL_TYPE_OID },
    { SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP, false, false, true, true,
      "Switch Enable TC + COLOR -> DSCP MAP", SAI_ATTR_VAL_TYPE_OID },

    { SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE, false, false, false, false,
      "Switch shell enable", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_SWITCH_PROFILE_ID, false, true, false, true,
      "Switch profile id", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO, false, false, false, false,
      "Switch hardware info", SAI_ATTR_VAL_TYPE_U8LIST },
    { SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME, false, false, false, false,
      "Switch firmware pathname", SAI_ATTR_VAL_TYPE_U8LIST },
    { SAI_SWITCH_ATTR_INIT_SWITCH, true, true, true, true,
      "Switch init switch", SAI_ATTR_VAL_TYPE_BOOL },
    { SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY, false, true, true, false,
      "Switch state change notify", SAI_ATTR_VAL_TYPE_PTR },
    { SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY, false, true, true, false,
      "Switch shutdown request notify", SAI_ATTR_VAL_TYPE_PTR },
    { SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY, false, true, true, false,
      "Switch fdb event notify", SAI_ATTR_VAL_TYPE_PTR },
    { SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY, false, true, true, false,
      "Switch port state change notify", SAI_ATTR_VAL_TYPE_PTR },
    { SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY, false, true, true, false,
      "Switch packet event notify", SAI_ATTR_VAL_TYPE_PTR },

    /******* wrire only *********/
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }

};
static const sai_vendor_attribute_entry_t switch_vendor_attribs[] = {

    { SAI_SWITCH_ATTR_PORT_NUMBER,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_port_number_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_port_list_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_PORT_MAX_MTU,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_max_mtu_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_CPU_PORT,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_cpu_port_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_MAX_VIRTUAL_ROUTERS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_max_vr_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_FDB_TABLE_SIZE,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_fdb_size_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_L3_NEIGHBOR_TABLE_SIZE,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_l3_neighbor_size_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_L3_ROUTE_TABLE_SIZE,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_l3_route_table_size_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_MEMBERS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_lag_member_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_NUMBER_OF_LAGS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_num_of_lags_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_MEMBERS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_ecmp_members_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_NUMBER_OF_ECMP_GROUPS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_num_of_ecmp_groups_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_NUMBER_OF_UNICAST_QUEUES,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_num_of_unicast_queues_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_NUMBER_OF_MULTICAST_QUEUES,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_num_of_multicast_queues_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_NUMBER_OF_QUEUES,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_num_of_queues_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_NUMBER_OF_CPU_QUEUES,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_num_of_cpu_queues_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ON_LINK_ROUTE_SUPPORTED,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_on_link_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_OPER_STATUS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_oper_status_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_MAX_TEMP,
      { false, false, false, false },
      { false, false, false, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ACL_TABLE_MINIMUM_PRIORITY,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_acl_table_min_prio_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_acl_table_max_prio_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_acl_entry_min_prio_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_acl_entry_max_prio_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_DEFAULT_VLAN_ID,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_default_vlan_id_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_default_stp_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_default_vr_id_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_default_1q_bridge_id_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_INGRESS_ACL,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_acl_get_prv, (void*)SAI_SWITCH_ATTR_INGRESS_ACL,
      mrvl_sai_switch_acl_set_prv, (void*)SAI_SWITCH_ATTR_INGRESS_ACL },
    { SAI_SWITCH_ATTR_EGRESS_ACL,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_acl_get_prv, (void*)SAI_SWITCH_ATTR_EGRESS_ACL,
      mrvl_sai_switch_acl_set_prv, (void*)SAI_SWITCH_ATTR_EGRESS_ACL },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_TRAFFIC_CLASSES,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_max_tc_classes_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUP_HIERARCHY_LEVELS,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_max_num_sched_group_hierarchy_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_SCHEDULER_GROUPS_PER_HIERARCHY_LEVEL,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_max_num_sched_group_per_hierarchy_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_QOS_MAX_NUMBER_OF_CHILDS_PER_SCHEDULER_GROUP,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_max_num_childs_per_sched_group_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_TOTAL_BUFFER_SIZE,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_total_buf_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_INGRESS_BUFFER_POOL_NUM,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_ingress_buf_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_EGRESS_BUFFER_POOL_NUM,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_egress_buf_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_DEFAULT_TRAP_GROUP,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_default_trap_group_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_HASH,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_ecmp_hash_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_HASH,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_switch_lag_hash_get_prv, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_SWITCHING_MODE,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_mode_get_prv, NULL,
      mrvl_sai_switch_mode_set_prv, NULL },
    { SAI_SWITCH_ATTR_BCAST_CPU_FLOOD_ENABLE,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_MCAST_CPU_FLOOD_ENABLE,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_SRC_MAC_ADDRESS,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_src_mac_get_prv, NULL,
      mrvl_sai_switch_src_mac_set_prv, NULL },
    { SAI_SWITCH_ATTR_MAX_LEARNED_ADDRESSES,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_FDB_AGING_TIME,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_aging_time_get_prv, NULL,
      mrvl_sai_switch_aging_time_set_prv, NULL },
    { SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_PACKET_ACTION,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_PACKET_ACTION,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_ecmp_hash_algorithm_get_prv, NULL,
      mrvl_sai_switch_ecmp_hash_algorithm_set_prv, NULL },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_ecmp_hash_seed_get_prv, NULL,
      mrvl_sai_switch_ecmp_hash_seed_set_prv, NULL },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_ecmp_hash_symmetric_get_prv, NULL,
      mrvl_sai_switch_ecmp_hash_symmetric_set_prv, NULL },
    { SAI_SWITCH_ATTR_ECMP_HASH_IPV4,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_HASH_IPV4_IN_IPV4,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_HASH_IPV6,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_lag_hash_seed_get_prv, NULL,
      mrvl_sai_switch_lag_hash_seed_set_prv, NULL },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV4,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV6,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_COUNTER_REFRESH_INTERVAL,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_counter_refresh_get_prv, NULL,
      mrvl_sai_switch_counter_refresh_set_prv, NULL },


    { SAI_SWITCH_ATTR_QOS_DEFAULT_TC,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_default_tc_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_default_tc_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP,
      NULL/*mrvl_sai_switch_qos_dot1p_to_tc_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_DOT1P_TO_TC_MAP,
      NULL/*mrvl_sai_switch_qos_dot1p_to_color_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP,
      NULL/*mrvl_sai_switch_qos_dscp_to_tc_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP,
      NULL/*mrvl_sai_switch_qos_dscp_to_color_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP,
      NULL/*mrvl_sai_switch_qos_tc_to_queue_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,
      NULL/*mrvl_sai_switch_qos_tc_and_color_to_dot1p_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_sai_switch_qos_map_id_get_prv, (void*)SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,
      NULL/*mrvl_sai_switch_qos_tc_and_color_to_dscp_set_prv*/, NULL },


    { SAI_SWITCH_ATTR_SWITCH_SHELL_ENABLE,
      { false, false, true, true },
      { false, false, true, true },
      NULL/**/, NULL,
      NULL/**/, NULL },
    { SAI_SWITCH_ATTR_SWITCH_PROFILE_ID,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_profile_id_get_prv, NULL,
      mrvl_sai_switch_profile_id_set_prv, NULL },
    { SAI_SWITCH_ATTR_SWITCH_HARDWARE_INFO,
      { false, false, false, false },
      { false, false, false, false },
      NULL/**/, NULL,
      NULL/**/, NULL },
    { SAI_SWITCH_ATTR_FIRMWARE_PATH_NAME,
      { false, false, false, false },
      { false, false, false, false },
      NULL/**/, NULL,
      NULL/**/, NULL },
    { SAI_SWITCH_ATTR_INIT_SWITCH,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_init_get_prv, NULL,
      mrvl_sai_switch_init_set_prv, NULL },
    { SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_notify_fn_get_prv, NULL,
      mrvl_sai_switch_notify_fn_set_prv, NULL },
    { SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_notify_fn_get_prv, NULL,
      mrvl_sai_switch_notify_fn_set_prv, NULL },
    { SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_notify_fn_get_prv, NULL,
      mrvl_sai_switch_notify_fn_set_prv, NULL },
    { SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_notify_fn_get_prv, NULL,
      mrvl_sai_switch_notify_fn_set_prv, NULL },
    { SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY,
      { true, true, true, true },
      { true, true, true, true },
      mrvl_sai_switch_notify_fn_get_prv, NULL,
      mrvl_sai_switch_notify_fn_set_prv, NULL }
};

extern FPA_STATUS fpaWrapInitialize(IN void);


int port_status[SAI_MAX_NUM_OF_PORTS];

pthread_t thread_port;

void mrvl_sai_inform_all_ports_status_adv(void)
{
  sai_port_oper_status_notification_t port_data;
  sai_object_id_t port_id;
  int i;
  for ( i = 0; i < SAI_MAX_NUM_OF_PORTS; i++)
  {
	  port_data.port_state = (port_status[i] ) ? SAI_PORT_OPER_STATUS_UP : SAI_PORT_OPER_STATUS_DOWN;
	  mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, i, &port_id);
	  port_data.port_id = port_id;
	  mrvl_sai_notification_callbacks.on_port_state_change(1, &port_data);
  }
}

#include <unistd.h>

void * mrvl_sai_port_status_task(void * arg)
{
	int i, up;
	FPA_STATUS st;
	FPA_PORT_PROPERTIES_STC   properties;
	sai_object_id_t port_id;
	sai_port_oper_status_notification_t port_data;
	char extra_data[64];


	MRVL_SAI_LOG_ENTER();
	memset(&port_status, 0, sizeof(port_status));
	while (1)
	{
		for ( i = 0; i < SAI_MAX_NUM_OF_PORTS; i++)
		{
			properties.flags = ( FPA_PORT_PROPERTIES_STATE_FLAG );
			st =  fpaLibPortPropertiesGet(0, i, &properties);
			if ( st != FPA_OK)
			{
				up = 0;
			}
			else
			{
				up =!(properties.state & FPA_PORT_STATE_LINK_DOWN);
			}
			if (port_status[i] != up )
			{
				if ( up )
				{
					properties.flags = ( FPA_PORT_PROPERTIES_CURRSPEED_FLAG );
                    sprintf(extra_data, ",speed (%d)\n", properties.currSpeed);
				}
				else
				{
					sprintf(extra_data,"\n");
				}
                mrvl_sai_netdev_set_carrier(i,up);
				MRVL_SAI_LOG_INF("port %d status changed now %s %s", i, (up) ? "UP" : "Down", extra_data);
				port_status[i] = up;
				port_data.port_state = (up) ? SAI_PORT_OPER_STATUS_UP : SAI_PORT_OPER_STATUS_DOWN;
				mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, i, &port_id);
				port_data.port_id = port_id;
				if (mrvl_sai_notification_callbacks.on_port_state_change)
				{
					mrvl_sai_notification_callbacks.on_port_state_change(1, &port_data);
				}
			}
		}
		sleep(2);
	}
	MRVL_SAI_LOG_EXIT();
}
void mrvl_sai_simple_server(void);

/* switch initialization */
static sai_status_t mrvl_initialize_switch(_In_ sai_object_id_t switch_id)
{
	sai_object_id_t vr_id;
	sai_attribute_t attr_list[2];
    char          switch_str[MAX_LIST_VALUE_STR_LEN];
	FPA_STATUS fpa_status;
    uint32_t err, switch_idx;
    FPA_SRCMAC_LEARNING_MODE_ENT learning_mode = FPA_SRCMAC_LEARNING_AUTO_E;
    pthread_t t;
    FPA_MAC_ADDRESS_STC     defaultSrcMac = {{0,0,0,0x11,0x22,0x33}}, fpaSrcMac;
    pthread_t thread_console;
    sai_mac_t srcMac;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(switch_id, SAI_OBJECT_TYPE_SWITCH, &switch_idx)) {
        MRVL_SAI_LOG_ERR("invalid switch_idx %d\n",switch_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_switch_object_id_to_str(SAI_OBJECT_TYPE_SWITCH, switch_id, switch_str);
   
    if ( mrvl_sai_switch_init_first_time )
    {
        MRVL_SAI_LOG_NTC("Initialize %s for the first time\n", switch_str);

        fpa_status = fpaLibInit();
        if (fpa_status != FPA_OK){
        	MRVL_SAI_LOG_ERR("Error %d initializing switch !!\n", (int)fpa_status);
        	MRVL_SAI_LOG_EXIT();
          MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }
        fpa_status = fpaLibSwitchSrcMacLearningSet(switch_idx, learning_mode);
        if (fpa_status != FPA_OK){
             fprintf(stderr,  "Error %d initializing MAC learning mode !!\n", (int)fpa_status);
              MRVL_SAI_LOG_EXIT();
              MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }
        err = pthread_create(&t, NULL, (void *)mrvl_sai_fdb_wait_for_au_event, NULL);
        if (err != 0) {
          	MRVL_SAI_LOG_ERR("Error %d initializing Address Update task !!\n", (int)err);
          	MRVL_SAI_LOG_EXIT();
            MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
        }
        if (mrvl_sai_notification_callbacks.on_fdb_event!= NULL) {
        	MRVL_SAI_LOG_NTC("FDB event callback -accepted \n");
        }else {
            MRVL_SAI_LOG_WRN("No FDB event callback\n");
        }
        /* Get mgmt interface's MAC address \
           if found - set as switch source MAC address, aswell as to all ports \
           else - use default src MAC address */
        if ( mrvl_sai_netdev_get_mac("eth0", srcMac) == 0 )
        {
            memcpy(fpaSrcMac.addr, srcMac, sizeof(sai_mac_t));
            fpa_status = fpaLibSwitchSrcMacAddressSet(switch_idx, SAI_SWITCH_DEFAULT_MAC_MODE_CNS, &fpaSrcMac);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_EXIT();
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }

            MRVL_SAI_LOG_INF("fpaLibSwitchSrcMacAddressSet: setting eth0 MAC: %x:%x:%x:%x:%x:%x\n", fpaSrcMac.addr[0], fpaSrcMac.addr[1], fpaSrcMac.addr[2], 
                             fpaSrcMac.addr[3], fpaSrcMac.addr[4], fpaSrcMac.addr[5]);

        }
        else
        {
            fpa_status = fpaLibSwitchSrcMacAddressSet(switch_idx, SAI_SWITCH_DEFAULT_MAC_MODE_CNS, &defaultSrcMac);
            if (fpa_status != FPA_OK) {
                MRVL_SAI_LOG_EXIT();
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }
            MRVL_SAI_LOG_INF("fpaLibSwitchSrcMacAddressSet: setting default MAC: %x:%x:%x:%x:%x:%x\n", defaultSrcMac.addr[0], defaultSrcMac.addr[1], defaultSrcMac.addr[2], 
                             defaultSrcMac.addr[3], defaultSrcMac.addr[4], defaultSrcMac.addr[5]);
        }

    	if (mrvl_sai_route_init() != SAI_STATUS_SUCCESS){
    		MRVL_SAI_LOG_ERR("initialize sai route db failed\n");
    		MRVL_SAI_LOG_EXIT();
    		MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    	}

        if (mrvl_sai_acl_init() != SAI_STATUS_SUCCESS){
    		MRVL_SAI_LOG_ERR("initialize sai acl db failed\n");
    		MRVL_SAI_LOG_EXIT();
    		MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    	}

    	mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id);
    	attr_list[0].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
    	attr_list[0].value.booldata = 1;
        attr_list[1].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
    	attr_list[1].value.booldata = 1;
    	mrvl_sai_create_virtual_router(&vr_id, switch_id, 2, attr_list);
    }

	MRVL_SAI_LOG_NTC("Initialize switch\n");


	/* notifications */
    if (mrvl_sai_notification_callbacks.on_switch_state_change) {
    	mrvl_sai_notification_callbacks.on_switch_state_change(switch_idx, SAI_SWITCH_OPER_STATUS_UP);
    }

 	if (mrvl_sai_notification_callbacks.on_port_state_change) {
 		if ( mrvl_sai_switch_init_first_time ) {
 			memset(&port_status, 0, sizeof(port_status));
 			mrvl_sai_inform_all_ports_status_adv();
     		err = pthread_create(&thread_port, NULL, (void *)mrvl_sai_port_status_task, NULL);
		    if (err )
		    {
		    	MRVL_SAI_LOG_ERR("Created port status task thread Failed err %d \n", err);
		    }
 		}
 		else
 		{
 			mrvl_sai_inform_all_ports_status_adv();
 		}
 	}
    if ( mrvl_sai_switch_init_first_time )
    {
        err = pthread_create(&thread_console, NULL, (void *)mrvl_sai_simple_server, NULL);
        if (err )
        {
            MRVL_SAI_LOG_ERR("Created SAI simple server thread failed: err %d \n", err);
        }
	}

    MRVL_SAI_LOG_EXIT();
    mrvl_sai_switch_init_first_time = 0;
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Create switch
 *
 * SDK initialization/connect to SDK. After the call the capability attributes should be
 * ready for retrieval via sai_get_switch_attribute(). Same Switch Object id should be
 * given for create/connect for each NPU.
 *
 * @param[out] switch_id The Switch Object ID
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_switch(
        _Out_ sai_object_id_t *switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    sai_status_t  status; 
    char          list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *attr_val       = NULL;
    uint32_t                    attr_idx;

    MRVL_SAI_LOG_ENTER();

    if (NULL == switch_id) {
        MRVL_SAI_LOG_ERR("NULL switch_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, switch_attribs, switch_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, switch_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create switch, %s\n", list_str);

    if (true == mrvl_switch_is_created)
    {
    	MRVL_SAI_LOG_ERR("Switch already created - first call remove_switch\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    /* check mandatory attribute SAI_SWITCH_ATTR_INIT_SWITCH */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_INIT_SWITCH, &attr_val, &attr_idx));
    mrvl_switch_is_created = attr_val->booldata;

    /* SAI_SWITCH_ATTR_SWITCH_PROFILE_ID */
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_SWITCH_PROFILE_ID, &attr_val, &attr_idx))){
        mrvl_profile_id = attr_val->u32;
    }

    /* SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY */
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_SWITCH_STATE_CHANGE_NOTIFY, &attr_val, &attr_idx))){
        mrvl_sai_notification_callbacks.on_switch_state_change = (sai_switch_state_change_notification_fn)attr_val->ptr;
    }

    /* SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY */
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_SHUTDOWN_REQUEST_NOTIFY, &attr_val, &attr_idx))){
        mrvl_sai_notification_callbacks.on_switch_shutdown_request = (sai_switch_shutdown_request_notification_fn)attr_val->ptr;
    }

    /* SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY */
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY, &attr_val, &attr_idx))){
        mrvl_sai_notification_callbacks.on_fdb_event = (sai_fdb_event_notification_fn)attr_val->ptr;
    }

    
    /* SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY */
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_PORT_STATE_CHANGE_NOTIFY, &attr_val, &attr_idx))){
        mrvl_sai_notification_callbacks.on_port_state_change = (sai_port_state_change_notification_fn)attr_val->ptr;
    }

   /* SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY */
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_SWITCH_ATTR_PACKET_EVENT_NOTIFY, &attr_val, &attr_idx))){
        mrvl_sai_notification_callbacks.on_packet_event = (sai_packet_event_notification_fn)attr_val->ptr;
    }

    /* create SAI switch object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, switch_id))) {
        MRVL_SAI_API_RETURN(status);
    }
    if (mrvl_switch_is_created) {
        status = mrvl_initialize_switch(*switch_id);
    }


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);    

}
/**
 * @brief Remove/disconnect Switch
 *
 * Release all resources associated with currently opened switch
 *
 * @param[in] switch_id The Switch id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_switch(
        _In_ sai_object_id_t switch_id)
{
    uint32_t            switch_idx;       

    MRVL_SAI_LOG_ENTER();        
    MRVL_SAI_LOG_WRN("remove_switch\n");

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(switch_id, SAI_OBJECT_TYPE_SWITCH, &switch_idx)) {
        MRVL_SAI_LOG_ERR("invalid switch_idx %d\n",switch_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_switch_is_created = 0;
    memset(&mrvl_sai_notification_callbacks, 0, sizeof(sai_switch_notification_t));

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

/**
 * @brief Set switch attribute value
 *
 * @param[in] switch_id Switch id
 * @param[in] attr Switch attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_set_switch_attribute(
        _In_ sai_object_id_t switch_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = switch_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;
    uint32_t            switch_idx;  

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == switch_id) {
        MRVL_SAI_LOG_ERR("Invalid switch_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(switch_id, SAI_OBJECT_TYPE_SWITCH, &switch_idx)) {
        MRVL_SAI_LOG_ERR("invalid switch_idx %d\n",switch_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (switch_idx != SAI_DEFAULT_ETH_SWID_CNS){
        MRVL_SAI_LOG_ERR("non default switch_idx %d\n",switch_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_switch_object_id_to_str(SAI_OBJECT_TYPE_SWITCH, switch_id, key_str);
    MRVL_SAI_LOG_NTC("Set attributes for switch %s\n", key_str);

    status = mrvl_sai_utl_set_attribute(&key, key_str, switch_attribs, switch_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}

/**
 * @brief Get switch attribute value
 *
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of switch attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_switch_attribute(
        _In_ sai_object_id_t switch_id,
        _In_ sai_uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = switch_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status;
    uint32_t switch_idx;

    MRVL_SAI_LOG_ENTER();


    if (SAI_NULL_OBJECT_ID == switch_id) {
        MRVL_SAI_LOG_ERR("Invalid switch_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(switch_id, SAI_OBJECT_TYPE_SWITCH, &switch_idx)) {
        MRVL_SAI_LOG_ERR("invalid switch_idx %d\n",switch_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (switch_idx != SAI_DEFAULT_ETH_SWID_CNS){
        MRVL_SAI_LOG_ERR("non default switch_idx %d\n",switch_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_switch_object_id_to_str(SAI_OBJECT_TYPE_SWITCH, switch_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, switch_attribs, switch_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, switch_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Get Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}


const sai_switch_api_t switch_api = {
    mrvl_sai_create_switch,
    mrvl_sai_remove_switch,
    mrvl_sai_set_switch_attribute,
    mrvl_sai_get_switch_attribute

};
