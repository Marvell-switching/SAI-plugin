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
#include <pthread.h>

#undef  __MODULE__
#define __MODULE__ SAI_SWITCH

sai_switch_notification_t mrvl_sai_notification_callbacks;
uint32_t                  mrvl_sai_gh_sdk = 0;
uint32_t                  mrvl_sai_switch_aging_time = SAI_DEFAULT_FDB_AGING_TIME_CNS;

pthread_t mrvl_sai_au_thread;

/* TODO - find another solution for this WA */
extern uint32_t  *fpaSysPortMappingPtr;
extern uint32_t fpaSysBridgingTblSize;
extern uint32_t osTaskCreate
(
    IN  char    *name,
    IN  uint32_t  prio,
    IN  uint32_t  stack,
    IN  unsigned (*start_addr)(void*),
    IN  void    *arglist,
    OUT uint32_t *tid
);


/* Switching mode [sai_switch_switching_mode_t]
 *  (default to SAI_SWITCHING_MODE_STORE_AND_FORWARD) */
static sai_status_t mrvl_sai_switch_mode_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    MRVL_SAI_LOG_ENTER();

    switch (value->s32) {
    case SAI_SWITCHING_MODE_STORE_AND_FORWARD:
        break;

    case SAI_SWITCHING_MODE_CUT_THROUGH:
    default:
         MRVL_SAI_LOG_ERR("Invalid switching mode value %d\n", value->s32); 
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
    fpa_status = fpaLibSwitchAgingTimeoutSet(0 /*switchId*/, value->u32);
    if (fpa_status != FPA_OK) {
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

    MRVL_SAI_LOG_ENTER();
    memcpy(src_mac.addr, value->mac, FPA_MAC_ADDRESS_SIZE);  
    fpa_status = fpaLibSwitchSrcMacAddressSet(0 /*switchId*/, SAI_SWITCH_DEFAULT_MAC_MODE_CNS, &src_mac); 
    if (fpa_status != FPA_OK) {
        return SAI_STATUS_FAILURE;
    }
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
        return SAI_STATUS_BUFFER_OVERFLOW;
    }
    value->objlist.count = SAI_MAX_NUM_OF_PORTS; 
    for (i=0; i < SAI_MAX_NUM_OF_PORTS; i++) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, fpaSysPortMappingPtr[i], &value->objlist.list[i]))) {
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

/* Default SAI STP instance ID [sai_object_id_t] */
static sai_status_t mrvl_sai_switch_default_stp_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP_INSTANCE, 0, &value->oid))) {
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
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, 0, &value->oid))) {
        return status;
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

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

/* maximum priority for ACL table [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_table_max_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

/* minimum priority for ACL entry [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_entry_min_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

/* maximum priority for ACL entry [sai_uint32_t] */
static sai_status_t mrvl_sai_switch_acl_entry_max_prio_get_prv(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

/* Switching mode [sai_switch_switching_mode_t]
 *  (default to SAI_SWITCHING_MODE_STORE_AND_FORWARD) */
static sai_status_t mrvl_sai_switch_mode_get_prv(_In_ const sai_object_key_t   *key,
                                  _Inout_ sai_attribute_value_t *value,
                                  _In_ uint32_t                  attr_index,
                                  _Inout_ vendor_cache_t        *cache,
                                  void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_SWITCHING_MODE_STORE_AND_FORWARD;

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
    fpa_status = fpaLibSwitchSrcMacAddressGet(0 /*switchId*/, &mode, &src_mac); 
    if (fpa_status != FPA_OK) {
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
    fpa_status = fpaLibSwitchAgingTimeoutGet(0, &value->u32);
    if (fpa_status != FPA_OK) {
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
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
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
    value->u32 = 0;
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
    value->u32 = 0;
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
    value->u32 = 0;
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
    value->u32 = 0;
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
    value->u32 = 0;
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
    value->u32 = 0;
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
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_TRAP_GROUP, 0, &value->oid))) {
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
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HASH, 0, &value->oid))) {
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
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HASH, 0, &value->oid))) {
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
      "Switch ACL table min prio", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ACL_TABLE_MAXIMUM_PRIORITY, false, false, false, true,
      "Switch ACL table max prio", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY, false, false, false, true,
      "Switch ACL entry min prio", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_SWITCH_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY, false, false, false, true,
      "Switch ACL entry max prio", SAI_ATTR_VAL_TYPE_U32 },

    /*METADATA not supported */
    
    { SAI_SWITCH_ATTR_DEFAULT_STP_INST_ID, false, false, false, true,
      "Switch Default SAI STP instance ID", SAI_ATTR_VAL_TYPE_OID },    
    { SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID, false, false, false, true,
      "Switch Default SAI Virtual Router ID", SAI_ATTR_VAL_TYPE_OID },    
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
    { SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION, false, false, true, true,
      "Switch flood control for unknown unicast address", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION, false, false, true, true,
      "Switch flood control for unknown broadcast address", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION, false, false, true, true,
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
        
    /******* wrire only *********/
    { SAI_SWITCH_ATTR_PORT_BREAKOUT, false, false, true, false,
      "Switch port breakout mode", SAI_ATTR_VAL_TYPE_OID },
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
    { SAI_SWITCH_ATTR_FDB_UNICAST_MISS_ACTION,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_FDB_BROADCAST_MISS_ACTION,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_FDB_MULTICAST_MISS_ACTION,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },    
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_ECMP_DEFAULT_SYMMETRIC_HASH,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
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
    { SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_ALGORITHM,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_DEFAULT_SYMMETRIC_HASH,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV4,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_SWITCH_ATTR_LAG_HASH_IPV4_IN_IPV4,
      { false, false, false, false },
      { false, false, true, true },
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
      NULL/*mrvl_sai_switch_qos_dot1p_to_tc_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_dot1p_to_tc_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DOT1P_TO_COLOR_MAP,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_dot1p_to_color_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_dot1p_to_color_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DSCP_TO_TC_MAP,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_dscp_to_tc_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_dscp_to_tc_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_DSCP_TO_COLOR_MAP,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_dscp_to_color_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_dscp_to_color_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_TC_TO_QUEUE_MAP,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_tc_to_queue_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_tc_to_queue_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_tc_and_color_to_dot1p_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_tc_and_color_to_dot1p_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,
      { false, false, true, true },
      { false, false, true, true },
      NULL/*mrvl_sai_switch_qos_tc_and_color_to_dscp_get_prv*/, NULL,
      NULL/*mrvl_sai_switch_qos_tc_and_color_to_dscp_set_prv*/, NULL },
    { SAI_SWITCH_ATTR_PORT_BREAKOUT,
      { false, false, false, false },
      { false, false, true, true },
      NULL, NULL,
      NULL, NULL },

};

