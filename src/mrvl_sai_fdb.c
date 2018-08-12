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
#define __MODULE__ SAI_FDB

extern uint32_t                  mrvl_sai_switch_aging_time;

#define MRVL_SAI_FDB_CONVERT_EVENT_TYPE_MAC(fpa_type, sai_type)\
        (sai_type = (fpa_type == FPA_EVENT_ADDRESS_UPDATE_NEW_E)?SAI_FDB_EVENT_LEARNED:(fpa_type == FPA_EVENT_ADDRESS_UPDATE_AGED_E)?SAI_FDB_EVENT_AGED:SAI_FDB_EVENT_FLUSHED)

static mrvl_sai_fdb_table_t mrvl_sai_fdb_table[SAI_FDB_TABLE_SIZE_CNS] = {};

static sai_status_t mrvl_sai_fdb_endpoint_ip_get_prv(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             void                          *arg);
static sai_status_t mrvl_sai_fdb_endpoint_ip_set_prv(_In_ const sai_object_key_t      *key,
                                             _In_ const sai_attribute_value_t *value,
                                             void                             *arg);

static void mrvl_sai_fdb_key_to_str_prv(_In_ const sai_fdb_entry_t* fdb_entry, _Out_ char *key_str)
{
    snprintf(key_str, MAX_KEY_STR_LEN, "fdb entry mac [%02x:%02x:%02x:%02x:%02x:%02x]",
             fdb_entry->mac_address[0],
             fdb_entry->mac_address[1],
             fdb_entry->mac_address[2],
             fdb_entry->mac_address[3],
             fdb_entry->mac_address[4],
             fdb_entry->mac_address[5]);
}

static sai_status_t mrvl_sai_fdb_endpoint_ip_get_prv(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                             void                          *arg)
{
     MRVL_SAI_LOG_ENTER();
     memset(&(value->ipaddr.addr.ip4), 0, sizeof(sai_ip4_t));
     MRVL_SAI_LOG_EXIT();
     return SAI_STATUS_SUCCESS;
}
static sai_status_t mrvl_sai_fdb_endpoint_ip_set_prv(_In_ const sai_object_key_t      *key,
                                             _In_ const sai_attribute_value_t *value,
                                             void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_IMPLEMENTED;
}
static sai_status_t mrvl_sai_fdb_get_entry_prv(_In_ const sai_fdb_entry_t* fdb_entry,
                                           _Inout_ FPA_FLOW_TABLE_ENTRY_STC *fpa_flow_entry)
{
    uint64_t cookie;
    FPA_STATUS fpa_status;
    cookie = MRVL_SAI_FDB_CREATE_COOKIE_MAC(fdb_entry->bv_id, fdb_entry->mac_address);
    fpa_flow_entry->cookie = cookie;
    fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, fpa_flow_entry);
    return mrvl_sai_utl_fpa_to_sai_status(fpa_status); 
}

/* Set FDB entry type [sai_fdb_entry_type_t] */
static sai_status_t mrvl_sai_fdb_type_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->u32 != SAI_FDB_ENTRY_TYPE_STATIC) {
        return SAI_STATUS_NOT_SUPPORTED;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* FDB entry port id [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_AND_SET)
 * The port id here can refer to a generic port object such as SAI port object id,
 * SAI LAG object id and etc. on. */
static sai_status_t mrvl_sai_fdb_port_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_status_t status;
    uint32_t     port_id, group;
    const sai_fdb_entry_t *fdb_entry = &key->key.fdb_entry;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;
    FPA_STATUS fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC    parsed_group_identifier;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_fdb_get_entry_prv(fdb_entry, &fpa_flow_entry))) {
        return status;
    }


    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_BRIDGE_PORT, &port_id))) {
        return status;
    }
    fpaLibGroupIdentifierParse(fpa_flow_entry.data.l2_bridging.groupId, &parsed_group_identifier);
    status = mrvl_sai_utl_create_l2_int_group(port_id, parsed_group_identifier.vlanId, SAI_VLAN_TAGGING_MODE_TAGGED, false, &group);
    if (status != SAI_STATUS_SUCCESS) {
        return status; 
    }
	fpa_flow_entry.data.l2_bridging.groupId  = group;
	fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS,FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &fpa_flow_entry, 0);
	if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to modify entry %llx in FDB table status = %d\n", fpa_flow_entry.cookie, fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Set FDB entry packet action [sai_packet_action_t] */
