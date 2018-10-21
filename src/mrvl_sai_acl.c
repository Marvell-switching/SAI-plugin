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
#define __MODULE__ SAI_ACL

#define SAI_ACL_ENTRIES_PER_GROUP_MAX_NUM    1000
#define SAI_ACL_ENTRIES_PER_TABLE_MAX_NUM    SAI_ACL_ENTRIES_PER_GROUP_MAX_NUM
#define SAI_ACL_GROUP_MAX_NUM        4
#define SAI_ACL_TABLES_MAX_NUM        32*SAI_ACL_GROUP_MAX_NUM
#define SAI_ACL_ENTRIES_MAX_NUM       SAI_ACL_ENTRIES_PER_GROUP_MAX_NUM*SAI_ACL_GROUP_MAX_NUM
#define SAI_ACL_COUNTERS_MAX_NUM     1024 /*temporary - needs to be changed! */
#define SAI_ACL_RANGES_MAX_NUM       8
#define SAI_ACL_PORTLIST_MAX_NUM       28 /* current limitation of supported ports in ACL */

#define SAI_ACL_UINT_32_MASK 0xFFFFFFFF
#define SAI_ACL_UINT_16_MASK 0xFFFF
#define SAI_ACL_VLAN_MASK    0x0FFF
#define SAI_ACL_UINT_8_MASK 0xFF
#define SAI_ACL_ARP_OPCODE_REQUEST 1
#define SAI_ACL_ARP_OPCODE_REPLY   2
#define SAI_ACL_INVALID_INDEX 0xFFFFFFFF
#define SAI_ACL_INVALID_INTERFACE 0xFFFFFFFF



typedef enum
{
    SAI_ACL_MATCH_FIELD_SRC_IPV6,
    SAI_ACL_MATCH_FIELD_DST_IPV6,
    SAI_ACL_MATCH_FIELD_SRC_MAC,
    SAI_ACL_MATCH_FIELD_DST_MAC,
    SAI_ACL_MATCH_FIELD_SRC_IP,
    SAI_ACL_MATCH_FIELD_DST_IP,
    SAI_ACL_MATCH_FIELD_IN_PORT,
    SAI_ACL_MATCH_FIELD_OUT_PORT,
    SAI_ACL_MATCH_FIELD_SRC_PORT,
    SAI_ACL_MATCH_FIELD_OUTER_VLAN_ID,
    SAI_ACL_MATCH_FIELD_OUTER_VLAN_PRI,
    SAI_ACL_MATCH_FIELD_L4_SRC_PORT,
    SAI_ACL_MATCH_FIELD_L4_DST_PORT,
    SAI_ACL_MATCH_FIELD_ETHER_TYPE,
    SAI_ACL_MATCH_FIELD_IP_PROTOCOL,
    SAI_ACL_MATCH_FIELD_DSCP,
    SAI_ACL_MATCH_FIELD_ECN,
    SAI_ACL_MATCH_FIELD_ACL_IP_TYPE,
    SAI_ACL_MATCH_FIELD_ACL_IP_FRAG,
    SAI_ACL_MATCH_FIELD_ICMP_TYPE,
    SAI_ACL_MATCH_FIELD_ICMP_CODE,
    SAI_ACL_MATCH_FIELD_RANGE,

    SAI_ACL_MATCH_FIELD_LAST
} mrvl_acl_match_fields_ent;

typedef struct {
    uint32_t                aggr_portbitmap;
    uint32_t                bound_lagbitmap;
} mrvl_acl_bound_lag_id_t;

typedef union {
    uint32_t                bound_portbitmap;
    uint32_t                bound_vlan;
    uint32_t                bound_switch;
    mrvl_acl_bound_lag_id_t bound_lag;
} mrvl_acl_bound_interface_id_t;

typedef struct _mrvl_acl_bound_interface_t {
    uint32_t                      type;
    mrvl_acl_bound_interface_id_t id;
} mrvl_acl_bound_interface_t;

typedef struct _mrvl_acl_group_db_t {
	bool                          is_used;
    uint32_t                      stage;
    uint32_t                      type;
    uint32_t                      bind_point_types_bitmap;
	sai_object_id_t               group_id;
	uint32_t                      group_size;         /* max num of entries in group */
	uint32_t                      entries_count;      /* current num of entries */
    uint32_t                      tables_count; /* current num of tables */
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type;
} mrvl_acl_group_db_t;

typedef struct _mrvl_acl_table_db_t {
    bool                          is_used;
    sai_acl_stage_t               stage;
    uint32_t                      attr_table_size; 
    uint32_t                      table_size; /* SAI_ACL_TABLE_ATTR_SIZE or SAI_ACL_ENTRIES_MAX_NUM */
    uint32_t                      priority;
    bool                          is_dynamic_size; /* true when uSAI_ACL_TABLE_ATTR_SIZE is 0 */
    uint32_t                      entries_count; /* current num of entries */
    uint32_t                      head_entry_index; /* index of first entry linked to the table */
    uint32_t                      acl_group_index;
    uint32_t                      table_fields_bitmap;
    uint32_t                      table_actions_bitmap;
    uint32_t                      bind_point_types_bitmap;
    mrvl_acl_bound_interface_t    bound_interface;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type;
    bool                          packet_count_enable;
    bool                          byte_count_enable;
    uint32_t                      prev_acl_table_index; /* linked ACL tables per port */
    uint32_t                      next_acl_table_index;
} mrvl_acl_table_db_t;

typedef struct _mrvl_acl_entry_db_t {
    bool                      is_used;
    uint32_t                  table_index;
    uint64_t                  fpa_cookie;
    uint16_t                  priority;
    uint32_t                  attr_port; /* to be downloaded to FPA must be equal to bound port */
    uint32_t                  attr_vlan; /* to be downloaded to FPA must be equal to bound vlan */
    bool                      admin_state;
    sai_object_id_t           range_id; /* assigned range id, null if not assinged */
    uint32_t                  entry_fields_bitmap;
    uint32_t                  entry_actions_bitmap;
    sai_acl_action_data_t     entry_action_data[SAI_ACL_MAX_ACTION_TYPES];
    uint32_t                  prev_acl_entry_index;/* linked ACL entries per table */
    uint32_t                  next_acl_entry_index;
} mrvl_acl_entry_db_t;

typedef struct _mrvl_acl_bound_db_t {
    uint32_t                  bound_tables_count; /* current num of tables bound to port */
    uint32_t                  head_table_index;
} mrvl_acl_bound_db_t;

typedef struct _mrvl_acl_bound_lag_db_t {
    mrvl_acl_bound_db_t       lag_bound_db;
    uint32_t                  portbitmap;
} mrvl_acl_bound_lag_db_t;

typedef struct _mrvl_acl_range_db_t {
    bool                      is_used;
    uint32_t                  entry_index;
    sai_u32_range_t           range_limit;
    uint32_t                  range_type;
} mrvl_acl_range_db_t;

static mrvl_acl_group_db_t mrvl_sai_acl_group_db[SAI_ACL_GROUP_MAX_NUM] = {};

static mrvl_acl_table_db_t mrvl_sai_acl_table_db[SAI_ACL_TABLES_MAX_NUM] = {};

static mrvl_acl_entry_db_t mrvl_sai_acl_entry_db[SAI_ACL_ENTRIES_MAX_NUM] = {};

static mrvl_acl_range_db_t mrvl_sai_acl_range_db[SAI_ACL_RANGES_MAX_NUM] = {};

static mrvl_acl_bound_db_t mrvl_sai_acl_bound_port_db[SAI_ACL_PORTLIST_MAX_NUM] = {};

static mrvl_acl_bound_lag_db_t mrvl_sai_acl_bound_lag_db[SAI_LAG_MAX_GROUPS_CNS] = {};

static mrvl_acl_bound_db_t mrvl_sai_acl_bound_switch_db = {};

static mrvl_acl_bound_db_t mrvl_sai_acl_bound_vlan_db[SAI_MAX_NUM_OF_VLANS] = {};

#define SAI_ACL_MIN_PRIORITY        0
#define SAI_ACL_MAX_PRIORITY        32*1024
#define SAI_ACL_DYNAMIC_SIZE        0

typedef struct _sai_acl_match_field_to_attribs_t {
	mrvl_acl_match_fields_ent  field;
	sai_attr_id_t              table_attr_id;
	sai_attr_id_t              entry_attr_id;
} sai_acl_match_field_to_attribs_t;

/* SAI_ACL_BIND_POINT_TYPE_PORT is on */
/*  SAI_ACL_BIND_POINT_TYPE_LAG is on */
/*  SAI_ACL_BIND_POINT_TYPE_VLAN is on */
static const uint32_t default_bind_point_type_bitmap = 0x7; /* 000111 */

/* SAI_ACL_ACTION_TYPE_PACKET_ACTION is on */
static const uint32_t default_actions_type_bitmap = 0x4; /* 000100 */

static const sai_acl_match_field_to_attribs_t acl_match_field_to_attribs[] = {
    { SAI_ACL_MATCH_FIELD_SRC_IPV6,       SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6,       SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6},
    { SAI_ACL_MATCH_FIELD_DST_IPV6,       SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6,       SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6 },
    { SAI_ACL_MATCH_FIELD_SRC_MAC,        SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,        SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC },
    { SAI_ACL_MATCH_FIELD_DST_MAC,        SAI_ACL_TABLE_ATTR_FIELD_DST_MAC,        SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC },
    { SAI_ACL_MATCH_FIELD_SRC_IP,         SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,         SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP },
    { SAI_ACL_MATCH_FIELD_DST_IP,         SAI_ACL_TABLE_ATTR_FIELD_DST_IP,         SAI_ACL_ENTRY_ATTR_FIELD_DST_IP },
    { SAI_ACL_MATCH_FIELD_IN_PORT,        SAI_ACL_TABLE_ATTR_FIELD_IN_PORT,        SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT },
    { SAI_ACL_MATCH_FIELD_IN_PORT,        SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT,       SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT },
    { SAI_ACL_MATCH_FIELD_SRC_PORT,       SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT,       SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT },
    { SAI_ACL_MATCH_FIELD_OUTER_VLAN_ID,  SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID,  SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID },
    { SAI_ACL_MATCH_FIELD_OUTER_VLAN_PRI, SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI, SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI },
    { SAI_ACL_MATCH_FIELD_L4_SRC_PORT,    SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,    SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT},
    { SAI_ACL_MATCH_FIELD_L4_DST_PORT,    SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT,    SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT },
    { SAI_ACL_MATCH_FIELD_ETHER_TYPE,     SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,     SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE },
    { SAI_ACL_MATCH_FIELD_IP_PROTOCOL,    SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL,    SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL },
    { SAI_ACL_MATCH_FIELD_ACL_IP_TYPE,    SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE,    SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE },
    { SAI_ACL_MATCH_FIELD_ACL_IP_FRAG,    SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG,    SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG },
    { SAI_ACL_MATCH_FIELD_ICMP_TYPE,      SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE,      SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE },
    { SAI_ACL_MATCH_FIELD_ICMP_CODE,      SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE,      SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE },
    { SAI_ACL_MATCH_FIELD_DSCP,           SAI_ACL_TABLE_ATTR_FIELD_DSCP,           SAI_ACL_ENTRY_ATTR_FIELD_DSCP },
    { SAI_ACL_MATCH_FIELD_ECN,            SAI_ACL_TABLE_ATTR_FIELD_ECN,            SAI_ACL_ENTRY_ATTR_FIELD_ECN },
    { SAI_ACL_MATCH_FIELD_RANGE,          SAI_ACL_TABLE_ATTR_FIELD_ACL_RANGE_TYPE, SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE },
    { SAI_ACL_MATCH_FIELD_LAST,           SAI_ACL_TABLE_ATTR_FIELD_END,            SAI_ACL_ENTRY_ATTR_FIELD_END}
  };

static sai_status_t mrvl_acl_table_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

static sai_status_t mrvl_acl_table_match_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

static sai_status_t mrvl_acl_table_entry_list_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg);

static sai_status_t mrvl_acl_table_available_entries_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg);

static sai_status_t mrvl_acl_table_available_counters_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg);

/* ACL TABLE ATTRIBUTES */
static const sai_attribute_entry_t acl_table_attribs[] = {
    { SAI_ACL_TABLE_ATTR_ACL_STAGE, true, true, false, true,
      "ACL Table Stage", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST, false, true, false, true,
      "ACL Table Bind point type list", SAI_ATTR_VALUE_TYPE_INT32_LIST},
    { SAI_ACL_TABLE_ATTR_SIZE, false, true, false, true,
      "ACL Table Size", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST, false, true, false, true,
      "ACL Table Action type list", SAI_ATTR_VALUE_TYPE_INT32_LIST},
    { SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6, false, true, false, true,
      "Src IPV6 Address", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6, false, true, false, true,
      "Dst IPV6 Address", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC, false, true, false, true,
      "Src MAC Address", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_DST_MAC, false, true, false, true,
      "Dst MAC Address", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_SRC_IP, false, true, false, true,
      "Src IPv4 Address", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_DST_IP, false, true, false, true,
      "Dst IPv4 Address", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS, false, true, false, true,
      "In-Ports", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS, false, true, false, true,
      "Out-Ports", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_IN_PORT, false, true, false, true,
      "In-Port", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT, false, true, false, true,
      "Out-Port", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT, false, true, false, true,
      "Src-Port", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID, false, true, false, true,
      "Outer Vlan-Id", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI, false, true, false, true,
      "Outer Vlan-Priority", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI, false, true, false, true,
      "Outer Vlan-CFI", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID, false, true, false, true,
      "Inner Vlan-Id", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI, false, true, false, true,
      "Inner Vlan-Priority", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI, false, true, false, true,
      "Inner Vlan-CFI", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT, false, true, false, true,
      "L4 Src Port", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT, false, true, false, true,
      "L4 Dst Port", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE, false, true, false, true,
      "EtherType", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL, false, true, false, true,
      "IP Protocol", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_DSCP, false, true, false, true,
      "IP Dscp", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ECN, false, true, false, true,
      "IP Ecn", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_TTL, false, true, false, true,
      "Ip Ttl", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_TOS, false, true, false, true,
      "Ip Tos", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS, false, true, false, true,
      "Ip Flags", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS, false, true, false, false,
      "Tcp Flags", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE, false, true, false, true,
      "Ip Type", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG, false, true, false, true,
      "Ip Frag", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL, false, false, false, false,
      "IPV6 Flow Label", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_TC, false, true, false, true,
      "Class-of-Service", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE, false, true, false, true,
      "ICMP Type", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE, false, true, false, true,
      "ICMP Code", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN, false, true, false, true,
      "Vlan tags", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META, false, false, false, false,
      "FDB DST user meta data", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META, false, false, false, false,
      "ROUTE DST User Meta data", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META, false, false, false, false,
      "Neighbor DST User Meta Data", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META, false, false, false, false,
      "Port User Meta Data", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META, false, false, false, false,
      "Vlan User Meta Data", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META, false, true, false, true,
      "Meta Data carried from previous ACL Stage", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT, false, true, false, false,
      "DST MAC address match in FDB", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, false, true, false, false,
      "DST IP address match in neighbor table", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, false, true, false, false,
       "DST IP address match in route table", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_ACL_TABLE_ATTR_ENTRY_LIST, false, false, false, true,
       "ACL table entries associated with this table", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY, false, false, false, true,
       "ACL table available entries", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER, false, false, false, true,
       "ACL table available counters", SAI_ATTR_VALUE_TYPE_UINT32 },
    
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};

/* ACL TABLE VENDOR ATTRIBUTES */
static const sai_vendor_attribute_entry_t acl_table_vendor_attribs[] = {
	{ SAI_ACL_TABLE_ATTR_ACL_STAGE,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_table_attrib_get, (void*)SAI_ACL_TABLE_ATTR_ACL_STAGE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_table_attrib_get, (void*)SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_SIZE,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_table_attrib_get, (void*)SAI_ACL_TABLE_ATTR_SIZE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_table_attrib_get, (void*)SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6,
	  NULL, NULL},
	{ SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6,
	  NULL, NULL},
	{ SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_DST_MAC,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_DST_MAC,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_SRC_IP,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_DST_IP,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_DST_IP,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_IN_PORTS,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_OUT_PORTS,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_IN_PORT,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_IN_PORT,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_CFI,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_ID,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_PRI,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_INNER_VLAN_CFI,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_DSCP,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_DSCP,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ECN,
	  { true, false, false, true },
	  { true, false, false, true },
      mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_ECN,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_TTL,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_TOS,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_IP_FLAGS,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_TCP_FLAGS,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_IPV6_FLOW_LABEL,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_TC,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE,
	  { true, false, false, true },
	  { true, false, false, true },
	  mrvl_acl_table_match_attrib_get, (void*)SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_PACKET_VLAN,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_FDB_DST_USER_META,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ROUTE_DST_USER_META,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_DST_USER_META,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_PORT_USER_META,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_VLAN_USER_META,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ACL_USER_META,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_FDB_NPU_META_DST_HIT,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT,
	  { false, false, false, false },
	  { false, false, false, false },
	  NULL, NULL,
	  NULL, NULL },
	{ SAI_ACL_TABLE_ATTR_FIELD_ROUTE_NPU_META_DST_HIT,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL },
    { SAI_ACL_TABLE_ATTR_ENTRY_LIST,
	  { false, false, false, true },
	  { false, false, false, true },
	  mrvl_acl_table_entry_list_get, NULL,
	  NULL, NULL },
    { SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY,
	  { false, false, false, true },
	  { false, false, false, true },
	  mrvl_acl_table_available_entries_get, NULL,
	  NULL, NULL },
    { SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER,
	  { false, false, false, true },
	  { false, false, false, true },
	  mrvl_acl_table_available_counters_get, NULL,
	  NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
	  { false, false, false, true },
	  { false, false, false, true },
	  NULL, NULL,
	  NULL, NULL }
    
};


static sai_status_t mrvl_acl_entry_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

static sai_status_t mrvl_acl_entry_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg);

static sai_status_t mrvl_acl_entry_match_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

static sai_status_t mrvl_acl_entry_match_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg);

static sai_status_t mrvl_acl_entry_action_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

static sai_status_t mrvl_acl_entry_action_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg);


