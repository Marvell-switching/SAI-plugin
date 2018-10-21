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
#define __MODULE__ SAI_PORT

#define SAI_PORT_SPEED_10M        10
#define SAI_PORT_SPEED_100M      100
#define SAI_PORT_SPEED_1G       1000
#define SAI_PORT_SPEED_2_5G     2500
#define SAI_PORT_SPEED_10G     10000
#define SAI_PORT_SPEED_20G     20000
#define SAI_PORT_SPEED_25G     25000
#define SAI_PORT_SPEED_40G     40000
#define SAI_PORT_SPEED_100G   100000
#define SAI_PORT_SPEED_1T    1000000
#define SAI_PORT_MAX_SPEEDS       10

static mrvl_port_info_t ports_db[SAI_MAX_NUM_OF_PORTS];

sai_status_t mrvl_port_state_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg);

sai_status_t mrvl_port_state_set(_In_ const sai_object_key_t *key,
                                 _In_ const sai_attribute_value_t *value,
                                 void *arg);

sai_status_t mrvl_port_hw_lane_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg);


sai_status_t mrvl_port_supported_speed_get(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg);

sai_status_t mrvl_port_speed_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg);

sai_status_t mrvl_port_speed_set(_In_ const sai_object_key_t      *key, 
                                 _In_ const sai_attribute_value_t *value, 
                                 void                             *arg);

sai_status_t mrvl_port_fc_get(_In_ const sai_object_key_t   *key,
                              _Inout_ sai_attribute_value_t *value,
                              _In_ uint32_t                  attr_index,
                              _Inout_ vendor_cache_t        *cache,
                              void                          *arg);

sai_status_t mrvl_port_fc_set(_In_ const sai_object_key_t      *key, 
                              _In_ const sai_attribute_value_t *value, 
                              void                             *arg);

sai_status_t mrvl_port_acl_binding_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       void                             *arg);

sai_status_t mrvl_port_acl_binding_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg);

sai_status_t mrvl_port_qos_num_queues_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);

sai_status_t mrvl_port_qos_queue_list_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);

sai_status_t mrvl_port_number_of_priority_groups_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);

sai_status_t mrvl_port_priority_group_list_get(_In_ const sai_object_key_t   *key,
                                               _Inout_ sai_attribute_value_t *value,
                                               _In_ uint32_t                  attr_index,
                                               _Inout_ vendor_cache_t        *cache,
                                               void                          *arg);

sai_status_t mrvl_port_sched_groups_num_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg);
    
sai_status_t mrvl_port_sched_groups_list_get(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             void                          *arg);

sai_status_t mrvl_port_internal_loopback_mode_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg);

sai_status_t mrvl_port_internal_loopback_mode_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg);

sai_status_t mrvl_port_mtu_get(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg);

sai_status_t mrvl_port_mtu_set(_In_ const sai_object_key_t      *key,
                               _In_ const sai_attribute_value_t *value,
                               void                             *arg);

sai_status_t mrvl_port_fec_get(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg);
    
sai_status_t mrvl_port_fec_set(_In_ const sai_object_key_t      *key,
                               _In_ const sai_attribute_value_t *value,
                               void                             *arg);
    
sai_status_t mrvl_port_auto_negotiation_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg);

sai_status_t mrvl_port_auto_negotiation_set(_In_ const sai_object_key_t      *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void                             *arg);
    
sai_status_t mrvl_port_type_get(_In_ const sai_object_key_t   *key,
                                _Inout_ sai_attribute_value_t *value,
                                _In_ uint32_t                  attr_index,
                                _Inout_ vendor_cache_t        *cache,
                                void                          *arg);

sai_status_t mrvl_port_supported_breakout_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);

sai_status_t mrvl_port_current_breakout_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg);

sai_status_t mrvl_port_supported_fec_mode_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);

sai_status_t mrvl_port_duplex_mode_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg);

sai_status_t mrvl_port_update_dscp_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg);

sai_status_t mrvl_port_update_dscp_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       void                             *arg);

sai_status_t mrvl_port_qos_default_tc_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);

sai_status_t mrvl_port_qos_default_tc_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);

sai_status_t mrvl_port_qos_map_id_get(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg);

sai_status_t mrvl_port_qos_map_id_set(_In_ const sai_object_key_t      *key,
                                      _In_ const sai_attribute_value_t *value,
                                      void                             *arg);

sai_status_t mrvl_port_mirror_session_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);

sai_status_t mrvl_port_mirror_session_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);

sai_status_t mrvl_port_samplepacket_session_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg);

sai_status_t mrvl_port_samplepacket_session_set(_In_ const sai_object_key_t      *key,
                                                _In_ const sai_attribute_value_t *value,
                                                void                             *arg);
    
sai_status_t mrvl_port_pfc_control_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg);

sai_status_t mrvl_port_pfc_control_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       void                             *arg);

sai_status_t mrvl_port_sched_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg);

sai_status_t mrvl_port_sched_set(_In_ const sai_object_key_t      *key,
                                 _In_ const sai_attribute_value_t *value,
                                 void                             *arg);
    
sai_status_t mrvl_port_ingress_buffer_profile_list_get(_In_ const sai_object_key_t   *key,
                                                       _Inout_ sai_attribute_value_t *value,
                                                       _In_ uint32_t                  attr_index,
                                                       _Inout_ vendor_cache_t        *cache,
                                                       void                          *arg);

sai_status_t mrvl_port_ingress_buffer_profile_list_set(_In_ const sai_object_key_t      *key,
                                                       _In_ const sai_attribute_value_t *value,
                                                       void                             *arg);

sai_status_t mrvl_port_egress_buffer_profile_list_get(_In_ const sai_object_key_t   *key,
                                                      _Inout_ sai_attribute_value_t *value,
                                                      _In_ uint32_t                  attr_index,
                                                      _Inout_ vendor_cache_t        *cache,
                                                      void                          *arg);
sai_status_t mrvl_port_egress_buffer_profile_list_set(_In_ const sai_object_key_t      *key,
                                                      _In_ const sai_attribute_value_t *value,
                                                      void                             *arg);

sai_status_t mrvl_port_storm_control_policer_set(_In_ const sai_object_key_t      *key,
                                                      _In_ const sai_attribute_value_t *value,
                                                      _In_ void                        *arg);
    
sai_status_t mrvl_port_storm_control_policer_get(_In_ const sai_object_key_t   *key,
                                                      _Inout_ sai_attribute_value_t *value,
                                                      _In_ uint32_t                  attr_index,
                                                      _Inout_ vendor_cache_t        *cache,
                                                      _In_ void                     *arg);

sai_status_t mrvl_port_egress_block_set(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        _In_ void                        *arg);

sai_status_t mrvl_port_egress_block_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        _In_ void                     *arg);

sai_status_t mrvl_port_pool_list_get(_In_ const sai_object_key_t   *key,
                                     _Inout_ sai_attribute_value_t *value,
                                     _In_ uint32_t                  attr_index,
                                     _Inout_ vendor_cache_t        *cache,
                                     _In_ void                     *arg);

sai_status_t mrvl_port_media_type_get(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg);

sai_status_t mrvl_port_media_type_set(_In_ const sai_object_key_t      *key,
                                      _In_ const sai_attribute_value_t *value,
                                      void                             *arg);

