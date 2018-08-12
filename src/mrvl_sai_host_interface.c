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


#undef  __MODULE__
#define __MODULE__ SAI_HOST_INTERFACE

#include "sai.h"
#include "mrvl_sai.h"
#include    <errno.h> 
#include    <unistd.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>




/* define consecutive enumaration for sai trap IDs enum _sai_hostif_trap_type_t
   when adding in sai new trap IDs need to update also this enumaration */
typedef enum _sai_consecutive_hostif_trap_type_t
{   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_START, 
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_STP = INTERNAL_SAI_HOSTIF_TRAP_TYPE_START,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_LACP,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_EAPOL,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_LLDP,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_PVRST,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET,    
  
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_DHCP,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_OSPF,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_PIM,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_VRRP,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_BGP,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_DHCPV6,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_OSPFV6,
  
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_VRRPV6,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_BGPV6,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE,
    
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_IP2ME,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_SSH,
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_SNMP,   
   
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR,
  
    INTERNAL_SAI_HOSTIF_TRAP_TYPE_TTL_ERROR,

    INTERNAL_SAI_HOSTIF_TRAP_TYPE_END    

} sai_consecutive_hostif_trap_type_t;

/* check and convert sai trapid to consecutive internal trapid */
inline uint32_t mrvl_sai_host_interface_conv_trap_type (_In_ sai_hostif_trap_type_t sai_trap_type)
{
    switch(sai_trap_type)                                           
    {                                                             
    case SAI_HOSTIF_TRAP_TYPE_STP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_STP; break;             
    case SAI_HOSTIF_TRAP_TYPE_LACP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_LACP; break;             
    case SAI_HOSTIF_TRAP_TYPE_EAPOL: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_EAPOL; break;             
    case SAI_HOSTIF_TRAP_TYPE_LLDP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_LLDP; break;            
    case SAI_HOSTIF_TRAP_TYPE_PVRST:  return INTERNAL_SAI_HOSTIF_TRAP_TYPE_PVRST; break;             
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY; break;             
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE; break;             
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT; break;             
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT; break;             
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT; break;             
    case SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET; break;             
    case SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST; break;             
    case SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE; break;             
    case SAI_HOSTIF_TRAP_TYPE_DHCP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_DHCP; break;             
    case SAI_HOSTIF_TRAP_TYPE_OSPF: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_OSPF; break;             
    case SAI_HOSTIF_TRAP_TYPE_PIM: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_PIM; break;             
    case SAI_HOSTIF_TRAP_TYPE_VRRP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_VRRP; break;             
    case SAI_HOSTIF_TRAP_TYPE_BGP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_BGP; break;             
    case SAI_HOSTIF_TRAP_TYPE_DHCPV6: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_DHCPV6; break;             
    case SAI_HOSTIF_TRAP_TYPE_OSPFV6: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_OSPFV6; break;             
    case SAI_HOSTIF_TRAP_TYPE_VRRPV6: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_VRRPV6; break;             
    case SAI_HOSTIF_TRAP_TYPE_BGPV6: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_BGPV6; break;             
    case SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY; break;             
    case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2; break;             
    case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT; break;             
    case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE:  return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE; break;             
    case SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT; break;             
    case SAI_HOSTIF_TRAP_TYPE_IP2ME: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_IP2ME; break;             
    case SAI_HOSTIF_TRAP_TYPE_SSH: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_SSH; break;             
    case SAI_HOSTIF_TRAP_TYPE_SNMP: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_SNMP; break;             
    case SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR; break;             
    case SAI_HOSTIF_TRAP_TYPE_TTL_ERROR: return INTERNAL_SAI_HOSTIF_TRAP_TYPE_TTL_ERROR; break;             
    default: return SAI_STATUS_INVALID_PARAMETER; break;                             
    } 
}
                              
/* convert sai trapid to consecutive internal trapid */
#define PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(_sai_trap_type, _internal_trap_type) \
    _internal_trap_type = mrvl_sai_host_interface_conv_trap_type(_sai_trap_type)


typedef struct _mrvl_sai_hostif_set_entry_t {
    bool port; 
    bool vlan;
}mrvl_sai_hostif_set_entry_t;

typedef struct _mrvl_sai_hostif_list_entry_t {
    mrvl_sai_utl_dlist_elem_STC list;   /* element in linked list */
    sai_object_id_t             data;   /* data */
    bool                        valid;  /* element is valid */
} mrvl_sai_hostif_list_entry_t;

typedef struct _mrvl_sai_hostif_trap_entry_t {
    mrvl_sai_hostif_set_entry_t  valid;    /* entry is valid */
    sai_packet_action_t          action;   /* trap action */   
    mrvl_sai_hostif_set_entry_t  action_not_set; /* flag if the user set trap action attribute before trap port_list/vlan_list attribute */    
    mrvl_sai_utl_dlist_elem_STC  hostif_portlist_header; /* header of sorted linked list of ports */
    mrvl_sai_utl_dlist_elem_STC  hostif_vlanlist_header; /* header of sorted linked list of vlans */
    uint32_t portList_count;    /* number of ports in linked list */
    uint32_t vlanList_count;    /* number of vlans in linked list */
} mrvl_sai_hostif_trap_entry_t; 
   
static mrvl_sai_hostif_trap_entry_t mrvl_sai_hostif_trap_table[INTERNAL_SAI_HOSTIF_TRAP_TYPE_END] = {};

static const sai_attribute_entry_t host_interface_attribs[] = {
    { SAI_HOSTIF_ATTR_TYPE, true, true, false, true,
      "Host interface type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_ATTR_OBJ_ID, false, true, false, true,
      "Host interface associated port or router interface", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_HOSTIF_ATTR_NAME, true, true, true, true,
      "Host interface name", SAI_ATTR_VALUE_TYPE_CHARDATA },
    { SAI_HOSTIF_ATTR_OPER_STATUS, false, true, true, true,
        "Host interface oper status", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_HOSTIF_ATTR_QUEUE, false, false, false, true,
        "Host interface queue used for packets going out", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_HOSTIF_ATTR_VLAN_TAG, false, true, true, true,
        "Host interface vlan tag strip/keep behaviour", SAI_ATTR_VALUE_TYPE_INT32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

sai_status_t mrvl_sai_host_interface_type_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
sai_status_t mrvl_sai_host_interface_rif_port_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
sai_status_t mrvl_sai_host_interface_name_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
sai_status_t mrvl_sai_host_interface_name_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);

sai_status_t mrvl_sai_host_interface_oper_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
sai_status_t mrvl_sai_host_interface_oper_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);

sai_status_t mrvl_sai_host_interface_vlan_tag_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg);

static const sai_vendor_attribute_entry_t host_interface_vendor_attribs[] = {
    { SAI_HOSTIF_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_host_interface_type_get, NULL,
      NULL, NULL },
    { SAI_HOSTIF_ATTR_OBJ_ID,
        { true, false, false, true },
        { true, false, false, true },
        mrvl_sai_host_interface_rif_port_get, NULL,
        NULL, NULL },
    { SAI_HOSTIF_ATTR_NAME,
        { true, false, true, true },
        { true, false, true, true },
        mrvl_sai_host_interface_name_get, NULL,
        mrvl_sai_host_interface_name_set, NULL },
    { SAI_HOSTIF_ATTR_OPER_STATUS,
        { true, false, true, true },
        { true, false, true, true },
        mrvl_sai_host_interface_oper_get, NULL,
        mrvl_sai_host_interface_oper_set, NULL },
    { SAI_HOSTIF_ATTR_QUEUE,
      { false, false, false, false },
      { true, false, true, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_ATTR_VLAN_TAG,
      { true, false, true, false },
      { true, false, true, true },
      NULL, NULL,
      mrvl_sai_host_interface_vlan_tag_set, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};
static void mrvl_sai_host_interface_key_to_str(_In_ sai_object_id_t hif_id, _Out_ char *key_str)
{
    uint32_t hif_data;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(hif_id, SAI_OBJECT_TYPE_HOSTIF, &hif_data)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid host interface");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "host interface %u", hif_data);
    }
}

static void mrvl_sai_host_interface_table_entry_key_to_str(_In_ sai_object_id_t hif_table_entry_id, _Out_ char *key_str)
{
    uint32_t hif_table_entry_data;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(hif_table_entry_id, SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY, &hif_table_entry_data)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid host interface table entry");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "host interface table entry %u", hif_table_entry_data);
    }
}
static const sai_attribute_entry_t host_interface_trap_attribs[] = {
    { SAI_HOSTIF_TRAP_ATTR_TRAP_TYPE, true, false, true, true,
      "Host interface trap type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION, true, false, true, true,
      "Host interface trap action", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY, false, false, false, false,
      "Host interface trap priority", SAI_ATTR_VALUE_TYPE_UINT32 },
/*    { SAI_HOSTIF_TRAP_ATTR_TRAP_CHANNEL, false, false, true, true,
      "trap channel to use", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_TRAP_ATTR_FD, false, false, false, false,
      "file descriptor", SAI_ATTR_VALUE_TYPE_UINT64 },
    { SAI_HOSTIF_TRAP_ATTR_PORT_LIST, false, false, true, true,
      "enable trap for a list of SAI ports", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
*/
    { SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST, false, false, true, true,
      "Disable trap for a list of SAI ports", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },
    { SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP, false, false, false, false,
      "Trap-group ID for the trap", SAI_ATTR_VALUE_TYPE_UINT64 },  
    { SAI_HOSTIF_TRAP_ATTR_VLAN_LIST, false, false, true, true,
      "Enable trap for a list of SAI vlans", SAI_ATTR_VALUE_TYPE_OBJECT_LIST },  
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

sai_status_t mrvl_hostif_trap_action_set(_In_ const sai_object_key_t *key,
                                         _In_ const sai_attribute_value_t *value,
                                         void *arg);

sai_status_t mrvl_hostif_trap_action_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg);

sai_status_t mrvl_hostif_trap_port_list_set(_In_ const sai_object_key_t *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void *arg);

sai_status_t mrvl_hostif_trap_port_list_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg);

sai_status_t mrvl_hostif_trap_vlan_list_set(_In_ const sai_object_key_t *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void *arg);

sai_status_t mrvl_hostif_trap_vlan_list_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg);


sai_status_t mrvl_hostif_trap_channel_set(_In_ const sai_object_key_t *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void *arg);

sai_status_t mrvl_hostif_trap_channel_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);