/* ACL ENTRY ATTRIBUTES */
static const sai_attribute_entry_t acl_entry_attribs[] = {   
	{ SAI_ACL_ENTRY_ATTR_TABLE_ID, true, true, false, true,
	  "ACL Entry Table Id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_PRIORITY, false, true, true, true,
	  "ACL Entry Priority ", SAI_ATTR_VALUE_TYPE_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_ADMIN_STATE, false, true, true, true,
	  "ACL Entry Admin State", SAI_ATTR_VALUE_TYPE_BOOL },
	{ SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6, false, true, true, true,
	  "Src IPV6 Address", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6, false, true, true, true,
	  "Dst IPV6 Address", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV6 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC, false, true, true, true,
	  "Src MAC Address", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC },
	{ SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC, false, true, true, true,
	  "Dst MAC Address", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_MAC },
	{ SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, false, true, true, true,
	  "Src IPv4 Address", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_DST_IP, false, true, true, true,
	  "Dst IPv4 Address", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS, false, true, true, true,
	  "In-Ports",  SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS, false, true, true, true,
	  "Out-Ports", SAI_ATTR_VALUE_TYPE_OBJECT_ID},
	{ SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, false, true, true, true,
	  "In-Port", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, false, true, true, true,
	  "Out-Port", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT, false, true, true, true,
	  "Src-Port", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID, false, true, true, true,
	  "Outer Vlan-Id", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI, false, true, true, true,
	  "Outer Vlan-Priority", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI, false, true, true, true,
	  "Outer Vlan-CFI", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID, false, true, true, true,
	  "Inner Vlan-Id", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI, false, true, true, true,
	  "Inner Vlan-Priority", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI, false, true, true, true,
	  "Inner Vlan-CFI", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT, false, true, true, true,
	  "L4 Src Port", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT, false, true, true, true,
	  "L4 Dst Port", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE, false, true, true, true,
	  "EtherType", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL, false, true, true, true,
	  "IP Protocol", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_DSCP, false, true, true, true,
	  "Ip Dscp", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ECN, false, true, true, true,
	  "Ip Ecn", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_TTL, false, true, true, true,
	  "Ip Ttl", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_TOS, false, true, true, true,
	  "Ip Tos", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS, false, true, true, true,
	  "Ip Flags", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS, false, true, true, true,
	  "Tcp Flags", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE, false, true, true, true,
	  "Ip Type",  SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG, false, true, true, true,
	  "Ip Frag", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL, false, false, false, false,
	  "IPV6 Flow Label",  SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_TC, false, true, true, true,
	  "Class-of-Service (Traffic Class)", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE, false, true, true, true,
	  "ICMP Type", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE, false, true, true, true,
	  "ICMP Code", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN, false, true, true, true,
	  "Vlan tags", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META, false, false, false, false,
	  "FDB DST user meta data", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META, false, false, false, false,
	  "ROUTE DST User Meta data", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META, false, false, false, false,
	  "Neighbor DST User Meta Data", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META, false, false, false, false,
	  "Port User Meta Data", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META, false, false, false, false,
	  "Vlan User Meta Data", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META, false, true, true, true,
	  "Meta Data carried from previous ACL Stage", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT, false, false, false, false,
	  "DST MAC address match in FDB", SAI_ATTR_VALUE_TYPE_MAC },
	{ SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT, false, false, false, false,
	  "DST IP address match in neighbor table", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT, false, false, false, false,
	  "DST IP address match in neighbor table", SAI_ATTR_VALUE_TYPE_ACL_FIELD_DATA_IPV4 },
	{ SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE, false, true, true, true,
	  "Range Type", SAI_ATTR_VALUE_TYPE_OBJECT_ID },



	{ SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT, false, true, true, true,
	  "Redirect Packet to a destination", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST, false, true, true, true,
	  "Redirect Packet to a destination list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
	{ SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION, false, true, true, true,
	  "Drop Packet", SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_FLOOD, false, true, true, false,
	  "Flood Packet on Vlan domain", SAI_ATTR_VALUE_TYPE_UNDETERMINED },
	{ SAI_ACL_ENTRY_ATTR_ACTION_COUNTER, false, true, true, true,
	  "Attach/detach counter id to the entry", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS, false, true, true, true,
	  "Ingress Mirror", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS, false, true, true, true,
	  "Egress Mirror", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER, false, true, true, true,
	  "Associate with policer", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL, false, true, true, false,
	  "Decrement TTL", SAI_ATTR_VALUE_TYPE_UNDETERMINED },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_TC, false, true, true, true,
	  "Set Class-of-Service",  SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR, false, true, true, true,
	  "Set packet color",  SAI_ATTR_VALUE_TYPE_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID, false, true, true, true,
	  "Set Packet Inner Vlan-Id", SAI_ATTR_VALUE_TYPE_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI, false, true, true, true,
	  "Set Packet Inner Vlan-Priority", SAI_ATTR_VALUE_TYPE_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, false, true, true, true,
	  "Set Packet Outer Vlan-Id", SAI_ATTR_VALUE_TYPE_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI, false, true, true, true,
	  "Set Packet Outer Vlan-Priority", SAI_ATTR_VALUE_TYPE_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC, false, true, true, true,
	  "Set Packet Src MAC Address", SAI_ATTR_VALUE_TYPE_MAC },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC, false, true, true, true,
	  "Set Packet Dst MAC Address", SAI_ATTR_VALUE_TYPE_MAC },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP, false, false, false, false,
	  "Set Packet Src IPv4 Address", SAI_ATTR_VALUE_TYPE_IPV4 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP, false, false, false, false,
	  "Set Packet Dst IPv4 Address", SAI_ATTR_VALUE_TYPE_IPV4 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6, false, false, false, false,
	  "Set Packet Src IPV6 Address", SAI_ATTR_VALUE_TYPE_IPV6 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6, false, false, false, false,
	  "Set Packet Dst IPV6 Address", SAI_ATTR_VALUE_TYPE_IPV6 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP, false, true, true, true,
	  "Set Packet DSCP", SAI_ATTR_VALUE_TYPE_ACL_ACTION_DATA_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN, false, true, true, true,
	  "Set Packet ECN", SAI_ATTR_VALUE_TYPE_UINT8 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT, false, false, false, false,
	  "Set Packet L4 Src Port", SAI_ATTR_VALUE_TYPE_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT, false, false, false, false,
	  "Set Packet L4 Dst Port", SAI_ATTR_VALUE_TYPE_UINT16 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE, false, false, false, false,
	  "Set ingress packet sampling", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE, false, false, false, false,
	  "Set egress packet sampling", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA, false, true, true, true,
	  "Set Meta Data", SAI_ATTR_VALUE_TYPE_UINT32 },
	{ SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST, false, true, true, true,
	  "Egress block port list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
	{ SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID, false, true, true, true,
	  "Set user def trap ID", SAI_ATTR_VALUE_TYPE_UINT32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};

/* ACL ENTRY VENDOR ATTRIBUTES */
static const sai_vendor_attribute_entry_t acl_entry_vendor_attribs[] = {
    { SAI_ACL_ENTRY_ATTR_TABLE_ID,
      {true, false, false, true},
      {true, false, false, true},
      mrvl_acl_entry_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_TABLE_ID,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_PRIORITY,
      {true, false, true, true},
      {true, false, true, true},
      mrvl_acl_entry_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_PRIORITY,
      mrvl_acl_entry_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_PRIORITY },
    { SAI_ACL_ENTRY_ATTR_ADMIN_STATE,
      {true, false, true, true},
      {true, false, true, true},
      mrvl_acl_entry_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_ADMIN_STATE,
      mrvl_acl_entry_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_ADMIN_STATE },
    { SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6 },
    { SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6 },
    { SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC },
    { SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC },
    { SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP },
    { SAI_ACL_ENTRY_ATTR_FIELD_DST_IP,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DST_IP,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DST_IP },
    { SAI_ACL_ENTRY_ATTR_FIELD_IN_PORTS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORTS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT },
    { SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT,
	  { true, false, true, true },
	  { true, false, true, true },
	  mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT,
	  mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT },
    { SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT,
      { true, false, true, true },
      { true, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID },
    { SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI },
    { SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_CFI,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_PRI,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_INNER_VLAN_CFI,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT },
    { SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT },
    { SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE },
    { SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL },
    { SAI_ACL_ENTRY_ATTR_FIELD_DSCP,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DSCP,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_DSCP },
    { SAI_ACL_ENTRY_ATTR_FIELD_ECN,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ECN,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ECN },
    { SAI_ACL_ENTRY_ATTR_FIELD_TTL,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_TOS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_IP_FLAGS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL, },
    { SAI_ACL_ENTRY_ATTR_FIELD_TCP_FLAGS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE },
    { SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG,
      mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG },
    { SAI_ACL_ENTRY_ATTR_FIELD_IPV6_FLOW_LABEL,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_TC,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE,
	  { true, false, true, true },
	  { true, false, true, true },
	  mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE,
	  mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE },
    { SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE,
	  { true, false, true, true },
	  { true, false, true, true },
	  mrvl_acl_entry_match_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE,
	  mrvl_acl_entry_match_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE },
    { SAI_ACL_ENTRY_ATTR_FIELD_PACKET_VLAN,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_FDB_DST_USER_META,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_DST_USER_META,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_DST_USER_META,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_PORT_USER_META,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_VLAN_USER_META,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_ACL_USER_META,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_FDB_NPU_META_DST_HIT,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_NEIGHBOR_NPU_META_DST_HIT,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_ROUTE_NPU_META_DST_HIT,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE,
      { false, false, false, false },
	  { false, false, false, false },
      NULL, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE,
      NULL, (void*)SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE },



    { SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_REDIRECT_LIST,
      { false, false, false, false},
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_action_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION,
      mrvl_acl_entry_action_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION },
    { SAI_ACL_ENTRY_ATTR_ACTION_FLOOD,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_COUNTER,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_action_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_ACTION_COUNTER,
      mrvl_acl_entry_action_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_ACTION_COUNTER },
    { SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_INGRESS,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    {  SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS,
       { false, false, false, false },
       { false, false, false, false },
       NULL, NULL,
       NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_DECREMENT_TTL,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_TC,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_action_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_ACTION_SET_TC,
      mrvl_acl_entry_action_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_ACTION_SET_TC },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_acl_entry_action_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR,
      mrvl_acl_entry_action_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_ACTION_SET_PACKET_COLOR },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_INNER_VLAN_PRI,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_MAC,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_MAC,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IP,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IP,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_SRC_IPV6,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_DST_IPV6,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP,
      { true, false, true, true},
      { true, false, true, true},
      mrvl_acl_entry_action_attrib_get, (void*)SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP,
      mrvl_acl_entry_action_attrib_set, (void*)SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_ECN,
      { true, false, true, true},
      { true, false, true, true},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_SRC_PORT,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_L4_DST_PORT,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_ACL_META_DATA,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_BLOCK_PORT_LIST,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
    { SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID,
      { false, false, false, false},
      { false, false, false, false},
      NULL, NULL,
      NULL, NULL },
};
static sai_status_t mrvl_acl_group_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

/* ACL TABLE GROUP ATTRIBUTES */
static const sai_attribute_entry_t acl_table_group_attribs[] = {
    { SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE, true, true, false, true,
      "ACL Table Group Stage", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST, false, true, false, true,
      "ACL Table Group Bind point type list", SAI_ATTR_VALUE_TYPE_INT32_LIST},
    { SAI_ACL_TABLE_GROUP_ATTR_TYPE, false, true, false, true,
      "ACL Table Group type", SAI_ATTR_VALUE_TYPE_UINT32 },
	{   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};

/* ACL TABLE GROUP VENDOR ATTRIBUTES */
static const sai_vendor_attribute_entry_t acl_table_group_vendor_attribs[] = {
	{ SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_group_attrib_get, (void*)SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE,
	  NULL, NULL },
	{ SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_group_attrib_get, (void*)SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST,
	  NULL, NULL },
	{ SAI_ACL_TABLE_GROUP_ATTR_TYPE,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_group_attrib_get, (void*)SAI_ACL_TABLE_GROUP_ATTR_TYPE,
	  NULL, NULL }
};

static sai_status_t mrvl_acl_group_member_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);


/* ACL TABLE GROUP MEMBER ATTRIBUTES */
static const sai_attribute_entry_t acl_table_group_member_attribs[] = {
    { SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID, true, true, false, true,
      "ACL Table Group Member group id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID, true, true, false, true,
      "ACL Table Group Member table id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY, true, true, false, true,
      "ACL Table Group Member Priority", SAI_ATTR_VALUE_TYPE_UINT32 },
	{   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};

/* ACL TABLE GROUP MEMBER VENDOR ATTRIBUTES */
static const sai_vendor_attribute_entry_t acl_table_group_member_vendor_attribs[] = {
	{ SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_group_member_attrib_get, (void*)SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID,
	  NULL, NULL },
	{ SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_group_member_attrib_get, (void*)SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID,
	  NULL, NULL },
	{ SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_group_member_attrib_get, (void*)SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY,
	  NULL, NULL }
};


static sai_status_t mrvl_acl_counter_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);

static sai_status_t mrvl_acl_counter_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg);

/* ACL COUNTERS ATTRIBUTES */
static const sai_attribute_entry_t acl_counter_attribs[] = {   
	{ SAI_ACL_COUNTER_ATTR_TABLE_ID, true, true, false, true,
	  "ACL Table id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
	{ SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT, false, true, false, true,
	  "Enable/disable packet count", SAI_ATTR_VALUE_TYPE_BOOL },
	{ SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT, false, true, false, true,
	  "Enable/disable byte count", SAI_ATTR_VALUE_TYPE_BOOL },
	{ SAI_ACL_COUNTER_ATTR_PACKETS, false, true, true, true,
	  "Get/set packet count", SAI_ATTR_VALUE_TYPE_UINT64 },
	{ SAI_ACL_COUNTER_ATTR_BYTES, false, true, true, true,
	  "Get/set byte count", SAI_ATTR_VALUE_TYPE_UINT64 },
	{   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
	  "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
	}
};
/* ACL COUNTERS VENDOR ATTRIBUTES */
static const sai_vendor_attribute_entry_t acl_counter_vendor_attribs[] = {
	{ SAI_ACL_COUNTER_ATTR_TABLE_ID,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_counter_attrib_get, (void*)SAI_ACL_COUNTER_ATTR_TABLE_ID,
	  NULL, NULL },
	{ SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_counter_attrib_get, (void*)SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT,
	  NULL, NULL },
	{ SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_counter_attrib_get, (void*)SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT,
	  NULL, NULL },
	{ SAI_ACL_COUNTER_ATTR_PACKETS,
	  {true, false, true, true},
	  {true, false, true, true},
	  mrvl_acl_counter_attrib_get, (void*)SAI_ACL_COUNTER_ATTR_PACKETS,
	  mrvl_acl_counter_attrib_set, (void*)SAI_ACL_COUNTER_ATTR_PACKETS},
	{ SAI_ACL_COUNTER_ATTR_BYTES,
	  {true, false, true, true},
	  {true, false, true, true},
	  mrvl_acl_counter_attrib_get, (void*)SAI_ACL_COUNTER_ATTR_BYTES,
	  mrvl_acl_counter_attrib_set, (void*)SAI_ACL_COUNTER_ATTR_BYTES }
};

static sai_status_t mrvl_acl_range_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg);


/* ACL RANGE ATTRIBUTES */
static const sai_attribute_entry_t acl_range_attribs[] = {
    { SAI_ACL_RANGE_ATTR_TYPE, true, true, false, true,
      "ACL Range type", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_ACL_RANGE_ATTR_LIMIT, true, true, false, true,
      "ACL Range limit", SAI_ATTR_VALUE_TYPE_UINT32_RANGE },
	{   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};

/* ACL RANGE VENDOR ATTRIBUTES */
static const sai_vendor_attribute_entry_t acl_range_vendor_attribs[] = {
	{ SAI_ACL_RANGE_ATTR_TYPE,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_range_attrib_get, (void*)SAI_ACL_RANGE_ATTR_TYPE,
	  NULL, NULL },
	{ SAI_ACL_RANGE_ATTR_LIMIT,
	  {true, false, false, true},
	  {true, false, false, true},
	  mrvl_acl_range_attrib_get, (void*)SAI_ACL_RANGE_ATTR_LIMIT,
	  NULL, NULL }
};


static sai_status_t mrvl_acl_clear_counters(
    _In_ uint32_t      acl_table_index)
{
	uint32_t acl_entry_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};

    MRVL_SAI_LOG_ENTER();

	/* For all entries in table */
	acl_entry_index = mrvl_sai_acl_table_db[acl_table_index].head_entry_index;
	fpa_table_type = mrvl_sai_acl_table_db[acl_table_index].fpa_table_type;
	while (acl_entry_index != SAI_ACL_INVALID_INDEX){

		memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
		fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;

		/* get by cookie from FPA */
		fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
		if (fpa_status != FPA_OK) {
			MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
			status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
			MRVL_SAI_API_RETURN(status);
		}
		fpaLibFlowEntryStatisticsClear(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
		if (fpa_status != FPA_OK) {
			MRVL_SAI_LOG_ERR("Failed to clear counters from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
			status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
			MRVL_SAI_API_RETURN(status);
		}

		acl_entry_index = mrvl_sai_acl_entry_db[acl_entry_index].next_acl_entry_index;
	}

    return SAI_STATUS_SUCCESS;
}


/* ACL counter get per attribute functions */
static sai_status_t mrvl_acl_counter_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_table_index = 0, acl_entry_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};
    FPA_FLOW_ENTRY_COUNTERS_STC counters = {0};


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_COUNTER, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_table_db[acl_table_index].is_used == true);

	switch ((int64_t)arg) {
    case SAI_ACL_COUNTER_ATTR_TABLE_ID:
	    /* create ACL table object */
	    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &value->oid))) {
	    	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
	    	MRVL_SAI_API_RETURN(status);
	    }
        break;
    case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
		value->booldata = mrvl_sai_acl_table_db[acl_table_index].packet_count_enable;
        break;
    case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
    	value->booldata = mrvl_sai_acl_table_db[acl_table_index].byte_count_enable;
		break;
    case SAI_ACL_COUNTER_ATTR_PACKETS:
    case SAI_ACL_COUNTER_ATTR_BYTES:

    	 if (((int64_t)arg == SAI_ACL_COUNTER_ATTR_PACKETS) && (mrvl_sai_acl_table_db[acl_table_index].packet_count_enable == false)){
    		 MRVL_SAI_LOG_ERR("Unable to get packet's counter acl_table_index - %d\n", acl_table_index);
    		 MRVL_SAI_API_RETURN(status);
    	 }
    	 if (((int64_t)arg == SAI_ACL_COUNTER_ATTR_BYTES) && (mrvl_sai_acl_table_db[acl_table_index].byte_count_enable == false)){
    		 MRVL_SAI_LOG_ERR("Unable to get packet's counter acl_table_index - %d\n", acl_table_index);
    		 MRVL_SAI_API_RETURN(status);
    	 }
        /* For all entries in table */
    	acl_entry_index = mrvl_sai_acl_table_db[acl_table_index].head_entry_index;
    	fpa_table_type = mrvl_sai_acl_table_db[acl_table_index].fpa_table_type;
    	while (acl_entry_index != SAI_ACL_INVALID_INDEX){

    		memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
    		fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;

    		/* get by cookie from FPA */
    		fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
    		if (fpa_status != FPA_OK) {
    			MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
    			status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    			MRVL_SAI_API_RETURN(status);
    		}
    		fpaLibFlowEntryStatisticsGet(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry, &counters);
    		if (fpa_status != FPA_OK) {
    			MRVL_SAI_LOG_ERR("Failed to clear counters from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
    			status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    			MRVL_SAI_API_RETURN(status);
    		}
    		if ((int64_t)arg == SAI_ACL_COUNTER_ATTR_PACKETS){
    			value->u64 += counters.packetCount;
    		}
    		else {
    			value->u64 += counters.byteCount;
    		}
    		acl_entry_index = mrvl_sai_acl_entry_db[acl_entry_index].next_acl_entry_index;
    	}
        break;
    default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}
/* ACL counter set per attribute functions */
static sai_status_t mrvl_acl_counter_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{
	uint32_t acl_table_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_COUNTER, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid table %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_table_db[acl_table_index].is_used == true);

	switch ((int64_t)arg) {
    case SAI_ACL_COUNTER_ATTR_PACKETS:
    case SAI_ACL_COUNTER_ATTR_BYTES:
		 if (((int64_t)arg == SAI_ACL_COUNTER_ATTR_PACKETS) && (mrvl_sai_acl_table_db[acl_table_index].packet_count_enable == false)){
			 MRVL_SAI_LOG_ERR("Unable to get packet's counter acl_table_index - %d\n", acl_table_index);
			 MRVL_SAI_API_RETURN(status);
		 }
		 if (((int64_t)arg == SAI_ACL_COUNTER_ATTR_BYTES) && (mrvl_sai_acl_table_db[acl_table_index].byte_count_enable == false)){
			 MRVL_SAI_LOG_ERR("Unable to get packet's counter acl_table_index - %d\n", acl_table_index);
			 MRVL_SAI_API_RETURN(status);
		 }

    	 mrvl_acl_clear_counters(acl_table_index);
    	break;
	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_bind_point_type_list_validate(_In_ const sai_s32_list_t        *attr_bind_point_types_list,
                                                           _In_ uint32_t                     attr_index,
                                                           _Out_ uint32_t                   *bind_point_types_bitmap)
{
    sai_acl_bind_point_type_t type;
    uint32_t                  i;

    assert(attr_bind_point_types_list != NULL);

    if (0 == attr_bind_point_types_list->count) {
        MRVL_SAI_LOG_ERR("Count of bind point types is 0\n");
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index;
    }

    for (i = 0; i < attr_bind_point_types_list->count; i++) {
        type = attr_bind_point_types_list->list[i];

        if (type >= SAI_ACL_MAX_BIND_POINT_TYPES) {
            MRVL_SAI_LOG_ERR("Invalid bind point type (%d) at index [%d] in the list\n", type, i);
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index;
        }

        mrvl_acl_set_bit_MAC(*bind_point_types_bitmap, type);

    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_set_mapping_field_for_table_attrib(_In_ const sai_attr_id_t    attr_id,
                                    			          _Out_ uint32_t *table_fields_bitmap)
{
    uint32_t attr_count;
    mrvl_acl_match_fields_ent field;
    assert(table_fields_bitmap != NULL);

    MRVL_SAI_LOG_ENTER();

    for (attr_count = 0;
    		SAI_ACL_TABLE_ATTR_FIELD_END != acl_match_field_to_attribs[attr_count].table_attr_id;
    		attr_count++) {
        if (attr_id == acl_match_field_to_attribs[attr_count].table_attr_id) {
        	field = acl_match_field_to_attribs[attr_count].field;
        	break;
        }
    }

    if (SAI_ACL_TABLE_ATTR_FIELD_END == acl_match_field_to_attribs[attr_count].table_attr_id){
    	return SAI_STATUS_FAILURE;
    }

    mrvl_acl_set_bit_MAC(*table_fields_bitmap, field);

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_is_set_mapping_field_for_table_attrib(_In_ sai_attr_id_t  attr_id,
                                    			            _In_ uint32_t table_fields_bitmap)
{
    uint32_t attr_count;
    mrvl_acl_match_fields_ent field;

    MRVL_SAI_LOG_ENTER();

    for (attr_count = 0;
    		SAI_ACL_TABLE_ATTR_FIELD_END != acl_match_field_to_attribs[attr_count].table_attr_id;
    		attr_count++) {
        if (attr_id == acl_match_field_to_attribs[attr_count].table_attr_id) {
        	field = acl_match_field_to_attribs[attr_count].field;
        	break;
        }
    }

    if (SAI_ACL_TABLE_ATTR_FIELD_END == acl_match_field_to_attribs[attr_count].table_attr_id)
    	return SAI_STATUS_FAILURE;

	if (!mrvl_acl_is_bit_set_MAC(table_fields_bitmap, field)){
		return SAI_STATUS_FAILURE;
	}

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
#if 0
static sai_status_t mrvl_sai_set_mapping_field_for_entry_attrib(_In_ const sai_attr_id_t    attr_id,
															    _In_ uint32_t table_fields_bitmap,
                                    			                _Out_ uint32_t *entry_fields_bitmap)
{
    uint32_t attr_count;
    mrvl_acl_match_fields_ent field;
    assert(entry_fields_bitmap != NULL);

    MRVL_SAI_LOG_ENTER();

    for (attr_count = 0;
    		SAI_ACL_ENTRY_ATTR_FIELD_END != acl_match_field_to_attribs[attr_count].entry_attr_id;
    		attr_count++) {
        if (attr_id == acl_match_field_to_attribs[attr_count].entry_attr_id) {
        	field = acl_match_field_to_attribs[attr_count].field;
        	break;
        }
    }

    if (SAI_ACL_TABLE_ATTR_FIELD_END != acl_match_field_to_attribs[attr_count].entry_attr_id){
    	/* check is field is allowed */
    	if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, field)){
    		mrvl_acl_set_bit_MAC(*entry_fields_bitmap, field);
    	}
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
#endif
static sai_status_t mrvl_sai_is_set_mapping_field_for_entry_attrib(_In_ sai_attr_id_t  attr_id,
                                    			            _In_ uint32_t entry_fields_bitmap)
{
    uint32_t attr_count;
    mrvl_acl_match_fields_ent field;

    MRVL_SAI_LOG_ENTER();

    for (attr_count = 0;
    		SAI_ACL_ENTRY_ATTR_FIELD_END != acl_match_field_to_attribs[attr_count].entry_attr_id;
    		attr_count++) {
        if (attr_id == acl_match_field_to_attribs[attr_count].entry_attr_id) {
        	field = acl_match_field_to_attribs[attr_count].field;
        	break;
        }
    }

    if (SAI_ACL_ENTRY_ATTR_FIELD_END == acl_match_field_to_attribs[attr_count].entry_attr_id)
        return SAI_STATUS_FAILURE;

	if (!mrvl_acl_is_bit_set_MAC(entry_fields_bitmap, field)){
		return SAI_STATUS_FAILURE;
	}

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* ACL group member get per attribute functions */
static sai_status_t mrvl_acl_group_member_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_group_index = 0, acl_table_index = 0;
    uint8_t  ext_data[RESERVED_DATA_LENGTH_CNS];
	sai_status_t status = SAI_STATUS_SUCCESS;
    sai_object_id_t acl_group_id, acl_table_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, &acl_group_index, ext_data))) {
        return status;
    }
    acl_table_index = (uint32_t)(ext_data[1] << 8 | ext_data[0]);
    if (acl_group_index >= SAI_ACL_GROUP_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_group_index %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    assert(mrvl_sai_acl_group_db[acl_group_index].is_used != false);
    assert(mrvl_sai_acl_table_db[acl_table_index].is_used != false);
    assert(mrvl_sai_acl_group_db[acl_group_index].tables_count != 0);

    /* create ACL Group object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, acl_group_index, &acl_group_id))) {
        MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE_GROUP acl_group_index - %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(status);
    }
    /* create ACL Table object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &acl_table_id))) {
        MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE_GROUP acl_table_index - %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(status);
    }

    assert(mrvl_sai_acl_group_db[acl_group_index].group_id == acl_group_id);
    assert(mrvl_sai_acl_table_db[acl_table_index].acl_group_index == acl_group_index);


	switch ((int64_t)arg) {
    case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
		value->oid = mrvl_sai_acl_group_db[acl_group_index].group_id;
        break;
	case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
		value->oid = acl_table_id;
		break;

    case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
        value->u32 = mrvl_sai_acl_table_db[acl_table_index].priority;
        break;

	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}

/* ACL group get per attribute functions */
static sai_status_t mrvl_acl_group_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_group_index = 0, i = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE_GROUP, &acl_group_index))) {
        return status;
    }

    if (acl_group_index >= SAI_ACL_GROUP_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_group_index %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_group_db[acl_group_index].is_used != false);

	switch ((int64_t)arg) {
    case SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE:
		value->u32 = mrvl_sai_acl_group_db[acl_group_index].stage;
        break;
    case SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST:
        value->u32list.count = 0;
        for (;i < SAI_ACL_MAX_BIND_POINT_TYPES; i++){ 
            if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_group_db[acl_group_index].bind_point_types_bitmap, i)){
                value->u32list.list[value->u32list.count] = i;
                value->u32list.count++;
            }                                                                                                                             
        }                                                                                                                                     
		break;
    case SAI_ACL_TABLE_GROUP_ATTR_TYPE:
        value->u32 = mrvl_sai_acl_group_db[acl_group_index].type;
        break;

	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}

