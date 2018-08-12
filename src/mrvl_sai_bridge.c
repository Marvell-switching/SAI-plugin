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
#define __MODULE__ SAI_BRIDGE

static mrvl_bridge_port_info_t bridge_ports_db[SAI_MAX_NUM_OF_BRIDGE_PORTS] = {};
extern sai_object_id_t mrvl_sai_default_bridge_id;

static void bridge_id_key_to_str(_In_ sai_object_id_t sai_bridge_id, _Out_ char *key_str);
static void bridge_port_id_key_to_str(_In_ sai_object_id_t sai_bridge_port_id, _Out_ char *key_str);

static sai_status_t mrvl_sai_bridge_type_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg);
static sai_status_t mrvl_sai_bridge_port_list_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
static sai_status_t mrvl_sai_bridge_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                          _Inout_ sai_attribute_value_t *value,
                                                          _In_ uint32_t                  attr_index,
                                                          _Inout_ vendor_cache_t        *cache,
                                                          void                          *arg);
static sai_status_t mrvl_sai_bridge_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                          _In_ const sai_attribute_value_t *value,
                                                          void                             *arg);
static sai_status_t mrvl_sai_bridge_learn_disable_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg);
static sai_status_t mrvl_sai_bridge_learn_disable_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg);
static sai_status_t mrvl_sai_bridge_flood_control_type_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg);
static sai_status_t mrvl_sai_bridge_flood_group_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                             *arg);

static const sai_attribute_entry_t mrvl_sai_bridge_attribs[] = {
    { SAI_BRIDGE_ATTR_TYPE, true, false, false, true,
      "Bridge type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_ATTR_PORT_LIST, false, false, false, true,
      "Bridge ports associated to bridge", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES, false, false, false, true,
      "Max number of learned MAC addresses", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_BRIDGE_ATTR_LEARN_DISABLE, false, false, false, true,
      "Disable learning on bridge", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE, false, false, false, true,
      "Bridge unknown unicast flood control type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP, false, true, false, true,
      "Bridge unknown unicast flood group", SAI_ATTR_VALUE_TYPE_OBJECT_ID},
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE, false, true, false, true,
      "Bridge unknown multicast flood control type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP, false, true, false, true,
      "Bridge unknown multicast flood group", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE, false, true, false, true,
      "Bridge broadcast flood control type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP, false, true, false, true,
      "Bridge broadcastt flood group", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_bridge_vendor_attribs[] = {
    { SAI_BRIDGE_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_type_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_port_list_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_MAX_LEARNED_ADDRESSES,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_max_learned_addresses_get, NULL,
      mrvl_sai_bridge_max_learned_addresses_set, NULL },
    { SAI_BRIDGE_ATTR_LEARN_DISABLE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_learn_disable_get, NULL,
      mrvl_sai_bridge_learn_disable_set, NULL },
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE,
      { true, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_flood_control_type_get, (void*)SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP,
      { true, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_flood_group_get, (void*)SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE,
      { true, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_flood_control_type_get, (void*)SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP,
      { true, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_flood_group_get, (void*)SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE,
      { true, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_flood_control_type_get, (void*)SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE,
      NULL, NULL },
    { SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP,
      { true, false, false, true },
      { false, false, false, true },
      mrvl_sai_bridge_flood_group_get, (void*)SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP,
      NULL, NULL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};
static sai_status_t mrvl_sai_bridge_port_type_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
static sai_status_t mrvl_sai_bridge_port_lag_or_port_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sai_bridge_port_vlan_id_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg);
static sai_status_t mrvl_sai_bridge_port_rif_id_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg);
static sai_status_t mrvl_sai_bridge_port_tunnel_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg);
static sai_status_t mrvl_sai_bridge_port_bridge_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg);
static sai_status_t mrvl_sai_bridge_port_bridge_id_set(_In_ const sai_object_key_t      *key,
                                                   _In_ const sai_attribute_value_t *value,
                                                   void                             *arg);
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_get(_In_ const sai_object_key_t   *key,
                                                           _Inout_ sai_attribute_value_t *value,
                                                           _In_ uint32_t                  attr_index,
                                                           _Inout_ vendor_cache_t        *cache,
                                                           void                          *arg);
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_set(_In_ const sai_object_key_t      *key,
                                                           _In_ const sai_attribute_value_t *value,
                                                           void                             *arg);
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                               _Inout_ sai_attribute_value_t *value,
                                                               _In_ uint32_t                  attr_index,
                                                               _Inout_ vendor_cache_t        *cache,
                                                               void                          *arg);
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                               _In_ const sai_attribute_value_t *value,
                                                               void                             *arg);
static sai_status_t mrvl_sai_bridge_port_admin_state_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sai_bridge_port_admin_state_set(_In_ const sai_object_key_t      *key,
                                                     _In_ const sai_attribute_value_t *value,
                                                     void                             *arg);