static const sai_attribute_entry_t        port_attribs[] = {
    { SAI_PORT_ATTR_TYPE, false, false, false, true,
        "Port type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_OPER_STATUS, false, false, false, true,
        "Port operational status", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE, false, false, false, true,
        "Port breakout mode(s) supported", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE, false, false, false, true,
        "Port current breakout mode", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES, false, false, false, true,
        "Port qos number of queues", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_PORT_ATTR_QOS_QUEUE_LIST, false, false, false, true,
        "Port qos queue list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS, false, false, false, true,
        "Port qos number of scheduled groups ", SAI_ATTR_VALUE_TYPE_UINT32},
    { SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST, false, false, false, true,
        "Port scheduler group list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_HW_LANE_LIST, false, false, false, true,
        "Port hw lane", SAI_ATTR_VALUE_TYPE_UINT32_LIST },
    { SAI_PORT_ATTR_SUPPORTED_SPEED, false, false, false, true,
        "Port supported speed", SAI_ATTR_VALUE_TYPE_UINT32_LIST },
    { SAI_PORT_ATTR_SUPPORTED_FEC_MODE, false, false, false, true,
        "Port support FEC mode", SAI_ATTR_VALUE_TYPE_INT32_LIST},
    { SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS, false, false, false, true,
        "Port number of ingress priority groups", SAI_ATTR_VALUE_TYPE_UINT32},
    { SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST, false, false, false, true,
        "Port ingress priority groups list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST},
    { SAI_PORT_ATTR_SPEED, false, false, true, true,
        "Port speed", SAI_ATTR_VALUE_TYPE_UINT32},
    { SAI_PORT_ATTR_FULL_DUPLEX_MODE, false, false, false, true,
        "Port full duplex settings", SAI_ATTR_VALUE_TYPE_BOOL},
    { SAI_PORT_ATTR_AUTO_NEG_MODE, false, false, false, true,
        "Port auto negotiation configuration", SAI_ATTR_VALUE_TYPE_BOOL},
    { SAI_PORT_ATTR_ADMIN_STATE, false, false, true, true,
        "Port admin state", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_PORT_ATTR_MEDIA_TYPE, false, false, false, true,
        "Port media type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_PORT_VLAN_ID, false, false, true, true,
        "Port VLAN ID", SAI_ATTR_VALUE_TYPE_UINT16 },
    { SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY, false, false, true, true,
        "Port default vlan priority", SAI_ATTR_VALUE_TYPE_UINT8 },
    { SAI_PORT_ATTR_DROP_UNTAGGED, false, false, false, true,
        "Port dropping of untagged frames (ingress)", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_PORT_ATTR_DROP_TAGGED, false, false, false, true,
        "Port dropping of tagged frames (ingress)", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE, false, false, false, true,
        "Port internal loopback mode", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_FEC_MODE, false, false, false, true,
        "Port FEC mode", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_UPDATE_DSCP, false, false, false, true,
        "Port update DSCP", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_PORT_ATTR_MTU, false, true, true, true,
        "Port MTU", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID, false, false, false, true,
        "Port flood storm control policer ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID, false, false, false, true,
        "Port broadcast storm control policer ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID, false, false, false, true,
        "Port multicast storm control policer ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE, false, false, true, true,
        "Port global flow control", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_PORT_ATTR_INGRESS_ACL, false, true, true, true,
        "Port bind point for ingress ACL objects", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_EGRESS_ACL, false, true, true, true,
        "Port bind point for egress ACL objects", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, false, false, false, true,
        "Port ingress mirror session", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_EGRESS_MIRROR_SESSION, false, false, false, true,
        "Port egress mirror session", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE, false, false, false, true,
        "Port ingress samplepacket enable", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE, false, false, false, true,
        "Port egress samplepacket enable", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_POLICER_ID, false, false, false, true,
        "Port policer ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_DEFAULT_TC, false, false, false, true,
        "Port QOS default TC", SAI_ATTR_VALUE_TYPE_UINT8 },
    { SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP, false, false, false, true,
        "Port DOT1P->TC map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP, false, false, false, true,
        "Port DOT1P->COLOR map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP, false, false, false, true,
        "Port DSCP->TC map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP, false, false, false, true,
        "Port DSCP->COLOR map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP, false, false, false, true,
        "Port TC->QUEUE map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP, false, false, false, true,
        "Port TC+COLOR->DOT1P map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP, false, false, false, true,
        "Port TC+COLOR->DSCP map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP, false, false, false, true,
        "Port TC->Priority Group map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP, false, false, false, true,
        "Port QOS PFC Priority->Priority Group map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP, false, false, false, true,
        "Port QOS PFC Priority->QUEUE map", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID, false, false, false, true,
        "Port scheduler profile ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST, false, false, false, true,
        "Port ingress buffer profile list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST, false, false, false, true,
        "Port egress buffer profile list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL, false, false, false, true,
        "Port priority flow control", SAI_ATTR_VALUE_TYPE_UINT8 },
    { SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST, false, false, false, true,
        "Port egress block port list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_PORT_ATTR_PORT_POOL_LIST, false, false, false, true,
        "List of port pools for the port", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t port_vendor_attribs[] = {
    { SAI_PORT_ATTR_TYPE,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_type_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_OPER_STATUS,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_state_get, (void*)SAI_PORT_ATTR_OPER_STATUS,
        NULL, NULL },
    { SAI_PORT_ATTR_SUPPORTED_BREAKOUT_MODE_TYPE,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_supported_breakout_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_CURRENT_BREAKOUT_MODE_TYPE,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_current_breakout_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_QOS_NUMBER_OF_QUEUES,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_qos_num_queues_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_QOS_QUEUE_LIST,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_qos_queue_list_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_QOS_NUMBER_OF_SCHEDULER_GROUPS,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_sched_groups_num_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_sched_groups_list_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_HW_LANE_LIST,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_hw_lane_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_SUPPORTED_SPEED,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_supported_speed_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_SUPPORTED_FEC_MODE,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_supported_fec_mode_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_NUMBER_OF_INGRESS_PRIORITY_GROUPS,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_number_of_priority_groups_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_priority_group_list_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_SPEED,
      { true, false, true, true },
      { true, false, true, true },
        mrvl_port_speed_get, NULL,
        mrvl_port_speed_set, NULL },
    { SAI_PORT_ATTR_FULL_DUPLEX_MODE,
        { false, false, false, true },
        { false, false, true, true },
        mrvl_port_duplex_mode_get, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_AUTO_NEG_MODE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_auto_negotiation_get, NULL,
        mrvl_port_auto_negotiation_set, NULL },
    { SAI_PORT_ATTR_ADMIN_STATE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_state_get, (void*)SAI_PORT_ATTR_ADMIN_STATE,
        mrvl_port_state_set, (void*)SAI_PORT_ATTR_ADMIN_STATE },
    { SAI_PORT_ATTR_MEDIA_TYPE,
        { false, false, false, false },
        { false, false, true, true },
        NULL, NULL,
        NULL, NULL },
    { SAI_PORT_ATTR_PORT_VLAN_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_lag_pvid_get , NULL,
        mrvl_port_lag_pvid_set, NULL },
    { SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_lag_default_vlan_prio_get, NULL,
        mrvl_port_lag_default_vlan_prio_set, NULL },
    { SAI_PORT_ATTR_DROP_UNTAGGED,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_lag_drop_tags_get, (void*)SAI_PORT_ATTR_DROP_UNTAGGED,
        mrvl_port_lag_drop_tags_set, (void*)SAI_PORT_ATTR_DROP_UNTAGGED },
    { SAI_PORT_ATTR_DROP_TAGGED,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_lag_drop_tags_get, (void*)SAI_PORT_ATTR_DROP_TAGGED,
        mrvl_port_lag_drop_tags_set, (void*)SAI_PORT_ATTR_DROP_TAGGED },
    { SAI_PORT_ATTR_INTERNAL_LOOPBACK_MODE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_internal_loopback_mode_get, NULL,
        mrvl_port_internal_loopback_mode_set, NULL },
    { SAI_PORT_ATTR_FEC_MODE,
        { true, false, true, true },
        { true, false, true, true },
        mrvl_port_fec_get, NULL,
        mrvl_port_fec_set, NULL },
    { SAI_PORT_ATTR_UPDATE_DSCP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_update_dscp_get, NULL,
        mrvl_port_update_dscp_set, NULL },
    { SAI_PORT_ATTR_MTU,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_mtu_get, NULL,
        mrvl_port_mtu_set, NULL },
    { SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID,
        { false, false, true, true},
        { false, false, true, true },
        mrvl_port_storm_control_policer_get, (void*)SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID,
        mrvl_port_storm_control_policer_set, (void*)SAI_PORT_ATTR_FLOOD_STORM_CONTROL_POLICER_ID },
    { SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID,
        { false, false, true, true},
        { false, false, true, true },
        mrvl_port_storm_control_policer_get, (void*)SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID,
        mrvl_port_storm_control_policer_set, (void*)SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID },
    { SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID,
        { false, false, true, true},
        { false, false, true, true },
        mrvl_port_storm_control_policer_get, (void*)SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID,
        mrvl_port_storm_control_policer_set, (void*)SAI_PORT_ATTR_MULTICAST_STORM_CONTROL_POLICER_ID },
    { SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_fc_get, NULL,
        mrvl_port_fc_set, NULL },
    { SAI_PORT_ATTR_INGRESS_ACL,
      { true, false, true, true },
      { true, false, true, true },
        mrvl_port_acl_binding_get, (void*)SAI_PORT_ATTR_INGRESS_ACL,
        mrvl_port_acl_binding_set, (void*)SAI_PORT_ATTR_INGRESS_ACL },
    { SAI_PORT_ATTR_EGRESS_ACL,
      { true, false, true, true },
      { true, false, true, true },
        mrvl_port_acl_binding_get, (void*)SAI_PORT_ATTR_EGRESS_ACL,
        mrvl_port_acl_binding_set, (void*)SAI_PORT_ATTR_EGRESS_ACL },
    { SAI_PORT_ATTR_INGRESS_MIRROR_SESSION,
        { false, false, true, true },
        { false, false, true, true },
      mrvl_port_mirror_session_get, (void*)SAI_PORT_ATTR_INGRESS_MIRROR_SESSION,
      mrvl_port_mirror_session_set, NULL },
    { SAI_PORT_ATTR_EGRESS_MIRROR_SESSION,
        { false, false, true, true },
        { false, false, true, true },
      mrvl_port_mirror_session_get, (void*)SAI_PORT_ATTR_EGRESS_MIRROR_SESSION,
      mrvl_port_mirror_session_set, NULL },
    { SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE,
        { false, false, true, true },
        { false, false, true, true },
      mrvl_port_samplepacket_session_get, (void*)SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE,
      mrvl_port_samplepacket_session_set, (void*)SAI_PORT_ATTR_INGRESS_SAMPLEPACKET_ENABLE },
    { SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE,
        { false, false, true, true },
        { false, false, true, true },
      mrvl_port_samplepacket_session_get, (void*)SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE,
      mrvl_port_samplepacket_session_set, (void*)SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE },
    { SAI_PORT_ATTR_POLICER_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_storm_control_policer_get, NULL,
        mrvl_port_storm_control_policer_set, NULL },
    { SAI_PORT_ATTR_QOS_DEFAULT_TC,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_default_tc_get, NULL,
        mrvl_port_qos_default_tc_set, NULL },
    { SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_DOT1P_TO_TC,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_DOT1P_TO_TC },
    { SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR },
    { SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_DSCP_TO_TC,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_DSCP_TO_TC },
    { SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_DSCP_TO_COLOR,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_DSCP_TO_COLOR },
    { SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_TC_TO_QUEUE,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_TC_TO_QUEUE },
    { SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P },
    { SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP },
    { SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP },
    { SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP },
    { SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_qos_map_id_get, (void*)SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE,
        mrvl_port_qos_map_id_set, (void*)SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE },
    { SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_sched_get, NULL,
        mrvl_port_sched_set, NULL },
    { SAI_PORT_ATTR_QOS_INGRESS_BUFFER_PROFILE_LIST,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_ingress_buffer_profile_list_get, NULL,
        mrvl_port_ingress_buffer_profile_list_set, NULL },
    { SAI_PORT_ATTR_QOS_EGRESS_BUFFER_PROFILE_LIST,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_egress_buffer_profile_list_get, NULL,
        mrvl_port_egress_buffer_profile_list_set, NULL },
    { SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_port_pfc_control_get, NULL,
        mrvl_port_pfc_control_set, NULL },
    { SAI_PORT_ATTR_EGRESS_BLOCK_PORT_LIST,
        { true, false, true, true },
        { true, false, true, true },
        mrvl_port_egress_block_get, NULL,
        mrvl_port_egress_block_set, NULL },
    { SAI_PORT_ATTR_PORT_POOL_LIST,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_port_pool_list_get, NULL,
        NULL, NULL },

    { END_FUNCTIONALITY_ATTRIBS_ID,
        { false, false, false, false },
        { false, false, false, false },
        NULL, NULL,
        NULL, NULL }
};

static sai_status_t mrvl_sai_port_get_port_by_index(_In_ uint32_t port_index, _Out_ uint32_t *db_index)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(db_index != NULL);

    for (i = 0; i < SAI_MAX_NUM_OF_PORTS; i++) {
        if (port_index == ports_db[i].index) {
            *db_index              = i;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_MAX_NUM_OF_PORTS) {
        status = SAI_STATUS_INVALID_PORT_NUMBER;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}
static sai_status_t mrvl_port_find_free_index_in_db(_Out_ uint32_t *free_index)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(free_index != NULL);

    for (i = 0; i < SAI_MAX_NUM_OF_PORTS; i++) {
        if (false == ports_db[i].is_present) {
            *free_index              = i;
            ports_db[i].is_present = true;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_MAX_NUM_OF_PORTS) {
    	MRVL_SAI_LOG_ERR("NO free indexes\n");
        status = SAI_STATUS_TABLE_FULL;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

mrvl_port_info_t* mrvl_sai_port_get_port_from_db(_In_ uint32_t idx)
{
    return &ports_db[idx];
}

sai_status_t mrvl_sai_port_add_to_lag(_In_ uint32_t port_idx, _In_ uint32_t lag_idx)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    mrvl_bridge_port_info_t *bport, port;
    uint32_t                real_lag_id;
    sai_vlan_id_t           vlan_id;
    sai_vlan_tagging_mode_t tagging_mode;

    real_lag_id = mrvl_lag_logical_to_real_index_MAC(lag_idx);
    MRVL_SAI_LOG_NTC("Adding port %d to lag %d\n", port_idx, real_lag_id);
    status = mrvl_sai_vlan_lag_port_update(lag_idx, port_idx, true);
    if (status != SAI_STATUS_SUCCESS) 
    { 
        MRVL_SAI_LOG_ERR("mrvl_sai_vlan_lag_port_update failed for port %d, LAG %d\n", port_idx, real_lag_id);
        MRVL_SAI_API_RETURN(status);
    }

    /* inform acl with added member port&lag */
    status = mrvl_sai_acl_lag_port_update(lag_idx, port_idx, true);
    if (status != SAI_STATUS_SUCCESS) 
    { 
        MRVL_SAI_LOG_ERR("mrvl_sai_acl_lag_port_update failed for port %d, LAG %d\n", port_idx, real_lag_id);
        MRVL_SAI_API_RETURN(status);
    }

    ports_db[port_idx].lag_idx = real_lag_id; 
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_port_remove_from_lag(_In_ uint32_t port_idx, _In_ uint32_t lag_idx)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    FPA_STATUS      fpa_status;
    mrvl_bridge_port_info_t *bport, port;
    uint32_t                real_lag_id;
    sai_vlan_id_t           vlan_id;
    sai_vlan_tagging_mode_t tagging_mode;
    int fd;

    real_lag_id = mrvl_lag_logical_to_real_index_MAC(lag_idx);
    MRVL_SAI_LOG_NTC("Removing port %d from lag %d\n", port_idx, real_lag_id);
    status = mrvl_sai_vlan_lag_port_update(lag_idx, port_idx, false);
    if (status != SAI_STATUS_SUCCESS) 
    { 
        MRVL_SAI_LOG_ERR("mrvl_sai_vlan_lag_port_update failed for port %d, LAG %d\n", port_idx, real_lag_id);
        MRVL_SAI_API_RETURN(status);
    }

    /* inform acl with added member port&lag */
    status = mrvl_sai_acl_lag_port_update(lag_idx, port_idx, false);
    if (status != SAI_STATUS_SUCCESS) 
    { 
        MRVL_SAI_LOG_ERR("mrvl_sai_acl_lag_port_update failed for port %d, LAG %d\n", port_idx, real_lag_id);
        MRVL_SAI_API_RETURN(status);
        
    }
    mrvl_sai_netdev_alloc(host_fd[port_idx].name);
    ports_db[port_idx].lag_idx = 0; 

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_port_set_port_in_lag(_In_ uint32_t port_idx, _In_ uint32_t lag_idx, _In_ bool add)
{
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (add)
    {
        status = mrvl_sai_port_add_to_lag(port_idx, lag_idx);
        if (status != SAI_STATUS_SUCCESS) 
        { 
            MRVL_SAI_LOG_ERR("Failed to add port %d to LAG %d\n", port_idx, lag_idx);
        }
    }
    else
    {
        status = mrvl_sai_port_remove_from_lag(port_idx, lag_idx);
        if (status != SAI_STATUS_SUCCESS) 
        { 
            MRVL_SAI_LOG_ERR("Failed to remove port %d from LAG %d\n", port_idx, lag_idx);
        }
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

bool mrvl_sai_is_port_lag_member(_In_ uint32_t port_idx)
{
    return (ports_db[port_idx].lag_idx != 0);
}

sai_status_t mrvl_sai_ports_init()
{
    uint32_t                    i;

    MRVL_SAI_LOG_ENTER();

    memset(ports_db, 0, SAI_MAX_NUM_OF_PORTS*sizeof(ports_db));
    for (i = 0; i < SAI_MAX_NUM_OF_PORTS; i++) {
        ports_db[i].is_present = true;
        ports_db[i].index = i; 
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Operational Status [sai_port_oper_status_t] */
/* Admin Mode [bool] */
sai_status_t mrvl_port_state_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;
        
    MRVL_SAI_LOG_ENTER();

    assert((SAI_PORT_ATTR_OPER_STATUS == (PTR_TO_INT)arg) || (SAI_PORT_ATTR_ADMIN_STATE == (PTR_TO_INT)arg));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    if (SAI_PORT_ATTR_OPER_STATUS == (PTR_TO_INT)arg) {
        portProperties.flags = FPA_PORT_PROPERTIES_STATE_FLAG;
    } else {
        portProperties.flags = FPA_PORT_PROPERTIES_CONFIG_FLAG;
    }
    
    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);
    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d attributes (speed)\n", port_id);
        return SAI_STATUS_FAILURE;
    }

    if (SAI_PORT_ATTR_OPER_STATUS == (PTR_TO_INT)arg) {
        if (portProperties.state & FPA_PORT_STATE_LINK_DOWN) {
            value->s32 = SAI_PORT_OPER_STATUS_DOWN;
        } else {
            value->s32 = SAI_PORT_OPER_STATUS_UP;
        }
    } else {
        if (portProperties.config & FPA_PORT_CONFIG_DOWN) {
            value->booldata = false;
        } else {
            value->booldata = true;
        }
    }
        
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Admin Mode [bool] */
sai_status_t mrvl_port_state_set(_In_ const sai_object_key_t *key,
                                 _In_ const sai_attribute_value_t *value,
                                 void *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    /* Get the current port config */
    memset(&portProperties, 0, sizeof(portProperties));
    portProperties.flags = FPA_PORT_PROPERTIES_CONFIG_FLAG;

    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d config (admin link)\n", 
                         port_id);
        return SAI_STATUS_FAILURE;
    }

    if (true == value->booldata) {
        portProperties.config = 0;
    } else {
        portProperties.config = FPA_PORT_CONFIG_DOWN;
    }

    fpa_status = fpaLibPortPropertiesSet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to set port %d config (admin link) %s\n", port_id, value->booldata == true ? "up" : "down");
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Hardware Lane list [sai_u32_list_t] */
sai_status_t mrvl_port_hw_lane_get(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }


    status = mrvl_sai_utl_fill_u32list(&port_id, 1, &value->u32list);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/* number of qos queues [sai_u32_list_t] */

sai_status_t mrvl_port_qos_num_queues_get(_In_ const sai_object_key_t   *key,
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

sai_status_t mrvl_port_qos_queue_list_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{

    sai_status_t status = SAI_STATUS_SUCCESS;
    sai_object_id_t data_obj;

    MRVL_SAI_LOG_ENTER();
  
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_QUEUE, 1, &data_obj))) {
        MRVL_SAI_LOG_ERR("Failed to create object SAI_OBJECT_TYPE_QUEUE\n");
        MRVL_SAI_API_RETURN(status);
        }
    
        
    /* fill object list for SAI_OBJECT_TYPE_QUEUE */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_fill_objlist(&data_obj, 1, &value->objlist))) {
        MRVL_SAI_LOG_ERR("Failed to fill objlist for SAI_OBJECT_TYPE_QUEUE\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_port_number_of_priority_groups_get(_In_ const sai_object_key_t   *key,
                                                            _Inout_ sai_attribute_value_t *value,
                                                            _In_ uint32_t                  attr_index,
                                                            _Inout_ vendor_cache_t        *cache,
                                                            void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    value->u32 = 1;
    MRVL_SAI_LOG_EXIT();
    return status;
}

sai_status_t mrvl_port_priority_group_list_get(_In_ const sai_object_key_t   *key,
                                                      _Inout_ sai_attribute_value_t *value,
                                                      _In_ uint32_t                  attr_index,
                                                      _Inout_ vendor_cache_t        *cache,
                                                      void                          *arg)
{
    sai_status_t     status = SAI_STATUS_SUCCESS;
    sai_object_id_t data_obj;

    MRVL_SAI_LOG_ENTER();

    /* create SAI priority group object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP, 1, &data_obj))) {
        MRVL_SAI_LOG_ERR("Failed to create object SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    /* fill object list for SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_fill_objlist(&data_obj ,1, &value->objlist))) {
        MRVL_SAI_LOG_ERR("Failed to fill objlist for SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/** Number of Scheduler groups on port [uint32_t]*/
sai_status_t mrvl_port_sched_groups_num_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg)
{
    sai_status_t        status = SAI_STATUS_SUCCESS;
    uint32_t            port_id;

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    value->u32 = 1;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_port_sched_groups_list_get(_In_ const sai_object_key_t   *key,
                                                    _Inout_ sai_attribute_value_t *value,
                                                    _In_ uint32_t                  attr_index,
                                                    _Inout_ vendor_cache_t        *cache,
                                                    void                          *arg)
{
    sai_status_t        status = SAI_STATUS_SUCCESS;
    uint32_t    port_id;
    sai_object_id_t data_obj;

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    /* create SAI scheduler group object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER_GROUP, 1, &data_obj))) {
        MRVL_SAI_API_RETURN(status);
    }
    
    /* fill object list for SAI_OBJECT_TYPE_SCHEDULER_GROUP */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_fill_objlist(&data_obj ,1, &value->objlist))) {
        MRVL_SAI_LOG_ERR("Failed to fill objlist for SAI_OBJECT_TYPE_SCHEDULER_GROUP\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}



/* list of supported port speed in Mbps [sai_u32_list_t] */
sai_status_t mrvl_port_supported_speed_get(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;
    uint32_t     num_of_supported_speed;
    uint32_t     speedlist[SAI_PORT_MAX_SPEEDS];
    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;
        
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    portProperties.flags = FPA_PORT_PROPERTIES_SUPPORTED_FLAG;
        
    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);
    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d supported (speed)\n", port_id);
        return SAI_STATUS_FAILURE;
    }

    num_of_supported_speed = 0;

    if( (FPA_PORT_FEAT_10MB_HD | FPA_PORT_FEAT_10MB_FD) & 
                                                portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_10M;
    }
    
    if( (FPA_PORT_FEAT_100MB_HD | FPA_PORT_FEAT_100MB_FD) & 
                                                portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_100M;
    }
            
    if( FPA_PORT_FEAT_1GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_1G;
    }

    if( FPA_PORT_FEAT_2_5GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_2_5G;
    }

    if( FPA_PORT_FEAT_10GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_10G;
    }

    if( FPA_PORT_FEAT_20GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_20G;
    }

    if( FPA_PORT_FEAT_25GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_25G;
    }

    if( FPA_PORT_FEAT_40GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_40G;
    }

    if( FPA_PORT_FEAT_100GB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_100G;
    }
           
    if( FPA_PORT_FEAT_1TB_FD & portProperties.supportedBmp) {
        speedlist[num_of_supported_speed++] = SAI_PORT_SPEED_1T;
    }
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_fill_u32list(speedlist, num_of_supported_speed, &value->u32list))) {
        MRVL_SAI_LOG_ERR("Failed to fill port %d supported speedlist\n", port_id);
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Speed in Mbps [uint32_t] */
sai_status_t mrvl_port_speed_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;
        
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    portProperties.flags = FPA_PORT_PROPERTIES_CURRSPEED_FLAG;
        
    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);
    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d attributes (speed)\n", port_id);
        return SAI_STATUS_FAILURE;
    }

    switch((FPA_PORT_FEAT_10MB_HD  |
            FPA_PORT_FEAT_10MB_FD  |
            FPA_PORT_FEAT_100MB_HD |
            FPA_PORT_FEAT_100MB_FD |
            FPA_PORT_FEAT_1GB_FD   |
            FPA_PORT_FEAT_2_5GB_FD |
            FPA_PORT_FEAT_10GB_FD  |
            FPA_PORT_FEAT_20GB_FD  |
            FPA_PORT_FEAT_25GB_FD  |
            FPA_PORT_FEAT_40GB_FD  |
            FPA_PORT_FEAT_100GB_FD |
            FPA_PORT_FEAT_1TB_FD) & portProperties.currSpeed) {
    case FPA_PORT_FEAT_10MB_HD:
    case FPA_PORT_FEAT_10MB_FD:
        value->u32 = SAI_PORT_SPEED_10M;
        break;

    case FPA_PORT_FEAT_100MB_HD:
    case FPA_PORT_FEAT_100MB_FD:
        value->u32 = SAI_PORT_SPEED_100M;
        break;

    case FPA_PORT_FEAT_1GB_FD:
        value->u32 = SAI_PORT_SPEED_1G;
        break;

    case FPA_PORT_FEAT_2_5GB_FD:
        value->u32 = SAI_PORT_SPEED_2_5G;
        break;

    case FPA_PORT_FEAT_10GB_FD:
        value->u32 = SAI_PORT_SPEED_10G;
        break;

    case FPA_PORT_FEAT_20GB_FD:
        value->u32 = SAI_PORT_SPEED_20G;
        break;

    case FPA_PORT_FEAT_25GB_FD:
        value->u32 = SAI_PORT_SPEED_25G;
        break;

    case FPA_PORT_FEAT_40GB_FD:
        value->u32 = SAI_PORT_SPEED_40G;
        break;

    case FPA_PORT_FEAT_100GB_FD:
        value->u32 = SAI_PORT_SPEED_100G;
        break;

    case FPA_PORT_FEAT_1TB_FD:
        value->u32 = SAI_PORT_SPEED_1T;
        break;
        
    default:
        MRVL_SAI_LOG_ERR("Unreconized port speed indication[0x%x]\n", 
                         portProperties.currSpeed);
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Speed in Mbps [uint32_t] */
sai_status_t mrvl_port_speed_set(_In_ const sai_object_key_t      *key, 
                                 _In_ const sai_attribute_value_t *value, 
                                 void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;
        
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    status = mrvl_port_speed_set_prv(port_id, value->u32);
    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to set port %d with speed %d", port_id, value->u32);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_port_speed_set_prv(_In_ uint32_t port_idx, 
                                     _In_ uint32_t speed)
{
    bool                        is_half_duplex;
    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;
        
    MRVL_SAI_LOG_ENTER();

    /* Get the current port features */
    memset(&portProperties, 0, sizeof(portProperties));
    portProperties.flags = FPA_PORT_PROPERTIES_FEATURES_FLAG;

    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_idx,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d features (speed)\n", 
                         port_idx);
        return SAI_STATUS_FAILURE;
    }

    if( (FPA_PORT_FEAT_10MB_HD & portProperties.featuresBmp) ||
        (FPA_PORT_FEAT_100MB_HD & portProperties.featuresBmp) ) {
        is_half_duplex = true;
    } else {
        is_half_duplex = false;
    }

    portProperties.featuresBmp &= ~(FPA_PORT_FEAT_10MB_HD  |
                                    FPA_PORT_FEAT_10MB_FD  |
                                    FPA_PORT_FEAT_100MB_HD |
                                    FPA_PORT_FEAT_100MB_FD |
                                    FPA_PORT_FEAT_1GB_FD   |
                                    FPA_PORT_FEAT_2_5GB_FD |
                                    FPA_PORT_FEAT_10GB_FD  |
                                    FPA_PORT_FEAT_20GB_FD  |
                                    FPA_PORT_FEAT_25GB_FD  |
                                    FPA_PORT_FEAT_40GB_FD  |
                                    FPA_PORT_FEAT_100GB_FD |
                                    FPA_PORT_FEAT_1TB_FD);

    switch(speed) {
    case SAI_PORT_SPEED_10M:
        if (true == is_half_duplex) {
            portProperties.featuresBmp |= FPA_PORT_FEAT_10MB_HD;
        } else {
            portProperties.featuresBmp |= FPA_PORT_FEAT_10MB_FD;
        }
        break;

    case SAI_PORT_SPEED_100M:
        if (true == is_half_duplex) {
            portProperties.featuresBmp |= FPA_PORT_FEAT_100MB_HD;
        } else {
            portProperties.featuresBmp |= FPA_PORT_FEAT_100MB_FD;
        }
        break;

    case SAI_PORT_SPEED_1G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_1GB_FD;
        break;

    case SAI_PORT_SPEED_2_5G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_2_5GB_FD;
        break;

    case SAI_PORT_SPEED_10G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_10GB_FD;
        break;

    case SAI_PORT_SPEED_20G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_20GB_FD;
        break;

    case SAI_PORT_SPEED_25G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_25GB_FD;
        break;

    case SAI_PORT_SPEED_40G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_40GB_FD;
        break;

    case SAI_PORT_SPEED_100G:
        portProperties.featuresBmp |= FPA_PORT_FEAT_100GB_FD;
        break;

    case SAI_PORT_SPEED_1T:
        portProperties.featuresBmp |= FPA_PORT_FEAT_1TB_FD;
        break;

    default:
            MRVL_SAI_LOG_ERR("Unsupported port speed [%d]\n", speed);
            return SAI_STATUS_FAILURE;
    }
        
    fpa_status = fpaLibPortPropertiesSet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_idx,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to set port %d attributes\features (speed)\n",
                         port_idx);
        return SAI_STATUS_FAILURE;
    }
  
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Flow Control state [sai_port_flow_control_mode_t] */
sai_status_t mrvl_port_fc_get(_In_ const sai_object_key_t   *key,
                              _Inout_ sai_attribute_value_t *value,
                              _In_ uint32_t                  attr_index,
                              _Inout_ vendor_cache_t        *cache,
                              void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;
        
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    portProperties.flags = FPA_PORT_PROPERTIES_FEATURES_FLAG;
        
    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);
    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d attributes (fc)\n", port_id);
        return SAI_STATUS_FAILURE;
    }

    switch((FPA_PORT_FEAT_PAUSE | FPA_PORT_FEAT_PAUSE_ASYM) &
                                        portProperties.featuresBmp) {
    case 0:
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_DISABLE;
        break;

    case (FPA_PORT_FEAT_PAUSE):
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE;
        break;

    case (FPA_PORT_FEAT_PAUSE | FPA_PORT_FEAT_PAUSE_ASYM):
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_RX_ONLY;
        break;

    case (FPA_PORT_FEAT_PAUSE_ASYM):
        value->s32 = SAI_PORT_FLOW_CONTROL_MODE_TX_ONLY;
        break;

    default:
        MRVL_SAI_LOG_ERR("Unreconized port flow control state[0x%x]\n", 
                         portProperties.featuresBmp);
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Flow Control state [sai_port_flow_control_mode_t] */
sai_status_t mrvl_port_fc_set(_In_ const sai_object_key_t      *key, 
                              _In_ const sai_attribute_value_t *value, 
                              void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;
    
    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;
        
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    /* Get the current port features */
    portProperties.flags = FPA_PORT_PROPERTIES_FEATURES_FLAG;

    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d attributes\features (fc)\n", 
                         port_id);
        return SAI_STATUS_FAILURE;
    }

    portProperties.featuresBmp &= ~(FPA_PORT_FEAT_PAUSE | FPA_PORT_FEAT_PAUSE_ASYM);

    switch(value->s32) {
    case SAI_PORT_FLOW_CONTROL_MODE_DISABLE:
        break;

    case SAI_PORT_FLOW_CONTROL_MODE_TX_ONLY:
        portProperties.featuresBmp |= FPA_PORT_FEAT_PAUSE_ASYM;
        break;

    case SAI_PORT_FLOW_CONTROL_MODE_RX_ONLY:
        portProperties.featuresBmp |= (FPA_PORT_FEAT_PAUSE | FPA_PORT_FEAT_PAUSE_ASYM);
        break;

    case SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE:
        portProperties.featuresBmp |= FPA_PORT_FEAT_PAUSE;
        break;

    default:
            MRVL_SAI_LOG_ERR("Unsupported port fc [%d]\n", value->s32);
            return SAI_STATUS_FAILURE;
    }
        
    fpa_status = fpaLibPortPropertiesSet(SAI_DEFAULT_ETH_SWID_CNS,
                                         port_id,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to set port %d attributes\features (fc)\n",
                         port_id);
        return SAI_STATUS_FAILURE;
    }
  
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/* Internal loopback control [sai_port_internal_loopback_mode_t] */
sai_status_t mrvl_port_internal_loopback_mode_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    value->s32 = SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Internal loopback control [sai_port_loopback_mode_t] */
sai_status_t mrvl_port_internal_loopback_mode_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    switch (value->s32) {
    case SAI_PORT_INTERNAL_LOOPBACK_MODE_NONE:
        break;

    case SAI_PORT_INTERNAL_LOOPBACK_MODE_PHY:
        break;

    case SAI_PORT_INTERNAL_LOOPBACK_MODE_MAC:
        break;

    default:
        MRVL_SAI_LOG_ERR("Invalid port internal loopback mode value %d\n", value->s32);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTR_VALUE_0);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* MTU [uint32_t] */