/* ACL range get per attribute functions */
static sai_status_t mrvl_acl_range_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_range_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_RANGE, &acl_range_index))) {
        return status;
    }

    if (acl_range_index >= SAI_ACL_RANGES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_range_index %d\n", acl_range_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

	switch ((int64_t)arg) {
    case SAI_ACL_RANGE_ATTR_TYPE:
		value->u32 = mrvl_sai_acl_range_db[acl_range_index].range_type;
        break;
	case SAI_ACL_RANGE_ATTR_LIMIT:
		memcpy(&value->u32range, &mrvl_sai_acl_range_db[acl_range_index].range_limit, sizeof(sai_u32_range_t));
		break;

	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}

/* ACL table get per attribute functions */
static sai_status_t mrvl_acl_table_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_table_index = 0, i = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_table_db[acl_table_index].is_used != false);

	switch ((int64_t)arg) {
    case SAI_ACL_TABLE_ATTR_ACL_STAGE:
		value->s32 = mrvl_sai_acl_table_db[acl_table_index].stage;
		value->u32 = mrvl_sai_acl_table_db[acl_table_index].stage;
        break;
    case SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST:
        value->u32list.count = 0;
        for (;i < SAI_ACL_MAX_BIND_POINT_TYPES; i++){ 
            if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, i)){
                value->u32list.list[value->u32list.count] = i;
                value->u32list.count++;
            }                                                                                                                             
        }                                                                                                                                     
		break;
    case SAI_ACL_TABLE_ATTR_SIZE:
		if (mrvl_sai_acl_table_db[acl_table_index].is_dynamic_size)
			value->u32 = 0;
		else
			value->u32 = mrvl_sai_acl_table_db[acl_table_index].table_size;
        break;
    case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
        value->u32list.count = 0;
        for (;i < SAI_ACL_MAX_ACTION_TYPES; i++){ 
            if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].table_actions_bitmap, i)){
                value->u32list.list[value->u32list.count] = i;
                value->u32list.count++;
            }                                                                                                                             
        }                                                                                                                                     
		break;
    default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}

/* get match attribute functions */
static sai_status_t mrvl_acl_table_match_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_table_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    value->booldata = false;
    assert(mrvl_sai_acl_table_db[acl_table_index].is_used != false);

    if (SAI_STATUS_SUCCESS == mrvl_sai_is_set_mapping_field_for_table_attrib((PTR_TO_INT)arg, mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap)){
    	value->booldata = true;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_acl_table_entry_list_get(
    _In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    uint32_t acl_table_index = 0, i = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    sai_object_id_t      *entry_list = NULL;
    uint32_t              entries_count = 0, ii;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    entry_list = (sai_object_id_t*)calloc(mrvl_sai_acl_table_db[acl_table_index].entries_count, sizeof(sai_object_id_t));
    if (!entry_list) {
    MRVL_SAI_LOG_ERR("Failed to allocate memory\n");
    MRVL_SAI_API_RETURN(SAI_STATUS_NO_MEMORY);
    }

    for (ii = 0;ii < SAI_ACL_ENTRIES_PER_TABLE_MAX_NUM; ii++){
        if ((mrvl_sai_acl_entry_db[ii].is_used) && (mrvl_sai_acl_entry_db[ii].table_index == acl_table_index)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_ENTRY, ii, &entry_list[entries_count])))
            {
                MRVL_SAI_LOG_ERR("Failed to create ACL entry index %d\n", i);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }

        entries_count++;
        }
    }
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_fill_objlist(entry_list, entries_count, &value->objlist)))
    {
        MRVL_SAI_LOG_ERR("Failed to fill objlist for SAI_OBJECT_TYPE_QUEUE\n");
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}

static sai_status_t mrvl_acl_table_available_entries_get(_In_ const sai_object_key_t   *key,
                                                         _Inout_ sai_attribute_value_t *value,
                                                         _In_ uint32_t                  attr_index,
                                                         _Inout_ vendor_cache_t        *cache,
                                                         void                          *arg)
{
    uint32_t acl_table_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t              free_entries = 0;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_table_db[acl_table_index].is_used == true);

    free_entries = mrvl_sai_acl_table_db[acl_table_index].table_size - mrvl_sai_acl_table_db[acl_table_index].entries_count; 
    
    value->u32 = MIN(SAI_ACL_ENTRIES_PER_TABLE_MAX_NUM, free_entries);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