static const sai_vendor_attribute_entry_t host_interface_trap_vendor_attribs[] = {
    { SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_hostif_trap_action_get, NULL,
      mrvl_hostif_trap_action_set, NULL },
    { SAI_HOSTIF_TRAP_ATTR_TRAP_PRIORITY,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
/* TODO need to support new attributes
    { SAI_HOSTIF_TRAP_ATTR_TRAP_CHANNEL,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_hostif_trap_channel_get, NULL,
      mrvl_hostif_trap_channel_set, NULL },
    { SAI_HOSTIF_TRAP_ATTR_FD,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_TRAP_ATTR_PORT_LIST,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_hostif_trap_port_list_get, NULL,
      mrvl_hostif_trap_port_list_set, NULL },
*/
    { SAI_HOSTIF_TRAP_ATTR_EXCLUDE_PORT_LIST,
      { true, false, true, false },
      { true, false, true, false },
      NULL, NULL },
    { SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },   
    { SAI_HOSTIF_TRAP_ATTR_VLAN_LIST,
      { false, false, true, true },
      { false, false, true, true },
      mrvl_hostif_trap_vlan_list_get, NULL,
      mrvl_hostif_trap_vlan_list_set, NULL },
};

static void mrvl_sai_host_interface_trap_key_to_str(_In_ sai_hostif_trap_type_t hostif_trapid, _Out_ char *key_str)
{    
    snprintf(key_str, MAX_KEY_STR_LEN, "host interface trap id 0x%x\n", hostif_trapid);
}

static const sai_attribute_entry_t host_interface_trap_group_attribs[] = {
    { SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE, false, false, false, true,
      "Hostif trap group admin mode", SAI_ATTR_VALUE_TYPE_BOOL },
    { SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE, true, false, true, true,
      "Hostif trap group CPU egress queue", SAI_ATTR_VALUE_TYPE_UINT32 },
    { SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER, false, false, false, false,
      "Hostif trap group policer id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

sai_status_t mrvl_sai_trap_group_admin_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
sai_status_t mrvl_sai_trap_group_admin_set(_In_ const sai_object_key_t      *key,
                                              _In_ const sai_attribute_value_t *value,
                                              void                             *arg);
sai_status_t mrvl_sai_trap_group_queue_set(_In_ const sai_object_key_t      *key,
                                              _In_ const sai_attribute_value_t *value,
                                              void                             *arg);
sai_status_t mrvl_sai_trap_group_queue_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg);
sai_status_t mrvl_sai_trap_group_policer_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg);
sai_status_t mrvl_sai_trap_group_policer_set(_In_ const sai_object_key_t      *key,
                                                _In_ const sai_attribute_value_t *value,
                                                void                             *arg);

static const sai_vendor_attribute_entry_t host_interface_trap_group_vendor_attribs[] = {
    { SAI_HOSTIF_TRAP_GROUP_ATTR_ADMIN_STATE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_trap_group_admin_get, NULL,
      mrvl_sai_trap_group_admin_set, NULL },
    { SAI_HOSTIF_TRAP_GROUP_ATTR_QUEUE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_trap_group_queue_get, NULL,
      mrvl_sai_trap_group_queue_set, NULL },
    { SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER,
      { true, false, true, true},
      { true, false, true, true},
      mrvl_sai_trap_group_policer_get, NULL,
      mrvl_sai_trap_group_policer_set, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

sai_status_t mrvl_sai_trap_group_admin_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->booldata = true; /*default */

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_trap_group_admin_set(_In_ const sai_object_key_t      *key,
                                              _In_ const sai_attribute_value_t *value,
                                              void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t mrvl_sai_trap_group_queue_set(_In_ const sai_object_key_t      *key,
                                              _In_ const sai_attribute_value_t *value,
                                              void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t mrvl_sai_trap_group_queue_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->u32 = 0; /*default*/

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_trap_group_policer_get(_In_ const sai_object_key_t   *key,
                                                _Inout_ sai_attribute_value_t *value,
                                                _In_ uint32_t                  attr_index,
                                                _Inout_ vendor_cache_t        *cache,
                                                void                          *arg)
{
    sai_status_t status;
    MRVL_SAI_LOG_ENTER();
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_POLICER, 1, &value->oid))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_trap_group_policer_set(_In_ const sai_object_key_t      *key,
                                                _In_ const sai_attribute_value_t *value,
                                                void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}

static const sai_attribute_entry_t host_interface_packet_attribs[] = {
    { SAI_HOSTIF_PACKET_ATTR_HOSTIF_TRAP_ID, false, false, false, true,
      "Hostif packet trap id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT, false, false, false, true,
      "Hostif packet ingress port", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_HOSTIF_PACKET_ATTR_INGRESS_LAG, false, false, false, true,
      "Hostif packet ingress lag", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE, true, false, false, false,
      "Hostif packet TX type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG, true, false, false, false,
      "Hostif packet egress port or lag", SAI_ATTR_VALUE_TYPE_OBJECT_ID },

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t host_interface_packet_vendor_attribs[] = {
    { SAI_HOSTIF_PACKET_ATTR_HOSTIF_TRAP_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_PACKET_ATTR_INGRESS_PORT,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_PACKET_ATTR_INGRESS_LAG,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE,
      { true, false, false, false },
      { true, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG,
      { true, false, false, false },
      { true, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

static const sai_attribute_entry_t host_interface_table_entry_attribs[] = {
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE, true, true, false, true,
      "Hostif table entry type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID, true, true, false, true,
      "Hostif table entry match field object id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID, true, true, false, true,
      "Hostif table entry match field trap id", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE, true, true, false, false,
      "Hostif table entry action channel", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF, true, true, false, false,
      "Hostif table entry action target hostif object", SAI_ATTR_VALUE_TYPE_OBJECT_ID },

    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};

/* Host interface table entry type (sai_hostif_table_entry_type_t)
 *  Host interface table entry match field object-id (sai_object_id_t)
 *  Host interface table entry match field trap-id (sai_object_id_t)
 */
sai_status_t mrvl_sai_table_entry_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg);

static const sai_vendor_attribute_entry_t host_interface_table_entry_vendor_attribs[] = {
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_table_entry_get, (void*)SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE,
      NULL, NULL },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_table_entry_get, (void*)SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID,
      NULL, NULL },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_table_entry_get, (void*)SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID,
      NULL, NULL },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      NULL, NULL,
      NULL, NULL },
    { SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF,
      { true, false, false, false },
      { true, false, false, true },
      NULL, NULL,
      NULL, NULL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};

sai_status_t mrvl_sai_table_entry_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t     status;
    uint32_t         hif_data, hif_ext_data;
    uint8_t         ext_data[RESERVED_DATA_LENGTH_CNS];
    
    
    MRVL_SAI_LOG_ENTER();

    assert((SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE == (long)arg) || (SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID == (long)arg) ||
           (SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID == (long)arg));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_ext_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hif_data, ext_data)))
        MRVL_SAI_API_RETURN(status);

    hif_ext_data = (uint32_t)(ext_data[1] << 8 | ext_data[0]);

    switch ((long)arg) {
    case SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE:
        value->s32 = hif_data;
        break;

    case SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID:
        switch (hif_data) {
        case SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT:
            return mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, hif_ext_data, &value->oid);

        case SAI_HOSTIF_TABLE_ENTRY_TYPE_LAG:
            return mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_LAG, hif_ext_data, &value->oid);

        case SAI_HOSTIF_TABLE_ENTRY_TYPE_VLAN:
            return mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, hif_ext_data, &value->oid);

        default:
            MRVL_SAI_LOG_ERR("Host table entry invalid object ID %u\n", hif_data);
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + attr_index;
        }
        break;

    case SAI_HOSTIF_TABLE_ENTRY_ATTR_TRAP_ID:
        return SAI_STATUS_NOT_IMPLEMENTED;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


host_fd_stc host_fd[SAI_MAX_NUM_OF_PORTS];

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

fd_set read_fd_set;
int read_fd_num=0;
pthread_t thread_asic, thread_tap;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

unsigned long tx_count=0;
unsigned long rx_count=0;
unsigned long rx_failed_count=0;

void * mrvl_sai_tap_listen(void * arg)
{
	fd_set rd;
    int i, size, fd;
    char data[2048];
    struct timeval  timeout;
    //FPA_STATUS st;
    FPA_PACKET_OUT_BUFFER_STC pkt;
    int select_rc;
    MRVL_SAI_LOG_ENTER();
    while (1)
    {
    	pthread_mutex_lock(&lock);
    	rd = read_fd_set;
    	pthread_mutex_unlock(&lock);
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        select_rc = select(FD_SETSIZE, &rd, NULL, NULL, &timeout);
        /* timeout */
        if ( select_rc == 0)
        {
            continue;
        }
        else if ( select_rc < 0) {
                MRVL_SAI_LOG_ERR("select errorno 0x%x\n", errno);
                if (errno == EINTR){
                    MRVL_SAI_LOG_ERR("select exit on error\n");
                    MRVL_SAI_LOG_EXIT();
                    return NULL;
                }
                continue;
       }
	   for (i = 0; i < SAI_MAX_NUM_OF_PORTS && select_rc > 0; i++)
	   {
		   if (host_fd[i].valid)
		   {
			   fd = host_fd[i].fd;
			   if (FD_ISSET(fd, &rd)) {
                   select_rc--;
                   size = 0;
				   do {
					   size = read(fd, &data, 2048);
				   }while ((size < 0) && (errno == EINTR));
                    if (size > 0 )
                    {
                        pkt.outPortNum = i;
                        pkt.pktDataSize = size;
                        pkt.pktDataPtr = data;
                        //st =
                        fpaLibPortPktSend(0, 0, &pkt);
                        //MRVL_SAI_LOG_WRN("### tap packet rx: name %s port %d, size %d, status %x\n", host_fd[i].name, i, size, st );
                        tx_count++;
                    }
				}
             }
	   }
    }
    MRVL_SAI_LOG_EXIT();
    return NULL;
}

void * mrvl_sai_asic_listen(void * arg)
{
	FPA_STATUS st;
	char data[2048];
	FPA_PACKET_BUFFER_STC pkt;
	pkt.pktDataPtr = data;
	int i, fd, ret;
	MRVL_SAI_LOG_ENTER();
	while(1)
	{
		st = fpaLibPktReceive(0, 0, &pkt);
		if ( st != FPA_OK)
		{
			//MRVL_SAI_LOG_WRN("asic packet rx: failes status %x\n", st);
			rx_failed_count++;
			continue;
		}
		rx_count++;
		//MRVL_SAI_LOG_WRN("&&& asic packet rx: port %d, size %d status %x\n", pkt.inPortNum, pkt.pktDataSize, st);
		i = pkt.inPortNum;
		if (host_fd[i].valid)
		{
			fd = host_fd[i].fd;
			do {
	            ret = write(fd, pkt.pktDataPtr, pkt.pktDataSize);
	        } while ((ret < 0) && (errno == EINTR));
			//MRVL_SAI_LOG_WRN("### asic packet tx tap : port %d, size %d sent to tap %s, ret %x\n", pkt.inPortNum, pkt.pktDataSize,host_fd[i].name, ret );
		}
		else
		{
			//MRVL_SAI_LOG_WRN("asic packet rx: port %d, no tap found\n", pkt.inPortNum);
		}
	}
	MRVL_SAI_LOG_EXIT();
	return NULL;
}

/**

 * @brief Create host interface
 *
 * @param[out] hif_id Host interface id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error


 */

