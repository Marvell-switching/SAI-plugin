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

/* VLAN database - holds info after VLAN creation */
static mrvl_vlan_info_t mrvl_sai_vlan_db[SAI_MAX_NUM_OF_VLANS] = {};

/**************************/
/*! VLAN package marcros !*/
/**************************/
#define MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id) ((vlan_id >= 1) && (vlan_id < 4095))

sai_status_t mrvl_sai_vlan_member_object_create(_In_ sai_vlan_id_t      vlan_id,
                                                _In_ uint32_t           bport_index,
                                                _Out_ sai_object_id_t   *vlan_member_id);

sai_status_t mrvl_sai_vlan_member_object_to_vlan_bport(_In_ sai_object_id_t  vlan_member_id,
                                                      _Out_ sai_vlan_id_t   *vlan_id,
                                                      _Out_ mrvl_bridge_port_info_t **bport);

static const sai_attribute_entry_t mrvl_sai_vlan_attribs[] = {  
    {   SAI_VLAN_ATTR_VLAN_ID, true, true, false, true,
        "Vlan id", SAI_ATTR_VALUE_TYPE_UINT16
    }, 
    {   SAI_VLAN_ATTR_MEMBER_LIST, false, false, false, true,
        "Vlan member list", SAI_ATTR_VALUE_TYPE_OBJECT_LIST
    },
    {   SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES, false, false, true, true,
        "Vlan Maximum number of learned MAC addresses", SAI_ATTR_VALUE_TYPE_UINT32
    },
    {   SAI_VLAN_ATTR_STP_INSTANCE, false, false, true, true,
        "Vlan associated STP instance", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_ATTR_LEARN_DISABLE, false, false, true, true,
        "disable learning on a VLAN", SAI_ATTR_VALUE_TYPE_BOOL
    },
    {   SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE, false, false, false, true,
        "Set IPv4 MC lookup on VLAN", SAI_ATTR_VALUE_TYPE_INT32
    },
    {   SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE, false, false, false, true,
        "Set IPv6 MC lookup on VLAN", SAI_ATTR_VALUE_TYPE_INT32
    },
    {   SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID, false, false, false, true,
        "VLAN unknown NON IP MC output group ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID, false, false, false, true,
        "VLAN unknown IPv4 MC output group ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID, false, false, false, true,
        "VLAN unknown IPv6 MC output group ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID, false, false, false, true,
        "VLAN unknown link local MC output group ID", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_ATTR_INGRESS_ACL, false, true, true, true,
        "VLAN bind to ingress acl", SAI_ATTR_VALUE_TYPE_OBJECT_ID 
    },
    {   SAI_VLAN_ATTR_EGRESS_ACL, false, true, true, true,
        "VLAN bind to egress acl", SAI_ATTR_VALUE_TYPE_OBJECT_ID 
    },
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};

static const sai_attribute_entry_t mrvl_sai_vlan_member_attribs[] = {   
    {   SAI_VLAN_MEMBER_ATTR_VLAN_ID, true, true, false, true,
        "Vlan member's VLAN id", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID, true, true, false, true,
        "Port id", SAI_ATTR_VALUE_TYPE_OBJECT_ID
    },
    {   SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE, false, true, true, true,
        "Vlan tagging mode", SAI_ATTR_VALUE_TYPE_INT32
    },
    {   END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
        "", SAI_ATTR_VALUE_TYPE_UNDETERMINED
    }
};
/* SAI_VLAN_ATTR_INGRESS_ACL and SAI_VLAN_ATTR_EGRESS_ACL [sai_object_id_t]
 * 
 */
