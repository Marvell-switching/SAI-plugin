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
#define __MODULE__ SAI_VLAN

#define MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id) ((vlan_id)>=1 && (vlan_id)<4095)

/* Storage layer data structures / variables to store the states */
/*static int number_of_vlans;*/

struct __vlan {
    sai_vlan_id_t id;
    int number_of_ports;
    sai_vlan_port_t* port_list;
};


static const sai_attribute_entry_t mrvl_sai_vlan_attribs[] = {   
    {   SAI_VLAN_ATTR_PORT_LIST, false, false, false, true,
        "Vlan port list", SAI_ATTR_VAL_TYPE_VLANPORTLIST
    },
    {   SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES, false, false, true, true,
        "Vlan Maximum number of learned MAC addresses", SAI_ATTR_VAL_TYPE_U32
    },
    {   SAI_VLAN_ATTR_STP_INSTANCE, false, false, true, true,
        "Vlan associated STP instance", SAI_ATTR_VAL_TYPE_U64
    },
    {   SAI_VLAN_ATTR_LEARN_DISABLE, false, false, true, true,
        "disable learning on a VLAN", SAI_ATTR_VAL_TYPE_BOOL
    },
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VAL_TYPE_UNDETERMINED
    }
};

/* Maximum number of learned MAC addresses [uint32_t]
 * zero means learning limit disable. (default to zero). */
static sai_status_t mrvl_sai_vlan_max_learned_addr_get_prv(_In_ const sai_object_key_t   *key,
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

/* Maximum number of learned MAC addresses [uint32_t]
 * zero means learning limit disable. (default to zero). */
static sai_status_t mrvl_sai_vlan_max_learned_addr_set_prv(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->u32 != 0) {
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* STP Instance that the VLAN is associated to [sai_object_id_t]
 * (default to default stp instance id)*/
static sai_status_t mrvl_sai_vlan_stp_get_prv(_In_ const sai_object_key_t   *key,
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

/* STP Instance that the VLAN is associated to [sai_object_id_t]
 * (default to default stp instance id)*/
static sai_status_t mrvl_sai_vlan_stp_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_status_t status;
    uint32_t     stp_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_STP_INSTANCE, &stp_id))) {
        return status;
    }
    if (stp_id != 0) { /* only default parameter is supported */
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT(); 
    return SAI_STATUS_SUCCESS;
}

    /** To disable learning on a VLAN. [bool] (CREATE_AND_SET)
      * (default set to false)
      * This should override port learn settings. If this is set to true on a vlan,
      * then the source mac learning is disabled for this vlan on a member port even
      * if learn is enable on the port(based on port learn attribute)
      */
static sai_status_t mrvl_sai_vlan_disable_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{

    MRVL_SAI_LOG_ENTER();

    value->booldata = false;

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

    /** To disable learning on a VLAN. [bool] (CREATE_AND_SET)
      * (default set to false)
      * This should override port learn settings. If this is set to true on a vlan,
      * then the source mac learning is disabled for this vlan on a member port even
      * if learn is enable on the port(based on port learn attribute)
      */
static sai_status_t mrvl_sai_vlan_disable_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    MRVL_SAI_LOG_ENTER();

    if (value->booldata != false) { /* only default parameter is supported */
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    MRVL_SAI_LOG_EXIT(); 
    return SAI_STATUS_SUCCESS;
}


/* List of ports in a VLAN [sai_vlan_port_list_t]*/
static sai_status_t mrvl_sai_vlan_port_list_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_vlan_id_t   vlan = key->vlan_id;
    uint32_t        list_size, group, port, counter = 0;
    uint64_t        cookie;
    FPA_STATUS      fpa_status;
    FPA_FLOW_TABLE_ENTRY_STC fpa_flow_entry;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_GROUP_BUCKET_ENTRY_STC bucket;
    
    MRVL_SAI_LOG_ENTER();
    list_size = value->vlanportlist.count;
    for (port = 0; port < SAI_MAX_NUM_OF_PORTS; port++) {
        cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
        fpa_flow_entry.cookie = cookie;
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, &fpa_flow_entry);
        if (fpa_status == FPA_NOT_FOUND) {
            continue;
        }
        if (counter < list_size) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &value->vlanportlist.list[counter].port_id)) {
                return SAI_STATUS_FAILURE;
            }
            /* read group id to grt tagging mode */
            parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
            parsed_group_identifier.portNum = port;
            parsed_group_identifier.vlanId = vlan;
            fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &group);
            if (fpa_status != FPA_OK){
                MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", vlan, port);
                return SAI_STATUS_FAILURE;
            }
            fpa_status = fpaLibGroupEntryBucketGet(SAI_DEFAULT_ETH_SWID_CNS, group, 0, &bucket);
            if (fpa_status != FPA_OK){
                MRVL_SAI_LOG_ERR("Failed to found group vlan %d port %d \n", vlan, port);
                return SAI_STATUS_FAILURE;
            }
            value->vlanportlist.list[counter].tagging_mode = (bucket.data.l2Interface.popVlanTagAction == true)? SAI_VLAN_PORT_UNTAGGED: SAI_VLAN_PORT_TAGGED;
        }
        counter++;
    }
    value->vlanportlist.count = counter;
    
    MRVL_SAI_LOG_EXIT(); 
    if (counter >= list_size) 
        return SAI_STATUS_BUFFER_OVERFLOW;
    else
        return SAI_STATUS_SUCCESS;
}