sai_status_t mrvl_sai_create_host_interface(_Out_ sai_object_id_t     * hif_id,
                                            _In_ sai_object_id_t        switch_id,
                                            _In_ uint32_t               attr_count,
                                            _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *type, *rif_port, *name;
    uint32_t                     type_index, rif_port_index, name_index, rif_data;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    static uint32_t              next_id = 0;
    char devname[32];
    char                         system_cmd[256], port_system_cmd[256];
    int fd, err, ret;
    sai_mac_t srcMac, exist_src_mac;
    char base_mac_dev[] ="eth0";
    MRVL_SAI_LOG_ENTER();
    
    if (NULL == hif_id) {
        MRVL_SAI_LOG_ERR("NULL host interface ID param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, host_interface_attribs, host_interface_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, host_interface_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create host interface, %s\n", list_str);

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_TYPE, &type, &type_index));
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_NAME, &name, &name_index));
    
    if (SAI_HOSTIF_TYPE_NETDEV == type->s32) {
        if (SAI_STATUS_SUCCESS !=
            (status =
                 mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_OBJ_ID, &rif_port,
                                     &rif_port_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute rif port id on create of host if netdev type\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }

        if (SAI_OBJECT_TYPE_ROUTER_INTERFACE == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS !=
                (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_data))) {
                MRVL_SAI_API_RETURN(status);
            }
            if ( strncmp("Ethernet", name->chardata, 8)!=0 )
            {
				snprintf(system_cmd, sizeof(system_cmd), "ip link add name %s type dummy", name->chardata);
				ret = system(system_cmd);
				if (0 != ret) {
					MRVL_SAI_LOG_WRN("create rif netdev %s failed, mybe already exist", name->chardata);
				}
            }
            else
            {
            	MRVL_SAI_LOG_WRN("create rif netdev with name %s since it can be a port ignored", name->chardata);
            }


        } else if (SAI_OBJECT_TYPE_PORT == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_PORT, &rif_data))) {
                MRVL_SAI_API_RETURN(status);
            }
    		if ( read_fd_num == 0)
    		{
    			memset(host_fd, 0, sizeof(host_fd));
    		    FD_ZERO(&read_fd_set);
    		}
            /* create netdev  if not exist */
            memcpy(devname, name->chardata, 32);
            if ( strcmp(devname, host_fd[rif_data].name ) == 0)
            {
            	fd = host_fd[rif_data].fd;
            }
            else
            {
            	fd = mrvl_sai_netdev_alloc(devname);
            }
    		if ( mrvl_sai_netdev_get_mac(base_mac_dev, srcMac) == 0 )
    		{
    			if ((mrvl_sai_netdev_get_mac(devname, exist_src_mac) != 0) ||
    				memcmp(exist_src_mac, srcMac, sizeof(sai_mac_t)) !=0  )
    			{
    				mrvl_sai_netdev_set_mac(devname, srcMac);
    			}
    		}

    		host_fd[rif_data].fd = fd;
    		memcpy(host_fd[rif_data].name, devname, 32);
    		pthread_mutex_lock(&lock);
    		FD_SET(fd, &read_fd_set);
    		pthread_mutex_unlock(&lock);
    		host_fd[rif_data].valid = 1;
            /* default port oper status should be down */
            mrvl_sai_netdev_set_carrier(rif_data,0);
    		if ( read_fd_num == 0)
    		{
    			err = pthread_create(&thread_tap, NULL, mrvl_sai_tap_listen, NULL);
    			MRVL_SAI_LOG_NTC("Created tap listen thread err %d \n", err);
    			err = pthread_create(&thread_asic, NULL, mrvl_sai_asic_listen, NULL);
    			MRVL_SAI_LOG_NTC("Created asic listen thread err %d \n", err);
    		}
  			read_fd_num++;
        } else {
            MRVL_SAI_LOG_ERR("Invalid rif port object type %s", SAI_TYPE_STR(sai_object_type_query(rif_port->oid)));
            status = SAI_STATUS_INVALID_ATTR_VALUE_0 + rif_port_index;
            MRVL_SAI_API_RETURN(status);
        }
    } else if (SAI_HOSTIF_TYPE_FD == type->s32) {
    	 MRVL_SAI_LOG_ERR(" SAI_HOSTIF_TYPE_FD not supported\n");
    } else {
        MRVL_SAI_LOG_ERR("Invalid host interface type %d\n", type->s32);
        status = SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
        MRVL_SAI_API_RETURN(status);
    }
   
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HOSTIF, next_id++, hif_id))) {
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_host_interface_key_to_str(*hif_id, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);
    
    snprintf(port_system_cmd, sizeof(port_system_cmd), "ip link set %s up", name->chardata);
    ret = system(port_system_cmd);
    if (0 != ret) {
        MRVL_SAI_LOG_WRN("Setting port %s up failed\n", name->chardata);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**

 * @brief Remove host interface
 *
 * @param[in] hif_id Host interface id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 
 */

sai_status_t mrvl_sai_remove_host_interface(_In_ sai_object_id_t hif_id)
{
    char         key_str[MAX_KEY_STR_LEN];
    uint32_t     hif_data;
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_host_interface_key_to_str(hif_id, key_str);
    MRVL_SAI_LOG_NTC("Remove host interface %s\n", key_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(hif_id, SAI_OBJECT_TYPE_HOSTIF, &hif_data))) {
    	MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**


 * @brief Set host interface attribute
 *
 * @param[in] hif_id Host interface id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_set_host_interface_attribute(_In_ sai_object_id_t hif_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = hif_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t		   status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_host_interface_key_to_str(hif_id, key_str);
    status =  mrvl_sai_utl_set_attribute(&key, key_str, host_interface_attribs, host_interface_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**


 * @brief Get host interface attribute
 *
 * @param[in] hif_id Host interface id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error

 */

sai_status_t mrvl_sai_get_host_interface_attribute(_In_ sai_object_id_t     hif_id,
                                               _In_ uint32_t            attr_count,
                                               _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = hif_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t			status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_host_interface_key_to_str(hif_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key,
                              key_str,
                              host_interface_attribs,
                              host_interface_vendor_attribs,
                              attr_count,
                              attr_list);
    MRVL_SAI_API_RETURN(status);
}

/* Type [sai_host_interface_type_t] */
sai_status_t mrvl_sai_host_interface_type_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    MRVL_SAI_LOG_ENTER();

    value->s32 = SAI_HOSTIF_TYPE_NETDEV;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Assosiated port or router interface [sai_object_id_t] */
sai_status_t mrvl_sai_host_interface_rif_port_get(_In_ const sai_object_key_t   *key,
                                              _Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t                  attr_index,
                                              _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &value->oid))) {
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Name [char[HOST_INTERFACE_NAME_SIZE]] (MANDATORY_ON_CREATE)
 * The maximum number of charactars for the name is HOST_INTERFACE_NAME_SIZE - 1 since
 * it needs the terminating null byte ('\0') at the end.  */
sai_status_t mrvl_sai_host_interface_name_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    uint32_t     hif_id;
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hif_id))) {
        return status;
    }

    strncpy(value->chardata, "name", SAI_HOSTIF_NAME_SIZE);

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Name [char[HOST_INTERFACE_NAME_SIZE]]
 * The maximum number of charactars for the name is HOST_INTERFACE_NAME_SIZE - 1 since
 * it needs the terminating null byte ('\0') at the end.  */
sai_status_t mrvl_sai_host_interface_name_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{
    uint32_t     hif_id;
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hif_id))) {
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_host_interface_oper_get(_In_ const sai_object_key_t   *key,
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

sai_status_t mrvl_sai_host_interface_oper_set(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{
    MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_host_interface_vlan_tag_set(_In_ const sai_object_key_t      *key,
                                                  _In_ const sai_attribute_value_t *value,
                                                  void                             *arg)
{
    uint32_t        hostif_idx;
    sai_status_t    status;
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_idx))) {
    	MRVL_SAI_API_RETURN(status);
    }

    if (!host_fd[hostif_idx].valid)
    {
        MRVL_SAI_LOG_ERR("Host interface %d is invalid\n", hostif_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    MRVL_SAI_LOG_EXIT(); 
    return SAI_STATUS_SUCCESS;
}

/* convert trap entry type from SAI to FPA */
static sai_status_t mrvl_sai_host_interface_trap_conv_entry_type(_In_  sai_hostif_trap_type_t hostif_trapid,                                                               
                                                                 _Out_ FPA_CONTROL_PKTS_ENTRY_TYPE_ENT *entry_type)
{
    switch(hostif_trapid) {
        case SAI_HOSTIF_TRAP_TYPE_STP:
        case SAI_HOSTIF_TRAP_TYPE_LACP:
        case SAI_HOSTIF_TRAP_TYPE_LLDP:
            *entry_type = FPA_CONTROL_PKTS_TYPE_IEEE_RESERVED_MULTICAST_E;                      
            break;

        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY:
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE:
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT:
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT:
        case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT:
            *entry_type = FPA_CONTROL_PKTS_TYPE_IGMP_CTRL_MESSAGES_E;                    
            break;

        case SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST:
        case SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE:
            *entry_type = FPA_CONTROL_PKTS_TYPE_ARP_REQUEST_MESSAGES_E;       
            break;

        case SAI_HOSTIF_TRAP_TYPE_DHCP:
            *entry_type = FPA_CONTROL_PKTS_TYPE_UDP_BROADCAST_CTRL_E;
            break;

        case SAI_HOSTIF_TRAP_TYPE_OSPF:
        case SAI_HOSTIF_TRAP_TYPE_PIM:
        case SAI_HOSTIF_TRAP_TYPE_VRRP:
        case SAI_HOSTIF_TRAP_TYPE_BGP:
            *entry_type = FPA_CONTROL_PKTS_TYPE_PROPRIETRY_L2_MULTICAST_E;               
            break;

        case SAI_HOSTIF_TRAP_TYPE_DHCPV6:
        case SAI_HOSTIF_TRAP_TYPE_OSPFV6:
        case SAI_HOSTIF_TRAP_TYPE_VRRPV6:
        case SAI_HOSTIF_TRAP_TYPE_BGPV6:
        case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_V2:
        case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_REPORT:
        case SAI_HOSTIF_TRAP_TYPE_IPV6_MLD_V1_DONE:
        case SAI_HOSTIF_TRAP_TYPE_MLD_V2_REPORT:
            *entry_type = FPA_CONTROL_PKTS_TYPE_IPV6_MLD_ICMP_MESSAGES_E;        
            break;    

        case SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET: /* TODO: Vitally need to define */
        case SAI_HOSTIF_TRAP_TYPE_EAPOL: /* TODO: Vitally need to define */
        case SAI_HOSTIF_TRAP_TYPE_PVRST: /* TODO: Vitally need to define */
        case SAI_HOSTIF_TRAP_TYPE_IPV6_NEIGHBOR_DISCOVERY: /* TODO: Phase 2 */
        case SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR: /* TODO: Vitally need to define */
        case SAI_HOSTIF_TRAP_TYPE_TTL_ERROR: /* TODO: Vitally need to define */
        default:
            MRVL_SAI_LOG_ERR("host trap id %d is not supported\n", hostif_trapid);        
            return SAI_STATUS_NOT_SUPPORTED;
        }
    return SAI_STATUS_SUCCESS;
}


/* check validity for trap entry type attribute port/vlan_list */
sai_status_t mrvl_sai_host_interface_trap_is_port_vlan_supported(_In_  sai_hostif_trap_type_t hostif_trapid,                                                               
                                                                 _Out_ bool *isSupported)
{
    switch(hostif_trapid) {
    case SAI_HOSTIF_TRAP_TYPE_STP:
    case SAI_HOSTIF_TRAP_TYPE_LACP:
    case SAI_HOSTIF_TRAP_TYPE_LLDP:
    case SAI_HOSTIF_TRAP_TYPE_OSPF:
    case SAI_HOSTIF_TRAP_TYPE_PIM:
    case SAI_HOSTIF_TRAP_TYPE_VRRP:
    case SAI_HOSTIF_TRAP_TYPE_BGP:
        *isSupported = false;
        break;
    default: 
        *isSupported = true;
        break;
    }

    return SAI_STATUS_SUCCESS;
}

/* check validity for trap entry type attribute port/vlan_list then convert form SAI to FPA */
static sai_status_t mrvl_sai_host_interface_trap_check_entry_type(_In_  sai_hostif_trap_type_t hostif_trapid,                                                               
                                                                  _Out_ FPA_CONTROL_PKTS_ENTRY_TYPE_ENT *entry_type)
{
    sai_status_t status; 
    bool isSuppoted;  
 
    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_host_interface_trap_is_port_vlan_supported(hostif_trapid,&isSuppoted))) {
        MRVL_SAI_LOG_ERR("Failed to check validity for trap entry attribute port/vlan\n");
        return status;
    }
    
    if (isSuppoted == false) {
        MRVL_SAI_LOG_ERR("port/vlan list attribute is not relevant for host trap id %d\n", hostif_trapid);        
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    return mrvl_sai_host_interface_trap_conv_entry_type(hostif_trapid,entry_type);    
}

/* set default trap action command for SAI */
static sai_status_t mrvl_sai_host_interface_trap_set_default_action(_In_ sai_hostif_trap_type_t hostif_trapid,                                                               
                                                                    _Inout_ FPA_FLOW_TABLE_ENTRY_STC *flowEntry)                                                                
{  
    switch(hostif_trapid) {
    case SAI_HOSTIF_TRAP_TYPE_STP:
    case SAI_HOSTIF_TRAP_TYPE_LACP:
    case SAI_HOSTIF_TRAP_TYPE_EAPOL: 
    case SAI_HOSTIF_TRAP_TYPE_LLDP:
    case SAI_HOSTIF_TRAP_TYPE_PVRST:
        /* trap action command drop */
        flowEntry->data.control_pkt.clearActions = 1; /* drop */
        flowEntry->data.control_pkt.outputPort = 0;   /* not trap */
        flowEntry->data.control_pkt.gotoTableNo = 0;  
        break;

    case SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET:
    case SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR:
    case SAI_HOSTIF_TRAP_TYPE_TTL_ERROR: 
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY: /* TODO: temp vitaly need to change default action for IGMP */
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE:
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT:
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT:
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT:

        /* trap action command trap */
        flowEntry->data.control_pkt.clearActions = 0; /* not drop */
        flowEntry->data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;   /* trap */
        flowEntry->data.control_pkt.gotoTableNo = 0;  
        break;

    default:
        /* trap action command forward */
        flowEntry->data.control_pkt.clearActions = 0; /* not drop */
        flowEntry->data.control_pkt.outputPort = 0;   /* not trap */
        flowEntry->data.control_pkt.gotoTableNo = FPA_FLOW_TABLE_TYPE_VLAN_E;  
        break;
    }

    return SAI_STATUS_SUCCESS;
}

/* get default trap action command for SAI */
sai_status_t mrvl_sai_host_interface_trap_get_default_action(_In_ sai_hostif_trap_type_t hostif_trapid,                                                               
                                                             _Out_ sai_packet_action_t *action)                                                                
{  
    switch(hostif_trapid) {
    case SAI_HOSTIF_TRAP_TYPE_STP:
    case SAI_HOSTIF_TRAP_TYPE_LACP:
    case SAI_HOSTIF_TRAP_TYPE_EAPOL: 
    case SAI_HOSTIF_TRAP_TYPE_LLDP:
    case SAI_HOSTIF_TRAP_TYPE_PVRST:

        *action = SAI_PACKET_ACTION_DROP;
        break;        

    case SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET:
    case SAI_HOSTIF_TRAP_TYPE_L3_MTU_ERROR:
    case SAI_HOSTIF_TRAP_TYPE_TTL_ERROR: 
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_QUERY: /* TODO: temp vitaly need to change default action for IGMP */
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_LEAVE:
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V1_REPORT:
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V2_REPORT:
    case SAI_HOSTIF_TRAP_TYPE_IGMP_TYPE_V3_REPORT:

        *action = SAI_PACKET_ACTION_TRAP;
        break;      

    default:
        *action = SAI_PACKET_ACTION_FORWARD;
        break;      
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_host_interface_trap_flow_entry_set(_In_ sai_hostif_trap_type_t hostif_trapid,                                                               
                                                                _Inout_ FPA_FLOW_TABLE_ENTRY_STC *flowEntry)                                                                
{
    sai_status_t status;   

    flowEntry->data.control_pkt.outputPort = FPA_OUTPUT_CONTROLLER;    
    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_host_interface_trap_conv_entry_type(hostif_trapid,&flowEntry->data.control_pkt.entry_type))) {
        MRVL_SAI_LOG_ERR("Failed to convert trap entry type\n");
        return status;
    }

    switch (flowEntry->data.control_pkt.entry_type) {
    case FPA_CONTROL_PKTS_TYPE_PROPRIETRY_L2_MULTICAST_E:
        flowEntry->data.control_pkt.match.dstMacMask.addr[0] = 0xFF;
        flowEntry->data.control_pkt.match.dstMacMask.addr[1] = 0xFF;
        flowEntry->data.control_pkt.match.dstMacMask.addr[2] = 0xFF;
        flowEntry->data.control_pkt.match.dstMacMask.addr[3] = 0;
        flowEntry->data.control_pkt.match.dstMacMask.addr[4] = 0;
        flowEntry->data.control_pkt.match.dstMacMask.addr[5] = 0;
        flowEntry->data.control_pkt.match.dstMac.addr[0] = 0x01;
        flowEntry->data.control_pkt.match.dstMac.addr[1] = 0;
        flowEntry->data.control_pkt.match.dstMac.addr[2] = 0x0C;
        flowEntry->data.control_pkt.match.inPortMask = 0;
        flowEntry->data.control_pkt.match.vlanIdMask = 0;
        flowEntry->data.control_pkt.match.ipProtocolMask = 0;
        flowEntry->data.control_pkt.match.etherTypeMask = 0;
        flowEntry->data.control_pkt.match.dstL4PortMask = 0;
        flowEntry->data.control_pkt.match.icmpV6TypeMask = 0;

        break;

    case FPA_CONTROL_PKTS_TYPE_IEEE_RESERVED_MULTICAST_E:
        memset(&flowEntry->data.control_pkt.match.dstMacMask, 0xFF, 6);
        flowEntry->data.control_pkt.match.dstMac.addr[0] = 0x01;
        flowEntry->data.control_pkt.match.dstMac.addr[1] = 0x80;
        flowEntry->data.control_pkt.match.dstMac.addr[2] = 0xC2;
        flowEntry->data.control_pkt.match.dstMac.addr[3] = 0;
        flowEntry->data.control_pkt.match.dstMac.addr[4] = 0;
        flowEntry->data.control_pkt.match.inPortMask = 0;
        flowEntry->data.control_pkt.match.vlanIdMask = 0;
        flowEntry->data.control_pkt.match.ipProtocolMask = 0;
        flowEntry->data.control_pkt.match.etherTypeMask = 0;
        flowEntry->data.control_pkt.match.dstL4PortMask = 0;
        flowEntry->data.control_pkt.match.icmpV6TypeMask = 0;

        if (hostif_trapid == SAI_HOSTIF_TRAP_TYPE_LLDP) 
            flowEntry->data.control_pkt.match.dstMac.addr[5] = 0xE;
        else if(hostif_trapid == SAI_HOSTIF_TRAP_TYPE_STP) 
            flowEntry->data.control_pkt.match.dstMac.addr[5] = 0;
        else if(hostif_trapid == SAI_HOSTIF_TRAP_TYPE_LACP) 
            flowEntry->data.control_pkt.match.dstMac.addr[5] = 2;

        break;

    case FPA_CONTROL_PKTS_TYPE_IGMP_CTRL_MESSAGES_E:
        memset(&flowEntry->data.control_pkt.match.dstMacMask, 0, 6);
        flowEntry->data.control_pkt.match.etherType = 0x800;
        flowEntry->data.control_pkt.match.etherTypeMask = 0x800;
        flowEntry->data.control_pkt.match.ipProtocol = 2;
        flowEntry->data.control_pkt.match.ipProtocolMask = 2;
        flowEntry->data.control_pkt.match.dstL4PortMask = 0; 
        flowEntry->data.control_pkt.match.icmpV6TypeMask = 0;           
        if (flowEntry->data.control_pkt.match.inPortMask != 0) { /* port mode */
            flowEntry->priority = 0xFFFFFFFF;
        }
        else { /* vlan mode */
            flowEntry->priority = 0;
        }

        break;

    case FPA_CONTROL_PKTS_TYPE_IPV6_MLD_ICMP_MESSAGES_E:
        memset(&flowEntry->data.control_pkt.match.dstMacMask, 0, 6);
        flowEntry->data.control_pkt.match.etherType = 0x86dd;
        flowEntry->data.control_pkt.match.etherTypeMask = 0x86dd;
        flowEntry->data.control_pkt.match.ipProtocol = 58;
        flowEntry->data.control_pkt.match.ipProtocolMask = 58;
        flowEntry->data.control_pkt.match.dstL4PortMask = 0; 
        if (flowEntry->data.control_pkt.match.inPortMask != 0) { /* port mode */
            flowEntry->priority = 0xFFFFFFFF;
        }
        else { /* vlan mode */
            flowEntry->priority = 0;
        }
        
        break;

    case FPA_CONTROL_PKTS_TYPE_UDP_BROADCAST_CTRL_E:
        memset(&flowEntry->data.control_pkt.match.dstMacMask, 0xFF, 6);
        memset(&flowEntry->data.control_pkt.match.dstMac, 0xFF, 6);
        flowEntry->data.control_pkt.match.etherType = 0x800;
        flowEntry->data.control_pkt.match.etherTypeMask = 0x800;
        flowEntry->data.control_pkt.match.ipProtocol = 17;
        flowEntry->data.control_pkt.match.ipProtocolMask = 17;
        flowEntry->data.control_pkt.match.icmpV6TypeMask = 0; 

        break;

    case FPA_CONTROL_PKTS_TYPE_ARP_REQUEST_MESSAGES_E:
        memset(&flowEntry->data.control_pkt.match.dstMacMask, 0xFF, 6);
        memset(&flowEntry->data.control_pkt.match.dstMac, 0xFF, 6);
        flowEntry->data.control_pkt.match.etherType = 0x806;
        flowEntry->data.control_pkt.match.etherTypeMask = 0x806;        
        flowEntry->data.control_pkt.match.ipProtocolMask = 0;
        flowEntry->data.control_pkt.match.icmpV6TypeMask = 0; 
        flowEntry->data.control_pkt.match.dstL4PortMask = 0; 
        if (flowEntry->data.control_pkt.match.inPortMask != 0) { /* port mode */
            flowEntry->priority = 0xFFFFFFFF;
        }
        else { /* vlan mode */
            flowEntry->priority = 0;
        }

        break;

    case FPA_CONTROL_PKTS_TYPE_RIPV1_CTRL_MESSAGES_E:
        memset(&flowEntry->data.control_pkt.match.dstMacMask, 0xFF, 6);
        memset(&flowEntry->data.control_pkt.match.dstMac, 0xFF, 6);
        flowEntry->data.control_pkt.match.etherType = 0x800;
        flowEntry->data.control_pkt.match.etherTypeMask = 0x800;
        flowEntry->data.control_pkt.match.ipProtocol = 17;        
        flowEntry->data.control_pkt.match.ipProtocolMask = 17;
        flowEntry->data.control_pkt.match.icmpV6TypeMask = 0;
        flowEntry->data.control_pkt.match.dstL4Port = 520;  
        flowEntry->data.control_pkt.match.dstL4PortMask = 520; 

        break;

    default:
        MRVL_SAI_LOG_ERR("entry type %d is invalid\n",flowEntry->data.control_pkt.entry_type);
        return SAI_STATUS_INVALID_PARAMETER;

        break;
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_host_interface_trap_set_default_action(hostif_trapid,flowEntry))) {
        MRVL_SAI_LOG_ERR("Failed to set default trap action command\n");
        return status;
    }
    
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_host_interface_trap_action_set(_In_ sai_hostif_trap_type_t hostif_traptype,
                                                            _In_ const sai_attribute_value_t *value,
                                                            _In_ FPA_FLOW_TABLE_ENTRY_STC *flowEntry,
                                                            _In_ uint32_t intf_type,
                                                            _In_ uint32_t portVlan)
{     
    FPA_STATUS fpa_status;   
    uint64_t cookie; 
    sai_consecutive_hostif_trap_type_t internal_hostif_traptype;     

    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_traptype,internal_hostif_traptype);
      
    cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(flowEntry->data.control_pkt.entry_type, intf_type, portVlan, 0 /*data*/);
    flowEntry->cookie = cookie; 

    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, flowEntry);
    if (fpa_status == FPA_NOT_FOUND) {
        MRVL_SAI_LOG_ERR("fpa CONTROL table index %llx do not exist\n", cookie);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }       

    /* trap action [sai_packet_action_t] */   
    switch (mrvl_sai_hostif_trap_table[internal_hostif_traptype].action) {
    case SAI_PACKET_ACTION_DROP:    /** Drop Packet in data plane */   
    case SAI_PACKET_ACTION_DENY:    /** This is a combination of sai packet action COPY_CANCEL and DROP */
        flowEntry->data.control_pkt.clearActions = 1; /* drop */
        flowEntry->data.control_pkt.outputPort = 0;   /* not trap */
        flowEntry->data.control_pkt.gotoTableNo = 0;  
        break;
    case SAI_PACKET_ACTION_COPY:    /** Copy Packet to CPU. */
    case SAI_PACKET_ACTION_TRAP:    /** This is a combination of sai packet action COPY and DROP. */
        flowEntry->data.control_pkt.clearActions = 0; /* not drop */
        flowEntry->data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;   /* trap */
        flowEntry->data.control_pkt.gotoTableNo = 0;  
        break;
    case SAI_PACKET_ACTION_LOG:     /** This is a combination of sai packet action COPY and FORWARD. */
         flowEntry->data.control_pkt.outputPort = SAI_OUTPUT_CONTROLLER;   /* trap */
         /* continue to forward*/   
    case SAI_PACKET_ACTION_FORWARD:
        flowEntry->data.control_pkt.clearActions = 0; /* not drop */
        flowEntry->data.control_pkt.outputPort = 0;   /* not trap */
        flowEntry->data.control_pkt.gotoTableNo = FPA_FLOW_TABLE_TYPE_VLAN_E;  
        break;
    case SAI_PACKET_ACTION_COPY_CANCEL:  /** Cancel copy the packet to CPU. */
    case SAI_PACKET_ACTION_TRANSIT:      /** This is a combination of sai packet action COPY_CANCEL and FORWARD */
    default:         
        MRVL_SAI_LOG_ERR("action %d is not supported\n", value->s32);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS,FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, flowEntry, 0);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to modify entry %llx in CONTROL table status = %d\n", flowEntry->cookie, fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    return SAI_STATUS_SUCCESS;
}