static sai_status_t mrvl_acl_table_available_counters_get(_In_ const sai_object_key_t   *key,
                                                         _Inout_ sai_attribute_value_t *value,
                                                         _In_ uint32_t                  attr_index,
                                                         _Inout_ vendor_cache_t        *cache,
                                                         void                          *arg)
{
    uint32_t acl_table_index = 0;
	uint32_t acl_entry_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};
    FPA_FLOW_ENTRY_COUNTERS_STC counters = {0};
    uint32_t              free_counters = 0;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_table_db[acl_table_index].is_used == true);

    /* For all entries in table */
    acl_entry_index = mrvl_sai_acl_table_db[acl_table_index].head_entry_index;
    fpa_table_type = mrvl_sai_acl_table_db[acl_table_index].fpa_table_type;
    while (acl_entry_index != SAI_ACL_INVALID_INDEX){
        memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
        fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;

        /* get by cookie from FPA */
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }
        fpaLibFlowEntryStatisticsGet(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry, &counters);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to clear counters from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }
        if (0 == counters.packetCount && 0 == counters.byteCount){
            free_counters++;
        }
        acl_entry_index = mrvl_sai_acl_entry_db[acl_entry_index].next_acl_entry_index;
    }

    value->u32 = MIN(free_counters, SAI_ACL_COUNTERS_MAX_NUM);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_acl_fill_table_match_fields(_In_ uint32_t attr_count,
														 _In_ const sai_attribute_t  *attr_list,
														 _In_ sai_acl_stage_t  stage,
														_Out_ uint32_t *table_fields_bitmap,
														_Out_ bool *is_ipv6_table,
														_Out_ bool *is_ipv4_table,
														_Out_ bool *is_non_ip_table,
														_Out_ bool *is_egress_ipv4_table)
{
    uint32_t i;

    MRVL_SAI_LOG_ENTER();

    assert(attr_list != NULL);
    assert(table_fields_bitmap != NULL);
    assert(is_ipv6_table != NULL);
    assert(is_ipv4_table != NULL);
    assert(is_non_ip_table != NULL);

    for (i = 0; i < attr_count; i++) {
        if (SAI_STATUS_SUCCESS !=
        		    	mrvl_sai_set_mapping_field_for_table_attrib(attr_list[i].id, table_fields_bitmap)){
        	MRVL_SAI_LOG_DBG("Unsupported attribute in field match db\n");
        }
    }

    if ((mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IPV6) ||
    	mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IPV6)) &&
       (mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IP) ||
		mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IP))){
		MRVL_SAI_LOG_ERR("Incorrect fields bitmap - ipv4 and ipv6 addresses \n");
		MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IPV6) ||
        mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IPV6)){
    	*is_ipv6_table = true;
    }
    else if (mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IP) ||
    		mrvl_acl_is_bit_set_MAC(*table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IP)){
    	if (stage == SAI_ACL_STAGE_EGRESS){
    		*is_egress_ipv4_table = true;
    	}
    	else {
    		*is_ipv4_table = true;
    	}
    }
    else
    	*is_non_ip_table = true;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_acl_table_validate_action_type_list(_In_ const sai_s32_list_t *attr_list, 
                                                                          _In_ uint32_t              attr_index,
                                                                          _In_ sai_acl_stage_t stage,
                                                                          _Out_ uint32_t *action_types_bitmap)
{
    uint32_t i;;
    sai_acl_action_type_t action_type;

    MRVL_SAI_LOG_ENTER();

    assert(attr_list != NULL);
    assert(action_types_bitmap != NULL);


    for (i = 0; i < attr_list->count; i++) {
        action_type = attr_list->list[i];

        if (action_type > SAI_ACL_ACTION_TYPE_SET_DO_NOT_LEARN) {
            MRVL_SAI_LOG_ERR("Invalid action type (%d) at index [%d] in the list\n", action_type, i);
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index;
        }

        if ((stage == SAI_ACL_STAGE_INGRESS) && 
           ((action_type == SAI_ACL_ACTION_TYPE_MIRROR_EGRESS) ||
            (action_type == SAI_ACL_ACTION_TYPE_EGRESS_SAMPLEPACKET_ENABLE) ||
            (action_type == SAI_ACL_ACTION_TYPE_EGRESS_BLOCK_PORT_LIST))){
            MRVL_SAI_LOG_ERR("Invalid combination stage (%d) action type (%d) at index [%d] in the list\n", stage, action_type, i);
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index;
        }
        else if ((stage == SAI_ACL_STAGE_EGRESS) && 
           ((action_type == SAI_ACL_ACTION_TYPE_MIRROR_INGRESS) ||
            (action_type == SAI_ACL_ACTION_TYPE_INGRESS_SAMPLEPACKET_ENABLE))){
            MRVL_SAI_LOG_ERR("Invalid combination stage (%d) action type (%d) at index [%d] in the list\n", stage, action_type, i);
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index;
        }

        if (mrvl_acl_is_bit_set_MAC(*action_types_bitmap, action_type)) {
            MRVL_SAI_LOG_ERR("Already exist action type (%d)", action_type);
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + attr_index;
        }

        mrvl_acl_set_bit_MAC(*action_types_bitmap, action_type);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* the function checks match fields attribute validity vs table attributes, validates attr values and builds fields bitmap and match fpa structure  */
static sai_status_t mrvl_sai_acl_fill_entry_match_fields(_In_ uint32_t attr_count, _In_ sai_attribute_t const *attr_list, _In_ uint32_t acl_table_index,
														_Out_ uint32_t *entry_fields_bitmap, _Out_ FPA_FLOW_TABLE_MATCH_FIELDS_ACL_POLICY_STC *fpa_match_entry,
                                                         _In_ uint32_t *port_attr, _In_ uint32_t *vlan_attr)
{
    const sai_attribute_value_t  *tmp_value, *src_mac, *dest_mac, *ip_type;
    uint32_t tmp_index = 0, in_port = 0, src_port = 0, out_port = 0, table_fields_bitmap = 0;
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    assert(attr_list != NULL);
    assert(entry_fields_bitmap != NULL);
    assert(fpa_match_entry != NULL);

    table_fields_bitmap = mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap;

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IPV6)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IPV6);
        	memcpy(&fpa_match_entry->ipv6.srcIp6, &tmp_value->aclfield.data.ip6, sizeof(tmp_value->aclfield.data.ip6));
        	memcpy(&fpa_match_entry->ipv6.srcIp6Mask, &tmp_value->aclfield.mask.ip6, sizeof(tmp_value->aclfield.mask.ip6));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPv6 is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IPV6)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IPV6);
        	memcpy(&fpa_match_entry->ipv6.dstIp6, &tmp_value->aclfield.data.ip6, sizeof(tmp_value->aclfield.data.ip6));
        	memcpy(&fpa_match_entry->ipv6.dstIp6Mask, &tmp_value->aclfield.mask.ip6, sizeof(tmp_value->aclfield.mask.ip6));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_DST_IPv6 is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC, &src_mac, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_MAC)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_MAC);
        	memcpy(&fpa_match_entry->srcMac, &src_mac->aclfield.data.mac, sizeof(src_mac->aclfield.data.mac));
        	memcpy(&fpa_match_entry->srcMacMask, &src_mac->aclfield.mask.mac, sizeof(src_mac->aclfield.mask.mac));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC, &dest_mac, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_MAC)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_MAC);
        	memcpy(&fpa_match_entry->dstMac, &dest_mac->aclfield.data.mac, sizeof(dest_mac->aclfield.data.mac));
        	memcpy(&fpa_match_entry->dstMacMask, &dest_mac->aclfield.mask.mac, sizeof(dest_mac->aclfield.mask.mac));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IP)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IP);
        	memcpy(&fpa_match_entry->srcIp4, &tmp_value->aclfield.data.ip4, sizeof(tmp_value->aclfield.data.ip4));
        	memcpy(&fpa_match_entry->srcIp4Mask, &tmp_value->aclfield.mask.ip4, sizeof(tmp_value->aclfield.mask.ip4));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_DST_IP, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IP)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IP);
        	memcpy(&fpa_match_entry->dstIp4, &tmp_value->aclfield.data.ip4, sizeof(tmp_value->aclfield.data.ip4));
        	memcpy(&fpa_match_entry->dstIp4Mask, &tmp_value->aclfield.mask.ip4, sizeof(tmp_value->aclfield.mask.ip4));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_DST_IP is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
            mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE, &ip_type, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_TYPE)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_TYPE);
            switch (ip_type->aclfield.data.u32) {
            case SAI_ACL_IP_TYPE_ANY:
                break;
            case SAI_ACL_IP_TYPE_IP:
                fpa_match_entry->isIp       = 1;
                fpa_match_entry->isIpMask   = 0xFF;
                break;
            case SAI_ACL_IP_TYPE_IPV4ANY:
                fpa_match_entry->isIp       = 1;
                fpa_match_entry->isIpMask   = 0xFF;
                fpa_match_entry->isIpv6     = 0; 
                fpa_match_entry->isIpv6Mask = 0xFF;
                break;
            case SAI_ACL_IP_TYPE_NON_IPV6:
                fpa_match_entry->isIpv6     = 0; /* not ipv6 */
                fpa_match_entry->isIpv6Mask = 0xFF;
                break;
            case SAI_ACL_IP_TYPE_IPV6ANY:
                fpa_match_entry->isIp       = 1;
                fpa_match_entry->isIpMask   = 0xFF;
                fpa_match_entry->isIpv6     = 1;
                fpa_match_entry->isIpv6Mask = 0xFF;
                break;
            case SAI_ACL_IP_TYPE_NON_IPV4:
                fpa_match_entry->isIpv6     = 1; /* not ipv4 */
                fpa_match_entry->isIpv6Mask = 0xFF;
                break;
            case SAI_ACL_IP_TYPE_NON_IP:
                fpa_match_entry->isIp       = 0;
                fpa_match_entry->isIpMask   = 0xFF;
                break;
            case SAI_ACL_IP_TYPE_ARP:
                MRVL_SAI_LOG_ERR("Unsupported value %d of attribute SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE\n",ip_type->aclfield.data.u32);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            case SAI_ACL_IP_TYPE_ARP_REPLY:
                fpa_match_entry->arpOpcode      = SAI_ACL_ARP_OPCODE_REPLY;
                fpa_match_entry->arpOpcodeMask  = 0xFFFF;
                break;
            case SAI_ACL_IP_TYPE_ARP_REQUEST:
                fpa_match_entry->arpOpcode      = SAI_ACL_ARP_OPCODE_REQUEST;
                fpa_match_entry->arpOpcodeMask  = 0xFFFF;
                break;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
            mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_FRAG)) {
            if (tmp_value->aclfield.data.u32 > SAI_ACL_IP_FRAG_NON_FRAG) {
                MRVL_SAI_LOG_ERR("Unsupported value %d of attribute SAI_ACL_ENTRY_ATTR_FIELD_IP_FRAG\n", tmp_value->aclfield.data.u32);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_FRAG);
            fpa_match_entry->ipv4Fragmented      = (uint8_t)tmp_value->aclfield.data.u32;
            fpa_match_entry->ipv4FragmentedMask  = (uint8_t)0xFF;
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_IP_FRAG is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }


    if ((mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IPV6) ||
    	mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IPV6) ||
        (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_TYPE) &&
    	((ip_type->aclfield.data.u32 == SAI_ACL_IP_TYPE_IPV6ANY) ||
        (ip_type->aclfield.data.u32 == SAI_ACL_IP_TYPE_NON_IPV4)))) &&
       (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IP) ||
		mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IP) ||
        (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_TYPE) &&
        ((ip_type->aclfield.data.u32 == SAI_ACL_IP_TYPE_IPV4ANY) ||
        (ip_type->aclfield.data.u32 == SAI_ACL_IP_TYPE_NON_IPV6))))){
		MRVL_SAI_LOG_ERR("Incorrect fields bitmap - ipv4 and ipv6 addresses \n");
		MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_OUT_PORT)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_OUT_PORT);
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(tmp_value->aclfield.data.oid, SAI_OBJECT_TYPE_PORT, &out_port))) {
                return status;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_IN_PORT)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_IN_PORT);
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(tmp_value->aclfield.data.oid, SAI_OBJECT_TYPE_PORT, &in_port))) {
                return status;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_PORT)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_PORT);
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(tmp_value->aclfield.data.oid, SAI_OBJECT_TYPE_PORT, &src_port))) {
                return status;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_IN_PORT) &&
         mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_PORT) &&
         (in_port != src_port)){
        MRVL_SAI_LOG_ERR("Illegal combination of SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT and SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT in_port - %d src_port - %d\n", in_port, src_port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_IN_PORT) &&
         mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_OUT_PORT)){
        MRVL_SAI_LOG_ERR("Illegal combination of SAI_ACL_MATCH_FIELD_IN_PORT and SAI_ACL_MATCH_FIELD_OUT_PORT in_port - %d out_port - %d\n", in_port, out_port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_IN_PORT) ||
        mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_PORT)){
		*port_attr = in_port;
    }
    else if (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_OUT_PORT)) {
        *port_attr = out_port;
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_OUTER_VLAN_ID)) {
            *vlan_attr = tmp_value->aclfield.data.u16;
            if (!mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, SAI_ACL_BIND_POINT_TYPE_VLAN)){
                mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_OUTER_VLAN_ID);
                fpa_match_entry->vlanId = tmp_value->aclfield.data.u16;
                fpa_match_entry->vlanIdMask = SAI_ACL_VLAN_MASK;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_OUTER_VLAN_PRI)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_OUTER_VLAN_PRI);
        	memcpy(&fpa_match_entry->vlanPcp, &tmp_value->aclfield.data.u16, sizeof(tmp_value->aclfield.data.u16));
        	memcpy(&fpa_match_entry->vlanPcpMask, &tmp_value->aclfield.mask.u16, sizeof(tmp_value->aclfield.mask.u16));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_L4_SRC_PORT)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_L4_SRC_PORT);
        	memcpy(&fpa_match_entry->srcL4Port, &tmp_value->aclfield.data.u16, sizeof(tmp_value->aclfield.data.u16));
        	memcpy(&fpa_match_entry->srcL4PortMask, &tmp_value->aclfield.mask.u16, sizeof(tmp_value->aclfield.mask.u16));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_L4_DST_PORT)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_L4_DST_PORT);
        	memcpy(&fpa_match_entry->dstL4Port, &tmp_value->aclfield.data.u16, sizeof(tmp_value->aclfield.data.u16));
        	memcpy(&fpa_match_entry->dstL4PortMask, &tmp_value->aclfield.mask.u16, sizeof(tmp_value->aclfield.mask.u16));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_ETHER_TYPE)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ETHER_TYPE);
        	memcpy(&fpa_match_entry->etherType, &tmp_value->aclfield.data.u16, sizeof(tmp_value->aclfield.data.u16));
        	memcpy(&fpa_match_entry->etherTypeMask, &tmp_value->aclfield.mask.u16, sizeof(tmp_value->aclfield.mask.u16));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_IP_PROTOCOL)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_IP_PROTOCOL);
        	memcpy(&fpa_match_entry->ipProtocol, &tmp_value->aclfield.data.u16, sizeof(tmp_value->aclfield.data.u16));
        	memcpy(&fpa_match_entry->ipProtocolMask, &tmp_value->aclfield.mask.u16, sizeof(tmp_value->aclfield.mask.u16));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_DSCP, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_DSCP)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DSCP);
        	memcpy(&fpa_match_entry->dscp, &tmp_value->aclfield.data.u8, sizeof(tmp_value->aclfield.data.u8));
        	memcpy(&fpa_match_entry->dscpMask, &tmp_value->aclfield.mask.u8, sizeof(tmp_value->aclfield.mask.u8));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_DSCP is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_ECN, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_ECN)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ECN);
        	memcpy(&fpa_match_entry->ipEcn, &tmp_value->aclfield.data.u8, sizeof(tmp_value->aclfield.data.u8));
        	memcpy(&fpa_match_entry->ipEcnMask, &tmp_value->aclfield.mask.u8, sizeof(tmp_value->aclfield.mask.u8));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_ECN is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_ICMP_TYPE)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ICMP_TYPE);
        	memcpy(&fpa_match_entry->icmpV4Type, &tmp_value->aclfield.data.u8, sizeof(tmp_value->aclfield.data.u8));
        	memcpy(&fpa_match_entry->icmpV4TypeMask, &tmp_value->aclfield.mask.u8, sizeof(tmp_value->aclfield.mask.u8));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_fields_bitmap, SAI_ACL_MATCH_FIELD_ICMP_CODE)) {
            mrvl_acl_set_bit_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ICMP_CODE);
        	memcpy(&fpa_match_entry->icmpV4Code, &tmp_value->aclfield.data.u8, sizeof(tmp_value->aclfield.data.u8));
        	memcpy(&fpa_match_entry->icmpV4CodeMask, &tmp_value->aclfield.mask.u8, sizeof(tmp_value->aclfield.mask.u8));
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE is not part of table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    if ((mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_IPV6) ||
    	mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_DST_IPV6)) &&
       (mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ICMP_TYPE) ||
		mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ICMP_CODE) ||
		mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_IP_PROTOCOL) ||
		mrvl_acl_is_bit_set_MAC(*entry_fields_bitmap, SAI_ACL_MATCH_FIELD_ACL_IP_FRAG))){
		MRVL_SAI_LOG_ERR("Incorrect fields bitmap - ipv4 fields and ipv6 addresses collision\n");
		MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* the function checks actions attribute validity vs table attributes, validates attr values and builds actions bitmap and actions structure  */
static sai_status_t mrvl_sai_acl_entry_actions_validation(_In_ uint32_t attr_count, _In_ const sai_attribute_t  *attr_list, _In_ uint32_t acl_table_index,
														_Out_ uint32_t *entry_actions_bitmap, _Out_ sai_acl_action_data_t *entry_action_data)
{
    const sai_attribute_value_t  *tmp_value;
    uint32_t tmp_index = 0, table_actions_bitmap = 0;

    MRVL_SAI_LOG_ENTER();

    assert(attr_list != NULL);
    assert(entry_actions_bitmap != NULL);

    table_actions_bitmap = mrvl_sai_acl_table_db[acl_table_index].table_actions_bitmap;

    /* handle SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION action */
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_actions_bitmap, SAI_ACL_ACTION_TYPE_PACKET_ACTION)) {
    		if ((tmp_value->aclaction.parameter.u32 != SAI_PACKET_ACTION_DROP) && (tmp_value->aclaction.parameter.u32 != SAI_PACKET_ACTION_FORWARD)){
    			MRVL_SAI_LOG_ERR("Value of SAI_ACL_ENTRY_ATTR_PACKET_ACTION is not available - %d\n", tmp_value->u32);
    			MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    		}
    		else {
                mrvl_acl_set_bit_MAC(*entry_actions_bitmap, SAI_ACL_ACTION_TYPE_PACKET_ACTION);
    			entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable        = tmp_value->aclaction.enable;
                entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 = tmp_value->aclaction.parameter.u32;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute action SAI_ACL_ACTION_TYPE_PACKET_ACTION is not set by table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    /* handle SAI_ACL_ENTRY_ATTR_ACTION_SET_TC action */
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_ACTION_SET_TC, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_TC)) {
    		if (tmp_value->u8 > SAI_QOS_NUM_QUEUES){
                MRVL_SAI_LOG_ERR("Value of SAI_ACL_ACTION_TYPE_SET_TC is out of range - %d\n", tmp_value->u8);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            else {
                mrvl_acl_set_bit_MAC(*entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_TC);
                entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable       = tmp_value->aclaction.enable;
                entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8 = tmp_value->aclaction.parameter.u8;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute action SAI_ACL_ACTION_TYPE_SET_TC is not set by table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }
    
    /* handle SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP action */
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_DSCP)) {
            if (tmp_value->u8 > SAI_QOS_DSCP_MAX){
                MRVL_SAI_LOG_ERR("Value of SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP is out of range - %d\n", tmp_value->u8);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            else {
                mrvl_acl_set_bit_MAC(*entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_DSCP);
                entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable       = tmp_value->aclaction.enable;
                entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8 = tmp_value->aclaction.parameter.u8;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute action SAI_ACL_ACTION_TYPE_SET_DSCP is not set by table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    /* handle SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID action */
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_ID)) {
            if (tmp_value->u8 > SAI_MAX_NUM_OF_VLANS){
                MRVL_SAI_LOG_ERR("Value of SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_ID is out of range - %d\n", tmp_value->u8);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            else {
                mrvl_acl_set_bit_MAC(*entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_ID);
                entry_action_data->enable       = tmp_value->aclaction.enable;
                entry_action_data->parameter.u8 = tmp_value->aclaction.parameter.u8;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute action SAI_ACL_ACTION_TYPE_SET_DSCP is not set by table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    /* handle SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI action */
    if (SAI_STATUS_SUCCESS ==
    		mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI, &tmp_value, &tmp_index)) {
        if (mrvl_acl_is_bit_set_MAC(table_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_PRI)) {
            if (tmp_value->u8 > SAI_QOS_UP_MAX){
                MRVL_SAI_LOG_ERR("Value of SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI is out of range - %d\n", tmp_value->u8);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            else {
                mrvl_acl_set_bit_MAC(*entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_PRI);
                entry_action_data->enable       = tmp_value->aclaction.enable;
                entry_action_data->parameter.u8 = tmp_value->aclaction.parameter.u8;
            }
        }
        else {
            MRVL_SAI_LOG_ERR("Attribute action SAI_ACL_ENTRY_ATTR_ACTION_SET_OUTER_VLAN_PRI is not set by table attributes\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

static sai_status_t mrvl_sai_acl_entry_match_fields_validation(_In_ uint32_t attr_count, _In_ sai_attribute_t const *attr_list, _In_ uint32_t acl_table_index,
														_Inout_ uint32_t *entry_fields_bitmap, _Inout_ FPA_FLOW_TABLE_MATCH_FIELDS_ACL_POLICY_STC *fpa_match_entry,
                                                        _In_ uint32_t *port_attr, _In_ uint32_t *vlan_attr)
{
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    assert(attr_list != NULL);
    assert(entry_fields_bitmap != NULL);
    assert(fpa_match_entry != NULL);

        /* are entry match fields subset of table match fields */
    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_acl_fill_entry_match_fields(attr_count, attr_list, acl_table_index,
        													entry_fields_bitmap, fpa_match_entry, port_attr, vlan_attr))){
        MRVL_SAI_LOG_ERR("No match for entry fields vs table fields\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (entry_fields_bitmap == 0){
		MRVL_SAI_LOG_ERR("At least one match field must be given - %x \n", entry_fields_bitmap);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    switch (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type) {
    case SAI_ACL_BIND_POINT_TYPE_PORT:
        if (*port_attr != SAI_ACL_INVALID_INTERFACE){ /* port_attr in set */
            if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_portbitmap, *port_attr)){
                MRVL_SAI_LOG_ERR("Bound port bitmap %d doesn't include port_attr attribute %d\n", 
                                 mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_portbitmap, *port_attr);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
        }
        break;
    case SAI_ACL_BIND_POINT_TYPE_VLAN:
        if (*vlan_attr != SAI_ACL_INVALID_INTERFACE){ /* vlan_attr in set */
            if (mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_vlan != *vlan_attr){
                MRVL_SAI_LOG_ERR("Bound vlan %d is different from vlanId attr %d\n", 
                                 mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_vlan, fpa_match_entry->vlanId);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
        }
        break;
    default:
        break;
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_acl_entry_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_entry_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }

    if (acl_entry_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_entry_index %d\n", acl_entry_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);

	switch ((int64_t)arg) {
	case SAI_ACL_ENTRY_ATTR_TABLE_ID:
	    /* create ACL table object */
	    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, mrvl_sai_acl_entry_db[acl_entry_index].table_index, &value->oid))) {
	    	MRVL_SAI_LOG_ERR("Can't create object id for SAI_OBJECT_TYPE_ACL_TABLE index - %d\n", mrvl_sai_acl_entry_db[acl_entry_index].table_index);
	    	MRVL_SAI_API_RETURN(status);
	    }
		break;
	case SAI_ACL_ENTRY_ATTR_PRIORITY:
		value->u32 = mrvl_sai_acl_entry_db[acl_entry_index].priority;
		break;
	case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
		value->booldata = mrvl_sai_acl_entry_db[acl_entry_index].admin_state;
		break;
	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}
 
/* currently unused, unabling API to avoid unreferenced function error*/
/*
static bool mrvl_sai_utl_is_mask_empty(_In_ uint8_t   *mask, _In_ uint32_t  maskSize)
{
   bool empty = true;
   uint32_t i;

   for (i = 0; i < maskSize; i++) {
      if (mask[i] != 0) {
         empty = false;
         break;
      }
   }
   return empty;
}
*/
static sai_status_t mrvl_acl_entry_match_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_entry_index = 0, acl_table_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }

    if (acl_entry_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_entry_index %d\n", acl_entry_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_is_set_mapping_field_for_entry_attrib((PTR_TO_INT)arg, mrvl_sai_acl_entry_db[acl_entry_index].entry_fields_bitmap))){
        MRVL_SAI_LOG_ERR("The attribute is not set: attr - %d\n", (PTR_TO_INT)arg);
        MRVL_SAI_API_RETURN(status);
    }

	memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
	fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;
	acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;
	fpa_table_type = mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].fpa_table_type;
	/* get by cookie from FPA */
	fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);

	if (fpa_status != FPA_OK) {
		MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
		status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		MRVL_SAI_API_RETURN(status);
	}

	switch ((int64_t)arg) {
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
        memcpy(&value->aclfield.data.ip6, &fpa_flow_entry.data.acl_policy.match.ipv6.srcIp6, sizeof(sai_ip6_t));
        memcpy(&value->aclfield.mask.ip6, &fpa_flow_entry.data.acl_policy.match.ipv6.srcIp6Mask, sizeof(sai_ip6_t));
        break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
        memcpy(&value->aclfield.data.ip6, &fpa_flow_entry.data.acl_policy.match.ipv6.dstIp6, sizeof(sai_ip6_t));
        memcpy(&value->aclfield.mask.ip6, &fpa_flow_entry.data.acl_policy.match.ipv6.dstIp6Mask, sizeof(sai_ip6_t));	
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
        memcpy(&value->aclfield.data.mac, &fpa_flow_entry.data.acl_policy.match.srcMac, sizeof(sai_mac_t));
        memcpy(&value->aclfield.mask.mac, &fpa_flow_entry.data.acl_policy.match.srcMacMask, sizeof(sai_mac_t));
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
        memcpy(&value->aclfield.data.mac, &fpa_flow_entry.data.acl_policy.match.dstMac, sizeof(sai_mac_t));
        memcpy(&value->aclfield.mask.mac, &fpa_flow_entry.data.acl_policy.match.dstMacMask, sizeof(sai_mac_t));
 		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
        memcpy(&value->aclfield.data.ip4, &fpa_flow_entry.data.acl_policy.match.srcIp4, sizeof(sai_ip4_t));
        memcpy(&value->aclfield.mask.ip4, &fpa_flow_entry.data.acl_policy.match.srcIp4Mask, sizeof(sai_ip4_t));
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
        memcpy(&value->aclfield.data.ip4, &fpa_flow_entry.data.acl_policy.match.dstIp4, sizeof(sai_ip4_t));
        memcpy(&value->aclfield.mask.ip4, &fpa_flow_entry.data.acl_policy.match.dstIp4Mask, sizeof(sai_ip4_t));
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
    case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
    case SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT:
        if (mrvl_sai_acl_entry_db[acl_entry_index].attr_port != SAI_ACL_INVALID_INTERFACE) {
            /* create port object */
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, mrvl_sai_acl_entry_db[acl_entry_index].attr_port, &value->aclfield.data.oid))) {
                MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_PORT port - %d\n", mrvl_sai_acl_entry_db[acl_entry_index].attr_port);
                MRVL_SAI_API_RETURN(status);
            }
        }
		break;
    case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
        value->aclfield.data.u16 = mrvl_sai_acl_entry_db[acl_entry_index].attr_vlan;
        value->aclfield.mask.u16 = SAI_ACL_VLAN_MASK;
		break;
    case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
        value->aclfield.data.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.vlanPcp;
        value->aclfield.mask.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.vlanPcpMask;
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
        value->aclfield.data.u16 = (sai_uint16_t)fpa_flow_entry.data.acl_policy.match.srcL4Port;
        value->aclfield.mask.u16 = (sai_uint16_t)fpa_flow_entry.data.acl_policy.match.srcL4PortMask;
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
        value->aclfield.data.u16 = (sai_uint16_t)fpa_flow_entry.data.acl_policy.match.dstL4Port;
        value->aclfield.mask.u16 = (sai_uint16_t)fpa_flow_entry.data.acl_policy.match.dstL4PortMask;
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
        value->aclfield.data.u16 = fpa_flow_entry.data.acl_policy.match.etherType;
        value->aclfield.mask.u16 = fpa_flow_entry.data.acl_policy.match.etherTypeMask;
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
        value->aclfield.data.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.ipProtocol;
        value->aclfield.mask.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.ipProtocolMask;
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
        value->aclfield.data.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.dscp;
        value->aclfield.mask.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.dscpMask;
        break;
    case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
        value->aclfield.data.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.ipEcn;
        value->aclfield.mask.u8 = (sai_uint8_t)fpa_flow_entry.data.acl_policy.match.ipEcnMask;
        break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
        value->aclfield.data.u8 = fpa_flow_entry.data.acl_policy.match.icmpV4Type;
        value->aclfield.mask.u8 = fpa_flow_entry.data.acl_policy.match.icmpV4TypeMask;
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
        value->aclfield.data.u8 = fpa_flow_entry.data.acl_policy.match.icmpV4Code;
        value->aclfield.mask.u8 = fpa_flow_entry.data.acl_policy.match.icmpV4CodeMask;
		break;
    case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
        if (fpa_flow_entry.data.acl_policy.match.isIpMask != 0) {
            if (fpa_flow_entry.data.acl_policy.match.isIp != 0) {
                if (fpa_flow_entry.data.acl_policy.match.isIpv6Mask != 0){
                    if (fpa_flow_entry.data.acl_policy.match.isIpv6 != 0)
                        value->aclfield.data.u32 = SAI_ACL_IP_TYPE_IPV6ANY;
                    else 
                        value->aclfield.data.u32 = SAI_ACL_IP_TYPE_IPV4ANY;
                }
                else 
                    value->aclfield.data.u32 = SAI_ACL_IP_TYPE_IP;
            }
            else value->aclfield.data.u32 = SAI_ACL_IP_TYPE_NON_IP;
        }
        else {
            if (fpa_flow_entry.data.acl_policy.match.isIpv6Mask != 0)
                if (fpa_flow_entry.data.acl_policy.match.isIpv6 == 0)
                    value->aclfield.data.u32 = SAI_ACL_IP_TYPE_NON_IPV6;
                else 
                    value->aclfield.data.u32 = SAI_ACL_IP_TYPE_NON_IPV4;
            else
                value->aclfield.data.u32 = SAI_ACL_IP_TYPE_ANY;
        }
		break;

    case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
        (fpa_flow_entry.data.acl_policy.match.ipv4Fragmented) ? 
            (value->aclfield.data.u32 = SAI_ACL_IP_FRAG_ANY) : (value->aclfield.data.u32 = SAI_ACL_IP_FRAG_NON_FRAG);
        (fpa_flow_entry.data.acl_policy.match.ipv4Fragmented) ? 
            (value->aclfield.mask.u32 = fpa_flow_entry.data.acl_policy.match.ipv4FragmentedMask) : (value->aclfield.mask.u32 = 0);
		break;

	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return ((int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_acl_entry_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{
	uint32_t acl_entry_index = 0, acl_table_index = 0, fpa_priority = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }

    if (acl_entry_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_entry_index %d\n", acl_entry_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);

	memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
	fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;
	acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;
	fpa_table_type = mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].fpa_table_type;
	/* get by cookie from FPA */
	fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);

	if (fpa_status != FPA_OK) {
		MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
		status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		MRVL_SAI_API_RETURN(status);
	}

	switch ((int64_t)arg) {
	case SAI_ACL_ENTRY_ATTR_PRIORITY:
		if (mrvl_sai_acl_entry_db[acl_entry_index].priority != value->u32){
			fpa_priority = mrvl_sai_acl_table_db[acl_table_index].priority * mrvl_sai_acl_table_db[acl_table_index].table_size + value->u32;
			fpa_flow_entry.priority = fpa_priority;
            fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry, 0);
            if (fpa_status != FPA_OK) {
               MRVL_SAI_LOG_ERR("Failed to update entry to FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
               status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
               MRVL_SAI_API_RETURN(status);
            }
            mrvl_sai_acl_entry_db[acl_entry_index].priority = value->u32;
        }
		break;
	case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
		if (mrvl_sai_acl_entry_db[acl_entry_index].admin_state != value->booldata){
            if (value->booldata == false) {
                fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_INVALID_FLAG;
            }
            fpa_status = fpaLibFlowEntrySetValid(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
            if (fpa_status != FPA_OK) {
               MRVL_SAI_LOG_ERR("Failed to update valid to FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
               status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
               MRVL_SAI_API_RETURN(status);
            }
            mrvl_sai_acl_entry_db[acl_entry_index].admin_state = value->booldata;
		}
		break;
	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return (SAI_STATUS_INVALID_ATTRIBUTE_0+(int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}


static sai_status_t mrvl_acl_entry_match_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{
	uint32_t acl_entry_index = 0, acl_table_index = 0, attr_port = SAI_ACL_INVALID_INTERFACE, attr_vlan = SAI_ACL_INVALID_INTERFACE;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry = {0};
    uint32_t entry_fields_bitmap = 0;
    sai_attribute_t attr_list = {0};


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }

    if (acl_entry_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_entry_index %d\n", acl_entry_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);
    acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;

	memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
	fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;
	acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;
	fpa_table_type = mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].fpa_table_type;
	/* get by cookie from FPA */
	fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);

	if (fpa_status != FPA_OK) {
		MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
		status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		MRVL_SAI_API_RETURN(status);
	}

    attr_list.id = (int64_t)arg;
    memcpy(&attr_list.value, value, sizeof(attr_list.value));
    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_acl_entry_match_fields_validation(1, &attr_list, acl_table_index,
        													&entry_fields_bitmap, &fpa_flow_entry.data.acl_policy.match,
                                                            &attr_port, &attr_vlan))){
        MRVL_SAI_LOG_ERR("Match fields validation failed\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_entry_db[acl_entry_index].attr_port = attr_port;
    mrvl_sai_acl_entry_db[acl_entry_index].attr_vlan = attr_vlan;

	/* update by cookie from FPA */
	fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry, 0);
	if (fpa_status != FPA_OK) {
		MRVL_SAI_LOG_ERR("Failed to update entry to FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
		status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		MRVL_SAI_API_RETURN(status);
	}

	mrvl_sai_acl_entry_db[acl_entry_index].entry_fields_bitmap |= entry_fields_bitmap;

    return SAI_STATUS_SUCCESS;
}

/*ACL ENTRY ACTION ATTR SET/GET */
static sai_status_t mrvl_acl_entry_action_attrib_get(
    _In_ const sai_object_key_t     *key,
    _Inout_ sai_attribute_value_t   *value,
    _In_ uint32_t                   attr_index,
    _Inout_ vendor_cache_t          *cache,
    void                            *arg)
{
	uint32_t acl_entry_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }

    if (acl_entry_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_entry_index %d\n", acl_entry_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);

    value->aclaction.enable = false;

	switch ((int64_t)arg) {
	case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
        if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_entry_db[acl_entry_index].entry_actions_bitmap, SAI_ACL_ACTION_TYPE_PACKET_ACTION)) {
    	    value->aclaction.enable = mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable;
            value->aclaction.parameter.u32 = mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32;
        }
		break;
	case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
        if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_entry_db[acl_entry_index].entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_TC)) {
            value->aclaction.enable = mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable;
            value->aclaction.parameter.u8 = mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8;
        }	
		break;
	case SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP:
        if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_entry_db[acl_entry_index].entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_DSCP)) {
            value->aclaction.enable = mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable;
            value->aclaction.parameter.u8 = mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8;
        }
		break;
	default:
    	MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", (int64_t)arg);
		return ((int64_t)arg);
	}

    return SAI_STATUS_SUCCESS;
}