static sai_status_t mrvl_sai_bridge_port_ingress_filtering_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg);
static sai_status_t mrvl_sai_bridge_port_ingress_filtering_set(_In_ const sai_object_key_t      *key,
                                                               _In_ const sai_attribute_value_t *value,
                                                               void                             *arg);
static sai_status_t mrvl_sai_bridge_port_tagging_mode_get(_In_ const sai_object_key_t   *key,
                                                          _Inout_ sai_attribute_value_t *value,
                                                          _In_ uint32_t                  attr_index,
                                                          _Inout_ vendor_cache_t        *cache,
                                                          void                          *arg);

static const sai_attribute_entry_t mrvl_sai_bridge_port_attribs[] = {
    { SAI_BRIDGE_PORT_ATTR_TYPE, true, true, false, true,
      "Bridge port type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID, true, true, false, true,
      "Bridge port associated port/lag ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_BRIDGE_PORT_ATTR_TAGGING_MODE, false, true, false, true,
      "Bridge port egress tagging mode", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID, false, true, false, true,
      "Bridge port associated VLAN ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID, false, true, false, true,
      "Bridge port associated rif ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID, false, true, false, true,
      "Bridge port associated tunnel ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID, false, true, false, true,
      "Bridge port associated bridge ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE, false, false, false, true,
      "Bridge port FDB learning mode", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES, false, false, false, true,
      "Bridge port max learned MAC addresses", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION, false, false, false, true,
      "Bridge port action for packets with unkown MAC address", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE, false, true, true, true,
      "Bridge port admin state", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING, false, true, false, true,
      "Bridge port ingress filtering", SAI_ATTR_VALUE_TYPE_BOOL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t mrvl_sai_bridge_port_vendor_attribs[] = {
    { SAI_BRIDGE_PORT_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_type_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_lag_or_port_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_TAGGING_MODE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_tagging_mode_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_VLAN_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_vlan_id_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_RIF_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_rif_id_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_TUNNEL_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_bridge_port_tunnel_id_get, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_BRIDGE_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_bridge_id_get, NULL,
      mrvl_sai_bridge_port_bridge_id_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_MODE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_fdb_learning_mode_get, NULL,
      mrvl_sai_bridge_port_fdb_learning_mode_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_MAX_LEARNED_ADDRESSES,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_max_learned_addresses_get, NULL,
      mrvl_sai_bridge_port_max_learned_addresses_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_FDB_LEARNING_LIMIT_VIOLATION_PACKET_ACTION,
      { true, false, false, false },
      { true, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_BRIDGE_PORT_ATTR_ADMIN_STATE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_admin_state_get, NULL,
      mrvl_sai_bridge_port_admin_state_set, NULL },
    { SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_bridge_port_ingress_filtering_get, NULL,
      mrvl_sai_bridge_port_ingress_filtering_set, NULL },
    
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

/**
 * @brief Bridge type
 *
 * @type sai_bridge_type_t
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 */
static sai_status_t mrvl_sai_bridge_type_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_object_id_t bridge_id   = key->key.object_id;
    uint32_t    bridge_data;
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->s32 = SAI_BRIDGE_TYPE_1Q;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief List of bridge ports associated to this bridge
 *
 * @type sai_object_list_t
 * @objects SAI_OBJECT_TYPE_BRIDGE_PORT
 * @flags READ_ONLY
 */
static sai_status_t mrvl_sai_bridge_port_list_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    uint32_t bridge_idx, ports_count = 0, i, j = 0;
    mrvl_bridge_port_info_t *bport;
    sai_object_id_t *port_list;
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_idx)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge object\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    for (i = 0; i < SAI_MAX_NUM_OF_BRIDGE_PORTS; i++) {
        if (true == bridge_ports_db[i].is_used) {
            ports_count++;
        }
    }
    port_list = calloc(ports_count, sizeof(*port_list));
    if (!port_list) {
        MRVL_SAI_API_RETURN(SAI_STATUS_NO_MEMORY);
    }
    for (i = 0; i < SAI_MAX_NUM_OF_BRIDGE_PORTS; i++) {
        if (bridge_idx != bridge_ports_db[i].bridge_id) {
            continue;
        }
        /* create SAI BRIDGE object */
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, bridge_ports_db[i].index, &port_list[j++]))) {
            MRVL_SAI_API_RETURN(status);
        }
    }
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_objlist(port_list, ports_count, &value->objlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill bridge ports objlist\n");
         MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * Zero means learning limit disable
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                          _Inout_ sai_attribute_value_t *value,
                                                          _In_ uint32_t                  attr_index,
                                                          _Inout_ vendor_cache_t        *cache,
                                                          void                          *arg)
{
    sai_object_id_t bridge_id   = key->key.object_id;
    uint32_t    bridge_data;
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->u32 = 0;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * Zero means learning limit disable
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                          _In_ const sai_attribute_value_t *value,
                                                          void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief To disable learning on a bridge
 *
 * @type bool
 * @flags CREATE_AND_SET
 * @default false
 */
static sai_status_t mrvl_sai_bridge_learn_disable_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg)
{
    sai_object_id_t bridge_id   = key->key.object_id;
    uint32_t    bridge_data;
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_data)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief To disable learning on a bridge
 *
 * @type bool
 * @flags CREATE_AND_SET
 * @default false
 */