inline static sai_status_t mrvl_sai_host_interface_trap_list_prv_set(_Inout_ FPA_FLOW_TABLE_ENTRY_STC *flowEntry,
                                                                     _In_ uint32_t intf_type,
                                                                     _In_ uint32_t portVlan)
{
    if (intf_type == 0) {    
            flowEntry->data.control_pkt.match.inPort = portVlan;
            flowEntry->data.control_pkt.match.inPortMask = 0xFFFFFFFF; 
            flowEntry->data.control_pkt.match.vlanIdMask = 0;       
    }
    else {
        flowEntry->data.control_pkt.match.vlanId = portVlan;
        flowEntry->data.control_pkt.match.vlanIdMask = 0xFFFF; 
        flowEntry->data.control_pkt.match.inPortMask = 0;       
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_host_interface_trap_list_set(_In_ sai_hostif_trap_type_t hostif_trapid,                                                               
                                                          _Inout_ FPA_FLOW_TABLE_ENTRY_STC *flowEntry,
                                                          _In_ uint32_t intf_type,
                                                          _In_ uint32_t portVlan)
{     
    FPA_STATUS fpa_status;   
    uint64_t cookie;    
    sai_status_t status;    

    cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(flowEntry->data.control_pkt.entry_type, intf_type, portVlan, 0 /*data*/);
    flowEntry->cookie = cookie; 

    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, flowEntry);
    if (fpa_status == FPA_NOT_FOUND) {
         /* init control pkt entry */ 
        fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, flowEntry);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to init CONTROL_PKT entry status = %d\n", fpa_status);            
            return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        }           
   
        flowEntry->cookie = cookie; 

        mrvl_sai_host_interface_trap_list_prv_set(flowEntry,intf_type,portVlan);        

        /* set default control pkt entry */
        status = mrvl_sai_host_interface_trap_flow_entry_set(hostif_trapid,flowEntry);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Failed to set default attributes for trap entry id %d\n",hostif_trapid);
            return status;
        }
                
         /* add control pkt entry */ 
        fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, flowEntry); 
        if ((fpa_status != FPA_OK) && (fpa_status != FPA_ALREADY_EXIST)){           
            MRVL_SAI_LOG_ERR("Failed to add entry %llx to CONTROL_PKT table status = %d\n", cookie, fpa_status);
            return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        }

    }
    else
    {
        mrvl_sai_host_interface_trap_list_prv_set(flowEntry,intf_type,portVlan);

        /* update existing control pkt entry*/
        fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS,FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, flowEntry, 0);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to modify entry %llx in CONTROL table status = %d\n", flowEntry->cookie, fpa_status);
            return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        }
    }

    return SAI_STATUS_SUCCESS;
}