static sai_status_t mrvl_acl_entry_action_attrib_set(
    _In_ const sai_object_key_t      *key,
    _In_ const sai_attribute_value_t *value,
    void                             *arg)
{
	uint32_t acl_entry_index = 0, acl_table_index = 0;
	sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry = {0};
    uint32_t entry_actions_bitmap = 0;
    sai_attribute_t attr_list = {0};
    sai_acl_action_data_t entry_action_data[SAI_ACL_MAX_ACTION_TYPES];
    bool do_update = false;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }

    if (acl_entry_index >= SAI_ACL_ENTRIES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_entry_index %d\n", acl_entry_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);
    acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;

	memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
	fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;
	acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;
	fpa_table_type = mrvl_sai_acl_table_db[acl_table_index].fpa_table_type;
	/* get by cookie from FPA */
	fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);

	if (fpa_status != FPA_OK) {
		MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
		status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
		MRVL_SAI_API_RETURN(status);
	}

    attr_list.id = (int64_t)arg;
    memcpy(&attr_list.value, value, sizeof(attr_list.value));
	memset(entry_action_data, 0, sizeof(entry_action_data));
    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_acl_entry_actions_validation(1, &attr_list, acl_table_index,
        													&entry_actions_bitmap, entry_action_data))){
        MRVL_SAI_LOG_ERR("Actions fields validation failed\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (mrvl_acl_is_all_0_MAC(entry_actions_bitmap)) {
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
    }

    /* handle actions */

    /* attr action SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION exist in list */
    if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_PACKET_ACTION) && 
       ((entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable != mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable) ||
       (entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 != mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32))){ /* case of update */
        if (entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable && entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 == SAI_PACKET_ACTION_DROP){
            fpa_flow_entry.data.acl_policy.instructionFlags &= ~(FPA_FLOW_TABLE_PCL_ACTION_GOTO_FLAG & 0xFFFFFFFF);
            fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_CLEAR_ACTION_FLAG;
        }
        else { /* forward */
            if (fpa_flow_entry.entryType != FPA_FLOW_TABLE_TYPE_EPCL_E){
                fpa_flow_entry.data.acl_policy.gotoTableNo = fpa_table_type + 1;
                fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_GOTO_FLAG & 0xFFFFFFF;
            }
            fpa_flow_entry.data.acl_policy.instructionFlags &= ~(FPA_FLOW_TABLE_PCL_ACTION_CLEAR_ACTION_FLAG & 0xFFFFFFFF);
        } 
        do_update = true;       
    }

    /* attr action SAI_ACL_ACTION_TYPE_SET_TC exist in list */
    if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_TC) && 
        ((entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable != mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable) ||
        (entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8 != mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8))){ /* case of update */
        if (entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable){ 
            fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_QUEUE_FLAG;
            fpa_flow_entry.data.acl_policy.queueId = entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8;
        }
        else {
            fpa_flow_entry.data.acl_policy.instructionFlags &= ~(FPA_FLOW_TABLE_PCL_ACTION_QUEUE_FLAG & 0xFFFFFFFF);
        }
        do_update = true;
    }

    /* attr action SAI_ACL_ACTION_TYPE_SET_DSCP exist in list */
    if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_DSCP) && 
        ((entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable != mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable) ||
        (entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8 != mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8))){ /* case of update */
        if (entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable){ 
            fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_DSCP_FLAG;
            fpa_flow_entry.data.acl_policy.dscp = entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8;
        }
        else {
            fpa_flow_entry.data.acl_policy.instructionFlags &= ~(FPA_FLOW_TABLE_PCL_ACTION_DSCP_FLAG & 0xFFFFFFFF);
        }
        do_update = true;
    }

    if (do_update){

    	/* update by cookie from FPA */
    	fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry, 0);
    	if (fpa_status != FPA_OK) {
    		MRVL_SAI_LOG_ERR("Failed to update entry to FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
    		status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    		MRVL_SAI_API_RETURN(status);
    	}

        /* update entry_action_data of mrvl_sai_acl_entry_db */
        mrvl_sai_acl_entry_db[acl_entry_index].entry_actions_bitmap |= entry_actions_bitmap;
        if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_PACKET_ACTION)) {
            memcpy(&mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION], &entry_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION], sizeof(sai_acl_action_data_t));
        }
        if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_TC)) {
            memcpy(&mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC], &entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC], sizeof(sai_acl_action_data_t));
        }
        if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_DSCP)) {
            memcpy(&mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP], &entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP], sizeof(sai_acl_action_data_t));
        }
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/* private functions implementation */
static void mrvl_sai_acl_object_id_to_str(_In_ sai_object_type_t acl_type, _In_ sai_object_id_t acl_id, _Out_ char *object_id_str)
{
    uint32_t acl_data, acl_group_data, acl_table_data;
    uint8_t  ext_data[RESERVED_DATA_LENGTH_CNS];

    if (acl_type == SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER){
        if (SAI_STATUS_SUCCESS == mrvl_sai_utl_object_to_ext_type(acl_id, acl_type, &acl_group_data, ext_data)){
            acl_table_data = (uint32_t)(ext_data[1] << 8 | ext_data[0]);
            snprintf(object_id_str, MAX_KEY_STR_LEN, " acl_group_data %u, acl_table_data %u", acl_group_data, acl_table_data);
            return;
        }
    }
    else if (SAI_STATUS_SUCCESS == mrvl_sai_utl_object_to_type(acl_id, acl_type, &acl_data)){
        snprintf(object_id_str, MAX_KEY_STR_LEN, " acl data %u", acl_data);
        return;
    }

    snprintf(object_id_str, MAX_KEY_STR_LEN, "invalid acl data");
}

/* find free index in mrvl_sai_acl_table_db */
static sai_status_t mrvl_acl_find_free_index_in_table_db(_Out_ uint32_t *free_index)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(free_index != NULL);

    for (i = 0; i < SAI_ACL_TABLES_MAX_NUM; i++) {
        if (false == mrvl_sai_acl_table_db[i].is_used) {
            *free_index              = i;
            mrvl_sai_acl_table_db[i].is_used = true;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_ACL_TABLES_MAX_NUM) {
    	MRVL_SAI_LOG_ERR("NO free indexes\n");
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

/* find free index in mrvl_sai_acl_entry_db */
static sai_status_t mrvl_acl_find_free_index_in_entry_db(_Out_ uint32_t *free_index)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(free_index != NULL);

    for (i = 0; i < SAI_ACL_ENTRIES_MAX_NUM; i++) {
        if (false == mrvl_sai_acl_entry_db[i].is_used) {
            *free_index              = i;
            mrvl_sai_acl_entry_db[i].is_used = true;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_ACL_ENTRIES_MAX_NUM) {
    	MRVL_SAI_LOG_ERR("NO free indexes\n");
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

/* find free index in mrvl_sai_acl_group_db */
static sai_status_t mrvl_acl_find_free_index_in_group_table_db(_Out_ uint32_t *free_index)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(free_index != NULL);

    for (i = 0; i < SAI_ACL_GROUP_MAX_NUM; i++) {
        if (false == mrvl_sai_acl_group_db[i].is_used) {
            *free_index              = i;
            mrvl_sai_acl_group_db[i].is_used = true;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_ACL_GROUP_MAX_NUM) {
    	MRVL_SAI_LOG_ERR("NO free indexes\n");
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}


/* find free index in mrvl_sai_acl_range_db */
static sai_status_t mrvl_acl_find_free_index_in_range_db(_Out_ uint32_t *free_index)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(free_index != NULL);

    for (i = 0; i < SAI_ACL_RANGES_MAX_NUM; i++) {
        if (false == mrvl_sai_acl_range_db[i].is_used) {
            *free_index              = i;
            mrvl_sai_acl_range_db[i].is_used = true;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_ACL_RANGES_MAX_NUM) {
    	MRVL_SAI_LOG_ERR("NO free indexes\n");
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    MRVL_SAI_LOG_EXIT();
    return status;
}

sai_status_t mrvl_acl_db_free_entries_get(_In_ sai_object_type_t  object_type,
                                          _Out_ uint32_t         *free_entries)
{
    uint32_t ii, count = 0;

    MRVL_SAI_LOG_ENTER();

    assert((object_type == SAI_OBJECT_TYPE_ACL_TABLE_GROUP) || (object_type == SAI_OBJECT_TYPE_ACL_TABLE));
    assert(free_entries != NULL);

    if (object_type == SAI_OBJECT_TYPE_ACL_TABLE_GROUP) {
        for (ii = 0; ii < SAI_ACL_GROUP_MAX_NUM; ii++) {
            if (false == mrvl_sai_acl_group_db[ii].is_used) {
                count++;
            }
        }
    } else { /* SAI_OBJECT_TYPE_ACL_TABLE */
        for (ii = 0; ii < SAI_ACL_TABLES_MAX_NUM; ii++) {
            if (false == mrvl_sai_acl_table_db[ii].is_used) {
                count++;
            }
        }
    }

    *free_entries = count;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static void mrvl_sai_acl_group_remove(_In_ uint32_t acl_acl_group_index)
{

    MRVL_SAI_LOG_ENTER();

    memset(&mrvl_sai_acl_group_db[acl_acl_group_index], 0, sizeof(mrvl_acl_group_db_t));
    MRVL_SAI_LOG_EXIT();
}


static void mrvl_sai_acl_table_remove(_In_ uint32_t acl_table_index)
{

    MRVL_SAI_LOG_ENTER();

    memset(&mrvl_sai_acl_table_db[acl_table_index], 0, sizeof(mrvl_acl_table_db_t));
    mrvl_sai_acl_table_db[acl_table_index].head_entry_index = SAI_ACL_INVALID_INDEX;
    mrvl_sai_acl_table_db[acl_table_index].acl_group_index = SAI_ACL_INVALID_INDEX;
    mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_MAX_BIND_POINT_TYPES;
    MRVL_SAI_LOG_EXIT();
}


/* add entry to table and remove entry from table internal functions */
static void mrvl_sai_acl_entry_add_to_table(_In_ uint32_t acl_entry_index, _In_ uint32_t acl_table_index)
{
	uint32_t            *head_entry_index = &mrvl_sai_acl_table_db[acl_table_index].head_entry_index;
	mrvl_acl_table_db_t *table            = &mrvl_sai_acl_table_db[acl_table_index];
	mrvl_acl_entry_db_t *new_entry        = &mrvl_sai_acl_entry_db[acl_entry_index];
	mrvl_acl_entry_db_t *table_head_entry;

    MRVL_SAI_LOG_ENTER();
    assert(table != NULL);

    if (SAI_ACL_INVALID_INDEX == *head_entry_index) { /*       first entry */
        *head_entry_index            = acl_entry_index;
        new_entry->next_acl_entry_index        = SAI_ACL_INVALID_INDEX;
        new_entry->prev_acl_entry_index        = SAI_ACL_INVALID_INDEX;
    } else { /* next_entry is added to head */
        table_head_entry             = &mrvl_sai_acl_entry_db[*head_entry_index];
        new_entry->next_acl_entry_index        = *head_entry_index;
        new_entry->prev_acl_entry_index        = SAI_ACL_INVALID_INDEX;
        table_head_entry->prev_acl_entry_index = acl_entry_index;
        *head_entry_index            = acl_entry_index;
    }

    mrvl_sai_acl_table_db[acl_table_index].entries_count++;
    mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].entries_count++;

    MRVL_SAI_LOG_DBG("Number entries for table acl_table_index - %d is %d\n", acl_table_index, mrvl_sai_acl_table_db[acl_table_index].entries_count);
    MRVL_SAI_LOG_DBG("Number entries for table acl_group_index - %d is %d\n", mrvl_sai_acl_table_db[acl_table_index].acl_group_index, mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].entries_count);
    MRVL_SAI_LOG_EXIT();

}

static void mrvl_sai_acl_entry_remove_from_table(_In_ uint32_t acl_entry_index, _In_ uint32_t acl_table_index)
{
    mrvl_acl_table_db_t *table;
    mrvl_acl_entry_db_t *entry;
    uint32_t        prev_index, next_index;


    MRVL_SAI_LOG_ENTER();

    assert(SAI_ACL_INVALID_INDEX != acl_table_index && SAI_ACL_INVALID_INDEX != acl_entry_index);

    table      = &mrvl_sai_acl_table_db[acl_table_index];
    entry      = &mrvl_sai_acl_entry_db[acl_entry_index];
    prev_index = entry->prev_acl_entry_index;
	next_index = entry->next_acl_entry_index;

	entry->is_used       = false;
	entry->next_acl_entry_index    = SAI_ACL_INVALID_INDEX;
	entry->prev_acl_entry_index    = SAI_ACL_INVALID_INDEX;

    if (SAI_ACL_INVALID_INDEX == prev_index) { /* removing of head_entry_index */
    	if (next_index != SAI_ACL_INVALID_INDEX)
    		mrvl_sai_acl_entry_db[next_index].prev_acl_entry_index = SAI_ACL_INVALID_INDEX;
        table->head_entry_index = next_index;
    } else {
    	mrvl_sai_acl_entry_db[prev_index].next_acl_entry_index = acl_entry_index;

        if (SAI_ACL_INVALID_INDEX != acl_entry_index) {
        	mrvl_sai_acl_entry_db[acl_entry_index].prev_acl_entry_index = prev_index;
        }
    }

    /* update counters */
    mrvl_sai_acl_table_db[acl_table_index].entries_count--;
    mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].entries_count--;

    MRVL_SAI_LOG_DBG("Number entries for table acl_table_index - %d is %d\n", acl_table_index, mrvl_sai_acl_table_db[acl_table_index].entries_count);
    MRVL_SAI_LOG_DBG("Number entries for table acl_group_index - %d is %d\n", mrvl_sai_acl_table_db[acl_table_index].acl_group_index, mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].entries_count);
    MRVL_SAI_LOG_EXIT();
}

/* link table to bound point db and unlink table from bound point db internal functions */
static void mrvl_sai_acl_table_link_to_bound_point(_In_ uint32_t acl_table_index, _In_ sai_acl_bind_point_type_t type, _In_ uint32_t iface)
{
	uint32_t                 *head_table_index  = 0;
	mrvl_acl_bound_db_t      *bound_point        = 0;
	mrvl_acl_table_db_t      *table_to_be_added = &mrvl_sai_acl_table_db[acl_table_index];
	mrvl_acl_table_db_t      *table_head_current;

    MRVL_SAI_LOG_ENTER();

    switch (type) {
    case SAI_ACL_BIND_POINT_TYPE_PORT:
        head_table_index = &mrvl_sai_acl_bound_port_db[iface].head_table_index;
        bound_point = &mrvl_sai_acl_bound_port_db[iface];
        break;
    case SAI_ACL_BIND_POINT_TYPE_LAG:
        head_table_index = &mrvl_sai_acl_bound_lag_db[iface].lag_bound_db.head_table_index;
        bound_point = &mrvl_sai_acl_bound_lag_db[iface].lag_bound_db;
        break;
    case SAI_ACL_BIND_POINT_TYPE_VLAN:
        head_table_index = &mrvl_sai_acl_bound_vlan_db[iface].head_table_index;
        bound_point = &mrvl_sai_acl_bound_vlan_db[iface];
        break;
    case SAI_ACL_BIND_POINT_TYPE_SWITCH:
        head_table_index = &mrvl_sai_acl_bound_switch_db.head_table_index;
        bound_point = &mrvl_sai_acl_bound_switch_db;
        break;
    default:
        break;
    }

    assert(bound_point != NULL);

    if (SAI_ACL_INVALID_INDEX == *head_table_index) { /*       first entry */
        *head_table_index                        = acl_table_index;
        table_to_be_added->next_acl_table_index  = SAI_ACL_INVALID_INDEX;
        table_to_be_added->prev_acl_table_index  = SAI_ACL_INVALID_INDEX;
    } else {
    	table_head_current                       = &mrvl_sai_acl_table_db[*head_table_index];
        table_to_be_added->next_acl_table_index  = *head_table_index;
        table_to_be_added->prev_acl_table_index  = SAI_ACL_INVALID_INDEX;
        table_head_current->prev_acl_table_index = acl_table_index;
        *head_table_index                        = acl_table_index;
    }

    bound_point->bound_tables_count++;

    MRVL_SAI_LOG_DBG("Number tables bound to bind_point - %d type - %d is %d\n", iface, type, bound_point->bound_tables_count);
    MRVL_SAI_LOG_EXIT();

}

static void mrvl_sai_acl_table_unlink_from_bound_point(_In_ uint32_t acl_table_index, _In_ sai_acl_bind_point_type_t type, _In_ uint32_t iface)
{
	mrvl_acl_bound_db_t      *bound_point = 0;
	mrvl_acl_table_db_t      *table_to_be_removed;
    uint32_t                  prev_index, next_index;


    MRVL_SAI_LOG_ENTER();
    assert(SAI_ACL_INVALID_INDEX != acl_table_index);
     
    switch (type) {
    case SAI_ACL_BIND_POINT_TYPE_PORT:
         assert(SAI_ACL_PORTLIST_MAX_NUM > iface);
         bound_point = &mrvl_sai_acl_bound_port_db[iface];
         break;
    case SAI_ACL_BIND_POINT_TYPE_LAG:
         assert(SAI_LAG_MAX_GROUPS_CNS > iface);
         bound_point = &mrvl_sai_acl_bound_lag_db[iface].lag_bound_db;
         break;
    case SAI_ACL_BIND_POINT_TYPE_VLAN:
         assert(SAI_MAX_NUM_OF_VLANS > iface);
         bound_point = &mrvl_sai_acl_bound_vlan_db[iface];
         break;
    case SAI_ACL_BIND_POINT_TYPE_SWITCH:
         bound_point = &mrvl_sai_acl_bound_switch_db;
         break;
    default:
        break;
    }

    table_to_be_removed      = &mrvl_sai_acl_table_db[acl_table_index];
    prev_index               = table_to_be_removed->prev_acl_table_index;
	next_index               = table_to_be_removed->next_acl_table_index;

 	table_to_be_removed->next_acl_table_index    = SAI_ACL_INVALID_INDEX;
	table_to_be_removed->prev_acl_table_index    = SAI_ACL_INVALID_INDEX;

    if (SAI_ACL_INVALID_INDEX == prev_index) { /* removing of head_entry_index */
    	if (next_index != SAI_ACL_INVALID_INDEX)
    		mrvl_sai_acl_table_db[next_index].prev_acl_table_index = SAI_ACL_INVALID_INDEX;
    	bound_point->head_table_index = next_index;
    } else {
    	mrvl_sai_acl_table_db[prev_index].next_acl_table_index = acl_table_index;

        if (SAI_ACL_INVALID_INDEX != acl_table_index) {
        	mrvl_sai_acl_table_db[acl_table_index].prev_acl_table_index = prev_index;
        }
    }

    bound_point->bound_tables_count--;
    MRVL_SAI_LOG_DBG("Number tables bound to bind point - %d type - %d is %d\n", iface, type, bound_point->bound_tables_count);
    MRVL_SAI_LOG_EXIT();
}

/* Update FPA when binding to table is done */
static sai_status_t mrvl_sai_acl_table_update(_In_ sai_acl_bind_point_type_t type,
                                              _In_ int64_t attr, 
                                              _In_ uint32_t acl_table_index,  
                                              _In_ bool bind, 
                                              _In_ uint32_t interface, 
                                              _In_ uint32_t portbitmap)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};
    uint32_t acl_entry_index = 0, acl_group_index = 0;

    MRVL_SAI_LOG_ENTER();

	acl_group_index = mrvl_sai_acl_table_db[acl_table_index].acl_group_index;
	fpa_table_type = mrvl_sai_acl_group_db[acl_group_index].fpa_table_type;
	/* update all ACL entries created on acl_table_id with bound_port */
	acl_entry_index = mrvl_sai_acl_table_db[acl_table_index].head_entry_index;

	while (acl_entry_index != SAI_ACL_INVALID_INDEX){
        memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
        fpa_flow_entry.cookie = mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie;
        /* get by cookie from FPA */
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);

        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to get entry from FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
            /* TODO rollback */
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }

        switch (type) {

        case SAI_ACL_BIND_POINT_TYPE_PORT:
            if ((attr != SAI_PORT_ATTR_INGRESS_ACL) && (attr != SAI_PORT_ATTR_EGRESS_ACL)){
                MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", attr);
                return (SAI_STATUS_INVALID_ATTRIBUTE_0+attr);
            }

            if (bind) {
                if ((mrvl_sai_acl_entry_db[acl_entry_index].attr_port != SAI_ACL_INVALID_INTERFACE) &&
                    (mrvl_sai_acl_entry_db[acl_entry_index].attr_port != (uint16_t)interface)){
                    MRVL_SAI_LOG_ERR("Unable to bind table %d missmatch between attr_port %d and bind interface %d\n",
                                    acl_table_index, mrvl_sai_acl_entry_db[acl_entry_index].attr_port, (uint16_t)interface);
                    MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
                }
                else {
                  mrvl_acl_set_bit_MAC(fpa_flow_entry.data.acl_policy.match.portBmpVal, interface);
                  mrvl_acl_set_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
                }
            }
            else {
                mrvl_acl_clear_bit_MAC(fpa_flow_entry.data.acl_policy.match.portBmpVal, interface);
                mrvl_acl_set_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
            }
            break;

        case SAI_ACL_BIND_POINT_TYPE_LAG:
            if ((attr != SAI_LAG_ATTR_INGRESS_ACL) && (attr != SAI_LAG_ATTR_EGRESS_ACL)){
                MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", attr);
                return (SAI_STATUS_INVALID_ATTRIBUTE_0+attr);
            }

            fpa_flow_entry.data.acl_policy.match.portBmpVal = portbitmap;
            mrvl_acl_set_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
            break;

        case SAI_ACL_BIND_POINT_TYPE_VLAN:
            if ((attr != SAI_VLAN_ATTR_INGRESS_ACL) && (attr != SAI_VLAN_ATTR_EGRESS_ACL)){
                MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", attr);
                return (SAI_STATUS_INVALID_ATTRIBUTE_0+attr);
            }

            if (bind) {
                if ((mrvl_sai_acl_entry_db[acl_entry_index].attr_vlan != SAI_ACL_INVALID_INTERFACE) &&
                    (mrvl_sai_acl_entry_db[acl_entry_index].attr_vlan != (uint16_t)interface)){
                    MRVL_SAI_LOG_ERR("Unable to bind table %d missmatch between attr_vlan %d and bind interface %d\n",
                                    acl_table_index, mrvl_sai_acl_entry_db[acl_entry_index].attr_vlan, (uint16_t)interface);
                    MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
                }
                else {
                    fpa_flow_entry.data.acl_policy.match.vlanId = (uint16_t)interface;
                    fpa_flow_entry.data.acl_policy.match.vlanIdMask = SAI_ACL_VLAN_MASK;

                    /* open to all ports */
                    mrvl_acl_clear_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpVal);
                    mrvl_acl_clear_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
                }
            }
            else { /* remove vlan */
                fpa_flow_entry.data.acl_policy.match.vlanId = 0;
                mrvl_acl_clear_all_MAC(fpa_flow_entry.data.acl_policy.match.vlanIdMask);

                /* close to all ports */
                mrvl_acl_clear_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpVal);
                mrvl_acl_set_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
            }
            break;

        case SAI_ACL_BIND_POINT_TYPE_SWITCH:
            if ((attr != SAI_SWITCH_ATTR_INGRESS_ACL) && (attr != SAI_SWITCH_ATTR_EGRESS_ACL)){
                MRVL_SAI_LOG_ERR("Not supported attribute - PRId64\n", attr);
                return (SAI_STATUS_INVALID_ATTRIBUTE_0+attr);
            }

            if (bind) { /* the rule must match to all ports - mask 0 */
                mrvl_acl_clear_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
            }
            else { /*no match to any port - set mask to no match to any port */
                mrvl_acl_set_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
            }
            break;

        default:
            MRVL_SAI_LOG_ERR("Not supported type - %d\n", type);
            return (SAI_STATUS_INVALID_PARAMETER);
        }

        /* update by cookie from FPA */
        fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry, 0);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to update entry to FPA: cookie - %llx, status = %d\n", fpa_flow_entry.cookie, fpa_status);
            /* TODO rollback */
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }
		acl_entry_index = mrvl_sai_acl_entry_db[acl_entry_index].next_acl_entry_index;
	}


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/* Bind ACL table to port */
sai_status_t mrvl_sai_acl_table_bind_to_port(_In_ void *arg,
											_In_ const sai_object_id_t object_id,
											_In_ uint32_t port)