sai_status_t mrvl_port_mtu_get(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    value->u32 = SAI_MAX_MTU_CNS;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* MTU [uint32_t] */
sai_status_t mrvl_port_mtu_set(_In_ const sai_object_key_t *key,
                               _In_ const sai_attribute_value_t *value,
                               void *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);

    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}
    


sai_status_t mrvl_port_fec_get(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->s32 = SAI_PORT_FEC_MODE_NONE; /* default */
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
    
sai_status_t mrvl_port_fec_set(_In_ const sai_object_key_t      *key,
                               _In_ const sai_attribute_value_t *value,
                               void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
    
sai_status_t mrvl_port_auto_negotiation_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->booldata = false;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_auto_negotiation_set(_In_ const sai_object_key_t      *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
    
/* Port type [sai_port_type_t] */
sai_status_t mrvl_port_type_get(_In_ const sai_object_key_t   *key,
                                _Inout_ sai_attribute_value_t *value,
                                _In_ uint32_t                  attr_index,
                                _Inout_ vendor_cache_t        *cache,
                                void                          *arg)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    uint32_t        port_id;

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    
    if (SAI_CPU_PORT_CNS == port_id)
        value->s32 = SAI_PORT_TYPE_CPU;
    else
        value->s32 = SAI_PORT_TYPE_LOGICAL;
        
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Breakout mode(s) supported [sai_s32_list_t] */
sai_status_t mrvl_port_supported_breakout_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    uint32_t        port_id;
    int32_t         modes[SAI_PORT_BREAKOUT_MODE_TYPE_MAX];

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_API_RETURN(status);
    }

    modes[0]  = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;
    status = mrvl_sai_utl_fill_s32list(modes, 1, &value->s32list);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Current breakout mode [sai_port_breakout_mode_type_t] */
sai_status_t mrvl_port_current_breakout_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_PORT_BREAKOUT_MODE_TYPE_4_LANE;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_supported_fec_mode_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    int32_t      modes[] = { SAI_PORT_FEC_MODE_NONE, SAI_PORT_FEC_MODE_RS, SAI_PORT_FEC_MODE_FC };
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    
    status = mrvl_sai_utl_fill_s32list(modes, sizeof(modes) / sizeof(modes[0]), &value->s32list);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_duplex_mode_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->booldata = true;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Update DSCP of outgoing packets [bool] */
sai_status_t mrvl_port_update_dscp_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Update DSCP of outgoing packets [bool] */
sai_status_t mrvl_port_update_dscp_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_qos_default_tc_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u8 = 0; /* default */
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_qos_default_tc_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_qos_map_id_get(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    sai_qos_map_type_t qos_map_type = (sai_qos_map_type_t)arg;

    MRVL_SAI_LOG_ENTER();

    assert(qos_map_type < SAI_QOS_MAP_TYPES_MAX);

    status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_QOS_MAP, 1, &value->oid);
    if (status != SAI_STATUS_SUCCESS) {
    
    value->oid = SAI_NULL_OBJECT_ID;
        MRVL_SAI_LOG_ERR("Error creating QOS map object\n");
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_qos_map_id_set(_In_ const sai_object_key_t      *key,
                                      _In_ const sai_attribute_value_t *value,
                                      void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_mirror_session_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->objlist.count = 0;
    memset(&(value->objlist.list), 0, sizeof(sai_object_id_t));

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_mirror_session_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_samplepacket_session_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->oid = SAI_NULL_OBJECT_ID;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_samplepacket_session_set(_In_ const sai_object_key_t      *key,
                                                _In_ const sai_attribute_value_t *value,
                                                void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
    
sai_status_t mrvl_port_pfc_control_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    value->u8 = 0;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_pfc_control_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_sched_get(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    uint32_t        port_idx;
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_idx))) {
        MRVL_SAI_LOG_ERR("Error converting PORT object to type\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SCHEDULER, 1, &value->oid))) {
        MRVL_SAI_LOG_ERR("Error creating scheduler object\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_sched_set(_In_ const sai_object_key_t      *key,
                                 _In_ const sai_attribute_value_t *value,
                                 void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}
    
sai_status_t mrvl_port_ingress_buffer_profile_list_get(_In_ const sai_object_key_t   *key,
                                                       _Inout_ sai_attribute_value_t *value,
                                                       _In_ uint32_t                  attr_index,
                                                       _Inout_ vendor_cache_t        *cache,
                                                       void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->objlist.count = 0;
    memset(&(value->objlist.list), 0, sizeof(sai_object_id_t));

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_ingress_buffer_profile_list_set(_In_ const sai_object_key_t      *key,
                                                       _In_ const sai_attribute_value_t *value,
                                                       void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_egress_buffer_profile_list_get(_In_ const sai_object_key_t   *key,
                                                      _Inout_ sai_attribute_value_t *value,
                                                      _In_ uint32_t                  attr_index,
                                                      _Inout_ vendor_cache_t        *cache,
                                                      void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->objlist.count = 0;
    memset(&(value->objlist.list), 0, sizeof(sai_object_id_t));

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_egress_buffer_profile_list_set(_In_ const sai_object_key_t      *key,
                                                      _In_ const sai_attribute_value_t *value,
                                                      void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_storm_control_policer_set(_In_ const sai_object_key_t      *key,
                                                      _In_ const sai_attribute_value_t *value,
                                                      _In_ void                        *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}
    
sai_status_t mrvl_port_storm_control_policer_get(_In_ const sai_object_key_t   *key,
                                                      _Inout_ sai_attribute_value_t *value,
                                                      _In_ uint32_t                  attr_index,
                                                      _Inout_ vendor_cache_t        *cache,
                                                      _In_ void                     *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->oid = SAI_NULL_OBJECT_ID;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
    

sai_status_t mrvl_port_egress_block_set(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        _In_ void                        *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_port_egress_block_get(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        _In_ void                     *arg)
{
    sai_object_id_t port_oid;
    sai_status_t    status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_oid))) {
            MRVL_SAI_LOG_ERR("Error creating port object\n");
            MRVL_SAI_API_RETURN(status);
    }
    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_fill_objlist(&port_oid, 1, &value->objlist))) {
            MRVL_SAI_LOG_ERR("Error filling object list using port object\n", port_oid);
            MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_port_pool_list_get(_In_ const sai_object_key_t   *key,
                                     _Inout_ sai_attribute_value_t *value,
                                     _In_ uint32_t                  attr_index,
                                     _Inout_ vendor_cache_t        *cache,
                                     _In_ void                     *arg)
{
    sai_object_id_t port_pool_oid;
    sai_status_t    status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT_POOL, 1, &port_pool_oid))) {
            MRVL_SAI_LOG_ERR("Error creating port object\n");
            MRVL_SAI_API_RETURN(status);
    }
    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_fill_objlist(&port_pool_oid, 1, &value->objlist))) {
            MRVL_SAI_LOG_ERR("Error filling object list using port pool object\n", port_pool_oid);
            MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Dropping of untagged frames on ingress [bool] */
/* Dropping of tagged frames on ingress [bool] */
sai_status_t mrvl_port_lag_drop_tags_get(_In_ const sai_object_key_t   *key,
                                     _Inout_ sai_attribute_value_t *value,
                                     _In_ uint32_t                  attr_index,
                                     _Inout_ vendor_cache_t        *cache,
                                     void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    assert((SAI_PORT_ATTR_DROP_UNTAGGED == (int64_t)arg) || (SAI_PORT_ATTR_DROP_TAGGED == (int64_t)arg));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Dropping of untagged frames on ingress [bool] */
/* Dropping of tagged frames on ingress [bool] */
sai_status_t mrvl_port_lag_drop_tags_set(_In_ const sai_object_key_t      *key,
                                     _In_ const sai_attribute_value_t *value,
                                     void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    assert((SAI_PORT_ATTR_DROP_UNTAGGED == (int64_t)arg) || (SAI_PORT_ATTR_DROP_TAGGED == (int64_t)arg)
           || (SAI_LAG_ATTR_DROP_UNTAGGED == (int64_t)arg) || (SAI_LAG_ATTR_DROP_TAGGED == (int64_t)arg));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    if (mrvl_sai_is_port_lag_member(port_id)) {
        MRVL_SAI_LOG_ERR("Failed to set drop tags attribute to port [%lx] - port is a lag member. Please use SAI_LAG_ATTR_DROP_UNTAGGED or SAI_LAG_ATTR_DROP_TAGGED\n",
                   port_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/* Media Type [sai_port_media_type_t] */
sai_status_t mrvl_port_media_type_get(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    value->s32 = SAI_PORT_MEDIA_TYPE_COPPER;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Media Type [sai_port_media_type_t] */
sai_status_t mrvl_port_media_type_set(_In_ const sai_object_key_t      *key,
                                      _In_ const sai_attribute_value_t *value,
                                      void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static void port_key_to_str(_In_ sai_object_id_t port_id, _Out_ char *key_str)
{
    uint32_t port;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_id, SAI_OBJECT_TYPE_PORT, &port))
    {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid port");
    } 
    else {
        snprintf(key_str, MAX_KEY_STR_LEN, "port %u", port);
    }
}

static void port_pool_key_to_str(_In_ sai_object_id_t port_pool_id, _Out_ char *key_str)
{
    uint32_t port_pool;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_pool_id, SAI_OBJECT_TYPE_PORT_POOL, &port_pool))
    {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid port pool");
    } 
    else {
        snprintf(key_str, MAX_KEY_STR_LEN, "port pool %u", port_pool);
    }
}


/**
 * @brief Set port attribute value.
 *
 * @param[in] port_id Port id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/

sai_status_t mrvl_set_port_attribute(_In_ sai_object_id_t port_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    port_key_to_str(port_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, port_attribs, port_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**



 * @brief Get port attribute value.
 *
 * @param[in] port_id Port id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */
sai_status_t mrvl_get_port_attribute(_In_ sai_object_id_t     port_id,
                                     _In_ uint32_t            attr_count,
                                     _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == port_id) {
        MRVL_SAI_LOG_ERR("NULL port id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    port_key_to_str(port_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, port_attribs, port_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/**


 * @brief Get port statistics counters.
 *
 * @param[in] port_id Port id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_get_port_stats(_In_ sai_object_id_t                port_id,
                                 _In_ uint32_t                       number_of_counters,
                                 _In_ const sai_port_stat_t         *counter_ids,
                                 _Out_ uint64_t                     *counters)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     ii, port;
    char         key_str[MAX_KEY_STR_LEN];

    FPA_STATUS   fpa_status;
    FPA_PORT_COUNTERS_EXT_STC   port_ext_statistics;

    MRVL_SAI_LOG_ENTER();

    port_key_to_str(port_id, key_str);
    /*MRVL_SAI_LOG_NTC("Get port stats %s\n", key_str);*/

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

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_id, SAI_OBJECT_TYPE_PORT, &port))) {
    	MRVL_SAI_API_RETURN(status);
    }

    fpa_status = fpaLibPortStatisticsExtGet(SAI_DEFAULT_ETH_SWID_CNS, 
                                            port, 
                                            &port_ext_statistics);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d extended counters\n", port);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    for (ii = 0; ii < number_of_counters; ii++) {
        switch (counter_ids[ii]) {
        case SAI_PORT_STAT_IF_IN_OCTETS:
            counters[ii] = port_ext_statistics.goodOctetsReceived +
                            port_ext_statistics.badOctetsReceived;
            break;

        case SAI_PORT_STAT_IF_IN_UCAST_PKTS:
            counters[ii] = port_ext_statistics.goodUnicastFramesReceived;
            break;

        case SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS:
            counters[ii] = port_ext_statistics.multicastFramesReceived + 
                            port_ext_statistics.broadcastFramesReceived;
            break;

        case SAI_PORT_STAT_IF_IN_ERRORS:
            counters[ii] = port_ext_statistics.rxErrorFrameReceived;
            break;

        case SAI_PORT_STAT_IF_IN_BROADCAST_PKTS:
            counters[ii] = port_ext_statistics.broadcastFramesReceived;
            break;

        case SAI_PORT_STAT_IF_IN_MULTICAST_PKTS:
            counters[ii] = port_ext_statistics.multicastFramesReceived;
            break;

        case SAI_PORT_STAT_IF_OUT_OCTETS:
            counters[ii] = port_ext_statistics.goodOctetsSent;
            break;

        case SAI_PORT_STAT_IF_OUT_UCAST_PKTS:
            counters[ii] = port_ext_statistics.unicastFrameSent;
            break;

        case SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS:
            counters[ii] = port_ext_statistics.multicastFramesSent +
                            port_ext_statistics.broadcastFramesSent;
            break;

        case SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS:
            counters[ii] = port_ext_statistics.broadcastFramesSent;
            break;

        case SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS:
            counters[ii] = port_ext_statistics.multicastFramesSent;
            break;

        case SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS:
            counters[ii] = port_ext_statistics.receiveFIFOoverrun;
            break;

        case SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS:
            counters[ii] = port_ext_statistics.multicastFramesReceived;
            break;

        case SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS:
            counters[ii] = port_ext_statistics.broadcastFramesReceived;
            break;

        case SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS:
            counters[ii] = port_ext_statistics.undersizePktsRecieved;
            break;

        case SAI_PORT_STAT_ETHER_STATS_FRAGMENTS:
            counters[ii] = port_ext_statistics.fragmentsRecieved;
            break;

        case SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames64Octets;
            break;

        case SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames65to127Octets;
            break;

        case SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames128to255Octets;
            break;

        case SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames256to511Octets;
            break;

        case SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames512to1023Octets;
            break;

        case SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames1024to1518Octets;
            break;

        case SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS:
            counters[ii] = port_ext_statistics.oversizePktsRecieved;
            break;

        case SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS:
            counters[ii] = port_ext_statistics.oversizePktsRecieved;
            break;

        case SAI_PORT_STAT_ETHER_STATS_JABBERS:
            counters[ii] = port_ext_statistics.rxJabberPksRecieved;
            break;

        case SAI_PORT_STAT_ETHER_STATS_COLLISIONS:
            counters[ii] = port_ext_statistics.collisions;
            break;

        case SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS:
            counters[ii] = port_ext_statistics.badCRC;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames64Octets;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames65to127Octets;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames128to255Octets;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames256to511Octets;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames512to1023Octets;
            break;

        case SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS:
            counters[ii] = port_ext_statistics.rxFrames1024to1518Octets;
            break;
        
        case SAI_PORT_STAT_IF_IN_DISCARDS:
        case SAI_PORT_STAT_IF_IN_UNKNOWN_PROTOS:
        case SAI_PORT_STAT_IF_IN_VLAN_DISCARDS:
        case SAI_PORT_STAT_IF_OUT_DISCARDS:
        case SAI_PORT_STAT_IF_OUT_ERRORS:
        case SAI_PORT_STAT_IF_OUT_QLEN:
        case SAI_PORT_STAT_ETHER_STATS_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS:
        case SAI_PORT_STAT_ETHER_TX_OVERSIZE_PKTS:
        case SAI_PORT_STAT_ETHER_STATS_TX_NO_ERRORS:
        case SAI_PORT_STAT_ETHER_STATS_RX_NO_ERRORS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_1519_TO_2047_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_2048_TO_4095_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_4096_TO_9216_OCTETS:
        case SAI_PORT_STAT_ETHER_STATS_PKTS_9217_TO_16383_OCTETS:
        case SAI_PORT_STAT_IP_IN_RECEIVES:
        case SAI_PORT_STAT_IP_IN_OCTETS:
        case SAI_PORT_STAT_IP_IN_UCAST_PKTS:
        case SAI_PORT_STAT_IP_IN_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IP_IN_DISCARDS:
        case SAI_PORT_STAT_IP_OUT_OCTETS:
        case SAI_PORT_STAT_IP_OUT_UCAST_PKTS:
        case SAI_PORT_STAT_IP_OUT_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IP_OUT_DISCARDS:
        case SAI_PORT_STAT_IPV6_IN_RECEIVES:
        case SAI_PORT_STAT_IPV6_IN_OCTETS:
        case SAI_PORT_STAT_IPV6_IN_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_IN_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_IN_MCAST_PKTS:
        case SAI_PORT_STAT_IPV6_IN_DISCARDS:
        case SAI_PORT_STAT_IPV6_OUT_OCTETS:
        case SAI_PORT_STAT_IPV6_OUT_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_OUT_NON_UCAST_PKTS:
        case SAI_PORT_STAT_IPV6_OUT_MCAST_PKTS:
        case SAI_PORT_STAT_IPV6_OUT_DISCARDS:
        case SAI_PORT_STAT_GREEN_WRED_DROPPED_PACKETS:
        case SAI_PORT_STAT_GREEN_WRED_DROPPED_BYTES:
        case SAI_PORT_STAT_YELLOW_WRED_DROPPED_PACKETS:
        case SAI_PORT_STAT_YELLOW_WRED_DROPPED_BYTES:
        case SAI_PORT_STAT_RED_WRED_DROPPED_PACKETS:
        case SAI_PORT_STAT_RED_WRED_DROPPED_BYTES:
        case SAI_PORT_STAT_WRED_DROPPED_PACKETS:
        case SAI_PORT_STAT_WRED_DROPPED_BYTES:
        case SAI_PORT_STAT_ECN_MARKED_PACKETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_1519_TO_2047_OCTETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_2048_TO_4095_OCTETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_4096_TO_9216_OCTETS:
        case SAI_PORT_STAT_ETHER_IN_PKTS_9217_TO_16383_OCTETS:  
        case SAI_PORT_STAT_ETHER_OUT_PKTS_64_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_65_TO_127_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_128_TO_255_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_256_TO_511_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_512_TO_1023_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_1024_TO_1518_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_1519_TO_2047_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_2048_TO_4095_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_4096_TO_9216_OCTETS:
        case SAI_PORT_STAT_ETHER_OUT_PKTS_9217_TO_16383_OCTETS:
        case SAI_PORT_STAT_IN_CURR_OCCUPANCY_BYTES:
        case SAI_PORT_STAT_IN_WATERMARK_BYTES:
        case SAI_PORT_STAT_IN_SHARED_CURR_OCCUPANCY_BYTES:
        case SAI_PORT_STAT_IN_SHARED_WATERMARK_BYTES:
        case SAI_PORT_STAT_OUT_CURR_OCCUPANCY_BYTES:
        case SAI_PORT_STAT_OUT_WATERMARK_BYTES:
        case SAI_PORT_STAT_OUT_SHARED_CURR_OCCUPANCY_BYTES:
        case SAI_PORT_STAT_OUT_SHARED_WATERMARK_BYTES:
        case SAI_PORT_STAT_IN_DROPPED_PKTS:
        case SAI_PORT_STAT_OUT_DROPPED_PKTS:
        case SAI_PORT_STAT_PAUSE_RX_PKTS:
        case SAI_PORT_STAT_PAUSE_TX_PKTS:
        case SAI_PORT_STAT_PFC_0_RX_PKTS:
        case SAI_PORT_STAT_PFC_0_TX_PKTS:
        case SAI_PORT_STAT_PFC_1_RX_PKTS:
        case SAI_PORT_STAT_PFC_1_TX_PKTS:
        case SAI_PORT_STAT_PFC_2_RX_PKTS:
        case SAI_PORT_STAT_PFC_2_TX_PKTS:
        case SAI_PORT_STAT_PFC_3_RX_PKTS:
        case SAI_PORT_STAT_PFC_3_TX_PKTS:
        case SAI_PORT_STAT_PFC_4_RX_PKTS:
        case SAI_PORT_STAT_PFC_4_TX_PKTS:
        case SAI_PORT_STAT_PFC_5_RX_PKTS:
        case SAI_PORT_STAT_PFC_5_TX_PKTS:
        case SAI_PORT_STAT_PFC_6_RX_PKTS:
        case SAI_PORT_STAT_PFC_6_TX_PKTS:
        case SAI_PORT_STAT_PFC_7_RX_PKTS:
        case SAI_PORT_STAT_PFC_7_TX_PKTS:
        case SAI_PORT_STAT_PFC_0_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_0_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_1_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_1_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_2_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_2_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_3_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_3_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_4_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_4_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_5_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_5_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_6_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_6_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_7_RX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_7_TX_PAUSE_DURATION:
        case SAI_PORT_STAT_PFC_0_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_1_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_2_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_3_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_4_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_5_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_6_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_PFC_7_ON2OFF_RX_PKTS:
        case SAI_PORT_STAT_EEE_TX_EVENT_COUNT:
        case SAI_PORT_STAT_EEE_RX_EVENT_COUNT:
        case SAI_PORT_STAT_EEE_TX_DURATION:
        case SAI_PORT_STAT_EEE_RX_DURATION:
            counters[ii] = 0;
            break;

        default:
            MRVL_SAI_LOG_ERR("Invalid port counter %d\n", counter_ids[ii]);
            counters[ii] = 0;
        }
    }

    /*MRVL_SAI_LOG_EXIT();*/

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Clear port's all statistics counters.
 *
 * @param[in] port_id Port id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_clear_port_all_stats(_In_ sai_object_id_t port_id)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port;
    char         key_str[MAX_KEY_STR_LEN];

    FPA_STATUS   fpa_status;

    MRVL_SAI_LOG_ENTER();

    port_key_to_str(port_id, key_str);
    MRVL_SAI_LOG_NTC("Clear port stats %s\n", key_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_id, SAI_OBJECT_TYPE_PORT, &port))) {
    	MRVL_SAI_API_RETURN(status);
    }

    fpa_status = fpaLibPortStatisticsClear(SAI_DEFAULT_ETH_SWID_CNS, port);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to clear port %d counters\n", port);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Default VLAN [sai_vlan_id_t]
 *   Untagged ingress frames are tagged with default VLAN
 */
sai_status_t mrvl_port_lag_pvid_get (_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{
    sai_status_t                status = SAI_STATUS_SUCCESS;
    uint32_t                    port_lag_idx;
    FPA_STATUS                  fpa_status;

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_oid_to_lag_port(key->key.object_id, &port_lag_idx);
    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to convert port oid %" PRIx64 " to port index\n", key->key.object_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    
    if (mrvl_sai_is_port_lag_member(port_lag_idx)) {
        MRVL_SAI_LOG_ERR("Failed to get PVID for port [%lx] - port is a lag member. Please use SAI_LAG_ATTR_PORT_VLAN_ID\n",
                   port_lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    fpa_status = fpaLibPortPvidGet(SAI_DEFAULT_ETH_SWID_CNS, port_lag_idx, &value->u16);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to get port %d pvid, status 0x%x\n",
                         port_lag_idx, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/* Default VLAN Priority [uint8_t]
 *  (default to 0) */
sai_status_t mrvl_port_lag_default_vlan_prio_get(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(0, port_id);
    fpa_flow_entry.cookie = cookie;
    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, 
                                            FPA_FLOW_TABLE_TYPE_VLAN_E, 
                                            &fpa_flow_entry);
    if (FPA_NOT_FOUND == fpa_status) {
        MRVL_SAI_LOG_ERR("No default VLAN priority entry %llx in fpa flow table status = %d\n",
                          cookie, fpa_status);
    } else if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get VLAN entry %llx in fpa flow table status = %d\n",
                          cookie, fpa_status);
    } else if (FPA_FLOW_VLAN_IGNORE_VAL == fpa_flow_entry.data.vlan.newTagPcp) {
        MRVL_SAI_LOG_ERR("Default VLAN priority for port %d was not configured\n", port_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    } else {
        value->u8 = fpa_flow_entry.data.vlan.newTagPcp;
    }

    MRVL_SAI_LOG_EXIT();
    return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
}

/* Port VLAN ID [sai_vlan_id_t]
 *   Untagged ingress frames are tagged with PVID
 */
sai_status_t mrvl_port_lag_pvid_set(_In_ const sai_object_key_t      *key,
                                    _In_ const sai_attribute_value_t *value,
                                    void                             *arg)
{
    sai_status_t                status = SAI_STATUS_SUCCESS;
    uint32_t                    port_lag_idx;
    FPA_STATUS                  fpa_status;

    MRVL_SAI_LOG_ENTER();

    status = mrvl_sai_utl_oid_to_lag_port(key->key.object_id, &port_lag_idx);
    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to convert port oid %" PRIx64 " to port index\n", key->key.object_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    
    if (mrvl_sai_is_port_lag_member(port_lag_idx)) {
        MRVL_SAI_LOG_ERR("Failed to set PVID to port [%lx] - port is a lag member. Please use SAI_LAG_ATTR_PORT_VLAN_ID\n",
                   port_lag_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    MRVL_SAI_LOG_NTC("Set port %d with pvid %d\n", port_lag_idx, value->u16);
    fpa_status = fpaLibPortPvidSet(SAI_DEFAULT_ETH_SWID_CNS, port_lag_idx, value->u16);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to set port %d with pvid %d, status 0x%x\n",
                         port_lag_idx, value->u16, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}

/* Default VLAN Priority [uint8_t]
 *  (default to 0) */
sai_status_t mrvl_port_lag_default_vlan_prio_set(_In_ const sai_object_key_t      *key,
                                             _In_ const sai_attribute_value_t *value,
                                             void                             *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port_id;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
        return status;
    }

    if (mrvl_sai_is_port_lag_member(port_id)) {
        MRVL_SAI_LOG_ERR("Failed to set PVID to port [%lx] - port is a lag member. Please use SAI_LAG_ATTR_DEFAULT_VLAN_PRIORITY\n",
                   port_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(0, port_id);
    fpa_flow_entry.cookie = cookie;
    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, 
                                            FPA_FLOW_TABLE_TYPE_VLAN_E, 
                                            &fpa_flow_entry);

    if (FPA_NOT_FOUND == fpa_status) {
        MRVL_SAI_LOG_ERR("No default VLAN entry %llx in fpa flow table status = 0x%x\n",
                          cookie, fpa_status);
    } else if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get VLAN entry %llx to fpa flow table status = 0x%x\n",
                          cookie, fpa_status);
    } else {
        MRVL_SAI_LOG_NTC("Modify default VLAN priority entry for port %d\n", port_id);
        fpa_flow_entry.data.vlan.newTagPcp = value->u8;
        fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS, 
                                           FPA_FLOW_TABLE_TYPE_VLAN_E, 
                                           &fpa_flow_entry, 0);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to modify VLAN entry %llx to fpa flow table status = 0x%x\n",
                             cookie, fpa_status);
        }
    }

    MRVL_SAI_LOG_EXIT();
    return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
}

sai_status_t mrvl_port_acl_binding_set(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       _In_ void                        *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port))) {
        return status;
    }

    if (port > SAI_MAX_NUM_OF_PORTS){
        MRVL_SAI_LOG_ERR("Invalid port %d\n", port);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (value->oid == SAI_NULL_OBJECT_ID){
    	/* unbind action */
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_unbind_from_port(arg, port))){
            MRVL_SAI_LOG_ERR("Unable to unbind port %d from ACL TABLE\n", port);
            return SAI_STATUS_INVALID_PARAMETER;
    	}
    }
    else {
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_bind_to_port(arg, value->oid, port))){
            MRVL_SAI_LOG_ERR("Unable to bind port %d to ACL TABLE\n", port);
            return SAI_STATUS_INVALID_PARAMETER;
    	}
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

