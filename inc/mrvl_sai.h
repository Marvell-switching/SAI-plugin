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

#if !defined (__MRVL_SAI_H_)
#define __MRVL_SAI_H_

#include	"fpaLibApis.h"
#include    "sai.h"
#include "utils/mrvl_sai_dlist.h"


extern service_method_table_t g_services;

extern const sai_switch_api_t       switch_api;
extern const sai_port_api_t         port_api;
extern const sai_fdb_api_t          fdb_api;
extern const sai_vlan_api_t         vlan_api;
extern const sai_hostif_api_t       host_interface_api;
extern const sai_router_interface_api_t router_interface_api;
extern const sai_acl_api_t          acl_api;
extern const sai_neighbor_api_t     neighbor_api;
extern const sai_next_hop_api_t     nexthop_api;
extern const sai_next_hop_group_api_t next_hop_group_api;
extern const sai_route_api_t        route_api;
extern const sai_virtual_router_api_t virtual_router_api;
extern const sai_lag_api_t		    lag_api;
extern const sai_scheduler_group_api_t	scheduler_group_api;
extern const sai_hash_api_t		hash_api;
extern const sai_stp_api_t		stp_api;
extern const sai_l2mc_api_t		l2mc_api;
extern const sai_l2mc_group_api_t		l2mc_group_api;
extern const sai_bridge_api_t		bridge_api;
/* stubs */
extern const sai_buffer_api_t		buffer_api;
extern const sai_mirror_api_t		mirror_api;
extern const sai_policer_api_t		policer_api;
extern const sai_qos_map_api_t		qos_map_api;
extern const sai_queue_api_t		queue_api;
extern const sai_samplepacket_api_t	samplepacket_api;
extern const sai_scheduler_api_t	scheduler_api;
extern const sai_tunnel_api_t		tunnel_api;
extern const sai_udf_api_t		udf_api;
extern const sai_wred_api_t		wred_api;
extern const sai_ipmc_api_t		ipmc_api;
extern const sai_rpf_group_api_t		rpf_api;
extern const sai_ipmc_group_api_t		ipmc_group_api;
extern const sai_mcast_fdb_api_t		mcast_fdb_api;
extern sai_status_t mrvl_sai_utl_fill_objlist(sai_object_id_t *data, uint32_t count, sai_object_list_t *list);
extern sai_status_t mrvl_sai_utl_fill_u32list(uint32_t *data, uint32_t count, sai_u32_list_t *list);
extern sai_status_t mrvl_sai_utl_fill_s32list(int32_t *data, uint32_t count, sai_s32_list_t *list);
extern sai_status_t mrvl_sai_utl_fill_vlanlist(sai_vlan_id_t *data, uint32_t count, sai_vlan_list_t *list);

#ifdef _LP64
typedef uint64_t PTR_TO_INT;
#define ALIGNMENT_SIZE      8
#else
typedef uint32_t PTR_TO_INT;
#define ALIGNMENT_SIZE      4
#endif

#define SAI_DEFAULT_ETH_SWID_CNS        0
#define SAI_DEFAULT_VRID_CNS            0
#define SAI_DEFAULT_RIF_MTU_CNS         1514
#define SAI_DEFAULT_RIF_NBR_MISS_ACTION_CNS SAI_PACKET_ACTION_TRAP
#define SAI_RIF_MTU_PROFILES_CNS        8
#define SAI_FIRST_PORT_CNS              (0x10000 | (1 << 8))
#define SAI_SWITCH_MAX_VR_CNS           128
#define SAI_CPU_PORT_CNS                63
#define SAI_MAX_MTU_CNS                 10240
#define SAI_QUEUES_PER_PORT_CNS         8
#define SAI_TOTAL_BUFFER_SIZE_KB_CNS    16
#define SAI_DEFAULT_FDB_AGING_TIME_CNS  0
#define SAI_SWITCH_DEFAULT_MAC_MODE_CNS 0 /*Unique MAC address per port(1) or same for all ports(0)*/
#define SAI_MAX_NUM_OF_PORTS	54
#define SAI_MAX_NUM_OF_LANES    4
#define SAI_MAX_NUM_OF_VLANS	4094
#define MAX_QUEUES 8

#define SAI_OUTPUT_CONTROLLER               FPA_OUTPUT_CONTROLLER
#define SAI_ROUTER_INTERFACE_TABLE_SIZE_CNS 128
#define SAI_NEIGHBOR_TABLE_SIZE_CNS         (8*1024)
#define SAI_NEXTHOP_TABLE_SIZE_CNS          1024
#define SAI_ROUTE_TABLE_SIZE_CNS            (8*1024)
#define SAI_ECMP_MAX_MEMBERS_IN_GROUP_CNS   64
#define SAI_ECMP_MAX_GROUPS_CNS             1024
#define SAI_ECMP_DEFAULT_HASH_ALGORITHM_CNS SAI_HASH_ALGORITHM_CRC
#define SAI_LAG_MAX_MEMBERS_IN_GROUP_CNS    64
#define SAI_LAG_MAX_GROUPS_CNS              (1024+1) /* LAG '0' is not valid in CPSS. LAG group range 1..SAI_LAG_MAX_GROUPS_CNS */
#define SAI_ACL_MIN_PRIORITY_CNS            0
#define SAI_ACL_MAX_PRIORITY_CNS            32*1024
#define SAI_QOS_MAP_TYPES_MAX               10
#define SAI_QOS_NUM_QUEUES                  8
#define SAI_QOS_UP_MIN                      0
#define SAI_QOS_UP_MAX                      7
#define SAI_QOS_DSCP_MIN                    0
#define SAI_QOS_DSCP_MAX                    63