static const sai_vendor_attribute_entry_t mrvl_sai_vlan_vendor_attribs[] = {
    {   SAI_VLAN_ATTR_PORT_LIST,
        { false, false, false, true },
        { false, false, false, true },
        mrvl_sai_vlan_port_list_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_max_learned_addr_get_prv, NULL,
        mrvl_sai_vlan_max_learned_addr_set_prv, NULL
    },
    {   SAI_VLAN_ATTR_STP_INSTANCE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_stp_get_prv, NULL,
        mrvl_sai_vlan_stp_set_prv, NULL
    },
    {   SAI_VLAN_ATTR_LEARN_DISABLE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_disable_get_prv, NULL,
        mrvl_sai_vlan_disable_set_prv, NULL
    },
};
static void mrvl_sai_vlan_key_to_str(_In_ sai_vlan_id_t vlan_id, _Out_ char *key_str)
{
    snprintf(key_str, MAX_KEY_STR_LEN, "vlan %u", vlan_id);
}

/*
 * Routine Description:
 *    Set VLAN attribute Value
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_set_vlan_attribute(_In_ sai_vlan_id_t vlan_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .vlan_id = vlan_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    return mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs, attr);
}


/*
 * Routine Description:
 *    Get VLAN attribute Value
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_vlan_attribute(_In_ sai_vlan_id_t       vlan_id,
                                     _In_ uint32_t            attr_count,
                                     _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .vlan_id = vlan_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    return mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs, attr_count, attr_list);
}


/*
 * Routine Description:
 *    Remove VLAN configuration (remove all VLANs).
 *
 * Arguments:
 *    None
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_all_vlans(void)
{
    MRVL_SAI_LOG_NTC("Remove all vlan\n");

    return SAI_STATUS_SUCCESS;
}


/*
 * Routine Description:
 *    Create a VLAN
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_create_vlan(_In_ sai_vlan_id_t vlan_id)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Create vlan %s\n", key_str);

    // make sure the given vlan_id satisfies the spec
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


/*
 * Routine Description:
 *    Remove a VLAN
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_vlan(_In_ sai_vlan_id_t vlan_id)
{
    char key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Remove vlan %s\n", key_str);

    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }
    
    MRVL_SAI_LOG_EXIT();

    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Add Port to VLAN
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *    [in] port_count - number of ports
 *    [in] port_list - pointer to membership structures
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_add_ports_to_vlan(_In_ sai_vlan_id_t          vlan_id,
                                    _In_ uint32_t               port_count,
                                    _In_ const sai_vlan_port_t* port_list)
{
    char                        key_str[MAX_KEY_STR_LEN];
    uint32_t                    input_index, port;
    sai_status_t                status;
    sai_vlan_tagging_mode_t     tagging_mode;
    uint32_t                    group;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;
    
    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Add ports to vlan %s\n", key_str);
    
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }


    if ((NULL == port_list) || (port_count == 0)) {
        MRVL_SAI_LOG_ERR("NULL value param or zero ports \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (input_index = 0; input_index < port_count; input_index++) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_list[input_index].port_id, SAI_OBJECT_TYPE_PORT, &port))) {
            return status;
        }
        /* skip CPU port, which doesn't need to be added/removed to vlan */
        if (SAI_CPU_PORT_CNS == port) {
            MRVL_SAI_LOG_NTC("add port to vlan %u - Skip CPU port\n", vlan_id);
            continue;
        }
        tagging_mode = port_list[input_index].tagging_mode;
        if (tagging_mode >= SAI_VLAN_PORT_PRIORITY_TAGGED){
            MRVL_SAI_LOG_ERR("Vlan port priority tagged %d not supported\n", tagging_mode);
            return SAI_STATUS_NOT_SUPPORTED;
        }

        /* add group L2 interface, if group exist update tagging mode */
         status = mrvl_sai_utl_create_l2_int_group(port, vlan_id, tagging_mode, true, &group);
         if (status != SAI_STATUS_SUCCESS)
             return status;
         
         fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, &fpa_flow_entry);
         if (fpa_status != FPA_OK) {
             MRVL_SAI_LOG_ERR("Failed to init VLAN entry status = %d\n", fpa_status);
             return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
         }
         
         /* set vlan entry */
         cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_id, port);
         fpa_flow_entry.cookie = cookie; 
         fpa_flow_entry.data.vlan.vlanId     = vlan_id;
         fpa_flow_entry.data.vlan.vlanIdMask = FPA_FLOW_VLAN_MASK_TAG;
         fpa_flow_entry.data.vlan.inPort = port;
         fpa_flow_entry.data.vlan.newTagVid = FPA_FLOW_VLAN_IGNORE_VAL;
         fpa_flow_entry.data.vlan.newTagPcp = FPA_FLOW_VLAN_IGNORE_VAL;


         fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, &fpa_flow_entry);
         if (status != FPA_OK) {
             MRVL_SAI_LOG_ERR("Failed to add VLAN entry %llx to fpa flow table status = %d\n", cookie, fpa_status);
             return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
         }
    }
    
    return SAI_STATUS_SUCCESS;
}