static sai_status_t mrvl_sai_bridge_learn_disable_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

static sai_status_t mrvl_sai_bridge_flood_control_type_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg)
{
    switch ((int64_t)arg) {
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_CONTROL_TYPE:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_CONTROL_TYPE:
            break;
        default:
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    value->s32 = SAI_BRIDGE_FLOOD_CONTROL_TYPE_SUB_PORTS;
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_sai_bridge_flood_group_get(_In_ const sai_object_key_t   *key,
                                                  _Inout_ sai_attribute_value_t *value,
                                                  _In_ uint32_t                  attr_index,
                                                  _Inout_ vendor_cache_t        *cache,
                                                  void                          *arg)
{
    switch ((int64_t)arg) {
        case SAI_BRIDGE_ATTR_UNKNOWN_UNICAST_FLOOD_GROUP:
        case SAI_BRIDGE_ATTR_UNKNOWN_MULTICAST_FLOOD_GROUP:
        case SAI_BRIDGE_ATTR_BROADCAST_FLOOD_GROUP:
            break;
        default:
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    value->oid = SAI_NULL_OBJECT_ID;
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static void bridge_id_key_to_str(_In_ sai_object_id_t sai_bridge_id, _Out_ char *key_str)
{
    uint32_t     bridge_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_bridge_id, SAI_OBJECT_TYPE_BRIDGE, &bridge_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid bridge id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "bridge id %u", bridge_id);
    }
}

static void bridge_port_id_key_to_str(_In_ sai_object_id_t sai_bridge_port_id, _Out_ char *key_str)
{
    uint32_t     bridge_port_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(sai_bridge_port_id, SAI_OBJECT_TYPE_BRIDGE_PORT, &bridge_port_id)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid bridge port id");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "bridge port %u", bridge_port_id);
    }
}

/**
 * @brief Create bridge
 *
 * @param[out] bridge_id Bridge ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_bridge(
        _Out_ sai_object_id_t* bridge_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	sai_status_t status = SAI_STATUS_SUCCESS;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t  *bridge_type;
    uint32_t    attr_idx;

    MRVL_SAI_LOG_ENTER();

    if (NULL == bridge_id) {
        MRVL_SAI_LOG_ERR("NULL bridge id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_bridge_attribs, mrvl_sai_bridge_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_bridge_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_ATTR_TYPE, &bridge_type, &attr_idx));
    if (bridge_type->s32 != SAI_BRIDGE_TYPE_1D) {
        MRVL_SAI_LOG_ERR("Not supported bridge type %d\n", bridge_type->s32);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    /* create SAI BRIDGE object (temporarily hardcoded)*/
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, 1, bridge_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove bridge
 *
 * @param[in] bridge_id Bridge ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_bridge (
        _In_ sai_object_id_t bridge_id)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Set attribute for bridge
 *
 * @param[in] bridge_id Bridge ID
 * @param[in] attr Attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_set_bridge_attribute(
        _In_ sai_object_id_t bridge_id,
        _In_ const sai_attribute_t *attr)
{
	const sai_object_key_t key = { .key.object_id = bridge_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    bridge_id_key_to_str(bridge_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_bridge_attribs, mrvl_sai_bridge_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get attributes of bridge
 *
 * @param[in] bridge_id Bridge ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_bridge_attribute(
        _In_ sai_object_id_t bridge_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.object_id = bridge_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == bridge_id) {
        MRVL_SAI_LOG_ERR("NULL bridge id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    bridge_id_key_to_str(bridge_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_bridge_attribs, mrvl_sai_bridge_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);    
}

/**
 * @brief Get bridge statistics counters.
 *
 * @param[in] bridge_id Bridge id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_sai_get_bridge_stats(
        _In_ sai_object_id_t bridge_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_bridge_stat_t *counter_ids,
        _Out_ uint64_t *counters)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Clear bridge statistics counters.
 *
 * @param[in] bridge_id Bridge id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_sai_clear_bridge_stats(
        _In_ sai_object_id_t bridge_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_bridge_stat_t *counter_ids)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Bridge port type
 *
 * @type sai_bridge_port_type_t
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 */
static sai_status_t mrvl_sai_bridge_port_type_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_object_id_t bridge_port_id = key->key.object_id;
    mrvl_bridge_port_info_t          *bport;
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    value->s32 = bport->port_type;
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated Port or Lag object id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_PORT, SAI_OBJECT_TYPE_LAG
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_PORT or SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_SUB_PORT
 */
static sai_status_t mrvl_sai_bridge_port_lag_or_port_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t          *bport;
    uint32_t                         port;
    sai_status_t                     status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    if (bport->port_type == SAI_BRIDGE_PORT_TYPE_PORT) {
        port = bport->logical_port;
    } else if (bport->port_type == SAI_BRIDGE_PORT_TYPE_SUB_PORT) {
        port = bport->parent;
    } else {
        MRVL_SAI_LOG_ERR("Invalid port type - %d\n", bport->port_type);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &value->oid))
    {
        MRVL_SAI_LOG_ERR("Failed to create object port\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated Vlan
 *
 * @type sai_uint16_t
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_SUB_PORT
 * @isvlan true
 */
static sai_status_t mrvl_sai_bridge_port_vlan_id_get(_In_ const sai_object_key_t   *key,
                                                 _Inout_ sai_attribute_value_t *value,
                                                 _In_ uint32_t                  attr_index,
                                                 _Inout_ vendor_cache_t        *cache,
                                                 void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    if (bport->port_type != SAI_BRIDGE_PORT_TYPE_SUB_PORT) {
        MRVL_SAI_LOG_ERR("Invalid bridge port type %d, must be SAI_BRIDGE_PORT_TYPE_SUB_PORT\n", bport->port_type);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    value->u16 = bport->vlan_id;
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated router inerface object id
 * Please note that for SAI_BRIDGE_PORT_TYPE_1Q_ROUTER,
 * all vlan interfaces are auto bounded for the bridge port.
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_ROUTER_INTERFACE
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_1D_ROUTER
 */
static sai_status_t mrvl_sai_bridge_port_rif_id_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    if (bport->port_type != SAI_BRIDGE_PORT_TYPE_1D_ROUTER) {
        MRVL_SAI_LOG_ERR("Invalid bridge port type %d, must be SAI_BRIDGE_PORT_TYPE_1D_ROUTER\n", bport->port_type);
        status = SAI_STATUS_INVALID_PARAMETER;
        MRVL_SAI_API_RETURN(status);
    }
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, bport->rif_id, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object rif\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated tunnel id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_TUNNEL
 * @flags MANDATORY_ON_CREATE | CREATE_ONLY
 * @condition SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_TUNNEL
 */
static sai_status_t mrvl_sai_bridge_port_tunnel_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    if (bport->port_type != SAI_BRIDGE_PORT_TYPE_TUNNEL) {
        MRVL_SAI_LOG_ERR("Invalid bridge port type %d, must be SAI_BRIDGE_PORT_TYPE_TUNNEL\n", bport->port_type);
        status = SAI_STATUS_INVALID_PARAMETER;
        MRVL_SAI_API_RETURN(status);
    }
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_TUNNEL, bport->tunnel_id, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object tunnel\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated bridge id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_BRIDGE
 * @flags CREATE_AND_SET
 * @default SAI_NULL_OBJECT_ID
 * @allownull true
 */
static sai_status_t mrvl_sai_bridge_port_bridge_id_get(_In_ const sai_object_key_t   *key,
                                                   _Inout_ sai_attribute_value_t *value,
                                                   _In_ uint32_t                  attr_index,
                                                   _Inout_ vendor_cache_t        *cache,
                                                   void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, bport->bridge_id, &value->oid)))
    {
        MRVL_SAI_LOG_ERR("Failed to create object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Associated bridge id
 *
 * @type sai_object_id_t
 * @objects SAI_OBJECT_TYPE_BRIDGE
 * @flags CREATE_AND_SET
 * @default SAI_NULL_OBJECT_ID
 * @allownull true
 */
static sai_status_t mrvl_sai_bridge_port_bridge_id_set(_In_ const sai_object_key_t      *key,
                                                       _In_ const sai_attribute_value_t *value,
                                                       void                             *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_BRIDGE, &bport->bridge_id)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge\n");
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief FDB Learning mode
 *
 * @type sai_bridge_port_fdb_learning_mode_t
 * @flags CREATE_AND_SET
 * @default SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW
 */
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_get(_In_ const sai_object_key_t   *key,
                                                           _Inout_ sai_attribute_value_t *value,
                                                           _In_ uint32_t                  attr_index,
                                                           _Inout_ vendor_cache_t        *cache,
                                                           void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }
    
    value->s32 = SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief FDB Learning mode
 *
 * @type sai_bridge_port_fdb_learning_mode_t
 * @flags CREATE_AND_SET
 * @default SAI_BRIDGE_PORT_FDB_LEARNING_MODE_HW
 */
static sai_status_t mrvl_sai_bridge_port_fdb_learning_mode_set(_In_ const sai_object_key_t      *key,
                                                           _In_ const sai_attribute_value_t *value,
                                                           void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_get(_In_ const sai_object_key_t   *key,
                                                               _Inout_ sai_attribute_value_t *value,
                                                               _In_ uint32_t                  attr_index,
                                                               _Inout_ vendor_cache_t        *cache,
                                                               void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }
    
    value->u32 = 0;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Maximum number of learned MAC addresses
 *
 * @type sai_uint32_t
 * @flags CREATE_AND_SET
 * @default 0
 */
static sai_status_t mrvl_sai_bridge_port_max_learned_addresses_set(_In_ const sai_object_key_t      *key,
                                                               _In_ const sai_attribute_value_t *value,
                                                               void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Admin Mode.
 *
 * Before removing a bridge port, need to disable it by setting admin mode
 * to false, then flush the FDB entries, and then remove it.
 *
 * @type bool
 * @flags CREATE_AND_SET
 * @default false
 */
static sai_status_t mrvl_sai_bridge_port_admin_state_get(_In_ const sai_object_key_t   *key,
                                                     _Inout_ sai_attribute_value_t *value,
                                                     _In_ uint32_t                  attr_index,
                                                     _Inout_ vendor_cache_t        *cache,
                                                     void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t  *bport;
    sai_status_t             status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }
    
    value->booldata = bport->admin_state;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_bridge_port_admin_state_set(_In_ const sai_object_key_t      *key,
                                                     _In_ const sai_attribute_value_t *value,
                                                     void                             *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t     *bport;
    sai_status_t                status = SAI_STATUS_SUCCESS;
    uint32_t                    index;
    FPA_STATUS                  fpa_status;
    FPA_PORT_PROPERTIES_STC     portProperties;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    /* Get the current port config */
    portProperties.flags = FPA_PORT_PROPERTIES_CONFIG_FLAG;

    fpa_status = fpaLibPortPropertiesGet(SAI_DEFAULT_ETH_SWID_CNS,
                                         bport->logical_port,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to get port %d config (admin link)\n", bport->logical_port);
        MRVL_SAI_API_RETURN(mrvl_sai_utl_fpa_to_sai_status(fpa_status));
    }

    if (true == value->booldata) {
        portProperties.config &= ~FPA_PORT_CONFIG_DOWN;
    } else {
        portProperties.config |= FPA_PORT_CONFIG_DOWN;
    }


    fpa_status = fpaLibPortPropertiesSet(SAI_DEFAULT_ETH_SWID_CNS,
                                         bport->logical_port,
                                         &portProperties);

    if (FPA_OK != fpa_status) {
        MRVL_SAI_LOG_ERR("Failed to set port %d config (admin link)\n", bport->logical_port);
        MRVL_SAI_API_RETURN(mrvl_sai_utl_fpa_to_sai_status(fpa_status));
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
* @brief Tagging mode of the bridge port
*
* Specifies the tagging mode to be used during egress.
*
* @type sai_bridge_port_tagging_mode_t
* @flags CREATE_AND_SET
* @default SAI_BRIDGE_PORT_TAGGING_MODE_TAGGED
* @validonly SAI_BRIDGE_PORT_ATTR_TYPE == SAI_BRIDGE_PORT_TYPE_SUB_PORT
*/
static sai_status_t mrvl_sai_bridge_port_tagging_mode_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t     *bport;
    sai_status_t                status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }
    value->s32 = bport->tag_mode;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
/**
     * @brief Ingress filtering (drop frames with unknown VLANs)
     *
     * @type bool
     * @flags CREATE_AND_SET
     * @default false
     */
static sai_status_t mrvl_sai_bridge_port_ingress_filtering_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_object_id_t bridge_port_id   = key->key.object_id;
    mrvl_bridge_port_info_t     *bport;
    sai_status_t                status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to index\n", bridge_port_id);
        MRVL_SAI_API_RETURN(status);
    }

    value->booldata = bport->ingress_filter;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_bridge_port_ingress_filtering_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

sai_status_t mrvl_sai_add_bridge_port(_In_ uint32_t bridge_idx,
                                      _In_ sai_bridge_port_type_t port_type,
                                      _Out_ mrvl_bridge_port_info_t **bridge_port)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;
    mrvl_bridge_port_info_t *port;

    MRVL_SAI_LOG_ENTER();
    for (i = 0; i < SAI_MAX_NUM_OF_BRIDGE_PORTS; i++) {
        if (false == bridge_ports_db[i].is_used) {
            port = &bridge_ports_db[i];
            port->is_used = true;
            port->index = i;
            port->bridge_id = bridge_idx;
            port->port_type = port_type;
            *bridge_port = port;

            MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
        }
    }

    if (i == SAI_MAX_NUM_OF_BRIDGE_PORTS) {
    	MRVL_SAI_LOG_ERR("Bridge ports number exceeded the maximum: %d\n", SAI_MAX_NUM_OF_BRIDGE_PORTS);
        status = SAI_STATUS_TABLE_FULL;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_port_to_bport(_In_ uint32_t port, _Out_ mrvl_bridge_port_info_t **bport)
{
    sai_status_t            status = SAI_STATUS_SUCCESS;
    mrvl_bridge_port_info_t *db_port;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(bport != NULL);
    for (i = 0; i < SAI_MAX_NUM_OF_BRIDGE_PORTS; i++) {
        db_port = &bridge_ports_db[i];
        if (port == db_port->logical_port) {
            *bport = db_port;
            MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
        }
    }

    if (i == SAI_MAX_NUM_OF_BRIDGE_PORTS) {
    	MRVL_SAI_LOG_ERR("Failed to find bridge port by port %d\n", port);
        status = SAI_STATUS_INVALID_PORT_NUMBER;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

sai_status_t mrvl_sai_bridge_port_object_create(_In_ uint32_t index, _Out_ sai_object_id_t *oid)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    assert(oid != NULL);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, index, oid))) {
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

uint32_t mrvl_sai_bridge_port_to_port(_In_ sai_object_id_t bridge_port_oid)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i, bport_log_port;

    MRVL_SAI_LOG_ENTER();
    assert(index != NULL);

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(bridge_port_oid, SAI_OBJECT_TYPE_BRIDGE_PORT, &bport_log_port)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        return 0xFFFFFFFF;
    }
    for (i = 0; i < SAI_MAX_NUM_OF_BRIDGE_PORTS; i++) {
        if (bport_log_port == bridge_ports_db[i].logical_port) {
            return i;
        }
    }

    if (i == SAI_MAX_NUM_OF_BRIDGE_PORTS) {
    	MRVL_SAI_LOG_ERR("Failed to find bridge port by port %d\n", bport_log_port);
        return 0xFFFFFFFF;
    }   
}

sai_status_t mrvl_sai_bridge_port_by_index(uint32_t index, mrvl_bridge_port_info_t **bport)
{
    if (index >= SAI_MAX_NUM_OF_BRIDGE_PORTS) {
        MRVL_SAI_LOG_ERR("Invalid bridge port idx %d\n", index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (!bridge_ports_db[index].is_used) {
        MRVL_SAI_LOG_ERR("Bridge port %d is removed or has not been created yet\n", index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    *bport = &bridge_ports_db[index];
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_sai_bridge_port_by_oid_get(sai_object_id_t oid, mrvl_bridge_port_info_t **port)
{
    uint32_t        bport_index;
    sai_status_t    status = SAI_STATUS_SUCCESS;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(oid, SAI_OBJECT_TYPE_BRIDGE_PORT, &bport_index)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert object bridge port\n");
        MRVL_SAI_API_RETURN(status);
    }
    return mrvl_sai_bridge_port_by_index(bport_index, port);
}
bool mrvl_sai_bridge_port_is_vlan_member(_In_ mrvl_bridge_port_info_t *bport)
{
    return bport->vlan_id != 0;
}
bool mrvl_sai_bridge_port_is_current_vlan_member(_In_ uint16_t bport_vlan_idx, _In_ uint16_t vlan)
{
    return (bport_vlan_idx == vlan);
}

sai_status_t mrvl_sai_bridge_init(void)
{
    mrvl_port_info_t *port;
    mrvl_bridge_port_info_t *bridge_port, *bport_router;
    sai_object_id_t bridge_id;
    uint8_t i;
    uint32_t            bridge_idx = 1, bport_idx;
    sai_status_t        status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Initializing bridge and bridge ports\n");
    memset(bridge_ports_db, 0, SAI_MAX_NUM_OF_BRIDGE_PORTS*sizeof(mrvl_bridge_port_info_t));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, bridge_idx, &mrvl_sai_default_bridge_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    for (i = 0; i < SAI_MAX_NUM_OF_PORTS; i++) 
    {
        port = mrvl_sai_port_get_port_from_db(i);
        if (port != NULL && port->is_present)
        {
            if (SAI_STATUS_SUCCESS != mrvl_sai_add_bridge_port(bridge_idx, SAI_BRIDGE_PORT_TYPE_PORT, &bridge_port)) 
            {
                MRVL_SAI_LOG_ERR("Failed to add bridge port to bridge_ports_db\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }
            bridge_port->logical_port = port->index;
            bridge_port->admin_state = true;

            if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_port_add(bridge_port, SAI_DEFAULT_VLAN_CNS, SAI_VLAN_TAGGING_MODE_UNTAGGED))
            {
                MRVL_SAI_LOG_ERR("Failed to add bridge port to default vlan\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }
        }
    }
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_add_bridge_port(bridge_idx, SAI_BRIDGE_PORT_TYPE_1Q_ROUTER, &bport_router))
    {
        MRVL_SAI_LOG_ERR("Failed to add bridge port router to bridge_ports_db\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    bport_router->admin_state = true;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Create bridge port
 *
 * @param[out] bridge_port_id Bridge port ID
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_bridge_port(
        _Out_ sai_object_id_t* bridge_port_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
	sai_status_t                 status = SAI_STATUS_SUCCESS;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t  *bridge_attr, *port_type_attr, *vlan_attr, *port_attr, *rif_attr, *tunnel_attr, *attr_val, *ing_filter_attr, *tagmode_attr;
    uint32_t                     attr_idx, bridge_idx, rif_idx, tunnel_idx, port_lag_idx, bport_idx, i, lag_idx, port;
    sai_object_list_t            port_objlist;
    sai_object_id_t              port_id[SAI_LAG_MAX_MEMBERS_IN_GROUP_CNS] = {0};
    sai_object_id_t              bridge_oid;
    mrvl_bridge_port_info_t      *bridge_port, *lag_bridge_port;
    bool                         admin_state;

    MRVL_SAI_LOG_ENTER();

    if (NULL == bridge_port_id) {   
        MRVL_SAI_LOG_ERR("NULL bridge port id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_bridge_port_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", list_str);

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_bridge_port_attribs, mrvl_sai_bridge_port_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed bridge port attributes check\n");
        MRVL_SAI_API_RETURN(status);
    }

    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_BRIDGE_ID, &bridge_attr, &attr_idx);
    bridge_oid = (status == SAI_STATUS_SUCCESS) ? bridge_attr->oid : mrvl_sai_default_bridge_id;
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(bridge_oid, SAI_OBJECT_TYPE_BRIDGE, &bridge_idx))) {
        MRVL_SAI_LOG_ERR("Failed to parse bridge id %" PRIx64 "\n", bridge_oid);
        MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS !=
           (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_TYPE, &port_type_attr, &attr_idx)))
    {
        MRVL_SAI_LOG_ERR("Missing mandatory attribute SAI_BRIDGE_PORT_ATTR_TYPE\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
    }

    /* Add bridge port to DB */ 
    if (SAI_STATUS_SUCCESS != mrvl_sai_add_bridge_port(bridge_idx, port_type_attr->s32, &bridge_port)) 
    {
        MRVL_SAI_LOG_ERR("Failed to add bridge port to bridge_ports_db\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

	if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_INGRESS_FILTERING, &ing_filter_attr, &attr_idx))) {
        if (port_type_attr->s32 != SAI_BRIDGE_PORT_TYPE_PORT) {
            MRVL_SAI_LOG_ERR("Ingress filter is only supported for bridge port type port\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_ATTR_NOT_SUPPORTED_0 + attr_idx);
        }
        bridge_port->ingress_filter = ing_filter_attr->booldata;
    }
    
    switch (bridge_port->port_type)
    {
        /** Port or LAG */
        case SAI_BRIDGE_PORT_TYPE_PORT:
            status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_PORT_ID, &port_attr, &attr_idx);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Missing mandatory SAI_BRIDGE_PORT_ATTR_PORT_ID attr\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
            }
            status = mrvl_sai_utl_oid_to_lag_port(port_attr->oid, &port_lag_idx);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Failed to convert port oid %" PRIx64 " to port index\n", port_attr->oid);
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }
            
            if (SAI_CPU_PORT_CNS == port_lag_idx)
            {
                MRVL_SAI_LOG_ERR("Invalid port id - CPU port\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx);
            }
            bridge_port->logical_port = port_lag_idx;
            break;

        /** {Port or LAG.vlan} */
        case SAI_BRIDGE_PORT_TYPE_SUB_PORT:
            status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_PORT_ID, &port_attr, &attr_idx);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Missing mandatory SAI_BRIDGE_PORT_ATTR_PORT_ID attr\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
            }
            status = mrvl_sai_utl_oid_to_lag_port(port_attr->oid, &port_lag_idx);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Failed to convert port oid %" PRIx64 " to port index\n", port_attr->oid);
                MRVL_SAI_API_RETURN(status);
            }
        
            if (SAI_CPU_PORT_CNS == port_lag_idx)
            {
                MRVL_SAI_LOG_ERR("Invalid port id - CPU port\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx);
            }
            bridge_port->parent = port_lag_idx;
            
            status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_VLAN_ID, &vlan_attr, &attr_idx);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Missing mandatory SAI_BRIDGE_PORT_ATTR_VLAN_ID attr\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
            }
            bridge_port->vlan_id = vlan_attr->u16;

            status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_TAGGING_MODE, &tagmode_attr, &attr_idx);
            if (SAI_STATUS_SUCCESS != status)
                bridge_port->tag_mode = SAI_BRIDGE_PORT_TAGGING_MODE_TAGGED; /* SAI default value */
            else
                bridge_port->tag_mode = tagmode_attr->s32;
           
            status = mrvl_sai_vlan_port_add(bridge_port, bridge_port->vlan_id, bridge_port->tag_mode);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Failed to add bridge port %d to vlan %d\n", bridge_port->logical_port, bridge_port->vlan_id);
                MRVL_SAI_API_RETURN(status);
            }
            bridge_port->logical_port = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(bridge_port->vlan_id, port_lag_idx); 
            break;

        /** bridge router port */
        case SAI_BRIDGE_PORT_TYPE_1Q_ROUTER:
            break;
        /** bridge router port */
        case SAI_BRIDGE_PORT_TYPE_1D_ROUTER:
            status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_RIF_ID, &rif_attr, &attr_idx);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Missing mandatory SAI_BRIDGE_PORT_ATTR_RIF_ID attr\n");
                MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
            }
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_attr->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
                MRVL_SAI_LOG_ERR("Failed to convert rif oid %" PRIx64 " to rif index\n", rif_attr->oid);
                MRVL_SAI_API_RETURN(status);
            }
            bridge_port->rif_id = rif_idx;
            break;

        /** bridge tunnel port */
        case SAI_BRIDGE_PORT_TYPE_TUNNEL:
             if (SAI_STATUS_SUCCESS ==
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_TUNNEL_ID, &tunnel_attr, &attr_idx))) {
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(tunnel_attr->oid, SAI_OBJECT_TYPE_TUNNEL, &tunnel_idx))) {
                    MRVL_SAI_LOG_ERR("Failed to convert tunnel oid %" PRIx64 " to tunnel index\n", tunnel_attr->oid);
                    MRVL_SAI_API_RETURN(status);
                }
                bridge_port->tunnel_id = tunnel_idx;
             }
            break;

        default:
            MRVL_SAI_LOG_ERR("Unsupported bridge port type %d\n", port_type_attr->s32);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_idx);
            break;
    }
    
    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_BRIDGE_PORT_ATTR_ADMIN_STATE, &attr_val, &attr_idx);
    if (SAI_STATUS_SUCCESS != status) {
        admin_state = false;
    } else {
        admin_state = attr_val->booldata;
    }

    /* create SAI BRIDGE PORT object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_object_create(bridge_port->index, bridge_port_id))) {
        MRVL_SAI_API_RETURN(status);
    }
    
    bridge_port_id_key_to_str(*bridge_port_id, key_str);
    MRVL_SAI_LOG_NTC("Successfully created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_bridge_port (
        _In_ sai_object_id_t bridge_port_id)
{
    char                        key_str[MAX_KEY_STR_LEN];
    mrvl_bridge_port_info_t     *bport;
    uint32_t                    index;

	MRVL_SAI_LOG_ENTER();

    bridge_port_id_key_to_str(bridge_port_id, key_str);
    MRVL_SAI_LOG_NTC("Remove bridge port %s\n", key_str);

    assert(SAI_STATUS_SUCCESS == mrvl_sai_bridge_port_by_oid_get(bridge_port_id, &bport));

    switch (bport->port_type) {
    case SAI_BRIDGE_PORT_TYPE_PORT:
    case SAI_BRIDGE_PORT_TYPE_SUB_PORT:
    case SAI_BRIDGE_PORT_TYPE_1Q_ROUTER:
    case SAI_BRIDGE_PORT_TYPE_1D_ROUTER:
    case SAI_BRIDGE_PORT_TYPE_TUNNEL:
        break;
    default:
        break;
    }
    
    memset(bport, 0, sizeof(*bport));
    
    MRVL_SAI_LOG_NTC("Removed bridge port %s\n", key_str);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Set attribute for bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 * @param[in] attr Attribute to set
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_set_bridge_port_attribute(
        _In_ sai_object_id_t bridge_port_id,
        _In_ const sai_attribute_t *attr)
{
	const sai_object_key_t key = { .key.object_id = bridge_port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    bridge_port_id_key_to_str(bridge_port_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_bridge_port_attribs, mrvl_sai_bridge_port_vendor_attribs, attr);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get attributes of bridge port
 *
 * @param[in] bridge_port_id Bridge port ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_bridge_port_attribute(
        _In_ sai_object_id_t bridge_port_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
	const sai_object_key_t key = { .key.object_id = bridge_port_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == bridge_port_id) {
        MRVL_SAI_LOG_ERR("NULL bridge port id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    bridge_port_id_key_to_str(bridge_port_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_bridge_port_attribs, mrvl_sai_bridge_port_vendor_attribs, attr_count, attr_list);
    
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_API_RETURN(status); 
}

/**
 * @brief Get bridge port statistics counters.
 *
 * @param[in] bridge_port_id Bridge port id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 * @param[out] counters Array of resulting counter values.
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_sai_get_bridge_port_stats(
        _In_ sai_object_id_t bridge_port_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_bridge_port_stat_t *counter_ids,
        _Out_ uint64_t *counters)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
 * @brief Clear bridge port statistics counters.
 *
 * @param[in] bridge_port_id Bridge port id
 * @param[in] number_of_counters Number of counters in the array
 * @param[in] counter_ids Specifies the array of counter ids
 *
 * @return #SAI_STATUS_SUCCESS on success, failure status code on error
 */
sai_status_t mrvl_sai_clear_bridge_port_stats(
        _In_ sai_object_id_t bridge_port_id,
        _In_ uint32_t number_of_counters,
        _In_ const sai_bridge_port_stat_t *counter_ids)
{
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}


/**
 * @brief Bridge methods table retrieved with sai_api_query()
 */
const sai_bridge_api_t bridge_api =
{
    mrvl_sai_create_bridge,
    mrvl_sai_remove_bridge,
    mrvl_sai_set_bridge_attribute,
    mrvl_sai_get_bridge_attribute,
    mrvl_sai_get_bridge_stats,
    mrvl_sai_clear_bridge_stats,
    mrvl_sai_create_bridge_port,
    mrvl_sai_remove_bridge_port,
    mrvl_sai_set_bridge_port_attribute,
    mrvl_sai_get_bridge_port_attribute,
    mrvl_sai_get_bridge_port_stats,
    mrvl_sai_clear_bridge_port_stats
};