extern uint32_t SAI_SYS_PORT_MAPPING[SAI_MAX_NUM_OF_PORTS];
/*
*  SAI operation type
*  Values must start with 0 base and be without gaps
*/
typedef enum sai_operation_t
{
    SAI_OPERATION_CREATE,
    SAI_OPERATION_REMOVE,
    SAI_OPERATION_SET,
    SAI_OPERATION_GET,
    SAI_OPERATION_MAX
} sai_operation_t;

/*
*  Attribute value types
*/
typedef enum _sai_attribute_value_type_t
{
    SAI_ATTR_VAL_TYPE_UNDETERMINED,
    SAI_ATTR_VAL_TYPE_BOOL,
    SAI_ATTR_VAL_TYPE_CHARDATA,
    SAI_ATTR_VAL_TYPE_U8, 
    SAI_ATTR_VAL_TYPE_S8,
    SAI_ATTR_VAL_TYPE_U16,
    SAI_ATTR_VAL_TYPE_S16,
    SAI_ATTR_VAL_TYPE_U32,
    SAI_ATTR_VAL_TYPE_S32,
    SAI_ATTR_VAL_TYPE_U64,
    SAI_ATTR_VAL_TYPE_S64,
    SAI_ATTR_VAL_TYPE_MAC,
    SAI_ATTR_VAL_TYPE_IPV4,
    SAI_ATTR_VAL_TYPE_IPV6,
    SAI_ATTR_VAL_TYPE_IPADDR,
    SAI_ATTR_VAL_TYPE_OID,
    SAI_ATTR_VAL_TYPE_U8LIST,/* todo add support */
    SAI_ATTR_VAL_TYPE_S8LIST,/* todo add support */
    SAI_ATTR_VAL_TYPE_OBJLIST,
    SAI_ATTR_VAL_TYPE_U32LIST,
    SAI_ATTR_VAL_TYPE_S32LIST,
    SAI_ATTR_VAL_TYPE_VLANLIST,
    SAI_ATTR_VAL_TYPE_ACLFIELD,
    SAI_ATTR_VAL_TYPE_ACLACTION,
    SAI_ATTR_VAL_TYPE_PTR,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_MAC,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV4,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV6,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_S8,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_S16,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U32,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_S32,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U64,
    SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_S64,
    SAI_ATTR_VAL_TYPE_ACLACTION_U8,
    SAI_ATTR_VAL_TYPE_ACLACTION_U32,
    SAI_ATTR_VAL_TYPE_U32RANGE,
} sai_attribute_value_type_t;


typedef struct _sai_attribute_entry_t {
    sai_attr_id_t              id;
    bool                       mandatory_on_create;
    bool                       valid_for_create;
    bool                       valid_for_set;
    bool                       valid_for_get;
    const char                *attrib_name;
    sai_attribute_value_type_t type;
} sai_attribute_entry_t;

typedef struct _mrvl_object_id_t {
    sai_uint8_t  object_type;
    sai_uint8_t  reserved[3];
    sai_uint32_t data;
} mrvl_object_id_t;

typedef sai_status_t(*sai_attribute_set_fn)(
    _In_ const sai_object_key_t *key,
    _In_ const sai_attribute_value_t *value,
    void *arg
    );

typedef union {
    int dummy;
} vendor_cache_t;


typedef sai_status_t(*sai_attribute_get_fn)(
    _In_ const sai_object_key_t *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t attr_index,
    _Inout_ vendor_cache_t *cache,
    void *arg
    );

typedef struct _sai_vendor_attribute_entry_t {
    sai_attr_id_t id;
    bool is_implemented[SAI_OPERATION_MAX];
    bool is_supported[SAI_OPERATION_MAX];
    sai_attribute_get_fn getter;
    void *getter_arg;
    sai_attribute_set_fn setter;
    void *setter_arg;
} sai_vendor_attribute_entry_t;

typedef struct _sai_port_info_t {
    uint32_t    port;
    uint32_t    tag;
}sai_port_info_t;

#define END_FUNCTIONALITY_ATTRIBS_ID 0xFFFFFFFF
#define MAX_ATTRIBS_NUMBUR 100

sai_status_t mrvl_sai_utl_is_valid_switch(_In_ const sai_object_id_t switch_id);

sai_status_t mrvl_sai_utl_check_attribs_metadata(_In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _In_ const sai_attribute_entry_t *functionality_attr,
    _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
    _In_ sai_operation_t oper);

sai_status_t mrvl_sai_utl_find_attrib_in_list(_In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _In_ sai_attr_id_t attrib_id,
    _Out_ const sai_attribute_value_t **attr_value,
    _Out_ uint32_t *index);

sai_status_t mrvl_sai_utl_set_attribute(_In_ const sai_object_key_t *key,
    _In_ const char *key_str,
    _In_ const sai_attribute_entry_t *functionality_attr,
    _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
    _In_ const sai_attribute_t *attr);

sai_status_t mrvl_sai_utl_get_attributes(_In_ const sai_object_key_t *key,
    _In_ const char *key_str,
    _In_ const sai_attribute_entry_t *functionality_attr,
    _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list);