sai_status_t mrvl_port_acl_binding_get(_In_ const sai_object_key_t   *key,
                                       _Inout_ sai_attribute_value_t *value,
                                       _In_ uint32_t                  attr_index,
                                       _Inout_ vendor_cache_t        *cache,
                                       void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     port;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_PORT, &port))) {
        MRVL_SAI_API_RETURN(status);
    }

    if (port > SAI_MAX_NUM_OF_PORTS && SAI_CPU_PORT_CNS != port){
        MRVL_SAI_LOG_ERR("Invalid port %d\n", port);
        return SAI_STATUS_SUCCESS;
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_get_table_id_per_port(arg, port, value))){
        MRVL_SAI_LOG_ERR("Unable to get assigned ACL table per port %d\n", port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}

/**
 * @brief Create port
 *
 * @param[out] port_id Port id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_port(
        _Out_ sai_object_id_t *port_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status = SAI_STATUS_SUCCESS;  
    uint32_t                     lane_count, index, db_index, ingress_acl_idx = 0, egress_acl_idx = 0;
    static uint32_t              port_idx = 0; /* TO DO: dynamic port idx allocation */
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    const sai_attribute_value_t *hw_lane_list = NULL;
    const sai_attribute_value_t *speed = NULL;   
    const sai_attribute_value_t *ingress_acl = NULL, *egress_acl = NULL;   

    MRVL_SAI_LOG_ENTER();

    if (NULL == port_id) {
        MRVL_SAI_LOG_ERR("NULL port id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (port_idx >= SAI_MAX_NUM_OF_PORTS) 
    {
        MRVL_SAI_LOG_ERR("Reached max number of ports possible: %d\n", SAI_MAX_NUM_OF_PORTS);
        MRVL_SAI_API_RETURN(SAI_STATUS_TABLE_FULL);
    }
    status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, port_attribs, port_vendor_attribs,
                                                 SAI_COMMON_API_CREATE);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed port attributes check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, port_attribs, MAX_LIST_VALUE_STR_LEN, list_str);    
    MRVL_SAI_LOG_NTC("Create port, %s\n", list_str);

    /* find port mandatory attributes: */
    if (SAI_STATUS_SUCCESS == 
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_PORT_ATTR_HW_LANE_LIST, &hw_lane_list, &index))) {
        lane_count = hw_lane_list->u32list.count;
        if (1 != lane_count && 4 != lane_count) {
            MRVL_SAI_LOG_ERR("Port HW lanes count %u is invalid (supported only 1, 4)\n", lane_count);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
        if (lane_count > SAI_MAX_NUM_OF_LANES) {
            MRVL_SAI_LOG_ERR("Port HW lanes count %u is bigger than %u\n", lane_count, SAI_MAX_NUM_OF_LANES);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS == 
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_PORT_ATTR_SPEED, &speed, &index)))
    {
        if (mrvl_port_speed_set_prv(SAI_SYS_PORT_MAPPING[port_idx], speed->u32) != SAI_STATUS_SUCCESS)
            MRVL_SAI_LOG_ERR("Failed to set speed %d for port %d\n", speed->u32, SAI_SYS_PORT_MAPPING[port_idx]);
    }
    
    /* check SAI_PORT_ATTR_INGRESS_ACL & SAI_PORT_ATTR_EGRESS_ACL */
    if (SAI_STATUS_SUCCESS == 
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_PORT_ATTR_INGRESS_ACL, &ingress_acl, &index)))
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(ingress_acl->oid, SAI_OBJECT_TYPE_ACL_TABLE, &ingress_acl_idx))) {
            MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS == 
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_PORT_ATTR_EGRESS_ACL, &egress_acl, &index)))
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(egress_acl->oid, SAI_OBJECT_TYPE_ACL_TABLE, &egress_acl_idx))) {
            MRVL_SAI_API_RETURN(status);
    }

    /* get free index */
    if (SAI_STATUS_SUCCESS != (status = mrvl_port_find_free_index_in_db(&db_index))){
        MRVL_SAI_LOG_ERR("No free index in ports DB\n");
        MRVL_SAI_API_RETURN(status);
    }

    ports_db[db_index].index = SAI_SYS_PORT_MAPPING[port_idx];
    ports_db[db_index].speed = speed->u32;
    ports_db[db_index].ingress_acl_idx = ingress_acl_idx;
    ports_db[db_index].egress_acl_idx = egress_acl_idx;
    ports_db[db_index].lanes.list = hw_lane_list->u32list.list;
    ports_db[db_index].lanes.count = hw_lane_list->u32list.count;

    /* create SAI port object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, SAI_SYS_PORT_MAPPING[port_idx], port_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    /* TODO: check if this call is required? */
    /*status = mrvl_sai_utl_create_l2_int_group_wo_vlan(SAI_SYS_PORT_MAPPING[port_idx]);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't create l2 interface group w/o vlan for port %d\n", SAI_SYS_PORT_MAPPING[port_idx]);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }*/

    port_key_to_str(*port_id, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);
    port_idx++;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);    

}