{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;

    MRVL_SAI_LOG_ENTER();

    if (((int64_t)arg != SAI_PORT_ATTR_INGRESS_ACL) && ((int64_t)arg != SAI_PORT_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (port >= SAI_ACL_PORTLIST_MAX_NUM){
        MRVL_SAI_LOG_ERR("Unsupported port %d is received\n", port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index)))
    {
        MRVL_SAI_LOG_ERR("Failed to convert ACL table oid %" PRIx64 " to acl table index\n", object_id);
        MRVL_SAI_API_RETURN(status);
    }

	if (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS){
		if ((int64_t)arg == SAI_PORT_ATTR_INGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d egress table stage, but bind acl attr - %d\n",
			    			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
			MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}
	else { /* SAI_ACL_STAGE_INGRESS */
		if ((int64_t)arg == SAI_PORT_ATTR_EGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d ingress table stage, but bind acl attr - %d\n",
							acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}

    /* verify if table bind_point_types_bitmap corresponding to port type */
    if (!mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, SAI_ACL_BIND_POINT_TYPE_PORT)){
        MRVL_SAI_LOG_ERR("Table's bind_point_types_bitmap - %d is not matched, table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* table can be bound to only one bound type */
    if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES) &&
        (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_PORT) ){
        MRVL_SAI_LOG_ERR("Table's bound to bind point - %d to table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_PORT, (int64_t)(arg), acl_table_index, true /* bind */, port, 0))){
		MRVL_SAI_LOG_ERR("Can't bind port - %d to table - %d \n", port, acl_table_index);
		MRVL_SAI_API_RETURN(status);
	}

	mrvl_sai_acl_table_link_to_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_PORT, port);

    mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_BIND_POINT_TYPE_PORT;
    mrvl_acl_set_bit_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_portbitmap, port);  

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* unbind  ACL table from the port */
sai_status_t mrvl_sai_acl_table_unbind_from_port(_In_ void *arg,
												_In_ uint32_t port)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;


    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_PORT_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_PORT_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (port >= SAI_ACL_PORTLIST_MAX_NUM){
        MRVL_SAI_LOG_ERR("Unsupported port %d is received\n", port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_port_db[port].bound_tables_count == 0){
        MRVL_SAI_LOG_ERR("Port is not bound to any table - %d\n", port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    while (mrvl_sai_acl_bound_port_db[port].head_table_index != SAI_ACL_INVALID_INDEX){
    	acl_table_index = mrvl_sai_acl_bound_port_db[port].head_table_index;
    	if ((((int64_t)arg == SAI_PORT_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_PORT_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_PORT, (int64_t)(arg), acl_table_index, false /* unbind */, port, 0))){
			MRVL_SAI_LOG_ERR("Can't unbind port - %d from table - %d \n", port, acl_table_index);
			MRVL_SAI_API_RETURN(status);
		}
		mrvl_sai_acl_table_unlink_from_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_PORT, port);

        mrvl_acl_clear_bit_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_portbitmap, port);
        if (mrvl_acl_is_all_0_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_portbitmap)) {
            mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_MAX_BIND_POINT_TYPES;
        }
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
#if 0 
get function support sai_object_list_t
sai_status_t mrvl_sai_acl_get_table_id_per_port(_In_ void *arg, _In_ uint32_t port, _Inout_ sai_attribute_value_t *value)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0, i = 0;
    sai_object_id_t acl_table_id = SAI_NULL_OBJECT_ID;
    bool obj_limit = false;

    MRVL_SAI_LOG_ENTER();

    if (value->objlist.count > 0) /* number of objects are requested */
    	obj_limit = true;

    while (mrvl_sai_acl_bound_port_db[port].head_table_index != SAI_ACL_INVALID_INDEX){
    	acl_table_index = mrvl_sai_acl_bound_port_db[port].head_table_index;
    	if ((((int64_t)arg == SAI_PORT_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_PORT_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &acl_table_id))) {
        	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }
        if (obj_limit && (i == value->objlist.count))
        	break;
    	value->objlist.list[i] = acl_table_id;
    	i++;
    }
    if (obj_limit == false)
    	value->objlist.count = i;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
#endif

sai_status_t mrvl_sai_acl_get_table_id_per_port(_In_ void *arg, _In_ uint32_t port, _Inout_ sai_attribute_value_t *value)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;
    sai_object_id_t acl_table_id = SAI_NULL_OBJECT_ID;

    MRVL_SAI_LOG_ENTER();

    if (((int64_t)arg != SAI_PORT_ATTR_INGRESS_ACL) && ((int64_t)arg != SAI_PORT_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_port_db[port].head_table_index == SAI_ACL_INVALID_INDEX){
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else {
    	acl_table_index = mrvl_sai_acl_bound_port_db[port].head_table_index;
    	if ((((int64_t)arg == SAI_PORT_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_PORT_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_PORT) &&
            (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES)){
            MRVL_SAI_LOG_ERR("Missmatch for table bind point %d of table acl_table_index - %d\n", 
                             mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
            MRVL_SAI_API_RETURN(status);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &acl_table_id))) {
        	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }

    	value->oid = acl_table_id;
    }


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* LAG */
/* the function called when port is added/removed to/from lag  */
sai_status_t mrvl_sai_acl_lag_port_update(_In_ uint32_t lag, _In_ uint32_t port, _In_ bool is_added)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0, 
             aggr_portbitmap = 0,
             portbitmap = 0,
             attr;

    MRVL_SAI_LOG_ENTER();

    if (mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.head_table_index == SAI_ACL_INVALID_INDEX){
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
    }

    if (port >= SAI_ACL_PORTLIST_MAX_NUM){
        MRVL_SAI_LOG_ERR("Unsupported port %d is received\n", port);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    acl_table_index = mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.head_table_index;
    if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_LAG) &&
        (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES)){
        MRVL_SAI_LOG_ERR("Missmatch for table bind point %d of table acl_table_index - %d\n", 
                         mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
        MRVL_SAI_API_RETURN(status);
    }

    if (!is_added && !mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_bound_lag_db[lag].portbitmap, port)){
        MRVL_SAI_LOG_ERR("Port %d lag %d missmatch\n", 
                         port, lag);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    if (is_added && mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_bound_lag_db[lag].portbitmap, port)){
        MRVL_SAI_LOG_ERR("Port %d lag %d missmatch\n", 
                         port, lag);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    if (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS)
        attr = SAI_LAG_ATTR_INGRESS_ACL;
    else 
        attr = SAI_LAG_ATTR_EGRESS_ACL;

    portbitmap      = mrvl_sai_acl_bound_lag_db[lag].portbitmap;
    aggr_portbitmap = mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.aggr_portbitmap;

    if (is_added) {
        mrvl_acl_set_bit_MAC(portbitmap, port);
        mrvl_acl_set_bit_MAC(aggr_portbitmap, port);
    }
    else {
        mrvl_acl_clear_bit_MAC(portbitmap, port);
        mrvl_acl_clear_bit_MAC(aggr_portbitmap, port);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_LAG, (int64_t)attr, acl_table_index, true /* bind */, lag, aggr_portbitmap))){
		MRVL_SAI_LOG_ERR("Can't update lag - %d by port %d to table - %d \n", lag, port, acl_table_index);
		MRVL_SAI_API_RETURN(status);
	}

    mrvl_sai_acl_bound_lag_db[lag].portbitmap = portbitmap; /* update new lag portbitmap */ 
    mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.aggr_portbitmap = aggr_portbitmap;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/* Bind ACL table to lag */
sai_status_t mrvl_sai_acl_table_bind_to_lag(_In_ void *arg,
											_In_ const sai_object_id_t object_id,
											_In_ uint32_t lag)

{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0, 
             aggr_portbitmap = 0,
             portbitmap = 0,
             port, i;
    sai_object_list_t port_objlist;
    sai_object_id_t   port_id[SAI_LAG_MAX_MEMBERS_IN_GROUP_CNS] = {0};

    MRVL_SAI_LOG_ENTER();

    if (((int64_t)arg != SAI_LAG_ATTR_INGRESS_ACL) && ((int64_t)arg != SAI_LAG_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index)))
		return status;

	if (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS){
		if ((int64_t)arg == SAI_LAG_ATTR_INGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d egress table stage, but bind acl attr - %d\n",
			    			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
			MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}
	else { /* SAI_ACL_STAGE_INGRESS */
		if ((int64_t)arg == SAI_LAG_ATTR_EGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d ingress table stage, but bind acl attr - %d\n",
							acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}

    /* verify if table bind_point_types_bitmap corresponding to port type */
    if (!mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, SAI_ACL_BIND_POINT_TYPE_LAG)){
        MRVL_SAI_LOG_ERR("Table's bind_point_types_bitmap - %d is not matched, table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* table can be bound to only one bound type */
    if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES) &&
        (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_LAG) ){
        MRVL_SAI_LOG_ERR("Table's bound to bind point - %d to table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* get port members of lag */
    port_objlist.count = 0;
    port_objlist.list = port_id;
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_lag_port_list(lag, &port_objlist))){
        MRVL_SAI_LOG_ERR("Can't get portbitmap for lag - %d to table - %d \n", lag, acl_table_index);
        MRVL_SAI_API_RETURN(status);
    }

    /* translate port_objlist to port bitmap */
    for (i=0; i<port_objlist.count; i++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_objlist.list[i], SAI_OBJECT_TYPE_PORT, &port)) {
            MRVL_SAI_LOG_ERR("invalid port_objlist\n");
            return SAI_STATUS_INVALID_PARAMETER;
        } 
        if (port >= SAI_ACL_PORTLIST_MAX_NUM){
            MRVL_SAI_LOG_ERR("Unsupported port %d is received\n", port);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
        mrvl_acl_set_bit_MAC(portbitmap, port);
    }

    aggr_portbitmap = portbitmap | mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.aggr_portbitmap;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_LAG, (int64_t)(arg), acl_table_index, true /* bind */, lag, aggr_portbitmap))){
		MRVL_SAI_LOG_ERR("Can't bind lag - %d to table - %d \n", lag, acl_table_index);
		MRVL_SAI_API_RETURN(status);
	}

	mrvl_sai_acl_table_link_to_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_LAG, lag);
    mrvl_sai_acl_bound_lag_db[lag].portbitmap = portbitmap; /* update new lag portbitmap */

    mrvl_sai_acl_table_db[acl_table_index].bound_interface.type                = SAI_ACL_BIND_POINT_TYPE_LAG;
    mrvl_acl_set_bit_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.bound_lagbitmap, lag);  
    mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.aggr_portbitmap = aggr_portbitmap;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* unbind  ACL table from the lag */