#define MAX_KEY_STR_LEN 100
#define MAX_VALUE_STR_LEN 100
#define MAX_LIST_VALUE_STR_LEN 1000

#define MRVL_SAI_UTL_ADD    0
#define MRVL_SAI_UTL_DEL    1
#define MRVL_SAI_UTL_MODIFY 2

sai_status_t mrvl_sai_utl_value_to_str(_In_ sai_attribute_value_t value,
                            _In_ sai_attribute_value_type_t type,
                            _In_ uint32_t max_length,
                            _Out_ char *value_str);
sai_status_t mrvl_sai_utl_attr_list_to_str(_In_ uint32_t attr_count,
                            _In_ const sai_attribute_t *attr_list,
                            _In_ const sai_attribute_entry_t *functionality_attr,
                            _In_ uint32_t max_length,
                            _Out_ char *list_str);
sai_status_t mrvl_sai_utl_ipprefix_to_str(_In_ sai_ip_prefix_t value,
                            _In_ uint32_t max_length,
                            _Out_ char *value_str);
sai_status_t mrvl_sai_utl_ipaddr_to_str(_In_ sai_ip_address_t value,
                            _In_ uint32_t max_length,
                            _Out_ char *value_str,
                            _Out_ int *chars_written);

sai_status_t mrvl_sai_utl_is_object_type(sai_object_id_t object_id, sai_object_type_t type);
sai_status_t mrvl_sai_utl_object_to_type(sai_object_id_t object_id, sai_object_type_t type, uint32_t *data);
sai_status_t mrvl_sai_utl_object_to_ext_type(sai_object_id_t object_id, sai_object_type_t type, uint32_t *data, uint32_t *data_ext);
sai_status_t mrvl_sai_utl_create_ext_object(sai_object_type_t type, uint32_t data, uint32_t data_ext, sai_object_id_t *object_id);
sai_status_t mrvl_sai_utl_create_object(sai_object_type_t type, uint32_t data, sai_object_id_t *object_id);
sai_status_t mrvl_sai_utl_create_l2_int_group(_In_ uint32_t port,
                                     _In_ uint32_t vlan,
                                     _In_ sai_vlan_tagging_mode_t tagged,
                                     _In_ bool tag_overwrite,
                                      _Inout_ uint32_t *group);
sai_status_t mrvl_sai_utl_delete_l2_int_group(_In_ uint32_t port,
                                              _In_ uint32_t vlan);
sai_status_t mrvl_sai_utl_create_l2_int_group_wo_vlan(_In_ uint32_t port);
sai_status_t mrvl_sai_utl_l2_int_group_get_tagging_mode(_In_ uint32_t port, _In_ uint32_t vlan, _Out_ sai_vlan_tagging_mode_t *tag_mode);
sai_status_t mrvl_sai_utl_l2_int_group_set_tagging_mode(_In_ uint32_t port, _In_ uint32_t vlan, _In_ sai_vlan_tagging_mode_t tag_mode);

sai_status_t mrvl_sai_utl_create_l3_unicast_group(_In_ uint32_t     index,
                                                  _In_ sai_mac_t    dst_mac,
                                                  _In_ uint32_t     rif_idx,
                                                  _In_ sai_packet_action_t action,
                                                  _Inout_ uint32_t  *group);
sai_status_t mrvl_sai_utl_delete_l3_unicast_group(_In_ uint32_t index);

sai_status_t mrvl_sai_utl_modify_l3_unicast_group(_In_ uint32_t     index,
                                                  _In_ sai_mac_t    dst_mac);

sai_status_t mrvl_sai_utl_create_l3_ecmp_group(_In_ uint32_t     index,
                                               _In_ uint32_t     algorithm,
                                               _In_ const sai_object_list_t *ecmp_objlist,
                                               _Inout_ uint32_t  *group);
sai_status_t mrvl_sai_utl_delete_l3_ecmp_group(_In_ uint32_t index);
sai_status_t mrvl_sai_utl_get_l3_ecmp_group_bucket_list(_In_ uint32_t index,
                                                        _Inout_ sai_object_list_t *ecmp_objlist);
sai_status_t mrvl_sai_utl_update_l3_ecmp_group_bucket_list(_In_ uint32_t index,
                                                           _In_ sai_object_list_t *ecmp_objlist,
                                                           _In_ uint32_t operation,
                                                           _Out_ uint32_t *numChanged);
sai_status_t mrvl_sai_utl_create_l2_lag_group(_In_ uint32_t    index,
                                              _Out_ uint32_t  *group);
sai_status_t mrvl_sai_utl_delete_l2_lag_group(_In_ uint32_t index);
sai_status_t mrvl_sai_utl_update_l2_lag_group_bucket(_In_  uint32_t     index,
                                                     _In_  sai_object_id_t port_oid,
                                                     _In_  uint32_t     operation,
                                                     _Out_ uint32_t     *numChanged);
sai_status_t mrvl_sai_utl_get_l2_lag_group_bucket_list(_In_ uint32_t             lag_idx,
                                                       _Inout_ sai_object_list_t *port_objlist);

sai_status_t mrvl_sai_utl_fpa_to_sai_status(int32_t fpa_status);


/*
 * Routine Description:
 *    Waits on AU events and call SAI callback in succh event
 *
 * Arguments:
 *    void
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS - on success
 *
 */
extern unsigned mrvl_sai_fdb_wait_for_au_event(void *args);