/**
 * @brief Remove port
 *
 * @param[in] port_id Port id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_port(
        _In_ sai_object_id_t port_id)
{
    uint32_t            port_idx;       

    MRVL_SAI_LOG_ENTER();        

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_id, SAI_OBJECT_TYPE_PORT, &port_idx)) {
        MRVL_SAI_LOG_ERR("invalid port_idx %d\n",port_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

/**
 * @brief Clear port statistics counters.
 *
 * @param[in] port_id Port id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error 
*/

sai_status_t mrvl_clear_port_stats(
    _In_ sai_object_id_t port_id,
    _In_ uint32_t number_of_counters,    
    _In_ const sai_port_stat_t *counter_ids
    )
{
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

sai_status_t mrvl_port_pool_port_id_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    _In_ void                     *arg);

sai_status_t mrvl_port_pool_buffer_pool_id_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    _In_ void                     *arg);

sai_status_t mrvl_port_pool_wred_profile_id_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    _In_ void                     *arg);

static const sai_attribute_entry_t  port_pool_attribs[] = {
    { SAI_PORT_POOL_ATTR_PORT_ID, true, true, false, true,
        "Port ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_POOL_ATTR_BUFFER_POOL_ID, true, true, false, true,
        "Port pool buffer pool ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID, false, true, false, true,
        "Port pool WRED profile ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t port_pool_vendor_attribs[] = {
    { SAI_PORT_POOL_ATTR_PORT_ID,
        { false, false, false, true },
        { false, false, false, true },
        NULL, NULL,
        NULL, NULL },
    { SAI_PORT_POOL_ATTR_BUFFER_POOL_ID,
        { false, false, false, true },
        { false, false, false, true },
        NULL, NULL,
        NULL, NULL },
    { SAI_PORT_POOL_ATTR_QOS_WRED_PROFILE_ID,
        { false, false, false, true },
        { false, false, false, true },
        NULL, NULL,
        NULL, NULL },

    { END_FUNCTIONALITY_ATTRIBS_ID,
        { false, false, false, false },
        { false, false, false, false },
        NULL, NULL,
        NULL, NULL }
};

sai_status_t mrvl_port_pool_port_id_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    _In_ void                     *arg)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