/** trap action [sai_packet_action_t] */
sai_status_t mrvl_hostif_trap_action_set(_In_ const sai_object_key_t *key,
                                         _In_ const sai_attribute_value_t *value,
                                         void *arg)
{
    sai_status_t status;       
    sai_hostif_trap_type_t hostif_trapid;   
    uint32_t portVlan, i;
    FPA_FLOW_TABLE_ENTRY_STC flowEntry;
    sai_packet_action_t old_action;
    mrvl_sai_hostif_list_entry_t *entryPtr;  
    mrvl_sai_utl_dlist_elem_STC *element;  
    sai_consecutive_hostif_trap_type_t internal_hostif_trapid; 

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_trapid))) {
    	MRVL_SAI_API_RETURN(status);
    }
    
    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_trapid,internal_hostif_trapid); 

    /* convert trap entry type from SAI to FPA */
    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_sai_host_interface_trap_conv_entry_type(hostif_trapid,&flowEntry.data.control_pkt.entry_type))) {
        MRVL_SAI_LOG_ERR("convert failed for trap id 0x%x\n",hostif_trapid);
        return status;
    }      

    switch(hostif_trapid) {    
    case SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST:   
    case SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE: 
        /* do nothing */       
        return SAI_STATUS_SUCCESS;
        break;
    case SAI_HOSTIF_TRAP_TYPE_STP:
    case SAI_HOSTIF_TRAP_TYPE_LACP:
    case SAI_HOSTIF_TRAP_TYPE_LLDP:  
    case SAI_HOSTIF_TRAP_TYPE_OSPF:
    case SAI_HOSTIF_TRAP_TYPE_PIM:
    case SAI_HOSTIF_TRAP_TYPE_VRRP:
    case SAI_HOSTIF_TRAP_TYPE_BGP:               
        portVlan = 0; /* not relevant */  
        if (mrvl_sai_hostif_trap_table[internal_hostif_trapid].action != value->s32) {
            /* update with new action */
            mrvl_sai_hostif_trap_table[internal_hostif_trapid].action = value->s32;            
            if (mrvl_sai_hostif_trap_table[internal_hostif_trapid].valid.port == false)
            {
                 /* init port list db */
                mrvl_sai_hostif_trap_table[internal_hostif_trapid].valid.port = true;
                mrvl_sai_utl_dlist_init(&mrvl_sai_hostif_trap_table[internal_hostif_trapid].hostif_portlist_header);
                /* insert single element port 0 as system configuration */        
                entryPtr = (mrvl_sai_hostif_list_entry_t *)malloc(sizeof(mrvl_sai_hostif_list_entry_t));
                if (entryPtr == NULL) {
                    MRVL_SAI_LOG_ERR("entry allocation failure for trapid[0x%x]\n",hostif_trapid);
                    return SAI_STATUS_INSUFFICIENT_RESOURCES;
                }
                entryPtr->data = 0;                        
                mrvl_sai_utl_dlist_insert_to_head(&mrvl_sai_hostif_trap_table[internal_hostif_trapid].hostif_portlist_header,&entryPtr->list); 
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_list_set(hostif_trapid, &flowEntry, 0 /* port */, entryPtr->data))) {
                return status;
                }
            }
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_action_set(hostif_trapid, value, &flowEntry, 0 /* port */, portVlan))) {
                return status;
            } 
        }        
        return SAI_STATUS_SUCCESS;
        break;
  
    default:
        /* do nothing */
        break;
    }

    /* save old action value */
    old_action = mrvl_sai_hostif_trap_table[internal_hostif_trapid].action;
    if (mrvl_sai_hostif_trap_table[internal_hostif_trapid].action != value->s32) {
        /* update new action in db */
        mrvl_sai_hostif_trap_table[internal_hostif_trapid].action = value->s32; 
    }

    /* user set trap action attribute before port_list/vlan_list trap attribute */
    if (mrvl_sai_hostif_trap_table[internal_hostif_trapid].portList_count == 0) {
        mrvl_sai_hostif_trap_table[internal_hostif_trapid].action_not_set.port = true;
    }
    if (mrvl_sai_hostif_trap_table[internal_hostif_trapid].vlanList_count == 0) {
        mrvl_sai_hostif_trap_table[internal_hostif_trapid].action_not_set.vlan = true;
    }

    /* go over all the ports in the db and update with new action */
    if ((mrvl_sai_hostif_trap_table[internal_hostif_trapid].portList_count != 0) && (old_action != value->s32)) {
        element = mrvl_sai_utl_dlist_get_first(&mrvl_sai_hostif_trap_table[internal_hostif_trapid].hostif_portlist_header);
        entryPtr = (mrvl_sai_hostif_list_entry_t*)element;                       
        for (i=0; i<mrvl_sai_hostif_trap_table[internal_hostif_trapid].portList_count; i++){                          
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_action_set(hostif_trapid, value, 
                                                                                        &flowEntry,  0 /* port */, entryPtr->data))) {
                return status;
            }  
            element = mrvl_sai_utl_dlist_get_next(element);
            entryPtr = (mrvl_sai_hostif_list_entry_t*)element;              
        }             
    }          

    /* go over all the vlans in the db and update with new action */
    if ((mrvl_sai_hostif_trap_table[internal_hostif_trapid].vlanList_count != 0) && (old_action != value->s32)) {
        element = mrvl_sai_utl_dlist_get_first(&mrvl_sai_hostif_trap_table[internal_hostif_trapid].hostif_vlanlist_header);
        entryPtr = (mrvl_sai_hostif_list_entry_t*)element;                       
        for (i=0; i<mrvl_sai_hostif_trap_table[internal_hostif_trapid].vlanList_count; i++){                          
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_action_set(hostif_trapid, value, 
                                                                                        &flowEntry,  1 /* vlan */, entryPtr->data))) {
                return status;
            }  
            element = mrvl_sai_utl_dlist_get_next(element);  
            entryPtr = (mrvl_sai_hostif_list_entry_t*)element;            
        }             
    }     

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}