typedef struct _sai_switch_notification_t {
    sai_switch_state_change_notification_fn     on_switch_state_change;
    sai_fdb_event_notification_fn               on_fdb_event;
    sai_port_state_change_notification_fn       on_port_state_change;
    sai_switch_shutdown_request_notification_fn on_switch_shutdown_request;
    sai_packet_event_notification_fn            on_packet_event;
} sai_switch_notification_t;

extern sai_switch_notification_t     mrvl_sai_notification_callbacks;
#define SAI_TYPE_CHECK_RANGE(type) (type < SAI_OBJECT_TYPE_MAX)

#define SAI_TYPE_STR(type) SAI_TYPE_CHECK_RANGE(type) ? mrvl_sai_type2str_arr[type] : "Unknown object type"

static __attribute__((__used__)) const char *mrvl_sai_type2str_arr[] = {
    /* SAI_OBJECT_TYPE_NULL = 0 */
    "NULL type",

    /*SAI_OBJECT_TYPE_PORT = 1 */
    "Port type",

    /*SAI_OBJECT_TYPE_LAG = 2 */
    "LAG type",

    /*SAI_OBJECT_TYPE_VIRTUAL_ROUTER = 3 */
    "Virtual router type",

    /* SAI_OBJECT_TYPE_NEXT_HOP = 4 */
    "Next hop type",

    /* SAI_OBJECT_TYPE_NEXT_HOP_GROUP = 5 */
    "Next hop group type",

    /* SAI_OBJECT_TYPE_ROUTER_INTERFACE = 6 */
    "Router interface type",

    /* SAI_OBJECT_TYPE_ACL_TABLE = 7 */
    "ACL table type",

    /* SAI_OBJECT_TYPE_ACL_ENTRY = 8 */
    "ACL entry type",

    /* SAI_OBJECT_TYPE_ACL_COUNTER = 9 */
    "ACL counter type",

    /* SAI_OBJECT_TYPE_ACL_RANGE = 10 */
    "ACL range type",

    /* SAI_OBJECT_TYPE_ACL_TABLE_GROUP = 11 */
    "ACL table group",

    /* SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER = 12 */
    "ACL table group member",

    /* SAI_OBJECT_TYPE_HOST_INTERFACE = 13 */
    "Host interface type",

    /* SAI_OBJECT_TYPE_MIRROR_SESSION = 14 */
    "Mirror type",

    /* SAI_OBJECT_TYPE_SAMPLEPACKET = 15 */
    "Sample packet type",

    /* SAI_OBJECT_TYPE_STP = 16 */
    "Stp instance type",

    /* SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP = 17 */
    "Trap group type",

    /* SAI_OBJECT_TYPE_POLICER = 18 */
    "Policer type",

    /* SAI_OBJECT_TYPE_WRED = 19 */
    "Wred type",

    /* SAI_OBJECT_TYPE_QOS_MAPS = 20 */
    "Qos map type",

    /* SAI_OBJECT_TYPE_QUEUE = 21 */
    "Queue type",

    /* SAI_OBJECT_TYPE_SCHEDULER = 22 */
    "scheduler type",

    /* SAI_OBJECT_TYPE_SCHEDULER_GROUP = 23 */
    "scheduler group type",

    /* SAI_OBJECT_TYPE_BUFFER_POOL      = 24*/
    "buffer pool",

    /* SAI_OBJECT_TYPE_BUFFER_PROFILE   = 25*/
    "buffer profile",

     /* SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP   = 26*/
    "ingress priority group",

    /* SAI_OBJECT_TYPE_LAG_MEMBER       = 27*/
    "lag number",

    /* SAI_OBJECT_TYPE_HASH             = 28*/
    "hash",

    /* SAI_OBJECT_TYPE_UDF              = 29*/
    "UDF",

    /* SAI_OBJECT_TYPE_UDF_MATCH        = 30*/
    "UDF match",

    /* SAI_OBJECT_TYPE_UDF_GROUP        = 31*/
    "UDF group",

    /* SAI_OBJECT_TYPE_FDB_ENTRY              = 32*/
    "FDB entry",

    /* SAI_OBJECT_TYPE_SWITCH           = 33*/
    "switch",

    /* SAI_OBJECT_TYPE_HOSTIF_TRAP             = 34*/
    "trap",

    /* SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY    = 35*/
    "trap table entry",

    /* SAI_OBJECT_TYPE_NEIGHBOR_ENTRY         = 36*/
    "neighbor",

    /* SAI_OBJECT_TYPE_ROUTE_ENTRY            = 37*/
    "route",

    /* SAI_OBJECT_TYPE_VLAN             = 38*/
    "vlan",

    /* SAI_OBJECT_TYPE_VLAN_MEMBER      = 39*/
    "vlan member",

    /* SAI_OBJECT_TYPE_HOSTIF_PACKET            = 40*/
    "packet",

    /* SAI_OBJECT_TYPE_TUNNEL_MAP               = 41*/
    "tunnel map",

    /* SAI_OBJECT_TYPE_TUNNEL                   = 42*/
    "tunnel",

    /* SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY  = 43*/
    "tunnel able entry",

    /* SAI_OBJECT_TYPE_FDB_FLUSH                = 44*/
    "fdb flush",

    /* SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER    = 45*/
    "fdb flush",

    /* SAI_OBJECT_TYPE_STP_PORT                 = 46*/
    "stp port",

    /* SAI_OBJECT_TYPE_RPF_GROUP                = 47*/
    "rpf group",

    /* SAI_OBJECT_TYPE_RPF_GROUP_MEMBER         = 48*/
    "rpf group member",

    /* SAI_OBJECT_TYPE_L2MC_GROUP               = 49*/
    "l2 mc group",

    /* SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER        = 50*/
    "l2 mc group member",

    /* SAI_OBJECT_TYPE_IPMC_GROUP               = 51*/
    "ipmc group",

    /* SAI_OBJECT_TYPE_IPMC_GROUP_MEMBER        = 52*/
    "ipmc group member",

    /* SAI_OBJECT_TYPE_L2MC_ENTRY               = 53*/
    "l2mc entry",

    /* SAI_OBJECT_TYPE_IPMC_ENTRY               = 54*/
    "ipmc entry",

    /* SAI_OBJECT_TYPE_MCAST_FDB_ENTRY          = 55*/
    "mcast fdb entry",

    /* SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP = 56*/
    "hostinf user defined trap",

    /* SAI_OBJECT_TYPE_BRIDGE                   = 57*/
    "bridge",

    /* SAI_OBJECT_TYPE_BRIDGE_PORT              = 58*/
    "bridge port",

    /* SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY         = 59*/
    "tunnel map entry",

    /* SAI_OBJECT_TYPE_MAX              = 60 */
    "MAX"
};