sai_status_t mrvl_port_pool_buffer_pool_id_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    _In_ void                     *arg)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

sai_status_t mrvl_port_pool_wred_profile_id_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    _In_ void                     *arg)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}


/**
 * @brief Create port pool
 *
 * @param[out] port_pool_id Port pool id
 * @param[in] switch_id Switch id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_create_port_pool(
        _Out_ sai_object_id_t *port_pool_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Remove port pool
 *
 * @param[in] port_pool_id Port pool id
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_remove_port_pool(
        _In_ sai_object_id_t port_pool_id)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Set port pool attribute value.
 *
 * @param[in] port_pool_id Port pool id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_set_port_pool_attribute(
        _In_ sai_object_id_t port_pool_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = port_pool_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

    /*port_pool_key_to_str(port_pool_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, port_pool_attribs, port_pool_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);*/
}

/**
 * @brief Get port pool attribute value.
 *
 * @param[in] port_pool_id Port pool id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_get_port_pool_attribute(
        _In_ sai_object_id_t port_pool_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = port_pool_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == port_pool_id) {
        MRVL_SAI_LOG_ERR("NULL port pool id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

    /*port_pool_key_to_str(port_pool_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, port_pool_attribs, port_pool_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);*/
}

/**
 * @brief Get port pool statistics counters.
 *
 * @param[in] port_pool_id Port pool id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_get_port_pool_stats(
        _In_ sai_object_id_t port_pool_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_port_pool_stat_t *counter_ids,
        _Out_ uint64_t *counters)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Clear port pool statistics counters.
 *
 * @param[in] port_pool_id Port pool id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_clear_port_pool_stats(
        _In_ sai_object_id_t port_pool_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_port_pool_stat_t *counter_ids)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

const sai_port_api_t port_api = {
    mrvl_create_port,
    mrvl_remove_port,
    mrvl_set_port_attribute,
    mrvl_get_port_attribute,
    mrvl_get_port_stats,
    mrvl_clear_port_stats,
    mrvl_clear_port_all_stats,
    mrvl_create_port_pool,
    mrvl_remove_port_pool,
    mrvl_set_port_pool_attribute,
    mrvl_get_port_pool_attribute,
    mrvl_get_port_pool_stats,
    mrvl_clear_port_pool_stats
};