sai_status_t mrvl_sai_vlan_acl_set_prv(_In_ const sai_object_key_t      *key,
                                       _In_ const sai_attribute_value_t *value,
                                       _In_ void                        *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t vlan_idx;
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
        MRVL_SAI_API_RETURN(status);
    }
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    if (value->oid == SAI_NULL_OBJECT_ID){
    	/* unbind action */
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_unbind_from_vlan(arg, vlan_idx))){
            MRVL_SAI_LOG_ERR("Unable to unbind vlan %d from ACL TABLE\n", vlan_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    	}
    }
    else {
    	if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_table_bind_to_vlan(arg, value->oid, vlan_idx))){
            MRVL_SAI_LOG_ERR("Unable to bind vlan %d to ACL TABLE\n", vlan_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    	}
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}
static sai_status_t mrvl_sai_vlan_acl_get_prv(_In_ const sai_object_key_t   *key,
                                         _Inout_ sai_attribute_value_t *value,
                                         _In_ uint32_t                  attr_index,
                                         _Inout_ vendor_cache_t        *cache,
                                         void                          *arg)
{
    sai_status_t            status = SAI_STATUS_SUCCESS;
    uint32_t                vlan_idx;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
        MRVL_SAI_API_RETURN(status);
    }
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_acl_get_table_id_per_vlan(arg, vlan_idx, value))){
        MRVL_SAI_LOG_ERR("Unable to get assigned ACL table per vlan %d\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_vlan_id_get_prv(_In_ const sai_object_key_t   *key,
                                             _Inout_ sai_attribute_value_t *value,
                                             _In_ uint32_t                  attr_index,
                                             _Inout_ vendor_cache_t        *cache,
                                              void                          *arg)
{
    uint32_t        vlan_idx;
    sai_status_t    status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
        MRVL_SAI_API_RETURN(status);
    }
    value->u16 = vlan_idx;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_vlan_id_set_prv(_In_ const sai_object_key_t      *key,
                                             _In_ const sai_attribute_value_t *value,
                                              void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->u16 != 0) {
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

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
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* Maximum number of learned MAC addresses [uint32_t]
 * zero means learning limit disable. (default to zero). */
static sai_status_t mrvl_sai_vlan_max_learned_addr_set_prv(_In_ const sai_object_key_t      *key,
                                                    _In_ const sai_attribute_value_t *value,
                                                    void                             *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->u32 != 0) {
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* STP Instance that the VLAN is associated to [sai_object_id_t]
 * (default to default stp instance id)*/
static sai_status_t mrvl_sai_vlan_stp_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP, 1, &value->oid))) {
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/* STP Instance that the VLAN is associated to [sai_object_id_t]
 * (default to default stp instance id)*/
static sai_status_t mrvl_sai_vlan_stp_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    uint32_t        stp_id;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_STP, &stp_id))) {
        MRVL_SAI_API_RETURN(status);
    }
    if (stp_id != 0) { /* only default parameter is supported */
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    }
    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
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
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
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
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    }
    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static sai_status_t mrvl_sai_vlan_IPv4_MC_lookup_key_type_get_prv(_In_ const sai_object_key_t   *key,
                                                                   _Inout_ sai_attribute_value_t *value,
                                                                   _In_ uint32_t                  attr_index,
                                                                   _Inout_ vendor_cache_t        *cache,
                                                                   void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->s32 != SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_MAC_DA) { /* only default parameter is supported */
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    }
    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_sai_vlan_IPv6_MC_lookup_key_type_get_prv(_In_ const sai_object_key_t   *key,
                                                                  _Inout_ sai_attribute_value_t *value,
                                                                  _In_ uint32_t                  attr_index,
                                                                  _Inout_ vendor_cache_t        *cache,
                                                                  void                          *arg)
{
    MRVL_SAI_LOG_ENTER();
    if (value->s32 != SAI_VLAN_MCAST_LOOKUP_KEY_TYPE_MAC_DA) { /* only default parameter is supported */
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    }
    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_sai_vlan_unknown_MC_output_group_get_prv(_In_ const sai_object_key_t   *key,
                                                                  _Inout_ sai_attribute_value_t *value,
                                                                  _In_ uint32_t                  attr_index,
                                                                  _Inout_ vendor_cache_t        *cache,
                                                                  void                          *arg)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t     vlan_idx;
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
        MRVL_SAI_API_RETURN(status);
    }
    /* make sure the given vlan_id satisfies the spec*/
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_L2MC_GROUP, 1, &value->oid))) {
        MRVL_SAI_LOG_ERR("Error creating l2 MC group object\n");
        MRVL_SAI_API_RETURN(status);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

/* List of ports in a VLAN [sai_object_list_t]*/
static sai_status_t mrvl_sai_vlan_member_list_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    uint32_t        port, counter = 0, vlan;
    mrvl_bridge_port_info_t *bport;
    sai_object_id_t *vlan_port_list = NULL;
    sai_status_t    status = SAI_STATUS_SUCCESS;
    mrvl_ports_bitmap_t empty_ports_bmp;
    MRVL_SAI_LOG_ENTER();
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(key->key.object_id, SAI_OBJECT_TYPE_VLAN, &vlan)) {
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    for (port = 0; port < SAI_MAX_NUM_OF_PORTS; port++) 
    {
        if (SAI_STATUS_SUCCESS == mrvl_sai_port_to_bport(port, &bport))
            if (mrvl_bmp_is_bit_set_MAC(mrvl_sai_vlan_db[vlan-1].ports_bitmap, bport->index)) 
                counter++;
    }

    if (0 == counter)
    {
        value->objlist.count = 0;
        free(vlan_port_list);
        MRVL_SAI_LOG_ERR("Vlan %d has 0 members\n", vlan);
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    
    vlan_port_list = (sai_object_id_t*)malloc(sizeof(sai_object_id_t) * counter);
    if (!vlan_port_list) {
        MRVL_SAI_LOG_ERR("Failed to allocate memory for vlan_port_list\n");
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_NO_MEMORY);
    }

    counter = 0;
    for (port = 0; port < SAI_MAX_NUM_OF_PORTS; port++)
    {
        if (SAI_STATUS_SUCCESS == mrvl_sai_port_to_bport(port, &bport))
        {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_vlan_member_object_create(vlan, bport->logical_port, &vlan_port_list[counter++]))) {
                MRVL_SAI_LOG_ERR("Failed to create VLAN member object for VLAN %d, port %d\n", vlan, bport->logical_port);
                free(vlan_port_list);
                MRVL_SAI_API_RETURN(status);
            }
        }
    }
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_fill_objlist(vlan_port_list, counter, &value->objlist)))
    {
         MRVL_SAI_LOG_ERR("Failed to fill vlan ports objlist\n");
         free(vlan_port_list);
         MRVL_SAI_API_RETURN(status);
    }
    
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** VLAN ID [sai_vlan_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_vlan_member_get_vlan_id_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
     sai_object_id_t            vlan_member_id = key->key.object_id;
     sai_vlan_id_t              vlan_id;
     mrvl_bridge_port_info_t    *bport;
    
    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_member_object_to_vlan_bport(vlan_member_id, &vlan_id, &bport)) {
        MRVL_SAI_LOG_ERR("Failed to convert vlan member oid %" PRIx64 " to vid and bridge port\n", vlan_member_id);
    	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_idx <= 4094\n", vlan_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan_id, &value->oid)) {
        MRVL_SAI_LOG_ERR("Failed to convert VLAN member object to type\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}


/** logical port ID [sai_object_id_t] (MANDATORY_ON_CREATE|CREATE_ONLY) */
static sai_status_t mrvl_sai_vlan_member_port_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_object_id_t         vlan_member_id = key->key.object_id;
    sai_vlan_id_t           vlan_id;
    mrvl_bridge_port_info_t *bport;
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_vlan_member_object_to_vlan_bport(vlan_member_id, &vlan_id, &bport))) {
        MRVL_SAI_LOG_ERR("Failed to convert vlan member oid %" PRIx64 " to vid and bridge port\n", vlan_member_id);
    	MRVL_SAI_API_RETURN(status);
    }
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE_PORT, bport->logical_port, &value->oid))) {
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** VLAN tagging mode [sai_vlan_tagging_mode_t] (CREATE_AND_SET)
 *     (default to SAI_VLAN_TAGGING_MODE_UNTAGGED) */
static sai_status_t mrvl_sai_vlan_member_tagging_get_prv(_In_ const sai_object_key_t   *key,
                                           _Inout_ sai_attribute_value_t *value,
                                           _In_ uint32_t                  attr_index,
                                           _Inout_ vendor_cache_t        *cache,
                                           void                          *arg)
{
    sai_object_id_t         vlan_member_id = key->key.object_id;
    sai_vlan_id_t           vlan_id;
    mrvl_bridge_port_info_t *bport;
    sai_vlan_tagging_mode_t tag_mode;
    sai_status_t            status = SAI_STATUS_SUCCESS;
    
    MRVL_SAI_LOG_ENTER();
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_vlan_member_object_to_vlan_bport(vlan_member_id, &vlan_id, &bport))) {
        MRVL_SAI_LOG_ERR("Failed to convert vlan member oid %" PRIx64 " to vid and bridge port\n", vlan_member_id);
    	MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_l2_int_group_get_tagging_mode(bport->logical_port, vlan_id, &tag_mode)){
        MRVL_SAI_LOG_ERR("Failed to get tagging mode for port %d, vlan %d\n", bport->logical_port, vlan_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }

    value->s32 = tag_mode;

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/** VLAN tagging mode [sai_vlan_tagging_mode_t] (CREATE_AND_SET)
 *     (default to SAI_VLAN_TAGGING_MODE_UNTAGGED) */
static sai_status_t mrvl_sai_vlan_member_tagging_set_prv(_In_ const sai_object_key_t *key, _In_ const sai_attribute_value_t *value, void *arg)
{
    sai_object_id_t  vlan_member_id = key->key.object_id;
    uint32_t         vlan_member;
    sai_vlan_tagging_mode_t tag_mode;

    MRVL_SAI_LOG_ENTER();
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    tag_mode = value->s32;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_l2_int_group_set_tagging_mode((vlan_member & 0xFFFF), ((vlan_member >> 16) & 0xFFFF), tag_mode)){
        MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_sai_vlan_member_object_create(_In_ sai_vlan_id_t      vlan_id,
                                                _In_ uint32_t           bport_index,
                                                _Out_ sai_object_id_t   *vlan_member_id)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    uint32_t        vlan_member;

    MRVL_SAI_LOG_ENTER();
    assert(vlan_member_id);

    vlan_member = ((vlan_id << 16) | bport_index);
    status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, vlan_member_id);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed to create VLAN member with bport %d, vlan %d\n", bport_index, vlan_id);
        MRVL_SAI_API_RETURN(status);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_vlan_member_object_to_vlan_bport(_In_ sai_object_id_t  vlan_member_id,
                                                      _Out_ sai_vlan_id_t   *vlan_id,
                                                      _Out_ mrvl_bridge_port_info_t **bport)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    sai_vlan_id_t   vlan_idx;
    uint32_t        port, vlan_member;

    MRVL_SAI_LOG_ENTER();

    assert(vlan_id);
    assert(bport);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))) {
    	MRVL_SAI_API_RETURN(status);
    }
    vlan_idx = ((vlan_member >> 16) & 0xFFFF);
    port = (vlan_member & 0xFFFF);
    
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    
    /* skip CPU port, which doesn't need to be added/removed to vlan */
    if (SAI_CPU_PORT_CNS == port) {
        MRVL_SAI_LOG_NTC("Invalid port - CPU port\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PORT_NUMBER);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_port_to_bport(port, bport))) {
        MRVL_SAI_API_RETURN(status);
    }

    *vlan_id = vlan_idx;
    
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

void mrvl_sai_vlan_portmap_set(sai_vlan_id_t vid, mrvl_bridge_port_info_t *bport, bool is_set)
{
    uint32_t                    i, lag_idx, port;
    sai_object_id_t             port_id[SAI_LAG_MAX_MEMBERS_IN_GROUP_CNS] = {0};
    sai_object_list_t           port_objlist;
    sai_status_t                status = SAI_STATUS_SUCCESS;

    assert(bport->index < SAI_MAX_NUM_OF_BRIDGE_PORTS);
    if (is_set && !mrvl_bmp_is_bit_set_MAC(mrvl_sai_vlan_db[vid-1].ports_bitmap, bport->index)) 
    {
        mrvl_bmp_set_bit_MAC(mrvl_sai_vlan_db[vid-1].ports_bitmap, bport->index);
        bport->vlan_ref_count++;
    } else if (!is_set && mrvl_bmp_is_bit_set_MAC(mrvl_sai_vlan_db[vid-1].ports_bitmap, bport->index)) {
        mrvl_bmp_clear_bit_MAC(mrvl_sai_vlan_db[vid-1].ports_bitmap, bport->index);
        bport->vlan_ref_count--;
    }
}

sai_status_t mrvl_sai_vlan_port_add(_In_ mrvl_bridge_port_info_t *bport, _In_ sai_vlan_id_t vlan_idx, sai_vlan_tagging_mode_t tagging_mode)
{
    sai_status_t                status = SAI_STATUS_SUCCESS;
    bool                        egressVlanIsTag;
    uint64_t                    cookie;
    FPA_STATUS                  fpa_status;

    MRVL_SAI_LOG_ENTER();
    
    cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_idx, bport->logical_port);
    egressVlanIsTag = (tagging_mode == SAI_VLAN_TAGGING_MODE_TAGGED)? true: false;
    fpa_status = fpaLibVlanSetMember(SAI_DEFAULT_ETH_SWID_CNS, 
                                     MRVL_SAI_UTL_ADD, 
                                     cookie, 
                                     bport->logical_port, 
                                     vlan_idx, 
                                     true, 
                                     egressVlanIsTag, 
                                     FPA_FLOW_VLAN_IGNORE_VAL, 
                                     FPA_FLOW_VLAN_IGNORE_VAL);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to add port %d to VLAN %d, ingress: tagged, egress: %s, status = 0x%x\n", 
                         bport->logical_port, vlan_idx, egressVlanIsTag == true ? "tagged" : "untagged", fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }
     
    mrvl_sai_vlan_portmap_set(vlan_idx, bport, true);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}