/* TRACE */

typedef enum {
	SYSLOG_LEVEL_QUIET,
	SYSLOG_LEVEL_FATAL,
	SYSLOG_LEVEL_ERROR,
	SYSLOG_LEVEL_INFO,
	SYSLOG_LEVEL_VERBOSE,
	SYSLOG_LEVEL_DEBUG1,
	SYSLOG_LEVEL_DEBUG2,
	SYSLOG_LEVEL_DEBUG3
} MRVL_SAI_LOG_LEVEL;


#define	SEVERITY_LEVEL_OFF		0x1		/* disable all logs and traces */
#define	SEVERITY_LEVEL_ALARM	0x2		/* about to crash and reboot */
#define	SEVERITY_LEVEL_ERROR	0x3		/* functionality is reduced */
#define	SEVERITY_LEVEL_ALERT	0x4		/* a violation has occurred */
#define	SEVERITY_LEVEL_WARN		0x5		/* unexpected, non critical event */
#define	SEVERITY_LEVEL_INFO		0x6		/* any other msg that is not a dbg msg */
#define	SEVERITY_LEVEL_DBG1		0x7		/* normal event debug message */
#define	SEVERITY_LEVEL_DBG2		0x8		/* debug specific message */
#define	SEVERITY_LEVEL_DBG3		0x9		/* maximum verbosity dbg specific msg */

#define	SEVERITY_LEVEL_LOWEST	SEVERITY_LEVEL_DBG3
#define	SEVERITY_LEVEL_DEFAULT	SEVERITY_LEVEL_DBG1 //SEVERITY_LEVEL_INFO

#define COLOR_NRM  "\x1B[0m"
#define COLOR_RED  "\x1B[31m"
#define COLOR_GRN  "\x1B[32m"
#define COLOR_YEL  "\x1B[33m"
#define COLOR_BLU  "\x1B[34m"
#define COLOR_MAG  "\x1B[35m"
#define COLOR_CYN  "\x1B[36m"
#define COLOR_WHT  "\x1B[37m"

typedef enum {
	APPID_GENERAL,
	APPID_LAST
} MRVL_SAI_APP_ID;

#define MRVL_SAI_TRACE_RETURN_MAC(status, params) \
	if (status != 0) {\
		mrvl_sai_trace params; \
		return status;\
	}

#define MRVL_SAI_TRACE_IS_ENABLE_MAC(app,severity) 	mrvl_sai_trace_is_enable(app,severity)


#define MRVL_SAI_TRACE_MAC(params) mrvl_sai_trace params

void mrvl_sai_trace (unsigned int appId, unsigned int level, char *format, ...);
void mrvl_sai_trace_init(void);
void mrvl_sai_trace_close(void);
bool mrvl_sai_trace_is_enable (unsigned int appId, unsigned int level);
void mrvl_sai_trace_set_all (uint32_t           enable,
					 MRVL_SAI_LOG_LEVEL      level);

#define MRVL_SAI_LOG_ENTER()           MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,"%s:%d:Enter function\n", __func__, __LINE__));
#define MRVL_SAI_LOG_EXIT()            MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG3,"%s:%d:Exit function\n", __func__, __LINE__));
#define MRVL_SAI_LOG_DBG(fmt, arg ...) MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG2,"%s:%d: "fmt, __func__, __LINE__, ## arg));
#define MRVL_SAI_LOG_NTC(fmt, arg ...) MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_DBG1,"%s:%d: "fmt, __func__, __LINE__, ## arg));
#define MRVL_SAI_LOG_INF(fmt, arg ...) MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_INFO,"%s:%d: "fmt, __func__, __LINE__, ## arg));
#define MRVL_SAI_LOG_WRN(fmt, arg ...) MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_WARN,"%s:%d: "fmt, __func__, __LINE__, ## arg));
#define MRVL_SAI_LOG_ERR(fmt, arg ...) MRVL_SAI_TRACE_MAC((APPID_GENERAL, SEVERITY_LEVEL_ERROR,"%s:%d: "fmt, __func__, __LINE__, ## arg));