sai_status_t mrvl_sai_acl_table_unbind_from_lag(_In_ void *arg,
												_In_ uint32_t lag)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0,
             aggr_portbitmap = 0,
             portbitmap = 0,
             port, i;
    sai_object_list_t port_objlist;
    sai_object_id_t   port_id[SAI_LAG_MAX_GROUPS_CNS] = {0};


    MRVL_SAI_LOG_ENTER();

    if (((int64_t)arg != SAI_LAG_ATTR_INGRESS_ACL) && ((int64_t)arg != SAI_LAG_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.bound_tables_count == 0){
        MRVL_SAI_LOG_ERR("Lag %d is not bound to any table\n", lag);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    while (mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.head_table_index != SAI_ACL_INVALID_INDEX){
    	acl_table_index = mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.head_table_index;
    	if ((((int64_t)arg == SAI_PORT_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_PORT_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        /* get port members of lag */
        port_objlist.count = 0;
        port_objlist.list = port_id;
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_lag_port_list(lag, &port_objlist))){
            MRVL_SAI_LOG_ERR("Can't get portbitmap for lag - %d to table - %d \n", lag, acl_table_index);
            MRVL_SAI_API_RETURN(status);
        }

        /* translate port_objlist to port bitmap */
        for (i=0; i<port_objlist.count; i++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_objlist.list[i], SAI_OBJECT_TYPE_PORT, &port)) {
                MRVL_SAI_LOG_ERR("invalid port_objlist\n");
                return SAI_STATUS_INVALID_PARAMETER;
            } 
            mrvl_acl_set_bit_MAC(portbitmap, port);
        }

        /* substruct the portbitmap from aggr_portbitmap */
        aggr_portbitmap = ~portbitmap & mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.aggr_portbitmap;

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_LAG, (int64_t)(arg), acl_table_index, false /* unbind */, lag, aggr_portbitmap))){
			MRVL_SAI_LOG_ERR("Can't unbind lag - %d from table - %d \n", lag, acl_table_index);
			MRVL_SAI_API_RETURN(status);
		}

		mrvl_sai_acl_table_unlink_from_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_LAG, lag);
        mrvl_sai_acl_bound_lag_db[lag].portbitmap = 0; /* update new lag portbitmap */

        mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.aggr_portbitmap = aggr_portbitmap;
        mrvl_acl_clear_bit_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.bound_lagbitmap, lag); 
        if (mrvl_acl_is_all_0_MAC(mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_lag.bound_lagbitmap))
            mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_MAX_BIND_POINT_TYPES;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_sai_acl_get_table_id_per_lag(_In_ void *arg, _In_ uint32_t lag, _Inout_ sai_attribute_value_t *value)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;
    sai_object_id_t acl_table_id = SAI_NULL_OBJECT_ID;

    MRVL_SAI_LOG_ENTER();

    if (((int64_t)arg != SAI_LAG_ATTR_INGRESS_ACL) && ((int64_t)arg != SAI_LAG_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.head_table_index == SAI_ACL_INVALID_INDEX){
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else {
    	acl_table_index = mrvl_sai_acl_bound_lag_db[lag].lag_bound_db.head_table_index;

        if ((((int64_t)arg == SAI_LAG_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_LAG_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_LAG) &&
            (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES)){
            MRVL_SAI_LOG_ERR("Missmatch for table bind point %d of table acl_table_index - %d\n", 
                             mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
            MRVL_SAI_API_RETURN(status);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &acl_table_id))) {
        	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }

    	value->oid = acl_table_id;
    }


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Bind table to switch */
sai_status_t mrvl_sai_acl_table_bind_to_switch(_In_ void *arg,
											_In_ const sai_object_id_t object_id,
											_In_ uint32_t switch_idx)

{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;

    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_SWITCH_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_SWITCH_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }


	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index)))
		return status;

	if (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS){
		if ((int64_t)arg == SAI_SWITCH_ATTR_INGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d egress table stage, but bind acl attr - %d\n",
			    			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
			    	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}
	else { /* SAI_ACL_STAGE_INGRESS */
		if ((int64_t)arg == SAI_SWITCH_ATTR_EGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d ingress table stage, but bind acl attr - %d\n",
							acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
					MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}

    /* verify if table bind_point_types_bitmap corresponding to switch type */
    if (!mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, SAI_ACL_BIND_POINT_TYPE_SWITCH)){
        MRVL_SAI_LOG_ERR("Table's bind_point_types_bitmap - %d is not matched, table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* verify if table table_fields_bitmap corresponding to switch type */
    if (mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap, SAI_ACL_MATCH_FIELD_SRC_PORT) ||
        mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap, SAI_ACL_MATCH_FIELD_IN_PORT) ||
        mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap, SAI_ACL_MATCH_FIELD_OUT_PORT)||
        mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap, SAI_ACL_MATCH_FIELD_OUTER_VLAN_ID)){
        MRVL_SAI_LOG_ERR("Table's table_fields_bitmap - %d is not matched, table - %d \n", mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* table can be bound to only once */
    if (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES) {
        MRVL_SAI_LOG_ERR("Table is already bound to bind point - %d to table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_SWITCH, (int64_t)(arg), acl_table_index, true /* bind */, 0, 0))){
		MRVL_SAI_LOG_ERR("Can't bind switch to table - %d \n", acl_table_index);
		MRVL_SAI_API_RETURN(status);
	}

	mrvl_sai_acl_table_link_to_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_SWITCH, 0);

    mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_BIND_POINT_TYPE_SWITCH;
    mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_switch = SAI_DEFAULT_ETH_SWID_CNS;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* unbind  ACL table from the switch */
sai_status_t mrvl_sai_acl_table_unbind_from_switch(_In_ void *arg,
												_In_ uint32_t switch_idx)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;

    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_SWITCH_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_SWITCH_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (mrvl_sai_acl_bound_switch_db.bound_tables_count == 0){
        MRVL_SAI_LOG_ERR("switch is not bound to any table\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    while (mrvl_sai_acl_bound_switch_db.head_table_index != SAI_ACL_INVALID_INDEX){
    	acl_table_index = mrvl_sai_acl_bound_switch_db.head_table_index;
    	if ((((int64_t)arg == SAI_PORT_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_PORT_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
		if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_SWITCH, (int64_t)(arg), acl_table_index, false /* unbind */, 0, 0))){
			MRVL_SAI_LOG_ERR("Can't unbind switch from table - %d \n", acl_table_index);
			MRVL_SAI_API_RETURN(status);
		}
		mrvl_sai_acl_table_unlink_from_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_SWITCH, 0);
        mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_MAX_BIND_POINT_TYPES;
        mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_switch = SAI_ACL_INVALID_INTERFACE;
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
sai_status_t mrvl_sai_acl_get_table_id_per_switch(_In_ void *arg, _In_ uint32_t switch_idx, _Inout_ sai_attribute_value_t *value)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;
    sai_object_id_t acl_table_id = SAI_NULL_OBJECT_ID;

    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_SWITCH_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_SWITCH_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_switch_db.head_table_index == SAI_ACL_INVALID_INDEX){
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else {
    	acl_table_index = mrvl_sai_acl_bound_switch_db.head_table_index;
    	if ((((int64_t)arg == SAI_SWITCH_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_SWITCH_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_SWITCH) &&
           (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES)){
            MRVL_SAI_LOG_ERR("Another interface type %d is bound to table acl_table_index - %d\n", 
                             mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
            MRVL_SAI_API_RETURN(status);
        }

        if (mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_switch != switch_idx) {
        	MRVL_SAI_LOG_ERR("switch is not bound to table acl_table_index - %d\n", acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &acl_table_id))) {
        	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }
    	value->oid = acl_table_id;
    }


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Bind table to vlan */
sai_status_t mrvl_sai_acl_table_bind_to_vlan(_In_ void *arg,
											_In_ const sai_object_id_t object_id,
											_In_ uint32_t vlan_idx)

{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;

    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_VLAN_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_VLAN_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(object_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index)))
		return status;

	if (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS){
		if ((int64_t)arg == SAI_VLAN_ATTR_INGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d egress table stage, but bind acl attr - %d\n",
			    			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
			MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}
	else { /* SAI_ACL_STAGE_INGRESS */
		if ((int64_t)arg == SAI_VLAN_ATTR_EGRESS_ACL){
			MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d ingress table stage, but bind acl attr - %d\n",
							acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
		}
	}

    /* verify if table bind_point_types_bitmap corresponding to vlan type */
    if (!mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, SAI_ACL_BIND_POINT_TYPE_VLAN)){
        MRVL_SAI_LOG_ERR("Table's bind_point_types_bitmap - %d is not matched, table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* table can be bound to only once */
    if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES) && 
        (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_VLAN)) {
        MRVL_SAI_LOG_ERR("Table's bound to bind point - %d to table - %d \n", mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_VLAN, (int64_t)(arg), acl_table_index, true /* true */, vlan_idx, 0))){
		MRVL_SAI_LOG_ERR("Can't bind vlan - %d to table - %d \n", vlan_idx, acl_table_index);
		MRVL_SAI_API_RETURN(status);
	}

	mrvl_sai_acl_table_link_to_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_VLAN, vlan_idx);

    mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_BIND_POINT_TYPE_VLAN;
	mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_vlan = vlan_idx;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* unbind  ACL table from the vlan */
sai_status_t mrvl_sai_acl_table_unbind_from_vlan(_In_ void *arg,
												_In_ uint32_t vlan_idx)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;


    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_VLAN_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_VLAN_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_vlan_db[vlan_idx].bound_tables_count == 0){
        MRVL_SAI_LOG_ERR("VLAN index is not bound to any table - %d\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    while (mrvl_sai_acl_bound_vlan_db[vlan_idx].head_table_index != SAI_ACL_INVALID_INDEX){
    	acl_table_index = mrvl_sai_acl_bound_vlan_db[vlan_idx].head_table_index;
    	if ((((int64_t)arg == SAI_VLAN_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_VLAN_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

		if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_update(SAI_ACL_BIND_POINT_TYPE_VLAN, (sai_int64_t)(arg), acl_table_index, false, vlan_idx, 0))){
			MRVL_SAI_LOG_ERR("Can't unbind vlan_idx - %d from table - %d \n", vlan_idx, acl_table_index);
			MRVL_SAI_API_RETURN(status);
		}
		mrvl_sai_acl_table_unlink_from_bound_point(acl_table_index, SAI_ACL_BIND_POINT_TYPE_VLAN, vlan_idx);

        mrvl_sai_acl_table_db[acl_table_index].bound_interface.type = SAI_ACL_MAX_BIND_POINT_TYPES;
        mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_vlan = SAI_ACL_INVALID_INTERFACE;

    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_sai_acl_get_table_id_per_vlan(_In_ void *arg, _In_ uint32_t vlan_idx, _Inout_ sai_attribute_value_t *value)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t acl_table_index = 0;
    sai_object_id_t acl_table_id = SAI_NULL_OBJECT_ID;

    MRVL_SAI_LOG_ENTER();
    if (((int64_t)arg != SAI_VLAN_ATTR_INGRESS_ACL) &&((int64_t)arg != SAI_VLAN_ATTR_EGRESS_ACL)){
        MRVL_SAI_LOG_ERR("Wrong attr %d is received\n",
                            (int64_t)arg);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_bound_vlan_db[vlan_idx].head_table_index == SAI_ACL_INVALID_INDEX){
        value->oid = SAI_NULL_OBJECT_ID;
    }
    else {
        acl_table_index = mrvl_sai_acl_bound_vlan_db[vlan_idx].head_table_index;
    	if ((((int64_t)arg == SAI_VLAN_ATTR_INGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_EGRESS)) ||
			(((int64_t)arg == SAI_VLAN_ATTR_EGRESS_ACL) && (mrvl_sai_acl_table_db[acl_table_index].stage == SAI_ACL_STAGE_INGRESS))){
        	MRVL_SAI_LOG_ERR("Missmatch for binding stages for acl_table_index - %d, mrvl_sai_acl_table_db[acl_table_index].stage - %d, bind acl attr - %d\n",
        			acl_table_index, mrvl_sai_acl_table_db[acl_table_index].stage, (int64_t)arg);
        	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }

        if ((mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_BIND_POINT_TYPE_VLAN)  &&
            (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES)){
            MRVL_SAI_LOG_ERR("Another interface type %d is bound to table acl_table_index - %d\n", 
                             mrvl_sai_acl_table_db[acl_table_index].bound_interface.type, acl_table_index);
            MRVL_SAI_API_RETURN(status);
        }

        if (mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_vlan != vlan_idx){
        	MRVL_SAI_LOG_ERR("Another vlan %d is bound to table acl_table_index - %d\n", mrvl_sai_acl_table_db[acl_table_index].bound_interface.id.bound_vlan, acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, &acl_table_id))) {
        	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
        	MRVL_SAI_API_RETURN(status);
        }

    	value->oid = acl_table_id;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
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
    const sai_object_key_t key = { .key.object_id = acl_table_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == acl_table_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_id, key_str);
    MRVL_SAI_LOG_NTC("Set attributes for ACL table %s\n", key_str);

    status = mrvl_sai_utl_set_attribute(&key, key_str, acl_table_attribs, acl_table_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
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
    const sai_object_key_t key = { .key.object_id = acl_table_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == acl_table_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for ACL table %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, acl_table_attribs, acl_table_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
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
    const sai_object_key_t key = { .key.object_id = acl_entry_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == acl_entry_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_entry_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_ENTRY, acl_entry_id, key_str);
    MRVL_SAI_LOG_NTC("Set attributes for ACL entry %s\n", key_str);

    status = mrvl_sai_utl_set_attribute(&key, key_str, acl_entry_attribs, acl_entry_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

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
    const sai_object_key_t key = { .key.object_id = acl_entry_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == acl_entry_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_ENTRY, acl_entry_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for ACL entry %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, acl_entry_attribs, acl_entry_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_entry_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Create an ACL table
 *
 * @param[out] acl_table_id The the ACL table id
 * @param[in] attr_count Number of attributes
 * @param[in] switch_id Switch Object id
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_create_acl_table(_Out_ sai_object_id_t* acl_table_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list)
{
    char    acl_str[MAX_KEY_STR_LEN];
    char    list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *stage, *size, *bind_point_types, *action_types;
    uint32_t tmp_index;
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_table_index  = 0, acl_table_size = 0;
    bool is_dynamic_sized = false;
    bool is_non_ip_table = false, is_ipv4_table = false, is_ipv6_table = false, is_egress_ipv4_table = false;
    uint32_t table_fields_bitmap = 0, table_actions_bitmap = 0, table_bind_point_bitmap = 0;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (NULL == acl_table_id) {
        MRVL_SAI_LOG_ERR("NULL acl_table_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_table_attribs, acl_table_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);

    /* check mandatory attr SAI_ACL_TABLE_ATTR_ACL_STAGE  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_ACL_STAGE, &stage, &tmp_index));
    if (SAI_ACL_STAGE_EGRESS < stage->s32) {
    	MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_ATTR_ACL_STAGE - %u\n", stage->s32);
        MRVL_SAI_API_RETURN(status);
    }

    /* check mandatory attr SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST  */
    if (SAI_STATUS_SUCCESS ==
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST, &bind_point_types, &tmp_index))){
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_bind_point_type_list_validate(&bind_point_types->s32list,
                                                              tmp_index,
                                                              &table_bind_point_bitmap))){
            MRVL_SAI_LOG_ERR("Invalid values for SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST\n");
            MRVL_SAI_API_RETURN(status);
        }
    }
    else {
        table_bind_point_bitmap = default_bind_point_type_bitmap;
    }


    /* check attr SAI_ACL_TABLE_ATTR_SIZE  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_SIZE, &size, &tmp_index))){
		if (size->u32 > SAI_ACL_ENTRIES_PER_TABLE_MAX_NUM){
			MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_ATTR_SIZE - %d\n", size->u32);
	        MRVL_SAI_API_RETURN(status);
		}
		else if (0 == size->u32) {
			MRVL_SAI_LOG_ERR("Table size received is zero. Value is set to DEFAULT TABLE SIZE \n");
            acl_table_size   = SAI_ACL_ENTRIES_PER_TABLE_MAX_NUM;
            is_dynamic_sized = true;
        } else {
            acl_table_size   = size->u32;
            is_dynamic_sized = false;
        }
    }
    else { /* SAI_ACL_TABLE_ATTR_SIZE attr is not present */
		acl_table_size   = SAI_ACL_ENTRIES_PER_TABLE_MAX_NUM;
		is_dynamic_sized = true;
	}

    /* check attr SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST  */
    if (SAI_STATUS_SUCCESS ==
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST, &action_types, &tmp_index))){
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_acl_table_validate_action_type_list(&action_types->s32list,
                                                              tmp_index,
                                                              (sai_acl_stage_t)(stage->u32),
                                                              &table_actions_bitmap))){
            MRVL_SAI_LOG_ERR("Invalid values for SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST\n");
            MRVL_SAI_API_RETURN(status);
        }
    }
    else {
        table_actions_bitmap = default_actions_type_bitmap;
    }


    /* validate there is at least one match field set */
    if (table_actions_bitmap == 0){
		MRVL_SAI_LOG_ERR("At least one match action must be given - %x \n", table_actions_bitmap);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* check match fields */
    if (SAI_STATUS_SUCCESS !=
        		(status = mrvl_sai_acl_fill_table_match_fields(attr_count, attr_list, stage->u32, &table_fields_bitmap, &is_ipv6_table, &is_ipv4_table, &is_non_ip_table, &is_egress_ipv4_table))){
        MRVL_SAI_API_RETURN(status);
    }

    /* validate there is at least one match field set */
    if (table_fields_bitmap == 0){
		MRVL_SAI_LOG_ERR("At least one match field must be given - %x \n", table_fields_bitmap);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    MRVL_SAI_LOG_DBG("READ-WRITE attributes: stage - %d, bind_point count - %d, actions count - %d, table size - %d\n",
    		stage->u32, bind_point_types->u32list.count, action_types->u32list.count, size->u32);

    if (true == is_non_ip_table){
    	fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    }
    else if (true == is_ipv4_table){
    	fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL1_E;
    }
    else if (true == is_egress_ipv4_table){
    	fpa_table_type = FPA_FLOW_TABLE_TYPE_EPCL_E;
    }
    else if (true == is_ipv6_table){
    	fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL2_E;
    }
    else {
		MRVL_SAI_LOG_ERR("Undefined table ip type is_non_ip_table - %d, is_ipv4_table - %d, is_ipv6_table - %d \n", is_non_ip_table, is_ipv4_table, is_ipv6_table);
        MRVL_SAI_API_RETURN(status);
	}

    /* get free index */
    if (SAI_STATUS_SUCCESS !=
        		(status = mrvl_acl_find_free_index_in_table_db(&acl_table_index))){
    	MRVL_SAI_LOG_ERR("No free index for ACL TABLE\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* create ACL table object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_index, acl_table_id))) {
    	mrvl_sai_acl_table_db[acl_table_index].is_used = false;
    	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE acl_table_index - %d\n", acl_table_index);
    	MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_acl_table_db[acl_table_index].is_used                = true;
    mrvl_sai_acl_table_db[acl_table_index].stage                  = stage->u32;
    mrvl_sai_acl_table_db[acl_table_index].attr_table_size        = size->u32;
    mrvl_sai_acl_table_db[acl_table_index].table_size             = acl_table_size;
    mrvl_sai_acl_table_db[acl_table_index].fpa_table_type         = fpa_table_type;

    mrvl_sai_acl_table_db[acl_table_index].entries_count          = 0;
    mrvl_sai_acl_table_db[acl_table_index].is_dynamic_size        = is_dynamic_sized;
    mrvl_sai_acl_table_db[acl_table_index].head_entry_index       = SAI_ACL_INVALID_INDEX;

    mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap  = table_bind_point_bitmap;
    mrvl_sai_acl_table_db[acl_table_index].bound_interface.type     = SAI_ACL_MAX_BIND_POINT_TYPES;
    mrvl_sai_acl_table_db[acl_table_index].table_fields_bitmap    = table_fields_bitmap;
    mrvl_sai_acl_table_db[acl_table_index].table_actions_bitmap   = table_actions_bitmap;

    mrvl_sai_acl_table_db[acl_table_index].prev_acl_table_index   = SAI_ACL_INVALID_INDEX;
    mrvl_sai_acl_table_db[acl_table_index].next_acl_table_index   = SAI_ACL_INVALID_INDEX;

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE, *acl_table_id, acl_str);
    MRVL_SAI_LOG_NTC("Created ACL table %s\n", acl_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
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
sai_status_t mrvl_remove_acl_table(_In_ sai_object_id_t acl_table_id)
{
    char acl_data_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_table_index = 0;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == acl_table_id) {
        MRVL_SAI_LOG_ERR("Invalid object param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE, acl_table_id, acl_data_str);
    MRVL_SAI_LOG_NTC("Delete  %s\n", acl_data_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(acl_table_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* error to delete not created entry */
    assert(mrvl_sai_acl_table_db[acl_table_index].is_used != false);

    /* error to delete table with valid group */
    assert(mrvl_sai_acl_table_db[acl_table_index].acl_group_index != SAI_ACL_INVALID_INDEX);

    /* can remove act table that is bound  */
    if (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type != SAI_ACL_MAX_BIND_POINT_TYPES){
		MRVL_SAI_LOG_ERR("Unable to remove ACT Table - bound to interface - %d\n", mrvl_sai_acl_table_db[acl_table_index].bound_interface.type);
		MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* error to delete table with entries */
    if (mrvl_sai_acl_table_db[acl_table_index].entries_count != 0){
		MRVL_SAI_LOG_ERR("Unable to remove ACT Table - number of entries exist - %d\n", mrvl_sai_acl_table_db[acl_table_index].entries_count);
		MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_table_remove(acl_table_index);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Create an ACL entry
 *
 * @param[out] acl_entry_id The ACL entry id
 * @param[in] switch_id The Switch Object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_create_acl_entry(_Out_ sai_object_id_t* acl_entry_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list)
{
    char    acl_str[MAX_KEY_STR_LEN];
    char    list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *priority, *table_id, *admin_state;
    uint32_t temp_index;
    uint32_t      acl_entry_index = 0, acl_table_index = 0, attr_port = SAI_ACL_INVALID_INTERFACE, attr_vlan = SAI_ACL_INVALID_INTERFACE;
    sai_status_t status = SAI_STATUS_SUCCESS;
    FPA_STATUS    fpa_status = FPA_OK;
    FPA_FLOW_TABLE_ENTRY_TYPE_ENT fpa_table_type = FPA_FLOW_TABLE_TYPE_PCL0_E;
    FPA_FLOW_TABLE_ENTRY_STC     fpa_flow_entry = {0};
    FPA_FLOW_TABLE_MATCH_FIELDS_ACL_POLICY_STC fpa_match_entry = {0};
    uint32_t entry_fields_bitmap = 0, entry_actions_bitmap = 0;
    uint32_t fpa_priority = 0;
    sai_acl_action_data_t entry_action_data[SAI_ACL_MAX_ACTION_TYPES];
    bool admin_value = false;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (NULL == acl_entry_id) {
        MRVL_SAI_LOG_ERR("NULL entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_entry_attribs, acl_entry_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_entry_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);

    /* check mandatory field SAI_ACL_ENTRY_ATTR_TABLE_ID  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_TABLE_ID, &table_id, &temp_index));
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(table_id->oid, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }
    if (false == mrvl_sai_acl_table_db[acl_table_index].is_used){
    	MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_ENTRY_ATTR_TABLE_ID acl_table_index - %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (mrvl_sai_acl_table_db[acl_table_index].entries_count == mrvl_sai_acl_table_db[acl_table_index].table_size) {
    	MRVL_SAI_LOG_ERR("Max entries for this Table is reached - %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* check field SAI_ACL_TABLE_ATTR_PRIORITY  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_PRIORITY, &priority, &temp_index))){
		if ((SAI_ACL_MIN_PRIORITY > priority->u32) &&
			(SAI_ACL_MAX_PRIORITY < priority->u32)) {
			MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_ENTRY_ATTR_PRIORITY - %d\n", priority->u32);
			MRVL_SAI_API_RETURN(status);
		}
    }
    else
    	priority = SAI_ACL_MIN_PRIORITY; /* TODO get SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY from switch object */

    /* check field SAI_ACL_ENTRY_ATTR_ADMIN_STATE  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_ENTRY_ATTR_ADMIN_STATE, &admin_state, &temp_index))){
    	admin_value = admin_state->booldata;
    }
    else
    	admin_value = true; /* default is enabled */

    if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_acl_entry_match_fields_validation(attr_count, attr_list, acl_table_index,
        													&entry_fields_bitmap, &fpa_match_entry, &attr_port, &attr_vlan))){
        MRVL_SAI_LOG_ERR("Match fields validation failed\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* are entry match actions subset of table actions */
    memset(entry_action_data, 0, sizeof(entry_action_data));
    if (SAI_STATUS_SUCCESS !=
        	(status = mrvl_sai_acl_entry_actions_validation(attr_count, attr_list, acl_table_index,
        													&entry_actions_bitmap, entry_action_data))){
        MRVL_SAI_LOG_ERR("No match for entry vs table actions\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* get free index */
    if (SAI_STATUS_SUCCESS !=
        		(status = mrvl_acl_find_free_index_in_entry_db(&acl_entry_index))){
    	MRVL_SAI_LOG_ERR("No free index for ACL ENTRY db\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* initialize FPA flow entry */
    fpa_table_type = mrvl_sai_acl_table_db[acl_table_index].fpa_table_type; /*mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].fpa_table_type;*/
    fpa_priority = mrvl_sai_acl_table_db[acl_table_index].priority * mrvl_sai_acl_table_db[acl_table_index].table_size + priority->u32;
    fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
    if (fpa_status != FPA_OK) {
    	/* free index */
    	mrvl_sai_acl_entry_db[acl_entry_index].is_used = false;
        MRVL_SAI_LOG_ERR("Failed to init acl entry status = %d\n", fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    fpa_flow_entry.entryType = fpa_table_type;
    fpa_flow_entry.cookie = MRVL_SAI_ACL_CREATE_COOKIE_MAC(acl_table_index, acl_entry_index);
    fpa_flow_entry.flowModFlags = 0;
    fpa_flow_entry.priority = fpa_priority;
    fpa_flow_entry.timeoutHardTime = 0;
    fpa_flow_entry.timeoutIdleTime = 0;
    memcpy(&fpa_flow_entry.data.acl_policy.match, &fpa_match_entry, sizeof(fpa_match_entry));
    if (mrvl_sai_acl_table_db[acl_table_index].bound_interface.type == SAI_ACL_MAX_BIND_POINT_TYPES){ /* invalid portBmp in order to not match entry */
            mrvl_acl_clear_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpVal);
            mrvl_acl_set_all_MAC(fpa_flow_entry.data.acl_policy.match.portBmpMask);
    }
    /* in case of valid bind type - > portBmp and portBmpMask are already set */

    /* handle admin attribute - set instruction in case of invalid, if not set - always valid  */
    if (admin_value == false)
        fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_INVALID_FLAG;

    /* handle actions */
    fpa_flow_entry.data.acl_policy.clearActions = true;
    if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_PACKET_ACTION))
    	fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_CLEAR_ACTION_FLAG;
    else if (fpa_flow_entry.entryType != FPA_FLOW_TABLE_TYPE_EPCL_E){
    	fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_GOTO_FLAG;
    	fpa_flow_entry.data.acl_policy.gotoTableNo = fpa_table_type + 1;
    }

    if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_TC)){
        fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_QUEUE_FLAG;
        fpa_flow_entry.data.acl_policy.queueId = entry_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8;
    }

    if (mrvl_acl_is_bit_set_MAC(entry_actions_bitmap, SAI_ACL_ACTION_TYPE_SET_DSCP)){
        fpa_flow_entry.data.acl_policy.instructionFlags |= FPA_FLOW_TABLE_PCL_ACTION_DSCP_FLAG;
        fpa_flow_entry.data.acl_policy.dscp = entry_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8;
    }

    /* add FPA flow entry */
    fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, fpa_table_type, &fpa_flow_entry);
    if (fpa_status != FPA_OK) {
    	mrvl_sai_acl_entry_db[acl_entry_index].is_used = false;
    	MRVL_SAI_LOG_ERR("Failed to add entry cookie %llx, fpa_table_type - %d , status = %d\n", fpa_flow_entry.cookie, fpa_table_type, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_entry_db[acl_entry_index].is_used              = true;
    mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie           = fpa_flow_entry.cookie;
    mrvl_sai_acl_entry_db[acl_entry_index].admin_state          = admin_value;
    mrvl_sai_acl_entry_db[acl_entry_index].priority             = priority->u32;
    mrvl_sai_acl_entry_db[acl_entry_index].table_index          = acl_table_index;
    mrvl_sai_acl_entry_db[acl_entry_index].attr_port            = attr_port;
    mrvl_sai_acl_entry_db[acl_entry_index].attr_vlan            = attr_vlan;
    mrvl_sai_acl_entry_db[acl_entry_index].entry_fields_bitmap  = entry_fields_bitmap;
    mrvl_sai_acl_entry_db[acl_entry_index].entry_actions_bitmap = entry_actions_bitmap;
    memcpy(mrvl_sai_acl_entry_db[acl_entry_index].entry_action_data, entry_action_data, sizeof(entry_action_data));

    mrvl_sai_acl_entry_add_to_table(acl_entry_index, acl_table_index);

    /* Create ACL entry object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_ENTRY, acl_entry_index, acl_entry_id))) {
    	mrvl_sai_acl_entry_remove_from_table(acl_entry_index, acl_table_index);
    	MRVL_SAI_LOG_ERR("Can't create object id for SAI_OBJECT_TYPE_ACL_ENTRY acl_entry_index - %d\n", acl_entry_index);
    	MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_ENTRY, *acl_entry_id, acl_str);
    MRVL_SAI_LOG_NTC("Created ACL entry %s\n", acl_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
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
sai_status_t mrvl_remove_acl_entry(_In_ sai_object_id_t acl_entry_id)
{
	char    acl_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_entry_index = 0, acl_table_index = 0;
    FPA_STATUS    fpa_status = FPA_OK;


    MRVL_SAI_LOG_ENTER();
    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_ENTRY, acl_entry_id, acl_str);
    MRVL_SAI_LOG_NTC("Delete  ACL entry %s\n", acl_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(acl_entry_id, SAI_OBJECT_TYPE_ACL_ENTRY, &acl_entry_index))) {
        return status;
    }
    assert(mrvl_sai_acl_entry_db[acl_entry_index].is_used != false);

    acl_table_index = mrvl_sai_acl_entry_db[acl_entry_index].table_index;

    /* delete from FPA */
    fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS,
    							mrvl_sai_acl_group_db[mrvl_sai_acl_table_db[acl_table_index].acl_group_index].fpa_table_type,
    								mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to delete entry from FPA: cookie - %llx, status = %d\n", mrvl_sai_acl_entry_db[acl_entry_index].fpa_cookie, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_entry_remove_from_table(acl_entry_index, acl_table_index);

    MRVL_SAI_LOG_EXIT();

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Create an ACL counter
 *
 * @param[out] acl_counter_id The ACL counter id
 * @param[out] switch_id The switch Object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_create_acl_counter(_Out_ sai_object_id_t* acl_counter_id,
                                     _In_ sai_object_id_t switch_id,
                                     _In_ uint32_t attr_count,
                                     _In_ const sai_attribute_t *attr_list)
{
    char    acl_str[MAX_KEY_STR_LEN];
    char    list_str[MAX_LIST_VALUE_STR_LEN];
    const sai_attribute_value_t *table_id, *packet_count_enable, *byte_count_enable;
    uint32_t temp_index;
    uint32_t  acl_table_index = 0;
    sai_status_t status = SAI_STATUS_SUCCESS;


    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (NULL == acl_counter_id) {
        MRVL_SAI_LOG_ERR("NULL entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_counter_attribs, acl_counter_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_counter_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);

    /* check mandatory field SAI_ACL_COUNTER_ATTR_TABLE_ID  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_COUNTER_ATTR_TABLE_ID, &table_id, &temp_index));
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(table_id->oid, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        return status;
    }

    if (false == mrvl_sai_acl_table_db[acl_table_index].is_used){
    	MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_COUNTER_ATTR_TABLE_ID acl_table_index - %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* check field SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT, &packet_count_enable, &temp_index))){
    	mrvl_sai_acl_table_db[acl_table_index].packet_count_enable = packet_count_enable->booldata;
    }

    /* check field SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT  */
    if (SAI_STATUS_SUCCESS ==
    		(status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT, &byte_count_enable, &temp_index))){
    	mrvl_sai_acl_table_db[acl_table_index].byte_count_enable = byte_count_enable->booldata;
    }

    /* Clear counters for all entries in table */
    mrvl_acl_clear_counters(acl_table_index);

    /* Create ACL counter object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_COUNTER, acl_table_index, acl_counter_id))) {
    	MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_ENTRY acl_entry_index - %d\n", acl_table_index);
    	MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_COUNTER, *acl_counter_id, acl_str);
    MRVL_SAI_LOG_NTC("Created ACL counter %s\n", acl_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
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
sai_status_t mrvl_remove_acl_counter(_In_ sai_object_id_t acl_counter_id)
{
	char    acl_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_table_index = 0;

    MRVL_SAI_LOG_ENTER();
    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_COUNTER, acl_counter_id, acl_str);
    MRVL_SAI_LOG_NTC("Delete ACL counter %s\n", acl_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(acl_counter_id, SAI_OBJECT_TYPE_ACL_COUNTER, &acl_table_index))) {
        return status;
    }

    assert(mrvl_sai_acl_table_db[acl_table_index].is_used == true);

    mrvl_sai_acl_table_db[acl_table_index].byte_count_enable = false;
    mrvl_sai_acl_table_db[acl_table_index].packet_count_enable = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

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
    const sai_object_key_t key = { .key.object_id = acl_counter_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_COUNTER, acl_counter_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for ACL Counter %s\n", key_str);

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
    const sai_object_key_t key = { .key.object_id = acl_counter_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_COUNTER, acl_counter_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for ACL counter %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, acl_counter_attribs, acl_counter_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_counter_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}


/**
 * @brief Create an ACL Range
 *
 * @param[out] acl_range_id The ACL range id
 * @param[in] switch_id The Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_acl_range(
        _Out_ sai_object_id_t *acl_range_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    char acl_str[MAX_LIST_VALUE_STR_LEN];
    uint32_t        range_type, acl_range_index = 0;
    sai_u32_range_t range_limit;
    sai_status_t status = SAI_STATUS_SUCCESS;
    const sai_attribute_value_t *att_val;
    uint32_t tmp_index;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (NULL == acl_range_id) {
        MRVL_SAI_LOG_ERR("NULL acl_range_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_range_attribs, acl_range_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_range_attribs, MAX_LIST_VALUE_STR_LEN, acl_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", acl_str);

    /* check mandatory field SAI_ACL_RANGE_ATTR_TYPE  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_RANGE_ATTR_TYPE, &att_val, &tmp_index));
    if (att_val->u32 > SAI_ACL_RANGE_TYPE_L4_DST_PORT_RANGE) {
        MRVL_SAI_LOG_ERR("Wrong value for SAI_ACL_RANGE_ATTR_TYPE - %d\n", att_val->u32);
        MRVL_SAI_API_RETURN(status);
    }
    else {
        range_type = att_val->u32;
    }

    /* check mandatory field SAI_ACL_RANGE_ATTR_LIMIT  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_RANGE_ATTR_LIMIT, &att_val, &tmp_index));
    if (att_val->u32range.min > att_val->u32range.max) {
        MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_RANGE_ATTR_LIMIT - [%d - %d]\n", att_val->u32range.min, att_val->u32range.max);
        MRVL_SAI_API_RETURN(status);
    }
    else {
        memcpy(&range_limit, &att_val->u32range, sizeof(att_val->u32range));
    }

    /* get free index */
    if (SAI_STATUS_SUCCESS !=
        		(status = mrvl_acl_find_free_index_in_range_db(&acl_range_index))){
    	MRVL_SAI_LOG_ERR("No free index for ACL ENTRY db\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* create ACL range object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_RANGE, acl_range_index, acl_range_id))) {
        MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_RANGE acl_range_index - %d\n", acl_range_index);
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_range_db[acl_range_index].entry_index = SAI_ACL_INVALID_INDEX;
    memcpy(&mrvl_sai_acl_range_db[acl_range_index].range_limit, &range_limit, sizeof(sai_u32_range_t));
    mrvl_sai_acl_range_db[acl_range_index].range_type = range_type;

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_RANGE, *acl_range_id, acl_str);
    MRVL_SAI_LOG_NTC("Created ACL range %s\n", acl_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove an ACL Range
 *
 * @param[in] acl_range_id The ACL range id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_acl_range(
        _In_ sai_object_id_t acl_range_id)
{
    char acl_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_range_index = 0;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_RANGE, acl_range_id, acl_str);
    MRVL_SAI_LOG_NTC("Delete ACL counter %s\n", acl_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(acl_range_id, SAI_OBJECT_TYPE_ACL_RANGE, &acl_range_index))) {
        return status;
    }

    assert(mrvl_sai_acl_range_db[acl_range_index].is_used == true);

    mrvl_sai_acl_range_db[acl_range_index].is_used = false;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
/**
 * @brief Set ACL range attribute
 *
 * @param[in] acl_range_id The ACL range id
 * @param[in] attr Attribute
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_set_acl_range_attribute(
        _In_ sai_object_id_t acl_range_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = acl_range_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    snprintf(key_str, MAX_KEY_STR_LEN, "acl_range %llx", (long long int)acl_range_id);
    return mrvl_sai_utl_set_attribute(&key, key_str, acl_range_attribs, acl_range_vendor_attribs, attr);
}

/**
 * @brief Get ACL range attribute
 *
 * @param[in] acl_range_id ACL range id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_get_acl_range_attribute(
        _In_ sai_object_id_t acl_range_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = acl_range_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    snprintf(key_str, MAX_KEY_STR_LEN, "acl_range_id %llx", (long long int)acl_range_id);
    status = mrvl_sai_utl_get_attributes(&key, key_str, acl_range_attribs, acl_range_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_range_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Create an ACL Table Group
 *
 * @param[out] acl_table_group_id The ACL group id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_acl_table_group(
        _Out_ sai_object_id_t *acl_table_group_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    char    acl_str[MAX_KEY_STR_LEN];
    char    list_str[MAX_LIST_VALUE_STR_LEN];
    uint32_t acl_group_index = 0;
    const sai_attribute_value_t *att_val;
    uint32_t tmp_index;
    sai_status_t status = SAI_STATUS_SUCCESS;
    sai_acl_table_group_type_t group_type;
    sai_acl_stage_t stage;
    sai_status_t group_bind_point_bitmap = 0;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_NULL_OBJECT_ID == acl_table_group_id) {
        MRVL_SAI_LOG_ERR("NULL acl_table_group_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_table_group_attribs, acl_table_group_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_group_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);

    /* check mandatory attr SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE, &att_val, &tmp_index));
    if (SAI_ACL_STAGE_EGRESS < att_val->u32) {
        MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE - %d\n", att_val->u32);
        MRVL_SAI_API_RETURN(status);
    }
    else {
        stage = att_val->u32;
    }

    /* check attr SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST  */
    if (SAI_STATUS_SUCCESS ==
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST, &att_val, &tmp_index))){
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_bind_point_type_list_validate(&att_val->s32list,
                                                              tmp_index,
                                                              &group_bind_point_bitmap))){
            MRVL_SAI_LOG_ERR("Invalid values for SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST\n");
            MRVL_SAI_API_RETURN(status);
        }
    }
    else {
        group_bind_point_bitmap = default_bind_point_type_bitmap;
    }

    /* check attr SAI_ACL_TABLE_GROUP_ATTR_TYPE  */
    if (SAI_STATUS_SUCCESS ==
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_GROUP_ATTR_TYPE, &att_val, &tmp_index))){
		if (SAI_ACL_TABLE_GROUP_TYPE_PARALLEL < att_val->u32) {
			MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_GROUP_ATTR_TYPE - %d\n", att_val->u32);
			MRVL_SAI_API_RETURN(status);
        }
        else {
            group_type = att_val->u32;
        }
    }
    else
    	group_type = SAI_ACL_TABLE_GROUP_TYPE_SEQUENTIAL; /* default is SAI_ACL_TABLE_GROUP_TYPE_SEQUENTIAL*/


    /* get free index */
    if (SAI_STATUS_SUCCESS !=
                (status = mrvl_acl_find_free_index_in_group_table_db(&acl_group_index))){
        MRVL_SAI_LOG_ERR("No free index for ACL TABLE GROUP\n");
        MRVL_SAI_API_RETURN(status);
    }

    /* create ACL Group object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, acl_group_index, acl_table_group_id))) {
        MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE_GROUP acl_group_index - %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_acl_group_db[acl_group_index].group_id = *acl_table_group_id;
	mrvl_sai_acl_group_db[acl_group_index].group_size = SAI_ACL_ENTRIES_PER_GROUP_MAX_NUM;
	mrvl_sai_acl_group_db[acl_group_index].is_used = true;
	mrvl_sai_acl_group_db[acl_group_index].entries_count = 0;
    mrvl_sai_acl_group_db[acl_group_index].bind_point_types_bitmap = group_bind_point_bitmap;
    mrvl_sai_acl_group_db[acl_group_index].stage = stage;
    mrvl_sai_acl_group_db[acl_group_index].type = group_type;

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, *acl_table_group_id, acl_str);
    MRVL_SAI_LOG_NTC("Created ACL table group %s\n", acl_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}


/**
 * @brief Delete an ACL Group
 *
 * @param[in] acl_table_group_id The ACL group id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_acl_table_group(
        _In_ sai_object_id_t acl_table_group_id)
{
    char acl_data_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_group_index = 0;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == acl_table_group_id) {
        MRVL_SAI_LOG_ERR("Invalid object param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, acl_table_group_id, acl_data_str);
    MRVL_SAI_LOG_NTC("Delete  %s\n", acl_data_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(acl_table_group_id, SAI_OBJECT_TYPE_ACL_TABLE_GROUP, &acl_group_index))) {
        return status;
    }

    if (acl_group_index >= SAI_ACL_GROUP_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_group_index %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    assert(mrvl_sai_acl_group_db[acl_group_index].is_used != false);

    assert(mrvl_sai_acl_group_db[acl_group_index].tables_count == 0);

    mrvl_sai_acl_group_remove(acl_group_index);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

/**
 * @brief Set ACL table group attribute
 *
 * @param[in] acl_table_group_id The ACL table group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_set_acl_table_group_attribute(
        _In_ sai_object_id_t acl_table_group_id,
        _In_ const sai_attribute_t *attr)
{

    const sai_object_key_t key = { .key.object_id = acl_table_group_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == acl_table_group_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_group_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, acl_table_group_id, key_str);
    MRVL_SAI_LOG_NTC("Set attributes for ACL entry %s\n", key_str);

    status = mrvl_sai_utl_set_attribute(&key, key_str, acl_table_group_attribs, acl_table_group_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get ACL table group attribute
 *
 * @param[in] acl_table_group_id ACL table group id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_get_acl_table_group_attribute(
        _In_ sai_object_id_t acl_table_group_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = acl_table_group_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == acl_table_group_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_group_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, acl_table_group_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for ACL group %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, acl_table_group_attribs, acl_table_group_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_group_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);

}

/**
 * @brief Create an ACL Table Group Member
 *
 * @param[out] acl_table_group_member_id The ACL table group member id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_create_acl_table_group_member(
        _Out_ sai_object_id_t *acl_table_group_member_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    char    acl_str[MAX_KEY_STR_LEN];
    char    list_str[MAX_LIST_VALUE_STR_LEN];
    uint32_t acl_group_index = 0, acl_table_index = 0;
    const sai_attribute_value_t *att_val;
    uint32_t tmp_index, priority;
    uint8_t data_ext[RESERVED_DATA_LENGTH_CNS];
    sai_status_t status = SAI_STATUS_SUCCESS;
    sai_acl_bind_point_type_t type;
    sai_object_id_t table_id, group_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (NULL == acl_table_group_member_id) {
        MRVL_SAI_LOG_ERR("NULL acl_table_group_member_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, acl_table_group_member_attribs, acl_table_group_member_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_group_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", list_str);

    /* check mandatory field SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID, &att_val, &tmp_index));
    if (SAI_NULL_OBJECT_ID == att_val->oid) {
        MRVL_SAI_LOG_ERR("NULL value for SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID - %d\n", att_val->oid);
        MRVL_SAI_API_RETURN(status);
    }
    else {
        group_id = att_val->oid;
    }

    /* check mandatory field SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID  */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID, &att_val, &tmp_index));
    if (SAI_NULL_OBJECT_ID == att_val->oid) {
        MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID - %d\n", att_val->u32);
        MRVL_SAI_API_RETURN(status);
    }
    else {
        table_id = att_val->oid;
    }

    /* check mandatory field SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY */
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY, &att_val, &tmp_index));
    if ((SAI_ACL_MAX_PRIORITY < att_val->u32) || (SAI_ACL_MIN_PRIORITY > att_val->u32)) {
        MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY - %d\n", att_val->u32);
        MRVL_SAI_API_RETURN(status);
    }
    else {
        priority = att_val->u32;
    }


    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(group_id, SAI_OBJECT_TYPE_ACL_TABLE_GROUP, &acl_group_index))) {
        MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID - %d\n", att_val->oid);
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(table_id, SAI_OBJECT_TYPE_ACL_TABLE, &acl_table_index))) {
        MRVL_SAI_LOG_ERR("Invalid value for SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID - %d\n", att_val->oid);
        MRVL_SAI_API_RETURN(status);
    }

    assert(mrvl_sai_acl_group_db[acl_group_index].is_used != false);
    assert(mrvl_sai_acl_table_db[acl_table_index].is_used != false);

    /* are table bind point list is a subset of group bind point list? */
    for (type = 0; type < SAI_ACL_MAX_BIND_POINT_TYPES; type++) {
        if ((mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_table_db[acl_table_index].bind_point_types_bitmap, type)) &&
            (!mrvl_acl_is_bit_set_MAC(mrvl_sai_acl_group_db[acl_group_index].bind_point_types_bitmap, type))) {
            MRVL_SAI_LOG_ERR("No match for bind point type (%d)", type);
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    /* verify stage matching */
    assert(mrvl_sai_acl_table_db[acl_table_index].stage == mrvl_sai_acl_group_db[acl_group_index].stage);

    /* verify size */
    if ((mrvl_sai_acl_table_db[acl_table_index].is_dynamic_size == false) &&
    		(mrvl_sai_acl_group_db[acl_group_index].entries_count + 
                mrvl_sai_acl_table_db[acl_table_index].table_size > 
                    mrvl_sai_acl_group_db[acl_group_index].group_size)) {
    	MRVL_SAI_LOG_ERR("Can't reserve table size - %d, current num of entries per group - %d, max group size %d\n",
    			mrvl_sai_acl_table_db[acl_table_index].table_size, 
                mrvl_sai_acl_group_db[acl_group_index].entries_count,
                mrvl_sai_acl_group_db[acl_group_index].group_size);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* create ACL Group member object */
    data_ext[0] = (uint8_t)(acl_table_index & 0xFF);
    data_ext[1] = (uint8_t)((acl_table_index >> 8) & 0xFF);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, acl_group_index, data_ext, acl_table_group_member_id))) {
        MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER acl_group_index - %d, acl_table_index - %d\n", acl_group_index, acl_table_index);
        MRVL_SAI_API_RETURN(status);
    }

    if (mrvl_sai_acl_group_db[acl_table_index].tables_count != 0){
        if (mrvl_sai_acl_group_db[acl_table_index].fpa_table_type != mrvl_sai_acl_table_db[acl_table_index].fpa_table_type){
            MRVL_SAI_LOG_ERR("no match for table and group group fpa_table_type - %d, table fpa_table_type - %d\n", 
                             mrvl_sai_acl_group_db[acl_table_index].fpa_table_type, 
                             mrvl_sai_acl_table_db[acl_table_index].fpa_table_type);
            MRVL_SAI_API_RETURN(status);
        }
    }
    else {
        mrvl_sai_acl_group_db[acl_table_index].fpa_table_type = mrvl_sai_acl_table_db[acl_table_index].fpa_table_type;
    }

    mrvl_sai_acl_table_db[acl_table_index].acl_group_index = acl_group_index;
    mrvl_sai_acl_table_db[acl_table_index].priority = priority;
    mrvl_sai_acl_group_db[acl_group_index].tables_count++;

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, *acl_table_group_member_id, acl_str);
    MRVL_SAI_LOG_NTC("Created ACL table group member %s\n", acl_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

/**
 * @brief Delete an ACL Group Member
 *
 * @param[in] acl_table_group_member_id The ACL table group member id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_remove_acl_table_group_member(
        _In_ sai_object_id_t acl_table_group_member_id)
{
    char acl_data_str[MAX_KEY_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t      acl_group_index = 0, acl_table_index = 0;
    uint8_t       ext_data[RESERVED_DATA_LENGTH_CNS];
    sai_object_id_t acl_group_id;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == acl_table_group_member_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_group_member_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, acl_table_group_member_id, acl_data_str);
    MRVL_SAI_LOG_NTC("Delete  %s\n", acl_data_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(acl_table_group_member_id, SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, &acl_group_index, ext_data))) {
        return status;
    }
    acl_table_index = (uint32_t)(ext_data[1] << 8 | ext_data[0]);
    if (acl_group_index >= SAI_ACL_GROUP_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_group_index %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (acl_table_index >= SAI_ACL_TABLES_MAX_NUM){
        MRVL_SAI_LOG_ERR("Invalid acl_table_index %d\n", acl_table_index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    /* create ACL Group object */
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, acl_group_index, &acl_group_id))) {
        MRVL_SAI_LOG_ERR("Can't create opbject id for SAI_OBJECT_TYPE_ACL_TABLE_GROUP acl_group_index - %d\n", acl_group_index);
        MRVL_SAI_API_RETURN(status);
    }
    assert(mrvl_sai_acl_group_db[acl_group_index].group_id == acl_group_id);
    assert(mrvl_sai_acl_table_db[acl_table_index].acl_group_index == acl_group_index);
    assert(mrvl_sai_acl_group_db[acl_group_index].is_used != false);
    assert(mrvl_sai_acl_table_db[acl_table_index].is_used != false);
    assert(mrvl_sai_acl_group_db[acl_group_index].tables_count != 0);
    assert(mrvl_sai_acl_table_db[acl_table_index].bound_interface.type == SAI_ACL_MAX_BIND_POINT_TYPES);
    assert(mrvl_sai_acl_table_db[acl_table_index].entries_count == 0);

    mrvl_sai_acl_table_db[acl_table_index].acl_group_index = 0;
    mrvl_sai_acl_group_db[acl_group_index].tables_count--;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

}

/**
 * @brief Set ACL table group member attribute
 *
 * @param[in] acl_table_group_member_id The ACL table group member id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_set_acl_table_group_member_attribute(
        _In_ sai_object_id_t acl_table_group_member_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = acl_table_group_member_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == acl_table_group_member_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_group_member_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, acl_table_group_member_id, key_str);
    MRVL_SAI_LOG_NTC("Set attributes for ACL group member %s\n", key_str);

    status = mrvl_sai_utl_set_attribute(&key, key_str, acl_table_group_member_attribs, acl_table_group_member_vendor_attribs, attr);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}


/**
 * @brief Get ACL table group member attribute
 *
 * @param[in] acl_table_group_id ACL table group member id
 * @param[in] attr_count Number of attributes
 * @param[out] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_get_acl_table_group_member_attribute(
        _In_ sai_object_id_t acl_table_group_member_id,
        _In_ uint32_t attr_count,
        _Out_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = acl_table_group_member_id };
    char                   key_str[MAX_LIST_VALUE_STR_LEN];
    sai_status_t status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == acl_table_group_member_id) {
        MRVL_SAI_LOG_ERR("Invalid acl_table_group_member_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_acl_object_id_to_str(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, acl_table_group_member_id, key_str);
    MRVL_SAI_LOG_NTC("Get attributes for ACL group member %s\n", key_str);

    status = mrvl_sai_utl_get_attributes(&key, key_str, acl_table_group_member_attribs, acl_table_group_member_vendor_attribs, attr_count, attr_list);

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, acl_table_group_member_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_DBG("Attribs %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}
/**
 * @brief Port methods table retrieved with sai_api_query()
 */

sai_status_t mrvl_sai_acl_init(void)
{
    sai_uint32_t   i;

    memset(mrvl_sai_acl_group_db, 0, sizeof(mrvl_acl_group_db_t)*SAI_ACL_GROUP_MAX_NUM);

    memset(mrvl_sai_acl_table_db, 0, sizeof(mrvl_sai_acl_table_db));
    for(i=0;i<SAI_ACL_TABLES_MAX_NUM; i++) {
      mrvl_sai_acl_table_db[i].head_entry_index =  SAI_ACL_INVALID_INDEX;
    }

    memset(mrvl_sai_acl_entry_db, 0, sizeof(mrvl_sai_acl_entry_db));
    for(i=0;i<SAI_ACL_ENTRIES_MAX_NUM; i++) {
      mrvl_sai_acl_entry_db[i].next_acl_entry_index =  SAI_ACL_INVALID_INDEX;
      mrvl_sai_acl_entry_db[i].prev_acl_entry_index =  SAI_ACL_INVALID_INDEX;
    }

    memset(mrvl_sai_acl_range_db, 0, sizeof(mrvl_sai_acl_range_db));

    memset(mrvl_sai_acl_bound_port_db, 0, sizeof(mrvl_sai_acl_bound_port_db));
    for(i=0;i<SAI_ACL_PORTLIST_MAX_NUM; i++) {
      mrvl_sai_acl_bound_port_db[i].head_table_index =  SAI_ACL_INVALID_INDEX;
    }

    memset(mrvl_sai_acl_bound_lag_db, 0, sizeof(mrvl_sai_acl_bound_lag_db));
    for(i=0;i<SAI_LAG_MAX_GROUPS_CNS; i++) {
      mrvl_sai_acl_bound_lag_db[i].lag_bound_db.head_table_index =  SAI_ACL_INVALID_INDEX;
    }

    memset(&mrvl_sai_acl_bound_switch_db, 0, sizeof(mrvl_sai_acl_bound_switch_db));
    mrvl_sai_acl_bound_switch_db.head_table_index = SAI_ACL_INVALID_INDEX;

    memset(mrvl_sai_acl_bound_vlan_db, 0, sizeof(mrvl_sai_acl_bound_vlan_db));
    for(i=0;i<SAI_MAX_NUM_OF_VLANS; i++) {
      mrvl_sai_acl_bound_vlan_db[i].head_table_index =  SAI_ACL_INVALID_INDEX;
    }

    return SAI_STATUS_SUCCESS;
}

const sai_acl_api_t acl_api = {
    mrvl_create_acl_table,
    mrvl_remove_acl_table,
    mrvl_set_acl_table_attribute,
    mrvl_get_acl_table_attribute,
    mrvl_create_acl_entry,
    mrvl_remove_acl_entry,
    mrvl_set_acl_entry_attribute,
    mrvl_get_acl_entry_attribute,
    mrvl_create_acl_counter,
    mrvl_remove_acl_counter,
    mrvl_set_acl_counter_attribute,
    mrvl_get_acl_counter_attribute,
    mrvl_create_acl_range,
    mrvl_remove_acl_range,
    mrvl_set_acl_range_attribute,
    mrvl_get_acl_range_attribute,
    mrvl_create_acl_table_group,
    mrvl_remove_acl_table_group,
    mrvl_set_acl_table_group_attribute,
    mrvl_get_acl_table_group_attribute,
    mrvl_create_acl_table_group_member,
    mrvl_remove_acl_table_group_member,
    mrvl_set_acl_table_group_member_attribute,
    mrvl_get_acl_table_group_member_attribute
};