sai_status_t mrvl_sai_vlan_port_remove(_In_ mrvl_bridge_port_info_t *bport, _In_ uint32_t vlan_idx)
{
    uint64_t                    cookie;
    sai_status_t                status = SAI_STATUS_SUCCESS;
    FPA_STATUS                  fpa_status;

    /* delete vlan entry */
    cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_idx, bport->logical_port);
    fpa_status = fpaLibVlanSetMember(SAI_DEFAULT_ETH_SWID_CNS,
                                     MRVL_SAI_UTL_DEL, 
                                     cookie, 
                                     bport->logical_port, 
                                     vlan_idx, 
                                     false, /* irrelevant */
                                     false, /* irrelevant */
                                     FPA_FLOW_VLAN_IGNORE_VAL, /* irrelevant */
                                     FPA_FLOW_VLAN_IGNORE_VAL /* irrelevant */);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to remove port %d from VLAN %d, status = 0x%x\n", bport->logical_port, vlan_idx, fpa_status);
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_vlan_portmap_set(vlan_idx, bport, false);
    MRVL_SAI_LOG_EXIT(); 
    MRVL_SAI_API_RETURN(status);
}
sai_status_t mrvl_sai_vlan_lag_port_update(_In_ uint32_t lag, _In_ uint32_t port, bool add)
{
    uint32_t                    vlan_id;
    uint64_t                    cookie;
    sai_vlan_tagging_mode_t     tagging_mode;
    mrvl_bridge_port_info_t     *bport, *bridge_port;
    sai_status_t                status = SAI_STATUS_SUCCESS;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_STATUS                  fpa_status;

    if (add)
    {
        /* Remove port from VLANs it is member of */
        for (vlan_id = 1; vlan_id < SAI_MAX_NUM_OF_VLANS; vlan_id++) 
        {
            cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_id, port);
            fpa_flow_entry.cookie = cookie;
            fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, 
                                                FPA_FLOW_TABLE_TYPE_VLAN_E, 
                                                &fpa_flow_entry);
            if (FPA_NOT_FOUND == fpa_status) {
                continue;
            }
            else if (FPA_OK == fpa_status)
            {
                fpa_status = fpaLibVlanSetMember(SAI_DEFAULT_ETH_SWID_CNS,
                                                         MRVL_SAI_UTL_DEL, 
                                                         cookie, 
                                                         port, 
                                                         vlan_id, 
                                                         false, /* irrelevant */
                                                         false, /* irrelevant */
                                                         FPA_FLOW_VLAN_IGNORE_VAL, /* irrelevant */
                                                         FPA_FLOW_VLAN_IGNORE_VAL /* irrelevant */);
                if (FPA_OK != fpa_status) {
                    MRVL_SAI_LOG_ERR("Failed to remove port %d from VLAN %d, status = 0x%x\n", port, vlan_id, fpa_status);
                    status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                    MRVL_SAI_API_RETURN(status);
                }
            }
        }
        
        /* Add port to VLANs the LAG is a member of */
        status = mrvl_sai_port_to_bport(lag, &bport);
        if (SAI_STATUS_SUCCESS == status)
        {
            if (bport->vlan_ref_count > 0)
            {
                for (vlan_id = 1; vlan_id < SAI_MAX_NUM_OF_VLANS; vlan_id++) 
                {
                    if (mrvl_bmp_is_bit_set_MAC(mrvl_sai_vlan_db[vlan_id-1].ports_bitmap, bport->index))
                    {
                        cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_id, port);
                        fpa_status = fpaLibVlanSetMember(SAI_DEFAULT_ETH_SWID_CNS, 
                                                         MRVL_SAI_UTL_ADD, 
                                                         cookie, 
                                                         port, 
                                                         vlan_id, 
                                                         true, 
                                                         true, 
                                                         FPA_FLOW_VLAN_IGNORE_VAL, 
                                                         FPA_FLOW_VLAN_IGNORE_VAL);
                        if (fpa_status != FPA_OK) {
                            MRVL_SAI_LOG_ERR("Failed to add port %d to VLAN %d, ingress: tagged, egress: tagged, status = 0x%x\n", 
                                             port, vlan_id, fpa_status);
                            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                        }
                    }
                }
            }
        }
    }
    else /* remove */
    {
        /* Remove port from VLANs the LAG is a member of */
        status = mrvl_sai_port_to_bport(lag, &bport);
        if (SAI_STATUS_SUCCESS == status)
        {
            if (bport->vlan_ref_count > 0)
            {
                for (vlan_id = 1; vlan_id < SAI_MAX_NUM_OF_VLANS; vlan_id++) 
                {
                    if (mrvl_bmp_is_bit_set_MAC(mrvl_sai_vlan_db[vlan_id-1].ports_bitmap, bport->index))
                    {
                        /* delete vlan entry */
                        cookie = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan_id, port);
                        fpa_status = fpaLibVlanSetMember(SAI_DEFAULT_ETH_SWID_CNS,
                                                         MRVL_SAI_UTL_DEL, 
                                                         cookie, 
                                                         port, 
                                                         vlan_id, 
                                                         false, /* irrelevant */
                                                         false, /* irrelevant */
                                                         FPA_FLOW_VLAN_IGNORE_VAL, /* irrelevant */
                                                         FPA_FLOW_VLAN_IGNORE_VAL /* irrelevant */);
                        if (fpa_status != FPA_OK) {
                            MRVL_SAI_LOG_ERR("Failed to remove port %d from VLAN %d, status = 0x%x\n", port, vlan_id, fpa_status);
                            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
                            MRVL_SAI_API_RETURN(status);
                        }
                    }
                }
            }
        }
        /* Reset port PVID to default VLAN */
        fpa_status = fpaLibPortPvidSet(SAI_DEFAULT_ETH_SWID_CNS, port, SAI_DEFAULT_VLAN_CNS);
        if (fpa_status != FPA_OK) {
            MRVL_SAI_LOG_ERR("Failed to set port %d with pvid %d, status 0x%x\n",
                             port, SAI_DEFAULT_VLAN_CNS, fpa_status);
            status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
            MRVL_SAI_API_RETURN(status);
        }
    }


    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