#define MRVL_SAI_FDB_EVENT_TO_STR_MAC(evt_type) \
    (evt_type==SAI_FDB_EVENT_LEARNED)?"New Address":(evt_type==SAI_FDB_EVENT_AGED)?"Aged Address":"Flushed Address"


#define UNREFERENCED_PARAMETER(X)
#define MRVL_SAI_FDB_CREATE_COOKIE_MAC(vlan_id, mac_address)\
        ((uint64_t)(vlan_id & 0xffff)<<48)|\
        ((uint64_t)mac_address[0]<<40) |\
        ((uint64_t)mac_address[1]<<32) |\
        ((uint64_t)mac_address[2]<<24) |\
        ((uint64_t)mac_address[3]<<16) |\
        ((uint64_t)mac_address[4]<<8)  |\
        ((uint64_t)mac_address[5])

#define MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port)\
        ((vlan & 0xFFFF)<<16)|\
        (port & 0xFFFF)

#define MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(type, intfType, portVlan, data)\
        ((uint64_t)(type & 0xf)<<60)|\
        ((uint64_t)(intfType & 0x1)<<48)|\
        ((uint64_t)(portVlan & 0xffff)<<32)|\
        ((uint64_t)data & 0xffffffff)

#define MRVL_SAI_ROUTER_INTF_CREATE_COOKIE_MAC(intfType, portVlan, ipType, macAddr)\
        ((uint64_t)(intfType & 0x1)<<63)|\
        ((uint64_t)(portVlan & 0x3fff)<<49)|\
        ((uint64_t)(ipType & 0x1)<<48)|\
        ((uint64_t)macAddr[0]<<40) |\
        ((uint64_t)macAddr[1]<<32) |\
        ((uint64_t)macAddr[2]<<24) |\
        ((uint64_t)macAddr[3]<<16) |\
        ((uint64_t)macAddr[4]<<8)  |\
        ((uint64_t)macAddr[5])

#define MRVL_SAI_ROUTER_COOKIE_MAC(type, portVlan)\
        ((uint64_t)(type & 0x1)<<32)|\
        ((uint64_t)(portVlan))

#define MRVL_SAI_ACL_CREATE_COOKIE_MAC(acl_table_index, acl_entry_index)\
        ((acl_table_index & 0xFFFF)<<16)|\
        (acl_entry_index & 0xFFFF)

#define MRVL_SAI_INVALID_ID_CNS 0xFFFFFFFF

/*
################################################################################
#                            Custom switch attributes                          #
################################################################################
*/
/* enable trap for a list of SAI vlans [sai_object_list_t]
    (default to all SAI vlans) */
#define  SAI_HOSTIF_TRAP_ATTR_VLAN_LIST     ((sai_attr_id_t)0x10000001)

/* structures for router interface module */
typedef struct _mrvl_sai_rif_table_t {
    bool                    valid;          /* entry is valid*/
    sai_mac_t               src_mac;        /* entry src mac address */
    uint32_t                mtu_idx;        /* index in the mtu table*/
    uint32_t                intf_type;      /* interface type (port or vlan) */    
    uint32_t                port_vlan_id;   /* the port or vlan id ( depand on intf_type) */
    uint32_t                vrf_id;         /* virtual router id */
    bool                    nbr_valid;      /* neighbor is valid on this rif */
    uint32_t                first_nbr_idx;  /* the first valid neighbor id (in the neighbor table) */
    sai_packet_action_t     nbr_miss_act;   /* what to do if neighbor don't exist (can be only trap or drop) */
    bool                    nh_valid;       /* next hop is valid on this rif */
    uint32_t                first_nh_idx;   /* the first valid next hop id (in the next hop table) */
    mrvl_sai_utl_dlist_elem_STC route_list_elem;
} mrvl_sai_rif_table_t;

typedef struct _mrvl_sai_rif_mtu_table_t {
    uint32_t                mtu_size;
    uint32_t                ref_cntr;
} mrvl_sai_rif_mtu_table_t;

/* structures for neighbor module */
typedef struct _mrvl_sai_nbr_table_t {
    bool                    used;          /* entry is valid*/
    sai_ip_address_t        inet_address;  /* entry inet address */
    uint32_t                next_nbr_id;   /* the next neighbor id (in the neighbor table) configured on the same rif */
    sai_mac_t               dst_mac;       /* entry dst mac address (used for arp table)*/
    uint32_t                next_hop_idx;
} mrvl_sai_nbr_table_t;

#define MRVL_SAI_NEIGHBOR_RESERVED_ID_CNS 0xFFFFFFFD
#define MRVL_SAI_NEIGHBOR_DROP_ID_CNS 0xFFFFFFFD
#define MRVL_SAI_NEIGHBOR_TRAP_ID_CNS 0xFFFFFFFE
/* structures for nexthop module */
typedef struct _mrvl_sai_nh_table_t {
    bool                    used;          /* entry is valid*/
    sai_ip_address_t        inet_address;  /* entry inet address */
    uint32_t                next_nh_id;   /* the next nexthop id (in the nexthop table) configured on the same rif */
    uint32_t                rif_id;       /* router interface id for this entry*/
    uint32_t                nbr_id;       /* neighbor id that this entry point to*/
    mrvl_sai_utl_dlist_elem_STC route_list_elem;
} mrvl_sai_nh_table_t;