/** trap action [sai_packet_action_t] */
sai_status_t mrvl_hostif_trap_action_get(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{    
    sai_consecutive_hostif_trap_type_t internal_hostif_traptype; 
    sai_hostif_trap_type_t hostif_traptype; 
    sai_status_t status;    

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_traptype))) {
        MRVL_SAI_API_RETURN(status);
    }

    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_traptype,internal_hostif_traptype);   

    if (hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST || hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE) {
        value->s32 = SAI_PACKET_ACTION_TRAP;
    }
    else
    {
        value->s32 = mrvl_sai_hostif_trap_table[internal_hostif_traptype].action;    
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}

static sai_status_t mrvl_sai_host_interface_trap_list_delete(_In_ FPA_FLOW_TABLE_ENTRY_STC flowEntry,
                                                             _In_ uint32_t intf_type,
                                                             _In_ uint32_t portVlan)
{     
    FPA_STATUS fpa_status;   
    uint64_t cookie;    

    cookie = MRVL_SAI_HOSTIF_CREATE_COOKIE_MAC(flowEntry.data.control_pkt.entry_type, intf_type, portVlan, 0 /*data*/);    

    flowEntry.cookie = cookie;

    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, &flowEntry);
    if (fpa_status == FPA_NOT_FOUND) {
        MRVL_SAI_LOG_ERR("fpa CONTROL table index %llx do not exist\n", cookie);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }   

    fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E, cookie);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to delete cookie %llx in CONTROL table status = %d\n", flowEntry.cookie, fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_hostif_trap_list_set(_In_ sai_hostif_trap_type_t hostif_trapid,
                                              _In_ const sai_attribute_value_t *value,                                             
                                              _Inout_ uint32_t *list_count,
                                              _Inout_ mrvl_sai_utl_dlist_elem_STC *list_header,
                                              _In_ uint32_t intf_type)
{
    sai_status_t status;            
    uint32_t i;
    FPA_FLOW_TABLE_ENTRY_STC flowEntry;    
    mrvl_sai_hostif_list_entry_t *entryPtr;  
    mrvl_sai_utl_dlist_elem_STC *element,*tmp_element;
    uint32_t elementFound, list_counter;    
    bool action_not_set;
    bool init_db;
    sai_consecutive_hostif_trap_type_t internal_hostif_trapid; 
    
    MRVL_SAI_LOG_ENTER();    

    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_trapid,internal_hostif_trapid);
    
    /* check validity for trap entry type attribute port/vlan_list then convert form SAI to FPA */
    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_sai_host_interface_trap_check_entry_type(hostif_trapid,&flowEntry.data.control_pkt.entry_type))) {
        MRVL_SAI_LOG_ERR("Validity check failed for trap id %d\n",hostif_trapid);
        return status;
    }      
       
    init_db = false;

    if (intf_type == 0 /* port */ && mrvl_sai_hostif_trap_table[internal_hostif_trapid].valid.port == false) {
        /* init port list db */
        mrvl_sai_hostif_trap_table[internal_hostif_trapid].valid.port = true;
        init_db = true;
    }
    if (intf_type == 1 /* vlan */ && mrvl_sai_hostif_trap_table[internal_hostif_trapid].valid.vlan == false) {
        /* init vlan list db */
        mrvl_sai_hostif_trap_table[internal_hostif_trapid].valid.vlan = true;
        init_db = true;
    }
    
    if (init_db == true) {
        init_db = false;
        *list_count = value->objlist.count;
        mrvl_sai_utl_dlist_init(list_header);
        /* insert elements */
        for (i=0; i<value->objlist.count; i++) {
            entryPtr = (mrvl_sai_hostif_list_entry_t *)malloc(sizeof(mrvl_sai_hostif_list_entry_t));
            if (entryPtr == NULL) {
                MRVL_SAI_LOG_ERR("entry allocation failure for trapid[%d]\n",hostif_trapid);
                return SAI_STATUS_INSUFFICIENT_RESOURCES;
            }
            entryPtr->data = value->objlist.list[i];                        
            mrvl_sai_utl_dlist_insert_sorted_element (list_header,
                                                      &entryPtr->list,
                                                      0,
                                                      offsetof(mrvl_sai_hostif_list_entry_t, data),
                                                      entryPtr->data);              
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_list_set(hostif_trapid, &flowEntry, intf_type, entryPtr->data))) {
                return status;
            }     
        }
    }
    else
    {
        /* delete all the elements */
        if (value->objlist.count == 0) {
            *list_count = 0;
            /* delete elements */
            while (mrvl_sai_utl_dlist_is_empty(list_header) == 0) {
				element = mrvl_sai_utl_dlist_remove_from_head(list_header);
				entryPtr = (mrvl_sai_hostif_list_entry_t*)element;
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_list_delete(flowEntry, intf_type, entryPtr->data))) {
                    return status;
                }     				
				free(element);                         
			}            
        }
        /* go over all the elements in the db and update them */  
        else
        {
            /* 1. insert new elements to the linked list */
            for (i=0; i<value->objlist.count; i++) {
                /* check if element already exist */
                element = mrvl_sai_utl_dlist_find_sorted_element(list_header,
                                                                 0,
                                                                 offsetof(mrvl_sai_hostif_list_entry_t, data),
                                                                 value->objlist.list[i],
                                                                 &elementFound);
                if (elementFound == false) {
                    entryPtr = (mrvl_sai_hostif_list_entry_t *)malloc(sizeof(mrvl_sai_hostif_list_entry_t));
                    if (entryPtr == NULL) {
                        MRVL_SAI_LOG_ERR("entry allocation failure for trapid[%d]\n",hostif_trapid);
                        return SAI_STATUS_INSUFFICIENT_RESOURCES;
                    }
                    (*list_count)++;
                    entryPtr->data = value->objlist.list[i];
                    entryPtr->valid = true;
                    mrvl_sai_utl_dlist_insert_before(element, &entryPtr->list);
                    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_list_set(hostif_trapid, &flowEntry, intf_type, entryPtr->data))) {
                        return status;
                    }  
                }
                else /* element was found */
                {
                    entryPtr = (mrvl_sai_hostif_list_entry_t*)element; 
                    entryPtr->valid = true;
                }
            }    
              
            /* 2. remove deleted elements from the linked list */
            element = mrvl_sai_utl_dlist_get_first(list_header);
            entryPtr = (mrvl_sai_hostif_list_entry_t*)element;    
            list_counter = *list_count;        
            for (i=0; i<list_counter; i++) {
                /* check if this element needs to be deleted */
                if (entryPtr->valid == false) 
                {
                    /* delete the element */
                    (*list_count)--;
                    if (*list_count > 0) {
                        tmp_element = mrvl_sai_utl_dlist_get_next(element);
                    }
                    mrvl_sai_utl_dlist_remove(element);
                    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_list_delete(flowEntry, intf_type, entryPtr->data))) {
                        return status;
                    } 
                    free(element);   
                    element = tmp_element; 
                    entryPtr = (mrvl_sai_hostif_list_entry_t*)element;   
                }
                else
                {
                    /* move to the next element */
                    entryPtr->valid = false;
                    element = mrvl_sai_utl_dlist_get_next(element); 
                    entryPtr = (mrvl_sai_hostif_list_entry_t*)element;            
                }
            }
        }
    }

    /* need to update also trap acion attribute */
    if (intf_type == 0 /* port */) {
        action_not_set = mrvl_sai_hostif_trap_table[internal_hostif_trapid].action_not_set.port;
    }
    else {
        action_not_set = mrvl_sai_hostif_trap_table[internal_hostif_trapid].action_not_set.vlan;    
    }

    if (action_not_set == true && *list_count != 0) {

        action_not_set = false;

        /* go over all the ports in the db and update with new action */
        element = mrvl_sai_utl_dlist_get_first(list_header);
        entryPtr = (mrvl_sai_hostif_list_entry_t*)element;                       
        for (i=0; i<*list_count; i++){                          
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_host_interface_trap_action_set(hostif_trapid, value, &flowEntry, intf_type, entryPtr->data))) {
                return status;
            }  
            element = mrvl_sai_utl_dlist_get_next(element); 
            entryPtr = (mrvl_sai_hostif_list_entry_t*)element;              
        }     
    }    
   
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}