static const sai_vendor_attribute_entry_t mrvl_sai_vlan_vendor_attribs[] = {
    {   SAI_VLAN_ATTR_VLAN_ID,
        { true, true, true, true },
        { true, true, true, true },
        mrvl_sai_vlan_id_get_prv, NULL,
        mrvl_sai_vlan_id_set_prv, NULL
    },
    {   SAI_VLAN_ATTR_MEMBER_LIST,
        { true, true, true, true },
        { true, true, true, true },
        mrvl_sai_vlan_member_list_get_prv, NULL,
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
    {   SAI_VLAN_ATTR_IPV4_MCAST_LOOKUP_KEY_TYPE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_IPv4_MC_lookup_key_type_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_IPV6_MCAST_LOOKUP_KEY_TYPE,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_IPv6_MC_lookup_key_type_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_unknown_MC_output_group_get_prv, (void*)SAI_VLAN_ATTR_UNKNOWN_NON_IP_MCAST_OUTPUT_GROUP_ID,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_unknown_MC_output_group_get_prv, (void*)SAI_VLAN_ATTR_UNKNOWN_IPV4_MCAST_OUTPUT_GROUP_ID,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_unknown_MC_output_group_get_prv, (void*)SAI_VLAN_ATTR_UNKNOWN_IPV6_MCAST_OUTPUT_GROUP_ID,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_unknown_MC_output_group_get_prv, (void*)SAI_VLAN_ATTR_UNKNOWN_LINKLOCAL_MCAST_OUTPUT_GROUP_ID,
        NULL, NULL
    },
    {   SAI_VLAN_ATTR_INGRESS_ACL,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_acl_get_prv, (void*)SAI_VLAN_ATTR_INGRESS_ACL,
        mrvl_sai_vlan_acl_set_prv, (void*)SAI_VLAN_ATTR_INGRESS_ACL 
    },
    {   SAI_VLAN_ATTR_EGRESS_ACL,
        { false, false, true, true },
        { false, false, true, true },
        mrvl_sai_vlan_acl_get_prv, (void*)SAI_VLAN_ATTR_EGRESS_ACL,
        mrvl_sai_vlan_acl_set_prv, (void*)SAI_VLAN_ATTR_EGRESS_ACL },
    { END_FUNCTIONALITY_ATTRIBS_ID,
      { false, false, false, false },
      { false, false, false, false },
      NULL, NULL,
      NULL, NULL }
};
static const sai_vendor_attribute_entry_t mrvl_sai_vlan_member_vendor_attribs[] = {
    {   SAI_VLAN_MEMBER_ATTR_VLAN_ID,
        { true, false, false, true },
        { true, false, false, true },
        mrvl_sai_vlan_member_get_vlan_id_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID,
        { true, false, false, true },
        { true, false, false, true },
        mrvl_sai_vlan_member_port_get_prv, NULL,
        NULL, NULL
    },
    {   SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE,
        { true, false, true, true },
        { true, false, true, true },
        mrvl_sai_vlan_member_tagging_get_prv, NULL,
        mrvl_sai_vlan_member_tagging_set_prv, NULL
    }
};


static void mrvl_sai_vlan_id_to_str(_In_ sai_object_id_t vlan_id, _Out_ char *key_str)
{
    uint32_t vlan_idx;
    if (SAI_STATUS_SUCCESS !=  mrvl_sai_utl_object_to_type(vlan_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx)) {
        return;
    }
    snprintf(key_str, MAX_KEY_STR_LEN, "vlan %u", vlan_idx);
}

static void mrvl_sai_vlan_member_key_to_str(_In_ sai_object_id_t vlan_member_id, _Out_ char *key_str)
{
    uint32_t vlan_member;

    if (SAI_STATUS_SUCCESS !=  mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member)) {
        return;
    } 
    snprintf(key_str, MAX_KEY_STR_LEN, "vlan member: vlan %u port %d", ((vlan_member >> 16) & 0xFFFF), (vlan_member & 0xFFFF));
}