typedef struct _mrvl_sai_next_hop_group_table_t {
    bool                    used;          /* entry is valid*/
    uint32_t                group_member_counter;
} mrvl_sai_next_hop_group_table_t;

typedef struct _mrvl_sai_lag_group_table_t {
    bool                    used;          /* entry is valid*/
    uint32_t                group_member_counter;
} mrvl_sai_lag_group_table_t;

typedef struct _mrvl_sai_route_hash_key_t
{
    uint32_t        vr_id;
    sai_ip_prefix_t destination;
} mrvl_sai_route_hash_key_t;

typedef struct _mrvl_sai_route_hash_data_t
{
    mrvl_sai_utl_dlist_elem_STC     list_elem;
    mrvl_sai_route_hash_key_t       *key_ptr;
    uint32_t                        cookie;
    bool                            valid;
    sai_packet_action_t             action;
    bool                            nh_valid;
    uint32_t                        nh_idx;
    uint32_t                        nh_type;
    uint32_t                        nbr_idx;
} mrvl_sai_route_hash_data_t;

typedef struct _mrvl_sai_route_hash_entry_t
{
    mrvl_sai_route_hash_key_t        key;
    mrvl_sai_route_hash_data_t       data;
} mrvl_sai_route_hash_entry_t;
sai_status_t mrvl_sai_rif_is_exist(_In_ uint32_t   rif_idx,
                                   _In_ bool        *is_exist);
sai_status_t mrvl_sai_rif_get_first_nbr_id(_In_ uint32_t   rif_idx,
                                           _In_ uint32_t   *first_nbr);
sai_status_t mrvl_sai_rif_add_first_nbr_id(_In_  uint32_t  rif_idx,
                                           _In_  uint32_t  new_first_nbr,
                                           _Out_ uint32_t  *old_nbr);
sai_status_t mrvl_sai_rif_get_first_nh_id(_In_  uint32_t   rif_idx,
                                          _Out_ uint32_t   *first_nh);
sai_status_t mrvl_sai_rif_add_first_nh_id(_In_  uint32_t  rif_idx,
                                           _In_  uint32_t  new_first_nh,
                                           _Out_ uint32_t  *old_nh);
sai_status_t mrvl_sai_rif_get_mtu_size(_In_  uint32_t   rif_idx,
                                       _Out_ uint32_t   *mtu_size);
sai_status_t mrvl_sai_rif_get_entry(_In_  uint32_t              rif_idx,
                                    _Out_ mrvl_sai_rif_table_t  *entry);
sai_status_t mrvl_sai_rif_get_nbr_miss_act(_In_ uint32_t               rif_idx,
                                           _Out_ sai_packet_action_t  *nbr_miss_act);
sai_status_t mrvl_sai_rif_add_route(_In_ uint32_t rif_idx,
                                    _In_ mrvl_sai_utl_dlist_elem_STC *route_list);
sai_status_t mrvl_sai_rif_del_route(_In_ uint32_t rif_idx,
                                    _In_ mrvl_sai_utl_dlist_elem_STC *route_list);
sai_status_t mrvl_sai_rif_update_nbr_id(uint32_t rif_idx, uint32_t nbr_idx, sai_ip_address_t *inet_address);
sai_status_t mrvl_sai_rif_get_vr_id(_In_ uint32_t    rif_idx, _Out_ uint32_t  *vr_idx);

sai_status_t mrvl_sai_get_match_neighbor_id(_In_  uint32_t          rif_idx ,
                                            _In_  const sai_ip_address_t  *ip_address,
                                            _Out_ uint32_t          *nbr_id);
sai_status_t mrvl_sai_neighbor_set_next_hop(_In_  uint32_t  nbr_idx ,
                                            _In_  uint32_t  nh_idx);
sai_status_t mrvl_sai_neighbor_get_next_hop(_In_  uint32_t  nbr_idx ,
                                            _Out_  uint32_t  *nh_idx);

sai_status_t mrvl_sai_next_hop_update_nbr_id(uint32_t rif_idx,
                                             uint32_t nbr_idx,
                                             sai_ip_address_t *inet_address,
                                             uint32_t *nh_idx);
sai_status_t mrvl_sai_next_hop_get_route_list(uint32_t nh_idx, mrvl_sai_utl_dlist_elem_STC **route_list);
sai_status_t mrvl_sai_next_hop_add_route(uint32_t nh_idx, mrvl_sai_utl_dlist_elem_STC *route_list);
sai_status_t mrvl_sai_next_hop_del_route(uint32_t nh_idx, mrvl_sai_utl_dlist_elem_STC *route_list);
sai_status_t mrvl_sai_next_hop_get_nbr_id(uint32_t nh_idx, uint32_t *nbr_idx);
sai_status_t mrvl_sai_next_hop_get_rif_id(uint32_t nh_idx, uint32_t *rif_idx);

sai_status_t mrvl_sai_route_init(void);
sai_status_t mrvl_sai_route_update_nbr_id(mrvl_sai_route_hash_data_t *route_entry_data, uint32_t nbr_idx);
sai_status_t mrvl_sai_route_update_nbr_id_if_match(mrvl_sai_route_hash_data_t *route_entry_data, sai_ip_address_t *inet_address, uint32_t nbr_idx);

sai_status_t mrvl_sai_virtual_router_is_valid(_In_ uint32_t   vr_idx, _Out_ bool *is_valid);
sai_status_t mrvl_sai_virtual_router_update_referance_cntr(_In_ uint32_t   vr_idx, _In_ bool add);
sai_status_t mrvl_sai_virtual_router_get_mac(_In_ uint32_t   vr_idx, _Out_ sai_mac_t mac_address);