#if 0
/** enable trap for a list of SAI ports [sai_object_list_t] */
sai_status_t mrvl_hostif_trap_port_list_set(_In_ const sai_object_key_t *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void *arg)
{
    sai_status_t status;  
    sai_consecutive_hostif_trap_type_t internal_hostif_traptype;  
    sai_hostif_trap_type_t hostif_traptype;       
    
    MRVL_SAI_LOG_ENTER(); 
       
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_traptype))) {
        MRVL_SAI_API_RETURN(status);
    }
                                             
    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_traptype,internal_hostif_traptype);

    if (hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST || hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE) {
        /* do nothing */
        return SAI_STATUS_SUCCESS;
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_hostif_trap_list_set(hostif_traptype, value, &mrvl_sai_hostif_trap_table[internal_hostif_traptype].portList_count,
                                      &mrvl_sai_hostif_trap_table[internal_hostif_traptype].hostif_portlist_header, 0 /* port */))) {
        MRVL_SAI_LOG_ERR("trap enable for port list failed for trap id %d\n",hostif_traptype);
        return status;
    }         
   
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}
#endif
static sai_status_t mrvl_hostif_trap_list_get(_Inout_ sai_attribute_value_t *value,
                                              _In_ uint32_t list_count,
                                              _In_ mrvl_sai_utl_dlist_elem_STC list_header)
{             
    mrvl_sai_hostif_list_entry_t *entryPtr;  
    mrvl_sai_utl_dlist_elem_STC *element;
    uint32_t i;
        
    value->objlist.count = list_count;
    element = mrvl_sai_utl_dlist_get_first(&list_header);
    entryPtr = (mrvl_sai_hostif_list_entry_t*)element;        
    for (i=0; i<list_count; i++) {
        value->objlist.list[i] = entryPtr->data;
        element = mrvl_sai_utl_dlist_get_next(element);
    }
          
    return SAI_STATUS_SUCCESS;

}
#if 0
/** enable trap for a list of SAI ports [sai_object_list_t] */
sai_status_t mrvl_hostif_trap_port_list_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{         
    sai_status_t status;  
    sai_consecutive_hostif_trap_type_t internal_hostif_traptype;  
    sai_hostif_trap_type_t hostif_traptype;       
    
    MRVL_SAI_LOG_ENTER();    
                                             
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_traptype))) {
        MRVL_SAI_API_RETURN(status);
    }

    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_traptype,internal_hostif_traptype);         

    if (hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST || hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE) {
        /* do nothing */
        return SAI_STATUS_NOT_SUPPORTED;
    }

    mrvl_hostif_trap_list_get(value,mrvl_sai_hostif_trap_table[internal_hostif_traptype].portList_count,
                              mrvl_sai_hostif_trap_table[internal_hostif_traptype].hostif_portlist_header);    
      
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;


}
#endif
/** enable trap for a list of SAI vlans [sai_object_list_t] */
sai_status_t mrvl_hostif_trap_vlan_list_set(_In_ const sai_object_key_t *key,
                                            _In_ const sai_attribute_value_t *value,
                                            void *arg)
{
    sai_status_t status;  
    sai_consecutive_hostif_trap_type_t internal_hostif_traptype = INTERNAL_SAI_HOSTIF_TRAP_TYPE_START;                 
    sai_hostif_trap_type_t hostif_traptype;       
    
    MRVL_SAI_LOG_ENTER();    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_traptype))) {
        MRVL_SAI_API_RETURN(status);
    }
                        
    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_traptype,internal_hostif_traptype);
             
    if (hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST || hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE) {
        /* do nothing */
        return SAI_STATUS_SUCCESS;
    }
                
    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_hostif_trap_list_set(hostif_traptype, value, &mrvl_sai_hostif_trap_table[internal_hostif_traptype].vlanList_count,
                                      &mrvl_sai_hostif_trap_table[internal_hostif_traptype].hostif_vlanlist_header, 1 /* vlan */))) {
        MRVL_SAI_LOG_ERR("trap enable for vlan list failed for trap id %d\n",hostif_traptype);
        return status;
    }  
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;          
}

/** enable trap for a list of SAI vlans [sai_object_list_t] */
sai_status_t mrvl_hostif_trap_vlan_list_get(_In_ const sai_object_key_t   *key,
                                            _Inout_ sai_attribute_value_t *value,
                                            _In_ uint32_t                  attr_index,
                                            _Inout_ vendor_cache_t        *cache,
                                            void                          *arg)
{         
    sai_status_t status;  
    sai_consecutive_hostif_trap_type_t internal_hostif_traptype;                 
    sai_hostif_trap_type_t hostif_traptype;       

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_HOSTIF, &hostif_traptype))) {
        MRVL_SAI_API_RETURN(status);
    }

    PRV_CONVERT_INTERNAL_TRAP_TYPE_MAC(hostif_traptype,internal_hostif_traptype);

    if (hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_REQUEST || hostif_traptype == SAI_HOSTIF_TRAP_TYPE_ARP_RESPONSE) {
        /* do nothing */
        return SAI_STATUS_NOT_SUPPORTED;
    }

    mrvl_hostif_trap_list_get(value,mrvl_sai_hostif_trap_table[internal_hostif_traptype].vlanList_count,
                              mrvl_sai_hostif_trap_table[internal_hostif_traptype].hostif_vlanlist_header);    
      
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}
#if 0
/** trap channel to use [sai_hostif_trap_channel_t] */
sai_status_t mrvl_hostif_trap_channel_set(_In_ const sai_object_key_t *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void *arg)
{       
    MRVL_SAI_LOG_ENTER();

    if (value->s32 != SAI_HOSTIF_TRAP_CHANNEL_NETDEV) {
        MRVL_SAI_LOG_ERR("Invalid host interface channel type %d\n", value->s32);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + value->s32;
    }    
       
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}