extern FPA_STATUS fpaWrapInitialize(IN void);

/*
 * Routine Description:
 *   SDK initialization. After the call the capability attributes should be
 *   ready for retrieval via sai_get_switch_attribute().
 *
 * Arguments:
 *   [in] profile_id - Handle for the switch profile.
 *   [in] switch_hardware_id - Switch hardware ID to open
 *   [in/opt] firmware_path_name - Vendor specific path name of the firmware
 *                                     to load
 *   [in] switch_notifications - switch notification table
 * Return Values:
 *   SAI_STATUS_SUCCESS on success
 *   Failure status code on error
 */
sai_status_t mrvl_sai_initialize_switch(_In_ sai_switch_profile_id_t                           profile_id,
                                    _In_reads_z_(SAI_MAX_HARDWARE_ID_LEN) char           * switch_hardware_id,
                                    _In_reads_opt_z_(SAI_MAX_FIRMWARE_PATH_NAME_LEN) char* firmware_path_name,
                                    _In_ sai_switch_notification_t                       * switch_notifications)
{
    FPA_STATUS fpa_status;
    uint32_t err;
    FPA_SRCMAC_LEARNING_MODE_ENT learning_mode = FPA_SRCMAC_LEARNING_AUTO_E;
    pthread_t t;

    if (NULL == switch_hardware_id) {
        fprintf(stderr, "NULL switch hardware ID passed to SAI switch initialize\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == switch_notifications) {
        fprintf(stderr, "NULL switch notifications passed to SAI switch initialize\n");

        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    if (true == mrvl_sai_gh_sdk)
    {
        fprintf(stderr, "Switch already initialized - first call shutdown_switch\n");
        return SAI_STATUS_FAILURE;
    }

    mrvl_sai_gh_sdk = 1;
    memcpy(&mrvl_sai_notification_callbacks, switch_notifications, sizeof(mrvl_sai_notification_callbacks));

    mrvl_sai_trace_init();

    fpa_status = fpaWrapInitialize();
    if (fpa_status != FPA_OK){
        fprintf(stderr,  "Error %d initializing switch !!\n", (int)fpa_status);
        return SAI_STATUS_FAILURE;
    }

	MRVL_SAI_LOG_NTC("Initialize switch\n");
    if (mrvl_sai_notification_callbacks.on_fdb_event!= NULL) {
        fpa_status = fpaLibSwitchSrcMacLearningSet(SAI_DEFAULT_ETH_SWID_CNS, learning_mode);
        if (fpa_status != FPA_OK){
            fprintf(stderr,  "Error %d initializing learning mode !!\n", (int)fpa_status);
            return SAI_STATUS_FAILURE;
        }
        err = pthread_create(&t, NULL, (void *)mrvl_sai_fdb_wait_for_au_event, NULL);
        if (err != 0) {
            fprintf(stderr,  "Error %d initializing Address Update task !!\n", (int)err);
            return SAI_STATUS_FAILURE;
        }        
    }else {
        MRVL_SAI_LOG_WRN("No FDB event callback\n");
    }
	if (mrvl_sai_route_init() != SAI_STATUS_SUCCESS){
		fprintf(stderr, "initialize sai route db failed\n");
		return SAI_STATUS_FAILURE;
	}
	
    return SAI_STATUS_SUCCESS; 
}

/*
 * Routine Description:
 *   Release all resources associated with currently opened switch
 *
 * Arguments:
 *   [in] warm_restart_hint - hint that indicates controlled warm restart.
 *                            Since warm restart can be caused by crash
 *                            (therefore there are no guarantees for this call),
 *                            this hint is really a performance optimization.
 *
 * Return Values:
 *   None
 */
void mrvl_sai_shutdown_switch(_In_ bool warm_restart_hint)
{
    MRVL_SAI_LOG_NTC("Shutdown switch\n");
    mrvl_sai_gh_sdk = 0;
    memset(&mrvl_sai_notification_callbacks, 0, sizeof(sai_switch_notification_t));

}

/*
 * Routine Description:
 *   SDK connect. This API connects library to the initialized SDK.
 *   After the call the capability attributes should be ready for retrieval
 *   via sai_get_switch_attribute().
 *
 * Arguments:
 *   [in] profile_id - Handle for the switch profile.
 *   [in] switch_hardware_id - Switch hardware ID to open
 *   [in] switch_notifications - switch notification table
 * Return Values:
 *   SAI_STATUS_SUCCESS on success
 *   Failure status code on error
 */
sai_status_t mrvl_sai_connect_switch(_In_ sai_switch_profile_id_t                profile_id,
                                 _In_reads_z_(SAI_MAX_HARDWARE_ID_LEN) char* switch_hardware_id,
                                 _In_ sai_switch_notification_t            * switch_notifications)
{
    if (NULL == switch_hardware_id) {
        fprintf(stderr, "NULL switch hardware ID passed to SAI switch connect\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == switch_notifications) {
        fprintf(stderr, "NULL switch notifications passed to SAI switch connect\n");

        return SAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(&mrvl_sai_notification_callbacks, switch_notifications, sizeof(mrvl_sai_notification_callbacks));

    /* Open an handle if not done already on init for init agent */
    if (0 == mrvl_sai_gh_sdk) {
#if 0
/*#ifndef _WIN32*/
        openlog("SAI", 0, LOG_USER);
#endif
    }

 /*   db_init_next_hop_group();*/

    MRVL_SAI_LOG_NTC("Connect switch\n");

    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *   Disconnect this SAI library from the SDK.
 *
 * Arguments:
 *   None
 * Return Values:
 *   None
 */
void mrvl_sai_disconnect_switch(void)
{
    MRVL_SAI_LOG_NTC("Disconnect switch\n");

    memset(&mrvl_sai_notification_callbacks, 0, sizeof(mrvl_sai_notification_callbacks));
}

/*
 * Routine Description:
 *    Set switch attribute value
 *
 * Arguments:
 *    [in] attr - switch attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_set_switch_attribute(_In_ const sai_attribute_t *attr)
{
    MRVL_SAI_LOG_ENTER();
    
    if (NULL == attr){
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return mrvl_sai_utl_set_attribute(NULL, "", switch_attribs, switch_vendor_attribs, attr);
}
/*
 * Routine Description:
 *    Get switch attribute value
 *
 * Arguments:
 *    [in] attr_count - number of switch attributes
 *    [inout] attr_list - array of switch attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_switch_attribute(_In_ uint32_t attr_count, _Inout_ sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ENTER();

    return mrvl_sai_utl_get_attributes(NULL, "", switch_attribs, switch_vendor_attribs, attr_count, attr_list);
}


const sai_switch_api_t switch_api = {
    mrvl_sai_initialize_switch,
    mrvl_sai_shutdown_switch,
    mrvl_sai_connect_switch,
    mrvl_sai_disconnect_switch,
    mrvl_sai_set_switch_attribute,
    mrvl_sai_get_switch_attribute,
};