/**
* @brief Set VLAN Attribute
*
* @param[in] vlan_id VLAN ID
* @param[in] attr Attribute structure containing ID and value
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_set_vlan_attribute(_In_ sai_object_id_t vlan_id,
                                         _In_ const sai_attribute_t *attr)

{
    const sai_object_key_t key = { .key.object_id = vlan_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_id_to_str(vlan_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**
* @brief Get VLAN Attribute
*
* @param[in] vlan_id VLAN ID
* @param[in] attr_count Number of attributes
* @param[inout] attr_list List of attribute structures containing ID and value
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_get_vlan_attribute(_In_ sai_object_id_t vlan_id,
                                         _In_ uint32_t attr_count,
                                         _Inout_ sai_attribute_t *attr_list)

{
    const sai_object_key_t  key = { .key.object_id = vlan_id };
    char                    key_str[MAX_KEY_STR_LEN];
    sai_status_t            status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == vlan_id) {
        MRVL_SAI_LOG_ERR("NULL VLAN id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }
    mrvl_sai_vlan_id_to_str(vlan_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}


/**
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
    char            key_str[MAX_KEY_STR_LEN];
    uint32_t        ii, vlan_idx;
    sai_status_t    status = SAI_STATUS_SUCCESS;
    sai_object_id_t vlan_id;

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Remove all vlans\n");

    for (ii = 0; ii < SAI_MAX_NUM_OF_VLANS; ii++)
    {
        if (mrvl_sai_vlan_db[ii].is_created)
        {
            vlan_idx = ii+1;
            memset(&vlan_id, 0, sizeof(sai_object_id_t));

            /* create SAI VLAN object */    
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan_idx, &vlan_id))) {
                MRVL_SAI_LOG_ERR("Failed to create object VLAN by index %d\n", vlan_idx);
                MRVL_SAI_API_RETURN(status);
            }
            status = mrvl_sai_remove_vlan(vlan_id);
            if (SAI_STATUS_SUCCESS != status)
            {
                MRVL_SAI_LOG_ERR("Failed to remove VLAN %d\n", vlan_idx);
                MRVL_SAI_API_RETURN(status);
            }
        }
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
* @brief Create a VLAN
*
* @param[out] vlan_id VLAN ID
* @param[in] switch_id Switch id
* @param[in] attr_count Number of attributes
* @param[in] attr_list Array of attributessrc/mrvl_sai_vlan.c:519:105: error: 'list_str' undeclared (first use in this function)
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_create_vlan(_Out_ sai_object_id_t *vlan_id,
                                  _In_ sai_object_id_t switch_id,
                                  _In_ uint32_t attr_count,
                                  _In_ const sai_attribute_t *attr_list)
{
    char                        key_str[MAX_KEY_STR_LEN];
    uint32_t                    vlan_idx = 0, attr_idx, acl_index;
    sai_status_t                status = SAI_STATUS_SUCCESS;
    const sai_attribute_value_t *vlan_attr, *ingress_acl_attr;

    MRVL_SAI_LOG_ENTER();

    if (NULL == vlan_id) {
        MRVL_SAI_LOG_ERR("NULL vlan_id param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }
    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_vlan_attribs, mrvl_sai_vlan_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_vlan_attribs, MAX_LIST_VALUE_STR_LEN, key_str);
    MRVL_SAI_LOG_NTC("Create vlan_id, attributes: %s\n", key_str);

    /* check mandatory attribute SAI_VLAN_ATTR_VLAN_ID */
    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_ATTR_VLAN_ID, &vlan_attr, &attr_idx));
    vlan_idx = vlan_attr->u16;

    /* make sure the given vlan_id satisfies the spec*/
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }

    if (mrvl_sai_vlan_db[vlan_idx-1].is_created) {
        MRVL_SAI_LOG_ERR("VLAN %d has already been created\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_ATTR_VALUE_0 + vlan_idx);
        
    }

    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_ATTR_INGRESS_ACL, &ingress_acl_attr, &attr_idx);
    if (status == SAI_STATUS_SUCCESS) {
        status = mrvl_sai_acl_table_bind_to_vlan((void*)SAI_VLAN_ATTR_INGRESS_ACL, ingress_acl_attr->oid, vlan_idx);
        if (SAI_STATUS_SUCCESS != status) {
            MRVL_SAI_LOG_ERR("Unable to bind vlan %d to ACL TABLE ingress\n", vlan_idx);
            MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
        }
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(ingress_acl_attr->oid, SAI_OBJECT_TYPE_ACL_TABLE, &acl_index))) {
            MRVL_SAI_API_RETURN(status);
        }
    }
    /* create SAI VLAN object */    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan_idx, vlan_id))) {
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_vlan_db[vlan_idx-1].ingress_acl = acl_index;
    mrvl_sai_vlan_db[vlan_idx-1].is_created = true;
    mrvl_bmp_clear_all_MAC(mrvl_sai_vlan_db[vlan_idx-1].ports_bitmap);
    mrvl_sai_vlan_id_to_str(*vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
* @brief Remove VLAN
*
* @param[in] vlan_id VLAN member ID
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_remove_vlan(_In_ sai_object_id_t vlan_id)
{
    char                key_str[MAX_KEY_STR_LEN];
    uint32_t            vlan_idx;
    mrvl_ports_bitmap_t empty_ports_bmp;
    sai_status_t        status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_id_to_str(vlan_id, key_str);
    MRVL_SAI_LOG_NTC("Remove vlan %s\n", key_str);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_id, SAI_OBJECT_TYPE_VLAN, &vlan_idx)) {
        MRVL_SAI_LOG_ERR("invalid vlan_idx %d\n",vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_idx <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    
    memset(&empty_ports_bmp, 0, sizeof(mrvl_ports_bitmap_t));
    if (0 != memcmp(&mrvl_sai_vlan_db[vlan_idx-1].ports_bitmap, &empty_ports_bmp, sizeof(mrvl_ports_bitmap_t)))
    {
        MRVL_SAI_LOG_ERR("Can't remove VLAN %d, VLAN has members\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_OBJECT_IN_USE);
    }
    if (!mrvl_sai_vlan_db[vlan_idx - 1].is_created) 
    {
        MRVL_SAI_LOG_ERR("VLAN %d has not been created\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }

    if (mrvl_sai_vlan_db[vlan_idx - 1].ingress_acl)
    {
        status = mrvl_sai_acl_table_unbind_from_vlan((void*)SAI_VLAN_ATTR_INGRESS_ACL, vlan_idx);
        if (SAI_STATUS_SUCCESS != status)
        {
            MRVL_SAI_LOG_ERR("Unable to unbind vlan %d from ACL TABLE ingress\n", vlan_idx); 
            MRVL_SAI_API_RETURN(status);
        }
    }

    if (mrvl_sai_vlan_db[vlan_idx - 1].egress_acl)
    {
        status = mrvl_sai_acl_table_unbind_from_vlan((void*)SAI_VLAN_ATTR_EGRESS_ACL, vlan_idx);
        if (SAI_STATUS_SUCCESS != status)
        {
            MRVL_SAI_LOG_ERR("Unable to unbind vlan %d from ACL TABLE egress\n", vlan_idx); 
            MRVL_SAI_API_RETURN(status);
        }
    }

    mrvl_sai_vlan_db[vlan_idx - 1].ingress_acl = 0; 
    mrvl_sai_vlan_db[vlan_idx - 1].egress_acl = 0; 
    mrvl_sai_vlan_db[vlan_idx - 1].is_created = false;
     
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(status);
}
 
/**
* @brief Create VLAN Member
*
* @param[out] vlan_member_id VLAN member ID
* @param[in] switch_id Switch id
* @param[in] attr_count Number of attributes
* @param[in] attr_list Array of attributes
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_create_vlan_member(_Out_ sai_object_id_t *vlan_member_id,
                                         _In_ sai_object_id_t switch_id,
                                         _In_ uint32_t attr_count,
                                         _In_ const sai_attribute_t *attr_list)
{
    char                        list_str[MAX_LIST_VALUE_STR_LEN];
    char                        key_str[MAX_KEY_STR_LEN];
    const sai_attribute_value_t *vlan_val, *port_val, *tagging_val;
    uint32_t                    attr_index;
    uint32_t                    vlan_idx;
    mrvl_bridge_port_info_t     *bport;
    sai_status_t                status = SAI_STATUS_SUCCESS;
    sai_vlan_tagging_mode_t     tagging_mode;
    uint32_t                    group, vlan_member;
    FPA_FLOW_TABLE_ENTRY_STC    fpa_flow_entry;
    FPA_STATUS                  fpa_status;
    uint64_t                    cookie;
    
    MRVL_SAI_LOG_ENTER();
    
    if (NULL == vlan_member_id) {
        MRVL_SAI_LOG_ERR("NULL vlan_member_id entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_valid_switch(switch_id)) {
        MRVL_SAI_LOG_ERR("INVALID switch_id object\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_vlan_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create vlan member, attributes: %s\n", list_str);

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_vlan_member_attribs, mrvl_sai_vlan_member_vendor_attribs, SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed vlan member attributes check\n");
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_VLAN_ID, &vlan_val, &attr_index))) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_val->oid, SAI_OBJECT_TYPE_VLAN, &vlan_idx))) {
            MRVL_SAI_LOG_ERR("Failed to convert vlan oid %" PRIx64 " to vlan index\n", vlan_val->oid);
            MRVL_SAI_API_RETURN(status);
        }
    }

    status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID, &port_val, &attr_index);
    if (SAI_STATUS_SUCCESS != status) 
    {
        MRVL_SAI_LOG_ERR("Failed to find attribute SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID in list\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
    }
    status = mrvl_sai_bridge_port_by_oid_get(port_val->oid, &bport);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed to convert bridge port oid %" PRIx64 " to bridge port index\n", port_val->oid);
        MRVL_SAI_API_RETURN(status);
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE, &tagging_val, &attr_index))) {
        tagging_mode = tagging_val->s32;
    } else {
        tagging_mode = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    }
    if (tagging_mode > SAI_VLAN_TAGGING_MODE_TAGGED){
        MRVL_SAI_LOG_ERR("Vlan tagging mode %d is not supported\n", tagging_mode);
        MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
    }

    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_idx)) {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_idx);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }

    /* skip CPU port, which doesn't need to be added/removed to vlan */
    if (SAI_CPU_PORT_CNS == bport->logical_port) {
        MRVL_SAI_LOG_ERR("Adding CPU port to VLAN is invalid\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PORT_NUMBER);
    }

    /* add group L2 interface, if group exist update tagging mode */
    status = mrvl_sai_vlan_port_add(bport, vlan_idx, tagging_mode);
    if (status != SAI_STATUS_SUCCESS) 
    { 
        MRVL_SAI_LOG_ERR("Failed to add port %d to vlan %d\n", bport->logical_port, vlan_idx);
        MRVL_SAI_API_RETURN(status);
    }
    
    status = mrvl_sai_vlan_member_object_create((sai_vlan_id_t)vlan_idx, bport->logical_port, vlan_member_id);
    if (SAI_STATUS_SUCCESS != status) {
        MRVL_SAI_LOG_ERR("Failed to create vlan member object from port %d, vlan %d\n", bport->logical_port, vlan_idx);
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_vlan_member_key_to_str(*vlan_member_id, key_str);
    MRVL_SAI_LOG_NTC("Created %s\n", key_str);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
* @brief Remove VLAN Member
*
* @param[in] vlan_member_id VLAN member ID
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_remove_vlan_member(_In_ sai_object_id_t  vlan_member_id)
{
    char                        key_str[MAX_KEY_STR_LEN];
    uint32_t                    port;
    sai_vlan_id_t               vlan_id;
    mrvl_bridge_port_info_t     *bport;
    sai_status_t                status = SAI_STATUS_SUCCESS;
    
    MRVL_SAI_LOG_ENTER();
    mrvl_sai_vlan_member_key_to_str(vlan_member_id, key_str);
    MRVL_SAI_LOG_NTC("Remove vlan member %s\n", key_str);
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_vlan_member_object_to_vlan_bport(vlan_member_id, &vlan_id, &bport))) {
    	MRVL_SAI_API_RETURN(status);
    }
    
    if (!MRVL_SAI_IS_VLAN_IN_RANGE_MAC(vlan_id))
    {
        MRVL_SAI_LOG_ERR("vlan_id (%d) must satisfy 1 <= vlan_id <= 4094\n", vlan_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_VLAN_ID);
    }
    
    /* skip CPU port, which doesn't need to be added/removed to vlan */
    if (SAI_CPU_PORT_CNS == bport->logical_port) {
        MRVL_SAI_LOG_ERR("Removing CPU port from VLAN is invalid\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PORT_NUMBER);
    }
      
    if (!mrvl_bmp_is_bit_set_MAC(mrvl_sai_vlan_db[vlan_id-1].ports_bitmap, bport->index)) {
        MRVL_SAI_LOG_ERR("Vlan member does not exist for this vlan %u and bridge port %u\n", vlan_id, bport->index);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }

    /* delete vlan entry */
    status = mrvl_sai_vlan_port_remove(bport, vlan_id);
    if (SAI_STATUS_SUCCESS != status)
    {
        MRVL_SAI_LOG_ERR("Failed to remove port %d from vlan %d", bport->logical_port, vlan_id);
        MRVL_SAI_API_RETURN(status);
    }
    
    MRVL_SAI_LOG_NTC("Successfully removed vlan member %s\n", key_str);
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/**
* @brief Set VLAN Member Attribute
*
* @param[in] vlan_member_id VLAN member ID
* @param[in] attr Attribute structure containing ID and value
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/

sai_status_t mrvl_sai_set_vlan_member_attribute(_In_ sai_object_id_t vlan_member_id,
                                                _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .key.object_id = vlan_member_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_vlan_member_key_to_str(vlan_member_id, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_vlan_member_attribs, mrvl_sai_vlan_member_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/**
 * @brief Get VLAN Member Attribute
 *
 * @param[in] vlan_member_id VLAN member ID
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list List of attribute structures containing ID and value
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */

sai_status_t mrvl_sai_get_vlan_member_attribute(_In_ sai_object_id_t vlan_member_id,
                                                _In_ uint32_t attr_count,
                                                _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .key.object_id = vlan_member_id };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t           status = SAI_STATUS_SUCCESS;

    MRVL_SAI_LOG_ENTER();

    if (SAI_NULL_OBJECT_ID == vlan_member_id) {
        MRVL_SAI_LOG_ERR("NULL VLAN member id\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_OBJECT_ID);
    }
    mrvl_sai_vlan_member_key_to_str(vlan_member_id, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_vlan_member_attribs, mrvl_sai_vlan_member_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}

/**
* @brief Bulk objects creation.
*
* @param[in] switch_id SAI Switch object id
* @param[in] object_count Number of objects to create
* @param[in] attr_count List of attr_count. Caller passes the number
*    of attribute for each object to create.
* @param[in] attr_list List of attributes for every object.
* @param[in] type Bulk operation type.
*
* @param[out] object_id List of object ids returned
* @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
*
* @return #SAI_STATUS_SUCCESS on success when all objects are created or #SAI_STATUS_FAILURE when
* any of the objects fails to create. When there is failure, Caller is expected to go through the
* list of returned statuses to find out which fails and which succeeds.
*/
sai_status_t mrvl_sai_create_vlan_members (
        _In_ sai_object_id_t switch_id,
        _In_ uint32_t object_count,
        _In_ const uint32_t *attr_count,
        _In_ const sai_attribute_t **attr_list,
        _In_ sai_bulk_op_error_mode_t mode,
        _Out_ sai_object_id_t *object_id,
        _Out_ sai_status_t *object_statuses)
{
    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Create vlan members: STUB\n");

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
 

/**
* @brief Bulk objects removal.
*
* @param[in] object_count Number of objects to create
* @param[in] object_id List of object ids
* @param[in] type Bulk operation type.
* @param[out] object_statuses List of status for every object. Caller needs to allocate the buffer.
*
* @return #SAI_STATUS_SUCCESS on success when all objects are removed or #SAI_STATUS_FAILURE when
* any of the objects fails to remove. When there is failure, Caller is expected to go through the
* list of returned statuses to find out which fails and which succeeds.
*/
sai_status_t mrvl_sai_remove_vlan_members(
        _In_ uint32_t object_count,
        _In_ const sai_object_id_t *object_id,
        _In_ sai_bulk_op_error_mode_t mode,
        _Out_ sai_status_t *object_statuses)
{

    MRVL_SAI_LOG_ENTER();
    MRVL_SAI_LOG_NTC("Remove vlan members: STUB\n");

    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
 
/**
* @brief Get vlan statistics counters.
*
* @param[in] vlan_id VLAN id
* @param[in] number_of_counters Number of counters in the array
* @param[in] counter_ids Specifies the array of counter ids
* @param[out] counters Array of resulting counter values.
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_get_vlan_stats(_In_ sai_object_id_t vlan_id,
                                     _In_ uint32_t number_of_counters,
                                     _In_ const sai_vlan_stat_t *counter_ids,
                                     _Out_ uint64_t *counters)
{
    uint32_t ii;

    UNREFERENCED_PARAMETER(vlan_id);
    UNREFERENCED_PARAMETER(number_of_counters);

    MRVL_SAI_LOG_ENTER();

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (NULL == counters) {
        MRVL_SAI_LOG_ERR("NULL counters array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    for (ii = 0; ii < number_of_counters; ii++) {
        counters[ii] = 0;
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}

/**
* @brief Clear vlan statistics counters.
*
* @param[in] vlan_id Vlan id
* @param[in] number_of_counters Number of counters in the array
* @param[in] counter_ids Specifies the array of counter ids
*
* @return #SAI_STATUS_SUCCESS on success Failure status code on error
*/
sai_status_t mrvl_sai_clear_vlan_stats(_In_ sai_object_id_t vlan_id,
                                       _In_ uint32_t number_of_counters,
                                       _In_ const sai_vlan_stat_t *counter_ids)
{
    UNREFERENCED_PARAMETER(vlan_id);
    UNREFERENCED_PARAMETER(number_of_counters);

    MRVL_SAI_LOG_ENTER();

    if (NULL == counter_ids) {
        MRVL_SAI_LOG_ERR("NULL counter ids array param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
}


const sai_vlan_api_t vlan_api = {
    mrvl_sai_create_vlan,
    mrvl_sai_remove_vlan,
    mrvl_sai_set_vlan_attribute,
    mrvl_sai_get_vlan_attribute,
    mrvl_sai_create_vlan_member,
    mrvl_sai_remove_vlan_member,
    mrvl_sai_set_vlan_member_attribute,
    mrvl_sai_get_vlan_member_attribute,
    mrvl_sai_create_vlan_members,
    mrvl_sai_remove_vlan_members, 
    mrvl_sai_get_vlan_stats,
    mrvl_sai_clear_vlan_stats
};