static sai_status_t mrvl_sai_fdb_action_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_status_t status;
    FPA_STATUS fpa_status;
    const sai_fdb_entry_t *fdb_entry = &key->key.fdb_entry;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;
    uint32_t group;
    FPA_GROUP_ENTRY_IDENTIFIER_STC    parsed_group_identifier;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_fdb_get_entry_prv(fdb_entry, &fpa_flow_entry))) {
        return status;
    }
    switch ((sai_packet_action_t)value->u32) {
    case SAI_PACKET_ACTION_DROP:    /** Drop Packet in data plane */        
    case SAI_PACKET_ACTION_DENY:    /** This is a combination of sai packet action COPY_CANCEL and DROP */
        fpa_flow_entry.data.l2_bridging.clearActions = 1;
        /*fpa_flow_entry.data.l2_bridging.groupId = -1;*/
        break;


    case SAI_PACKET_ACTION_COPY:    /** Copy Packet to CPU. */
    case SAI_PACKET_ACTION_TRAP:    /** This is a combination of sai packet action COPY and DROP. */
        fpa_flow_entry.data.l2_bridging.outputPort = SAI_OUTPUT_CONTROLLER;
        fpa_flow_entry.data.l2_bridging.groupId = -1;
        fpa_flow_entry.data.l2_bridging.clearActions = 0;
        break;


    case SAI_PACKET_ACTION_LOG:     /** This is a combination of sai packet action COPY and FORWARD. */
        fpa_flow_entry.data.l2_bridging.outputPort = SAI_OUTPUT_CONTROLLER;
        /* continue to forward*/   
    case SAI_PACKET_ACTION_FORWARD: /** Forward Packet in data plane. */
        fpaLibGroupIdentifierParse(fpa_flow_entry.data.l2_bridging.groupId, &parsed_group_identifier);
        status = mrvl_sai_utl_create_l2_int_group(parsed_group_identifier.portNum, parsed_group_identifier.vlanId, SAI_VLAN_TAGGING_MODE_TAGGED, false, &group);
        if (status != SAI_STATUS_SUCCESS) {
            return status; 
        }
        fpa_flow_entry.data.l2_bridging.clearActions = 0;
        fpa_flow_entry.data.l2_bridging.groupId = group;
        break;

    case SAI_PACKET_ACTION_COPY_CANCEL:  /** Cancel copy the packet to CPU. */
    case SAI_PACKET_ACTION_TRANSIT:      /** This is a combination of sai packet action COPY_CANCEL and FORWARD */
    default:
        MRVL_SAI_LOG_ERR("action %d is not supported\n", value->u32);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    fpa_status = fpaLibFlowEntryModify(SAI_DEFAULT_ETH_SWID_CNS,FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &fpa_flow_entry, 0);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to modify entry %llx in FDB table status = %d\n", fpa_flow_entry.cookie, fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get FDB entry type [sai_fdb_entry_type_t] */
static sai_status_t mrvl_sai_fdb_type_get_prv(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg)
{
    const sai_fdb_entry_t    *fdb_entry = &key->key.fdb_entry;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;
    sai_status_t             status;
    
    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_fdb_get_entry_prv(fdb_entry, &fpa_flow_entry))) {
        return status;
    }
    if (fpa_flow_entry.timeoutIdleTime == 0) {
        value->u32 = SAI_FDB_ENTRY_TYPE_STATIC; 
    } else {
        value->u32 = SAI_FDB_ENTRY_TYPE_DYNAMIC; 
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* FDB entry port id [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_AND_SET)
 * The port id here can refer to a generic port object such as SAI port object id,
 * SAI LAG object id and etc. on.
 * Port 0 is returned for entries with action = drop or action = trap
 * Since port is irrelevant for these actions, even if actual port is set
 * In case the action is changed from drop/trap to forward/log, need to also set port
 */
static sai_status_t mrvl_sai_fdb_port_get_prv(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg)
{
    sai_status_t status;
    const sai_fdb_entry_t *fdb_entry = &key->key.fdb_entry;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    uint32_t port;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_fdb_get_entry_prv(fdb_entry, &fpa_flow_entry))) {
        MRVL_SAI_LOG_ERR("Failed to get fdb entry\n");
        value->oid = SAI_NULL_OBJECT_ID;
        return status;
    }
    if ((fpa_flow_entry.data.l2_bridging.clearActions) ||
        (fpa_flow_entry.data.l2_bridging.groupId == 0) ||
        (fpa_flow_entry.data.l2_bridging.groupId == 0xFFFFFFFF)){
        port = 0;
    } else {
        fpaLibGroupIdentifierParse(fpa_flow_entry.data.l2_bridging.groupId, &parsed_group_identifier);
        port = parsed_group_identifier.portNum;
    }
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, port, &value->oid))) {
        MRVL_SAI_LOG_ERR("Failed to create object port\n");
        value->oid = SAI_NULL_OBJECT_ID;
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Get FDB entry packet action [sai_packet_action_t] */
static sai_status_t mrvl_sai_fdb_action_get_prv(_In_ const sai_object_key_t   *key,
                                 _Inout_ sai_attribute_value_t *value,
                                 _In_ uint32_t                  attr_index,
                                 _Inout_ vendor_cache_t        *cache,
                                 void                          *arg)
{
    sai_status_t status;
    const sai_fdb_entry_t *fdb_entry = &key->key.fdb_entry;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_fdb_get_entry_prv(fdb_entry, &fpa_flow_entry))) {
        return status;
    }
    value->s32 = SAI_PACKET_ACTION_TRAP;
    if (fpa_flow_entry.data.l2_bridging.clearActions) {
        value->s32 = SAI_PACKET_ACTION_DROP;
    } else if ((fpa_flow_entry.data.l2_bridging.groupId != 0) && (fpa_flow_entry.data.l2_bridging.groupId != 0xFFFFFFFF)){ 
        if (fpa_flow_entry.data.l2_bridging.outputPort == SAI_OUTPUT_CONTROLLER){
            value->s32 = SAI_PACKET_ACTION_LOG;
        }else {    
            value->s32 = SAI_PACKET_ACTION_FORWARD;
        }
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* find free index in mrvl_sai_fdb_table */
static sai_status_t mrvl_sai_fdb_find_free_index_in_db(_Out_ uint32_t *free_index)
{
    sai_status_t status;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(free_index != NULL);

    for (i = 0; i < SAI_FDB_TABLE_SIZE_CNS; i++) {
        if (false == mrvl_sai_fdb_table[i].used) {
            *free_index              = i;
            mrvl_sai_fdb_table[i].used = true;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_FDB_TABLE_SIZE_CNS) {
    	MRVL_SAI_LOG_ERR("NO free indices\n");
        status = SAI_STATUS_INSUFFICIENT_RESOURCES;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/* get entry from mrvl_sai_fdb_table */
static sai_status_t mrvl_sai_fdb_get_db_entry(_In_ uint64_t  cookie,
                                              _Out_ uint32_t *entry_index)
{
    sai_status_t status;
    uint32_t     i;

    MRVL_SAI_LOG_ENTER();
    assert(entry_index != NULL);

    for (i = 0; i < SAI_FDB_TABLE_SIZE_CNS; i++) {
        if (cookie == mrvl_sai_fdb_table[i].cookie) {
            if (false == mrvl_sai_fdb_table[i].used)
                MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);

            *entry_index = i;
            status                   = SAI_STATUS_SUCCESS;
            break;
        }
    }

    if (i == SAI_FDB_TABLE_SIZE_CNS) {
    	MRVL_SAI_LOG_ERR("Failed to get FDB entry\n");
        status = SAI_STATUS_ITEM_NOT_FOUND;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_fdb_db_free_entries_get(_In_ sai_switch_attr_t  resource_type,
                                                   _Out_ uint32_t         *free_entries)
{
    uint32_t i, count = 0;
    
    MRVL_SAI_LOG_ENTER();

    assert(resource_type == SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY);
    assert(free_entries != NULL);

    for (i = 0; i < SAI_FDB_TABLE_SIZE_CNS; i++) {
        if (false == mrvl_sai_fdb_table[i].used)
            count++;
    }

    *free_entries = count;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


static const sai_attribute_entry_t        mrvl_sai_fdb_attribs[] = {
    { SAI_FDB_ENTRY_ATTR_TYPE, true, true, true, true,
      "FDB entry type", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_FDB_ENTRY_ATTR_PACKET_ACTION, true, true, true, true,
      "FDB entry packet action", SAI_ATTR_VALUE_TYPE_INT32 },
    { SAI_FDB_ENTRY_ATTR_USER_TRAP_ID, false, true, false, false,
      "FDB entry user trap ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID },
    { SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID, true, true, true, true,
      "FDB entry port id", SAI_ATTR_VALUE_TYPE_UINT32},
    { SAI_FDB_ENTRY_ATTR_ENDPOINT_IP, false, true, false, true,
      "FDB tunnel endpoint IP", SAI_ATTR_VALUE_TYPE_INT32},
    
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VALUE_TYPE_UNDETERMINED }
};
static const sai_vendor_attribute_entry_t mrvl_sai_fdb_vendor_attribs[] = {
    { SAI_FDB_ENTRY_ATTR_TYPE,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_fdb_type_get_prv, NULL,
      mrvl_sai_fdb_type_set_prv, NULL },
    { SAI_FDB_ENTRY_ATTR_PACKET_ACTION,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_fdb_action_get_prv, NULL,
      mrvl_sai_fdb_action_set_prv, NULL },
    { SAI_FDB_ENTRY_ATTR_USER_TRAP_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL },
    { SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_fdb_port_get_prv, NULL,
      mrvl_sai_fdb_port_set_prv, NULL },
    { SAI_FDB_ENTRY_ATTR_ENDPOINT_IP,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_fdb_endpoint_ip_get_prv, NULL,
      mrvl_sai_fdb_endpoint_ip_set_prv, NULL }
};


/**
 * @brief Create FDB entry
 *
 * @param[in] fdb_entry FDB entry
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_create_fdb_entry(_In_ const sai_fdb_entry_t *fdb_entry,
                                   _In_ uint32_t               attr_count,
                                   _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *type, *action, *port;
    uint32_t                     type_index, action_index, port_index, port_id, bv_idx;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    FPA_FLOW_TABLE_ENTRY_STC     flowEntry;
    FPA_STATUS                   fpa_status;
    uint64_t                     cookie;
    uint32_t                     groupId = 0, table_index;
    mrvl_sai_fdb_bv_type_t       bv_type;
    
    MRVL_SAI_LOG_ENTER();
    
    if (NULL == fdb_entry) {
        MRVL_SAI_LOG_ERR("NULL fdb entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_fdb_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_DBG("Create FDB entry, Attribs: %s\n", list_str);

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_fdb_attribs, mrvl_sai_fdb_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    assert(SAI_STATUS_SUCCESS == 
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_FDB_ENTRY_ATTR_TYPE, &type, &type_index));

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_FDB_ENTRY_ATTR_PACKET_ACTION, &action, &action_index));


    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID, &port, &port_index));

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port->oid, SAI_OBJECT_TYPE_BRIDGE_PORT, &port_id))) {
    	MRVL_SAI_API_RETURN(status);
    }
    fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &flowEntry);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to init FDB entry status = %d\n", fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }
    /* set fdb entry */
    if (type->s32 == SAI_FDB_ENTRY_TYPE_DYNAMIC){
        if(mrvl_sai_switch_aging_time > 0){
            flowEntry.timeoutIdleTime = mrvl_sai_switch_aging_time;
        } else {
            flowEntry.timeoutIdleTime = 1; /* set entry dynamic even if the timeout is disabled */
        }
    }
    if (mrvl_sai_utl_is_object_type(fdb_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE) == SAI_STATUS_SUCCESS)
    {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(fdb_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE, &bv_idx))) {
            MRVL_SAI_API_RETURN(status);
        }
        bv_type = mrvl_sai_fdb_bv_type_bridge_E;
    }
    else /* SAI_OBJECT_TYPE_VLAN */
    {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(fdb_entry->bv_id, SAI_OBJECT_TYPE_VLAN, &bv_idx))) {
            MRVL_SAI_API_RETURN(status);
        }
        bv_type = mrvl_sai_fdb_bv_type_vlan_E;
    }

    cookie = MRVL_SAI_FDB_CREATE_COOKIE_MAC(bv_idx, fdb_entry->mac_address);
    flowEntry.cookie = cookie; 
    flowEntry.data.l2_bridging.match.vlanId     = bv_idx;
    flowEntry.data.l2_bridging.match.vlanIdMask = 0xFFFF;
    memcpy(flowEntry.data.l2_bridging.match.destMac.addr, fdb_entry->mac_address, FPA_MAC_ADDRESS_SIZE);
    memset(flowEntry.data.l2_bridging.match.destMacMask.addr, 0xFF, FPA_MAC_ADDRESS_SIZE);
    
    switch (action->s32) {
    case SAI_PACKET_ACTION_DROP:    /** Drop Packet in data plane */        
    case SAI_PACKET_ACTION_DENY:    /** This is a combination of sai packet action COPY_CANCEL and DROP */
        flowEntry.data.l2_bridging.clearActions = 1;
        flowEntry.data.l2_bridging.groupId = -1;
        break;

   
    case SAI_PACKET_ACTION_COPY:    /** Copy Packet to CPU. */
    case SAI_PACKET_ACTION_TRAP:    /** This is a combination of sai packet action COPY and DROP. */
        flowEntry.data.l2_bridging.outputPort = SAI_OUTPUT_CONTROLLER;
        flowEntry.data.l2_bridging.groupId = -1;
        break;

    
    case SAI_PACKET_ACTION_LOG:     /** This is a combination of sai packet action COPY and FORWARD. */
        flowEntry.data.l2_bridging.outputPort = SAI_OUTPUT_CONTROLLER;
        /* continue to forward*/   
    case SAI_PACKET_ACTION_FORWARD: /** Forward Packet in data plane. */
        status = mrvl_sai_utl_create_l2_int_group(port_id, bv_idx, SAI_VLAN_TAGGING_MODE_TAGGED, false, &groupId);
        if (status != SAI_STATUS_SUCCESS) {
        	MRVL_SAI_API_RETURN(status);
        }
        
        flowEntry.data.l2_bridging.groupId = groupId;
        break;
    
    case SAI_PACKET_ACTION_COPY_CANCEL:  /** Cancel copy the packet to CPU. */
    case SAI_PACKET_ACTION_TRANSIT:      /** This is a combination of sai packet action COPY_CANCEL and FORWARD */
    default:
        MRVL_SAI_LOG_ERR("action %d is not supported\n", action);
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
    }

    fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &flowEntry);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to add entry %llx to FDB table status = %d\n", cookie, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    /* get free index */
    if (SAI_STATUS_SUCCESS !=
        		(status = mrvl_sai_fdb_find_free_index_in_db(&table_index))){
    	MRVL_SAI_LOG_ERR("No free index in FDB table\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_fdb_table[table_index].cookie = cookie;
    mrvl_sai_fdb_table[table_index].bv_idx = bv_idx;
    mrvl_sai_fdb_table[table_index].bv_type = bv_type;
    mrvl_sai_fdb_table[table_index].port_idx = port_id;
    memcpy(mrvl_sai_fdb_table[table_index].mac_address, fdb_entry->mac_address, FPA_MAC_ADDRESS_SIZE);

    mrvl_sai_fdb_key_to_str_prv(fdb_entry, key_str);
    MRVL_SAI_LOG_NTC("Created FDB entry %s\n", key_str);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Remove FDB entry
 *
 * @param[in] fdb_entry FDB entry
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_remove_fdb_entry(_In_ const sai_fdb_entry_t *fdb_entry)
{
    char key_str[MAX_KEY_STR_LEN];
    FPA_STATUS                   fpa_status;
    uint64_t                     cookie;
    uint32_t                     bv_idx, entry_index;
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == fdb_entry) {
        MRVL_SAI_LOG_ERR("NULL fdb entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_fdb_key_to_str_prv(fdb_entry, key_str);
    MRVL_SAI_LOG_NTC("Remove FDB entry %s\n", key_str);

    if (mrvl_sai_utl_is_object_type(fdb_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE) == SAI_STATUS_SUCCESS)
    {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(fdb_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE, &bv_idx))) {
            MRVL_SAI_API_RETURN(status);
        }
    }
    else /* SAI_OBJECT_TYPE_VLAN */
    {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(fdb_entry->bv_id, SAI_OBJECT_TYPE_VLAN, &bv_idx))) {
            MRVL_SAI_API_RETURN(status);
        }
    }
    cookie = MRVL_SAI_FDB_CREATE_COOKIE_MAC(bv_idx, fdb_entry->mac_address);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_fdb_get_db_entry(cookie, &entry_index)))
        MRVL_SAI_API_RETURN(status);

    fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, cookie);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to delete entry %llx from FDB table status = %d\n", cookie, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    /* clear entry in DB */
    memset(&mrvl_sai_fdb_table[entry_index], 0, sizeof(mrvl_sai_fdb_table_t));

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
 * @brief Get FDB entry attribute value
 *
 * @param[in] fdb_entry FDB entry
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_get_fdb_entry_attribute(_In_ const sai_fdb_entry_t* fdb_entry,
                                          _In_ uint32_t               attr_count,
                                          _Inout_ sai_attribute_t    *attr_list)
{
    const sai_object_key_t key  = { .key.fdb_entry = *fdb_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == fdb_entry) {
        MRVL_SAI_LOG_ERR("NULL fdb entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_fdb_key_to_str_prv(fdb_entry, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_fdb_attribs, mrvl_sai_fdb_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Set FDB entry attribute value
 *
 * @param[in] fdb_entry FDB entry
 * @param[in] attr Attribute
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_set_fdb_entry_attribute(_In_ const sai_fdb_entry_t* fdb_entry, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = {.key.fdb_entry = *fdb_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == fdb_entry) {
        MRVL_SAI_LOG_ERR("NULL fdb entry param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    mrvl_sai_fdb_key_to_str_prv(fdb_entry, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_fdb_attribs, mrvl_sai_fdb_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}


/**
 * @brief Remove all FDB entries by attribute set in sai_fdb_flush_attr
 *
 * @param[in] switch_id Switch object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t mrvl_sai_flush_fdb_entries(_In_ sai_object_id_t switch_id,
                                        _In_ uint32_t attr_count, 
                                        _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *bport_attr, *bv_attr, *type;
    uint32_t                     attr_index, bv_idx;
    uint32_t                     port_id;
    mrvl_bridge_port_info_t      *bport;
    bool                         port_exist, bv_exist, type_exist;
    uint32_t       				 count = 0;
    FPA_FLOW_TABLE_ENTRY_STC	 fpa_flow_entry;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS fpa_status;

    
    MRVL_SAI_LOG_ENTER();
    
    port_exist = false; 
    bv_exist = false;
    type_exist = false;
    fpa_flow_entry.cookie = 0;
    
    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID,
                                 &bport_attr, &attr_index))) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_bridge_port_by_oid_get(bport_attr->oid, &bport))) {
        	MRVL_SAI_API_RETURN(status);
        }
        port_exist = true;
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_FDB_FLUSH_ATTR_BV_ID,
                                 &bv_attr, &attr_index))) 
    {
        if (mrvl_sai_utl_is_object_type(bv_attr->oid, SAI_OBJECT_TYPE_BRIDGE) == SAI_STATUS_SUCCESS)
        {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(bv_attr->oid, SAI_OBJECT_TYPE_BRIDGE, &bv_idx))) {
                MRVL_SAI_API_RETURN(status);
            }
            bv_exist = false;
        }
        else /* SAI_OBJECT_TYPE_VLAN */
        {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(bv_attr->oid, SAI_OBJECT_TYPE_VLAN, &bv_idx))) {
                MRVL_SAI_API_RETURN(status);
            }
        }
        bv_exist = true;
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_FDB_FLUSH_ATTR_ENTRY_TYPE,
                                 &type, &attr_index))) {
        type_exist = true;
    }

	memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry)); 
	memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier)); 
	
	fpa_flow_entry.entryType = FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E;
    if ((type_exist) && (type->s32 == SAI_FDB_FLUSH_ENTRY_TYPE_STATIC)){
		fpa_flow_entry.timeoutIdleTime = 0; 
    } else {
        fpa_flow_entry.timeoutIdleTime = 1; 
    }
    if (bv_exist) {
		fpa_flow_entry.data.l2_bridging.match.vlanId = bv_idx;
		fpa_flow_entry.data.l2_bridging.match.vlanIdMask = 0xFFFF;		
    }
	if (port_exist) {
		parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
        parsed_group_identifier.portNum = bport->logical_port;
		fpaLibGroupIdentifierBuild(&parsed_group_identifier, &fpa_flow_entry.data.l2_bridging.groupId);
	}else {
		fpa_flow_entry.data.l2_bridging.groupId = FPA_FLOW_INVALID_VAL;	
	}
    fpa_status = fpaLibFlowEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &fpa_flow_entry, 1); 
	if (fpa_status != FPA_OK) {
		MRVL_SAI_LOG_ERR("fpaLibFlowEntryDelete: delete FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E failed %d\n", (int)fpa_status);
        MRVL_SAI_API_RETURN(mrvl_sai_utl_fpa_to_sai_status(fpa_status));
	}

    MRVL_SAI_LOG_NTC("Deleted %d flows from FDB table\n",count);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *    Converts FPA Address Update message to SAI structure
 *
 * Arguments:
 *    fpa_au_event - FPA message format
 *
 * Return Values:
 *    fdb_entry - SAI message format for address update event  
 *    SAI_STATUS_SUCCESS - on success
 * 
 */