sai_status_t mrvl_sai_rif_dump(void);
sai_status_t mrvl_sai_neighbor_dump(void);
sai_status_t mrvl_sai_next_hop_dump(void);
sai_status_t mrvl_sai_route_dump(void);

sai_status_t mrvl_sai_host_interface_init(void);
sai_status_t mrvl_sai_host_interface_trap_get_default_action(_In_ sai_hostif_trap_type_t hostif_trapid, _Out_ sai_packet_action_t *action);
sai_status_t mrvl_sai_host_interface_trap_is_port_vlan_supported(_In_  sai_hostif_trap_type_t hostif_trapid,_Out_ bool *isSupported);

#define mrvl_acl_is_bit_set_MAC(fieldsBitMap, field_index) (fieldsBitMap & (1<<field_index))
#define mrvl_acl_set_bit_MAC(fieldsBitMap, field_index) (fieldsBitMap |= (1<<field_index))
#define mrvl_acl_clear_bit_MAC(fieldsBitMap, field_index) (fieldsBitMap &= ~((1<<field_index) & 0xFFFFFFFF))
#define mrvl_acl_set_all_MAC(fieldsBitMap) (fieldsBitMap = 0xFFFFFFFF)
#define mrvl_acl_clear_all_MAC(fieldsBitMap) (fieldsBitMap = 0)
#define mrvl_acl_is_all_0_MAC(fieldsBitMap) (fieldsBitMap == 0)
sai_status_t mrvl_sai_acl_init(void);
sai_status_t mrvl_sai_acl_table_bind_to_port(_In_ void *arg, _In_ const sai_object_id_t object_id, _In_ uint32_t port);
sai_status_t mrvl_sai_acl_table_unbind_from_port(_In_ void *arg, _In_ uint32_t port);
sai_status_t mrvl_sai_acl_get_table_id_per_port(_In_ void *arg, _In_ uint32_t port, _Inout_ sai_attribute_value_t *value);
sai_status_t mrvl_sai_acl_lag_port_update(_In_ uint32_t lag, _In_ uint32_t port, _In_ bool is_added);
sai_status_t mrvl_sai_acl_table_bind_to_lag(_In_ void *arg, _In_ const sai_object_id_t object_id, _In_ uint32_t lag);
sai_status_t mrvl_sai_acl_table_unbind_from_lag(_In_ void *arg, _In_ uint32_t lag);
sai_status_t mrvl_sai_acl_get_table_id_per_lag(_In_ void *arg, _In_ uint32_t lag, _Inout_ sai_attribute_value_t *value);
sai_status_t mrvl_sai_acl_table_bind_to_switch(_In_ void *arg, _In_ const sai_object_id_t object_id, _In_ uint32_t switch_idx);
sai_status_t mrvl_sai_acl_table_unbind_from_switch(_In_ void *arg, _In_ uint32_t port);
sai_status_t mrvl_sai_acl_get_table_id_per_switch(_In_ void *arg, _In_ uint32_t port, _Inout_ sai_attribute_value_t *value);
sai_status_t mrvl_sai_acl_table_bind_to_vlan(_In_ void *arg, _In_ const sai_object_id_t object_id, _In_ uint32_t vlan_idx);
sai_status_t mrvl_sai_acl_table_unbind_from_vlan(_In_ void *arg, _In_ uint32_t port);
sai_status_t mrvl_sai_acl_get_table_id_per_vlan(_In_ void *arg, _In_ uint32_t port, _Inout_ sai_attribute_value_t *value);
sai_status_t mrvl_sai_get_lag_port_list(
    _In_ const uint32_t          lag_id,
    _Out_ sai_object_list_t     *portbjlist);

#define SAI_STATUS_STUB SAI_STATUS_NOT_SUPPORTED
sai_status_t  mrvl_sai_return(const char *func_name, int line, sai_status_t status);
#define MRVL_SAI_API_RETURN(_status) \
	do { \
		if ( SAI_STATUS_SUCCESS != _status) \
			return mrvl_sai_return(__func__, __LINE__, _status); \
		else \
			return _status; \
	} while(0)
/*
#ifndef _WIN32
#define UNREFERENCED_PARAMETER(X)
#else
#define PRId64 "lld"
unsigned int if_nametoindex(const char *ifname);
char *if_indextoname(unsigned int ifindex, char *ifname);
#define IF_NAMESIZE 32
#endif
*/

int mrvl_sai_netdev_set_mac(char * name, unsigned char * mac_char);
int mrvl_sai_netdev_get_mac(char * name, unsigned char * mac_char);
int mrvl_sai_netdev_alloc(char *dev) ;
int mrvl_sai_netdev_set_carrier(uint32_t portId, int linkStatus);

typedef struct
{
	char name[32];
	int  fd;
	int  valid;
}host_fd_stc;

extern host_fd_stc host_fd[];

/* ACL definitions */
#define SAI_ACL_MAX_BIND_POINT_TYPES (SAI_ACL_BIND_POINT_TYPE_SWITCH + 1) 
#define SAI_ACL_MAX_ACTION_TYPES (SAI_ACL_ACTION_TYPE_SET_DO_NOT_LEARN + 1) 


#endif // __MRVL_SAI_H_