/*
 * Routine Description:
 *    Remove Port from VLAN
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *    [in] port_count - number of ports
 *    [in] port_list - pointer to membership structures
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_ports_from_vlan(_In_ sai_vlan_id_t          vlan_id,
                                         _In_ uint32_t               port_count,
                                         _In_ const sai_vlan_port_t* port_list)
{
    char                        key_str[MAX_KEY_STR_LEN];
    uint32_t                    input_index, port;
    sai_status_t                status;
    uint32_t                    group;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;
    
    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_key_to_str(vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Remove ports from vlan %s\n", key_str);
    
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_id);
        return SAI_STATUS_INVALID_VLAN_ID;
    }


    if ((NULL == port_list) || (port_count == 0)) {
        MRVL_SAI_LOG_ERR("NULL value param or zero ports \n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    for (input_index = 0; input_index < port_count; input_index++) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_list[input_index].port_id, SAI_OBJECT_TYPE_PORT, &port))) {
            return status;
        }
        /* skip CPU port, which doesn't need to be added/removed to vlan */
        if (SAI_CPU_PORT_CNS == port) {
            MRVL_SAI_LOG_NTC("remove port from vlan %u - Skip CPU port\n", vlan_id);
            continue;
        }
         
         /* delete vlan entry */
         memset(&fpa_flow_entry, 0, sizeof(fpa_flow_entry));
         cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_id, port);
         fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_VLAN_E, cookie);
         if (fpa_status != FPA_OK) {
             MRVL_SAI_LOG_ERR("Failed to delete entry %llx from VLAN table status = %d\n", cookie, fpa_status);
             return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
         }

         parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
         parsed_group_identifier.portNum = port;
         parsed_group_identifier.vlanId = vlan_id;
         fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &group);
         if (fpa_status != FPA_OK){
             MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", vlan_id, port);
             return SAI_STATUS_FAILURE;
         }
         fpa_status = fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, group);
         if (fpa_status == FPA_NOT_READY){
             MRVL_SAI_LOG_ERR("Failed to delete group vlan %d port %d (used by other tables)\n", vlan_id, port);
         }
    }
    
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *   Get vlan statistics counters.
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *    [in] counter_ids - specifies the array of counter ids
 *    [in] number_of_counters - number of counters in the array
 *    [out] counters - array of resulting counter values.
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_vlan_stats(_In_ sai_vlan_id_t                  vlan_id,
                                 _In_ const sai_vlan_stat_counter_t *counter_ids,
                                 _In_ uint32_t                       number_of_counters,
                                 _Out_ uint64_t                    * counters)
{
    uint32_t ii;

    UNREFERENCED_PARAMETER(vlan_id);
    UNREFERENCED_PARAMETER(number_of_counters);

    MRVL_SAI_LOG_ENTER();

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == counters) {
        MRVL_SAI_LOG_ERR("NULL counters array param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (ii = 0; ii < number_of_counters; ii++) {
        counters[ii] = 0;
    }

    MRVL_SAI_LOG_EXIT();
    return  SAI_STATUS_NOT_IMPLEMENTED;
}

/*
 * Routine Description:
 *   Clear vlan statistics counters.
 *
 * Arguments:
 *    [in] vlan_id - VLAN id
 *    [in] counter_ids - specifies the array of counter ids
 *    [in] number_of_counters - number of counters in the array
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_clear_vlan_stats(_In_ sai_vlan_id_t                  vlan_id,
                                 _In_ const sai_vlan_stat_counter_t *counter_ids,
                                 _In_ uint32_t                       number_of_counters)
{

    UNREFERENCED_PARAMETER(vlan_id);
    UNREFERENCED_PARAMETER(number_of_counters);

    MRVL_SAI_LOG_ENTER();

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    MRVL_SAI_LOG_EXIT();
    return  SAI_STATUS_NOT_IMPLEMENTED;
}


const sai_vlan_api_t vlan_api = {
    mrvl_sai_create_vlan,
    mrvl_sai_remove_vlan,
    mrvl_set_vlan_attribute,
    mrvl_sai_get_vlan_attribute,
    mrvl_sai_add_ports_to_vlan,
    mrvl_sai_remove_ports_from_vlan,
    mrvl_sai_remove_all_vlans,
    mrvl_sai_get_vlan_stats,
    mrvl_sai_clear_vlan_stats
};