sai_status_t mrvl_sai_fdb_event_convert_fpa_to_sai(FPA_EVENT_ADDRESS_MSG_STC *fpa_au_event, sai_fdb_event_notification_data_t *fdb_event)
{
	sai_object_type_t type;
	sai_object_id_t switch_id;
    sai_object_id_t bport;
    mrvl_bridge_port_info_t *bridge_port;
    sai_status_t status;

    memcpy(fdb_event->fdb_entry.mac_address,fpa_au_event->address.addr, sizeof(sai_mac_t));

	/* create SAI switch object */    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id)) {
        return SAI_STATUS_FAILURE;
    }
	fdb_event->fdb_entry.switch_id = switch_id;

    MRVL_SAI_LOG_NTC("FDB event received: vlan: %4u; mac: %x:%x:%x:%x:%x:%x; port: (%d); interface type: (%d); event type: %s(%d)\n",
                   fpa_au_event->vid,
                   fpa_au_event->address.addr[0],
                   fpa_au_event->address.addr[1],
                   fpa_au_event->address.addr[2],
                   fpa_au_event->address.addr[3],
                   fpa_au_event->address.addr[4],
                   fpa_au_event->address.addr[5],
                   fpa_au_event->interfaceNum,
                   fpa_au_event->interfaceType,
                   fpa_au_event->type == FPA_EVENT_ADDRESS_UPDATE_NEW_E ? "New" : "Aged",
                   fpa_au_event->type);

    MRVL_SAI_FDB_CONVERT_EVENT_TYPE_MAC(fpa_au_event->type, fdb_event->event_type);
   
	switch (fpa_au_event->interfaceType) {
	case FPA_INTERFACE_PORT_E:
		type = SAI_OBJECT_TYPE_BRIDGE_PORT;
		break;
	case FPA_INTERFACE_TRUNK_E:
		type = SAI_OBJECT_TYPE_LAG;
		break;
	case FPA_INTERFACE_VIDX_E:
	case FPA_INTERFACE_VID_E:
		type = SAI_OBJECT_TYPE_VLAN;
		break;
	case FPA_INTERFACE_DEVICE_E:
		type = SAI_OBJECT_TYPE_SWITCH;
        break;
	case FPA_INTERFACE_FABRIC_VIDX_E:
	case FPA_INTERFACE_INDEX_E:
		return SAI_STATUS_NOT_SUPPORTED;
		break;
	}

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_port_to_bport(fpa_au_event->interfaceNum, &bridge_port))) {
        MRVL_SAI_LOG_ERR("Failed to convert port %d to bridge port\n", fpa_au_event->interfaceNum);
        MRVL_SAI_API_RETURN(status);
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_bridge_port_object_create(bridge_port->index, &bport))
    {
        MRVL_SAI_LOG_ERR("Failed to create bridge port %d\n", bridge_port->index);
        return SAI_STATUS_FAILURE;
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, fpa_au_event->vid, &(fdb_event->fdb_entry.bv_id)))) {
        MRVL_SAI_API_RETURN(status);
    }

    /* Attributes */
    fdb_event->attr[0].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    fdb_event->attr[0].value.oid = bport;
    
    fdb_event->attr[1].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    fdb_event->attr[1].value.s32 = SAI_PACKET_ACTION_FORWARD;

    fdb_event->attr[2].id = SAI_FDB_ENTRY_ATTR_TYPE;
    fdb_event->attr[2].value.s32 = SAI_FDB_ENTRY_TYPE_DYNAMIC;

    fdb_event->attr_count = 3;
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Waits on AU events and call SAI callback in such event
 *
 * Arguments:
 *    void
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS - on success
 * 
 */