/** trap channel to use [sai_hostif_trap_channel_t] */
sai_status_t mrvl_hostif_trap_channel_get(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{             
    MRVL_SAI_LOG_ENTER();
    
    value->s32 = SAI_HOSTIF_TRAP_CHANNEL_NETDEV;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;

}
#endif

/**
 * Routine Description:
 *   @brief hostif receive function
 *
 * Arguments:
 *    @param[in]  hif_id  - host interface id
 *    @param[out] buffer - packet buffer
 *    @param[in,out] buffer_size - @param[in] allocated buffer size. @param[out] actual packet size in bytes
 *    @param[in,out] attr_count - @param[in] allocated list size. @param[out] number of attributes
 *    @param[out] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            SAI_STATUS_BUFFER_OVERFLOW if buffer_size is insufficient,
 *            and buffer_size will be filled with required size. Or
 *            if attr_count is insufficient, and attr_count
 *            will be filled with required count.
 *            Failure status code on error
 */
sai_status_t mrvl_sai_recv_host_interface_packet(_In_ sai_object_id_t  hif_id,
                                                 _Out_ void *buffer,
                                                 _Inout_ sai_size_t *buffer_size,
                                                 _Inout_ uint32_t *attr_count,
                                                 _Out_ sai_attribute_t *attr_list)
{
    sai_status_t                 status;        
    FPA_STATUS                   fpa_status;    
    FPA_PACKET_BUFFER_STC        pktPtr = {0}; 
    uint32_t                     timeout = 0; /* WaitForever : TBD need to check how to receive this param */
    sai_status_t 				 sai_status;
        
    MRVL_SAI_LOG_ENTER();    

    if (hif_id == 0) {
        MRVL_SAI_LOG_ERR("NULL host interface ID param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if ((NULL == buffer) || (buffer_size == 0)) {
        MRVL_SAI_LOG_ERR("NULL packet buffer or packet size is zero\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }   

    pktPtr.pktDataPtr = malloc(SAI_MAX_MTU_CNS*sizeof(uint8_t));
    pktPtr.pktDataSize = SAI_MAX_MTU_CNS;  

    fpa_status =  fpaLibPktReceive(SAI_DEFAULT_ETH_SWID_CNS,timeout ,&pktPtr);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to receive host interface packet %d\n", fpa_status);
        sai_status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(sai_status);
    }

    if (*buffer_size < pktPtr.pktDataSize) {
        MRVL_SAI_LOG_ERR("buffer_size is insufficient\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_BUFFER_OVERFLOW);
    } 

    memcpy(buffer,pktPtr.pktDataPtr,pktPtr.pktDataSize);  /* packet data */ 
    *buffer_size = pktPtr.pktDataSize;

    attr_list[0].id = SAI_HOSTIF_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
    attr_list[1].id = SAI_HOSTIF_ATTR_OBJ_ID;
    attr_list[1].value.u32 = pktPtr.inPortNum;
    attr_list[2].id = SAI_HOSTIF_ATTR_NAME;
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_get_host_interface_attribute(hif_id,1,&attr_list[2]))) {
    	MRVL_SAI_API_RETURN(status);
    }
    *attr_count = 3;    
    
    free(pktPtr.pktDataPtr);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/**
 * Routine Description:
 *   @brief hostif send function
 *
 * Arguments:
 *    @param[in] hif_id  - host interface id. only valid for send through FD channel. Use SAI_NULL_OBJECT_ID for send through CB channel.
 *    @param[in] buffer - packet buffer
 *    @param[in] buffer size - packet size in bytes
 *    @param[in] attr_count - number of attributes
 *    @param[in] attr_list - array of attributes
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t mrvl_sai_send_host_interface_packet(_In_ sai_object_id_t  hif_id,
                                                 _In_ void *buffer,
                                                 _In_ sai_size_t buffer_size,
                                                 _In_ uint32_t attr_count,
                                                 _In_ sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *type, *rif_port;
    uint32_t                     type_index, rif_port_index,rif_data = 0;   
    char                         list_str[MAX_LIST_VALUE_STR_LEN];    
    sai_attribute_t              hif_attr_list;
    FPA_STATUS                   fpa_status;    
    FPA_PACKET_OUT_BUFFER_STC    pktPtr = {0};     

    MRVL_SAI_LOG_ENTER();

    if (hif_id == 0) {
        MRVL_SAI_LOG_ERR("NULL host interface ID param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if ((NULL == buffer) || (buffer_size == 0)) {
        MRVL_SAI_LOG_ERR("NULL packet buffer or packet size is zero\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    } 

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, host_interface_attribs, host_interface_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, host_interface_attribs, MAX_LIST_VALUE_STR_LEN, list_str);    

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_TYPE, &type, &type_index));    

    if (SAI_HOSTIF_TYPE_NETDEV == type->s32) {
        if (SAI_STATUS_SUCCESS !=
            (status =
                 mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_OBJ_ID, &rif_port,
                                     &rif_port_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute rif port id on send packet of host with netdev type\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }

        if (SAI_OBJECT_TYPE_ROUTER_INTERFACE == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS !=
                (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_data))) {
                /* TODO need to add support after integration of code for router interface */
                return status;
            }
        } else if (SAI_OBJECT_TYPE_PORT == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_PORT, &rif_data))) {
            	MRVL_SAI_API_RETURN(status);
            }
        } else {
            MRVL_SAI_LOG_ERR("Invalid rif port object type %s", SAI_TYPE_STR(sai_object_type_query(rif_port->oid)));
            status = SAI_STATUS_INVALID_ATTR_VALUE_0 + rif_port_index;
            MRVL_SAI_API_RETURN(status);
        }
    } else if (SAI_HOSTIF_TYPE_FD == type->s32) {
    	MRVL_SAI_LOG_WRN("no handle for host interface type %d\n", type->s32);
    } else {
        MRVL_SAI_LOG_ERR("Invalid host interface type %d\n", type->s32);
        status = SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
        MRVL_SAI_API_RETURN(status);
    }

    pktPtr.outPortNum = rif_data;

    /* retreive ingress port */
    /* TODO inPortNum is constant. need to change it by adding support in the get function */
    hif_attr_list.id = SAI_HOSTIF_ATTR_OBJ_ID;    

    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_sai_get_host_interface_attribute(hif_id,1,&hif_attr_list))) {
        MRVL_SAI_LOG_ERR("Failed to get host interface attributes\n");
        MRVL_SAI_API_RETURN(status);
    }    

    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_sai_utl_find_attrib_in_list(1, &hif_attr_list, SAI_HOSTIF_ATTR_OBJ_ID, &rif_port,
                                             &rif_port_index))) {
        MRVL_SAI_LOG_ERR("Missing mandatory attribute ingress port id on send packet of host\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
    }   
    
    if (SAI_OBJECT_TYPE_ROUTER_INTERFACE == sai_object_type_query(rif_port->oid)) {
        if (SAI_STATUS_SUCCESS !=
                (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_data))) {
                /* TODO need to add support after integration of code for router interface */
        	MRVL_SAI_API_RETURN(status);
            }
        else if (SAI_OBJECT_TYPE_PORT == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_PORT, &rif_data))) {
            	MRVL_SAI_API_RETURN(status);
            }
        } else {
            MRVL_SAI_LOG_ERR("Invalid rif port object type %s", SAI_TYPE_STR(sai_object_type_query(rif_port->oid)));
            status = SAI_STATUS_INVALID_ATTR_VALUE_0 + rif_port_index;
            MRVL_SAI_API_RETURN(status);
        }
    }    

    pktPtr.pktDataPtr = buffer; 
    pktPtr.pktDataSize = buffer_size; 

    fpa_status =  fpaLibPortPktSend(SAI_DEFAULT_ETH_SWID_CNS, SAI_CPU_PORT_CNS /*inPortNum*/,&pktPtr); /* TODO change const inPortNum*/
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to send host interface packet %d\n", fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Create host interface table entry
 *
 * @param[out] hif_table_entry Host interface table entry
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_create_hostif_table_entry(
        _Out_ sai_object_id_t *hif_table_entry,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *type, *obj_id, *channel, *fd;
    uint32_t                     type_index, obj_id_index, obj_id_data, channel_index, fd_index;
    uint32_t                     fd_data, fd_ext_data;
    uint8_t                      ext_data[RESERVED_DATA_LENGTH_CNS];
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    
    if (NULL == hif_table_entry) {
        MRVL_SAI_LOG_ERR("NULL host interface table entry ID param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id))
    {
        MRVL_SAI_LOG_ERR("Invalid switch id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, host_interface_table_entry_attribs, host_interface_table_entry_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed host interface table entry attributes check\n");
        MRVL_SAI_API_RETURN(status);
    }
    
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, host_interface_table_entry_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create host interface table entry, %s\n", list_str);

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_TABLE_ENTRY_ATTR_TYPE, &type, &type_index));
    
    if ((SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT == type->s32) || (SAI_HOSTIF_TABLE_ENTRY_TYPE_LAG == type->s32) ||
        (SAI_HOSTIF_TABLE_ENTRY_TYPE_VLAN == type->s32)) {
        if (SAI_STATUS_SUCCESS !=
            (status =
                 mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID, &obj_id,
                                     &obj_id_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute obj ID on creation of host table entry type: port/lag/vlan\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }

        if ((SAI_HOSTIF_TABLE_ENTRY_TYPE_PORT == type->s32) || (SAI_HOSTIF_TABLE_ENTRY_TYPE_LAG == type->s32)) {
            if (SAI_STATUS_SUCCESS != 
                (status = mrvl_sai_utl_oid_to_lag_port(obj_id->oid, &obj_id_data))) {
                MRVL_SAI_API_RETURN(status);
            }
        } else {
            if (SAI_STATUS_SUCCESS != 
                (status = mrvl_sai_utl_object_to_type(obj_id->oid, SAI_OBJECT_TYPE_VLAN, &obj_id_data))) {
                MRVL_SAI_API_RETURN(status);
            }
        }
    } else {
        if (SAI_STATUS_ITEM_NOT_FOUND !=
            (status =
                 mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_TABLE_ENTRY_ATTR_OBJ_ID, &obj_id,
                                     &obj_id_index))) {
            MRVL_SAI_LOG_ERR("Invalid obj ID attribute on creation for trap/wildcard host interface table entry\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTRIBUTE_0 + obj_id_index);
        }

        /* TO DO: add support for table entry type TRAP ID\Wildcard */
    }
   
    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_TABLE_ENTRY_ATTR_CHANNEL_TYPE,
                                 &channel, &channel_index));
    
    if (SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_FD == channel->s32) {
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF, &fd,
                                     &fd_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute host if on creation of hostif table entry, channel type FD\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
        }

        status = mrvl_sai_utl_object_to_ext_type(fd->oid, SAI_OBJECT_TYPE_HOSTIF, &fd_data, ext_data);
        if (SAI_STATUS_SUCCESS != status)
            MRVL_SAI_API_RETURN(status);
        fd_ext_data = (uint32_t)(ext_data[1] << 8 | ext_data[0]);
        if (SAI_HOSTIF_TYPE_FD != fd_data) {
            MRVL_SAI_LOG_ERR("Can't set non FD host interface type %u\n", fd_data);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTR_VALUE_0 + fd_index);
        }
    } else {
        if (SAI_STATUS_ITEM_NOT_FOUND !=
            (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_TABLE_ENTRY_ATTR_HOST_IF, &fd,
                                     &fd_index))) {
            MRVL_SAI_LOG_ERR("Invalid attribute host IF for host table entry channel non FD on create\n");
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTRIBUTE_0 + fd_index);
        }

        /* TO DO: add support for channel type CB */
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY, obj_id_data, hif_table_entry))) {
        MRVL_SAI_API_RETURN(status);
    }
    mrvl_sai_host_interface_table_entry_key_to_str(*hif_table_entry, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove host interface table entry
 *
 * @param[in] hif_table_entry Host interface table entry
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_hostif_table_entry(
        _In_ sai_object_id_t hif_table_entry)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
    MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Set host interface table entry attribute
 *
 * @param[in] hif_table_entry Host interface table entry
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_set_hostif_table_entry_attribute(
        _In_ sai_object_id_t hif_table_entry,
        _In_ const sai_attribute_t *attr)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
    MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Get host interface table entry attribute
 *
 * @param[in] hif_table_entry Host interface table entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_hostif_table_entry_attribute(
        _In_ sai_object_id_t hif_table_entry,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
    MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}


/**

 * @brief Create host interface trap group
 *
 * @param[out] hostif_trap_group_id Host interface trap group id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error


 */
sai_status_t mrvl_sai_create_hostif_trap_group(
    _Out_ sai_object_id_t *hostif_trap_group_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list)
{
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

}
/**

 * @brief Remove host interface trap group
 *
 * @param[in] hostif_trap_group_id Host interface trap group id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error

 */

sai_status_t mrvl_sai_remove_hostif_trap_group(
    _In_ sai_object_id_t hostif_trap_group_id)
{
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

}
/**


 * @brief Set host interface trap group attribute value.
 *
 * @param[in] hostif_trap_group_id Host interface trap group id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_set_hostif_trap_group_attribute
(
    _In_ sai_object_id_t hostif_trap_group_id,
    _In_ const sai_attribute_t *attr
)
{
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

}
/**


 * @brief Get host interface trap group attribute value.
 *
 * @param[in] hostif_trap_group_id Host interface trap group id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error 
 */

sai_status_t mrvl_sai_get_hostif_trap_group_attribute(
    _In_ sai_object_id_t hostif_trap_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    ){
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

}
/**
 * @brief Create host interface trap
 *
 * @param[out] hostif_trap_id Host interface trap id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_hostif_trap(
        _Out_ sai_object_id_t *hostif_trap_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Remove host interface trap
 *
 * @param[in] hostif_trap_id Host interface trap id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_hostif_trap(
        _In_ sai_object_id_t hostif_trap_id)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Set trap attribute value.
 *
 * @param[in] hostif_trap_id Host interface trap id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */
sai_status_t mrvl_sai_set_hostif_trap_attribute(
        _In_ sai_object_id_t hostif_trap_id,
        _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = hostif_trap_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();    

    mrvl_sai_host_interface_trap_key_to_str(hostif_trap_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, host_interface_trap_attribs, host_interface_trap_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**


 * @brief Get trap attribute value.
 *
 * @param[in] hostif_trap_id Host interface trap id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */
sai_status_t mrvl_sai_get_hostif_trap_attribute(
        _In_ sai_object_id_t hostif_trap_id,
        _In_ uint32_t attr_count,
        _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = hostif_trap_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();
    
    mrvl_sai_host_interface_trap_key_to_str(hostif_trap_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, host_interface_trap_attribs, host_interface_trap_vendor_attribs,
                                       attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Create host interface user defined trap
 *
 * @param[out] hostif_user_defined_trap_id Host interface user defined trap id
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_hostif_user_defined_trap(
        _Out_ sai_object_id_t *hostif_user_defined_trap_id,
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t attr_count,
        _In_ const sai_attribute_t *attr_list)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Remove host interface user defined trap
 *
 * @param[in] hostif_user_defined_trap_id Host interface user defined trap id
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_hostif_user_defined_trap(
        _In_ sai_object_id_t hostif_user_defined_trap_id)
{
    MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}

/**
 * @brief Set user defined trap attribute value.
 *
 * @param[in] hostif_user_defined_trap_id Host interface user defined trap id
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error


 */

sai_status_t mrvl_sai_set_hostif_user_defined_trap_attribute(
    _In_ sai_object_id_t hostif_user_defined_trapid,
    _In_ const sai_attribute_t *attr
    )
{
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);

}
/**


 * @brief Get user defined trap attribute value.
 *
 * @param[in] hostif_user_defined_trap_id Host interface user defined trap id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error



 */

sai_status_t mrvl_sai_get_hostif_user_defined_trap_attribute(
    _In_ sai_object_id_t hostif_user_defined_trapid,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
	MRVL_SAI_LOG_ERR("STUB %s", __func__);
	MRVL_SAI_API_RETURN(SAI_STATUS_STUB);
}



const sai_hostif_api_t host_interface_api = {
    mrvl_sai_create_host_interface,
    mrvl_sai_remove_host_interface,
    mrvl_sai_set_host_interface_attribute,
    mrvl_sai_get_host_interface_attribute,

    mrvl_sai_create_hostif_table_entry,
    mrvl_sai_remove_hostif_table_entry,
    mrvl_sai_set_hostif_table_entry_attribute,
    mrvl_sai_get_hostif_table_entry_attribute,

    mrvl_sai_create_hostif_trap_group,
    mrvl_sai_remove_hostif_trap_group,
    mrvl_sai_set_hostif_trap_group_attribute,
    mrvl_sai_get_hostif_trap_group_attribute,

    mrvl_sai_create_hostif_trap,
    mrvl_sai_remove_hostif_trap,
    mrvl_sai_set_hostif_trap_attribute,
    mrvl_sai_get_hostif_trap_attribute,

    mrvl_sai_create_hostif_user_defined_trap,
    mrvl_sai_remove_hostif_user_defined_trap,
    mrvl_sai_set_hostif_user_defined_trap_attribute,
    mrvl_sai_get_hostif_user_defined_trap_attribute,

    mrvl_sai_recv_host_interface_packet,
    mrvl_sai_send_host_interface_packet
};