extern unsigned int mrvl_sai_fdb_wait_for_au_event(void *args)
{
    sai_fdb_event_notification_data_t sai_fdb_event;
    FPA_EVENT_ADDRESS_MSG_STC fpa_au_event;
    FPA_STATUS fpa_status =  FPA_OK;
    sai_attribute_t   fdb_attr[3];
    sai_fdb_event.attr = (sai_attribute_t *)fdb_attr;
    sai_fdb_event.attr_count = 3;
    MRVL_SAI_LOG_ENTER();


    while (1) {
        fpa_status = fpaLibBridgingAuMsgGet(SAI_DEFAULT_ETH_SWID_CNS, false, &fpa_au_event);
        if (fpa_status == FPA_OK) {
            /* Call SAI callback */
            if (mrvl_sai_notification_callbacks.on_fdb_event)
            {
                /* Convert FPA event to fdb_entry */
                if (SAI_STATUS_SUCCESS != mrvl_sai_fdb_event_convert_fpa_to_sai(&fpa_au_event, &sai_fdb_event)){
                    MRVL_SAI_LOG_ERR("Could not convert FPA message to SAI format\n");
	            }
                mrvl_sai_notification_callbacks.on_fdb_event(1,&sai_fdb_event);
            }
        } else if (fpa_status == FPA_NO_MORE) {
            MRVL_SAI_LOG_INF("No More AU messages pending\n");
        } else 
            MRVL_SAI_LOG_ERR("Got error from FPA AU Message Get\n");
    }
    return 0;
}


const sai_fdb_api_t fdb_api = {
    mrvl_sai_create_fdb_entry,
    mrvl_sai_remove_fdb_entry,
    mrvl_sai_set_fdb_entry_attribute,
    mrvl_sai_get_fdb_entry_attribute,
    mrvl_sai_flush_fdb_entries
};
