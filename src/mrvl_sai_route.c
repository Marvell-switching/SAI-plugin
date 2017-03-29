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
#include "utils/mrvl_sai_hash.h"

#undef  __MODULE__
#define __MODULE__ SAI_ROUTE



static void* mrvl_sai_route_hash_ptr;

static const sai_attribute_entry_t mrvl_sai_route_attribs[] = {
    { SAI_ROUTE_ATTR_PACKET_ACTION, false, true, true, true,
      "Route packet action", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_ROUTE_ATTR_TRAP_PRIORITY, false, true, true, true,
      "Route trap priority", SAI_ATTR_VAL_TYPE_U8 },
    { SAI_ROUTE_ATTR_NEXT_HOP_ID, false, true, true, true,
      "Route next hop ID", SAI_ATTR_VAL_TYPE_U32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static sai_status_t mrvl_sai_route_packet_action_get_prv(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
static sai_status_t mrvl_sai_route_trap_priority_get_prv(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg);
static sai_status_t mrvl_sai_route_next_hop_id_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg);
static sai_status_t mrvl_sai_route_packet_action_set_prv(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);
static sai_status_t mrvl_sai_route_trap_priority_set_prv(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg);
static sai_status_t mrvl_sai_route_next_hop_id_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg);

static const sai_vendor_attribute_entry_t mrvl_sai_route_vendor_attribs[] = {
    { SAI_ROUTE_ATTR_PACKET_ACTION,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_route_packet_action_get_prv, NULL,
      mrvl_sai_route_packet_action_set_prv, NULL },
    { SAI_ROUTE_ATTR_TRAP_PRIORITY,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_route_trap_priority_get_prv, NULL,
      mrvl_sai_route_trap_priority_set_prv, NULL },
    { SAI_ROUTE_ATTR_NEXT_HOP_ID,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_route_next_hop_id_get_prv, NULL,
      mrvl_sai_route_next_hop_id_set_prv, NULL },
};
static void mrvl_sai_route_key_to_str(_In_ const sai_unicast_route_entry_t* unicast_route_entry, _Out_ char *key_str)
{
    int res;

    res = snprintf(key_str, MAX_KEY_STR_LEN, "route ");
    mrvl_sai_utl_ipprefix_to_str(unicast_route_entry->destination, MAX_KEY_STR_LEN - res, key_str + res);
}


sai_status_t mrvl_sai_route_dump(void)
{
    char        key_str[MAX_KEY_STR_LEN];
    uint32_t    used = 0;
    /*int chars_written;*/
    mrvl_sai_route_hash_entry_t *entry_ptr;
    mrvl_sai_route_hash_key_t   key;

    printf("\nRoute table:\n");
    printf("|          ip and mask            | valid |   cookie | action | nh_v | type |  nh idx |  nbr idx |\n");
    printf("--------------------------------------------------------------------------------------------------\n");

    /* get first entry */
    entry_ptr = (mrvl_sai_route_hash_entry_t*)mrvl_sai_utl_GetNextSort(mrvl_sai_route_hash_ptr, NULL);
    while (entry_ptr != NULL) {
        mrvl_sai_utl_ipprefix_to_str(entry_ptr->key.destination  , MAX_KEY_STR_LEN, key_str);
        printf("|%33s|     %d | %8x | %s |    %d | %s |%8x | %8x |\n",key_str, entry_ptr->data.valid,
               entry_ptr->data.cookie,
               (entry_ptr->data.action == SAI_PACKET_ACTION_TRAP)?"  TRAP":
               ((entry_ptr->data.action == SAI_PACKET_ACTION_DROP)?"  DROP":
                ((entry_ptr->data.action == SAI_PACKET_ACTION_LOG)?"MIRROR":"   FRW")),
               entry_ptr->data.nh_valid,
               (entry_ptr->data.nh_valid == 0)? " -- ":
               ((entry_ptr->data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP)?" NH ":" RIF"),
               entry_ptr->data.nh_idx, entry_ptr->data.nbr_idx);

        memcpy(&key, &entry_ptr->key, sizeof(mrvl_sai_route_hash_key_t));
        entry_ptr = (mrvl_sai_route_hash_entry_t*)mrvl_sai_utl_GetNextSort(mrvl_sai_route_hash_ptr, &key);
        used++;

    }

    printf("\nTotal used: %d\n\n",used);
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_route_init(void)
{
    sai_status_t   status;

	status = mrvl_sai_utl_NewInitHash (
                    &mrvl_sai_route_hash_ptr,
                    sizeof(mrvl_sai_route_hash_key_t),
                    sizeof(mrvl_sai_route_hash_entry_t),
                    SAI_ROUTE_TABLE_SIZE_CNS,
                    SAI_ROUTE_TABLE_SIZE_CNS,
                    0,
                    SAI_ROUTE_TABLE_SIZE_CNS*2,
                    1,
                    1,
                    false);
	if( SAI_STATUS_SUCCESS != status) {
		 return SAI_STATUS_FAILURE;
	}
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_create_route_prv(_In_ const sai_unicast_route_entry_t* unicast_route_entry,
                                       _In_ mrvl_sai_route_hash_entry_t  *entry)
{
    FPA_FLOW_TABLE_ENTRY_STC       flow_entry;
    FPA_STATUS                     fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    uint32_t                       i;

    if (((entry->data.action == SAI_PACKET_ACTION_FORWARD) || (entry->data.action == SAI_PACKET_ACTION_LOG)) &&
        (entry->data.nh_valid == false) ) {
        MRVL_SAI_LOG_ERR("Packet action forward/log without next hop is not allowed\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, &flow_entry);
    }
    else {
        fpa_status = fpaLibFlowEntryInit(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_IPV6_E, &flow_entry);
    }

    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to init L3_UNICAST entry status = %d\n", fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    flow_entry.data.l3_unicast.groupId = 0xFFFFFFFF; /*invalid group*/
    if ((entry->data.nbr_idx == MRVL_SAI_NEIGHBOR_TRAP_ID_CNS) ||
        (entry->data.action == SAI_PACKET_ACTION_TRAP)   ||
        (entry->data.action == SAI_PACKET_ACTION_LOG)) {
        flow_entry.data.l3_unicast.outputPort = SAI_OUTPUT_CONTROLLER;
    } else if ((entry->data.nbr_idx == MRVL_SAI_NEIGHBOR_DROP_ID_CNS) ||
        (entry->data.action == SAI_PACKET_ACTION_DROP)) {
        flow_entry.data.l3_unicast.clearActions = true;
    }

    if (((entry->data.action == SAI_PACKET_ACTION_FORWARD) || (entry->data.action == SAI_PACKET_ACTION_LOG)) &&
        (entry->data.nbr_idx < MRVL_SAI_NEIGHBOR_RESERVED_ID_CNS)){
        memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
        parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
        parsed_group_identifier.index = entry->data.nbr_idx;
        fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &flow_entry.data.l3_unicast.groupId);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", entry->data.nbr_idx);
            return SAI_STATUS_FAILURE;
        }
    }

    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        flow_entry.data.l3_unicast.match.dstIp4 = unicast_route_entry->destination.addr.ip4;
        flow_entry.data.l3_unicast.match.dstIp4Mask = unicast_route_entry->destination.mask.ip4;
        flow_entry.data.l3_unicast.match.etherType = 0x800;
    }
    else {
        for (i=0; i<16; i++) {
            flow_entry.data.l3_unicast.match.dstIp6.s6_addr[i] = unicast_route_entry->destination.addr.ip6[i];
            flow_entry.data.l3_unicast.match.dstIp6Mask.s6_addr[i] = unicast_route_entry->destination.mask.ip6[i];
        }
        flow_entry.data.l3_unicast.match.etherType = 0x86dd;
    }

    flow_entry.data.l3_unicast.match.vrfId = entry->key.vr_id;

    flow_entry.cookie = entry->data.cookie;
    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
        fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, &flow_entry);
    }
    else {
        fpa_status = fpaLibFlowEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_IPV6_E, &flow_entry);
    }
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to add entry 0x%x to L3_UNICAST table status = %d\n", entry->data.cookie, fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }
    return SAI_STATUS_SUCCESS;
}


static sai_status_t mrvl_sai_route_get_rif_nbr_prv(_In_  uint32_t rif_idx ,
                                                   _In_  const sai_unicast_route_entry_t* unicast_route_entry,
                                                   _Out_ uint32_t *nbr_idx)
{
    sai_status_t        status;
    sai_packet_action_t nbr_miss_act;

    status = mrvl_sai_get_match_neighbor_id(rif_idx, (const sai_ip_address_t *)&unicast_route_entry->destination, nbr_idx);
    if (status != SAI_STATUS_SUCCESS) {
        /* neighbor doesn't exist - check default miss action*/
        MRVL_SAI_LOG_DBG("Can't find neighbor get default\n");
        status = mrvl_sai_rif_get_nbr_miss_act(rif_idx, &nbr_miss_act);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get nbr_miss_act\n");
            return SAI_STATUS_FAILURE;
        }
        if (nbr_miss_act == SAI_PACKET_ACTION_TRAP) {
            *nbr_idx = MRVL_SAI_NEIGHBOR_TRAP_ID_CNS;
        } else {
            *nbr_idx = MRVL_SAI_NEIGHBOR_DROP_ID_CNS;
        }
    }
    return SAI_STATUS_SUCCESS;
}


/*
 * Routine Description:
 *    Create Route
 *
 * Arguments:
 *    [in] unicast_route_entry - route entry
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 *
 */

sai_status_t mrvl_sai_create_route(_In_ const sai_unicast_route_entry_t* unicast_route_entry,
                               _In_ uint32_t                         attr_count,
                               _In_ const sai_attribute_t           *attr_list)
{
    sai_status_t                  status;
    const sai_attribute_value_t *action, *priority, *next_hop;
    uint32_t                     action_index, priority_index, next_hop_index;
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    char                         key_str[MAX_KEY_STR_LEN];
    mrvl_sai_route_hash_entry_t  entry, *used_entry_ptr;
    uint32_t                     nh_idx, nbr_idx, rif_idx, port_idx, rif_vr_id;
    bool                         vr_is_valid;
    uint32_t						trap_ext=0;
    MRVL_SAI_LOG_ENTER();

    if (NULL == unicast_route_entry) {
        MRVL_SAI_LOG_ERR("NULL unicast_route_entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, mrvl_sai_route_attribs, mrvl_sai_route_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        MRVL_SAI_API_RETURN(status);
    }

    mrvl_sai_route_key_to_str(unicast_route_entry, key_str);
    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, mrvl_sai_route_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    MRVL_SAI_LOG_NTC("Create %s\n", key_str);
    MRVL_SAI_LOG_NTC("Attribs %s\n", list_str);


    if (SAI_STATUS_SUCCESS ==
        (status =
             mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTE_ATTR_TRAP_PRIORITY, &priority, &priority_index))) {
        if (priority->u8 != 0) {
            MRVL_SAI_LOG_ERR("priority %d is not supported\n", priority->u8);
            MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
        }
    }
    memset(&entry, 0, sizeof(entry));

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTE_ATTR_PACKET_ACTION, &action, &action_index))) {
        entry.data.action = action->s32;
        if ((entry.data.action != SAI_PACKET_ACTION_FORWARD) &&
            (entry.data.action != SAI_PACKET_ACTION_TRAP)    &&
            (entry.data.action != SAI_PACKET_ACTION_DROP)) {
            MRVL_SAI_LOG_ERR("route action %d is not supported\n", entry.data.action);
            MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
        }
    } else {
       entry.data.action = SAI_PACKET_ACTION_FORWARD;
    }

#ifdef DEMO_TRY
/* default rouet always set to trap for arp to me WA , TODO: */
    if ( unicast_route_entry->destination.addr.ip4 == 0 )
    {
        entry.data.action = SAI_PACKET_ACTION_TRAP;
    }
#endif

    status = mrvl_sai_utl_object_to_type(unicast_route_entry->vr_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &entry.key.vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("vrid is invalid\n");
        MRVL_SAI_API_RETURN(status);
    }
    status = mrvl_sai_virtual_router_is_valid(entry.key.vr_id, &vr_is_valid);
    if ((status != SAI_STATUS_SUCCESS) || (vr_is_valid == false)) {
        MRVL_SAI_LOG_ERR("VIRTUAL_ROUTER %d invalid\n, vr_id");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4){
        memcpy(&entry.key.destination.addr.ip4, &unicast_route_entry->destination.addr.ip4, sizeof(sai_ip4_t));
        memcpy(&entry.key.destination.mask.ip4, &unicast_route_entry->destination.mask.ip4, sizeof(sai_ip4_t));
        entry.key.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    } else {
        memcpy(&entry.key.destination.addr.ip6, &unicast_route_entry->destination.addr.ip6, sizeof(sai_ip6_t));
        memcpy(&entry.key.destination.mask.ip6, &unicast_route_entry->destination.mask.ip6, sizeof(sai_ip6_t));
        entry.key.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    }

    if (SAI_STATUS_SUCCESS ==
        (status = mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_ROUTE_ATTR_NEXT_HOP_ID, &next_hop, &next_hop_index))) {

        if (mrvl_sai_utl_is_object_type(next_hop->oid, SAI_OBJECT_TYPE_NEXT_HOP) == SAI_STATUS_SUCCESS){
            status = mrvl_sai_utl_object_to_type(next_hop->oid, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx);
            status = mrvl_sai_next_hop_get_rif_id(nh_idx, &rif_idx);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("rif_idx  %d not exist\n", rif_idx);
                MRVL_SAI_API_RETURN(status);
            }

            status = mrvl_sai_next_hop_get_nbr_id(nh_idx, &nbr_idx);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("nh_idx  %d not exist\n", nh_idx);
                MRVL_SAI_API_RETURN(status);
            }
            entry.data.nh_idx = nh_idx;
            entry.data.nh_type = SAI_OBJECT_TYPE_NEXT_HOP;
            entry.data.nh_valid = true; 
            status = mrvl_sai_rif_get_vr_id(rif_idx, &rif_vr_id);
            if ((status != SAI_STATUS_SUCCESS) || (rif_vr_id!= entry.key.vr_id)) {
                MRVL_SAI_LOG_ERR("Invalid router interface vr_id %d\n", rif_vr_id);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
        } else if (mrvl_sai_utl_is_object_type(next_hop->oid, SAI_OBJECT_TYPE_PORT) == SAI_STATUS_SUCCESS) {
            status = mrvl_sai_utl_object_to_type(next_hop->oid, SAI_OBJECT_TYPE_PORT, &port_idx);
            if (SAI_CPU_PORT_CNS != port_idx) {
                MRVL_SAI_LOG_ERR("Invalid port passed as next hop id, only cpu port is valid - %u %u\n", port_idx, SAI_CPU_PORT_CNS);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PORT_NUMBER);
            } 
            trap_ext = 1;
            //entry.data.action = SAI_PACKET_ACTION_TRAP;
        } else {
            status = mrvl_sai_utl_object_to_type(next_hop->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("Invalid router interface\n");
                MRVL_SAI_API_RETURN(status);
            }
            trap_ext = 1;
            //entry.data.action = SAI_PACKET_ACTION_TRAP;
            #if 0
            if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
                /* working only if route ip mask is 32 */
                if (unicast_route_entry->destination.mask.ip4 != 0xFFFFFFFF) {
                    MRVL_SAI_LOG_ERR("Invalid router mask for rif interface\n");
                    MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
                }
            }
            else
            {
                /* working only if route ip mask is 128 */
                if (unicast_route_entry->destination.mask.ip6[0] != 0xFF && unicast_route_entry->destination.mask.ip6[1] != 0xFF &&
                    unicast_route_entry->destination.mask.ip6[2] != 0xFF && unicast_route_entry->destination.mask.ip6[3] != 0xFF &&
                    unicast_route_entry->destination.mask.ip6[4] != 0xFF && unicast_route_entry->destination.mask.ip6[5] != 0xFF &&
                    unicast_route_entry->destination.mask.ip6[6] != 0xFF && unicast_route_entry->destination.mask.ip6[7] != 0xFF &&
                    unicast_route_entry->destination.mask.ip6[8] != 0xFF && unicast_route_entry->destination.mask.ip6[9] != 0xFF &&
                    unicast_route_entry->destination.mask.ip6[10]!= 0xFF && unicast_route_entry->destination.mask.ip6[11]!= 0xFF &&
                    unicast_route_entry->destination.mask.ip6[12]!= 0xFF && unicast_route_entry->destination.mask.ip6[13]!= 0xFF &&
                    unicast_route_entry->destination.mask.ip6[14]!= 0xFF && unicast_route_entry->destination.mask.ip6[15]!= 0xFF) {
                    MRVL_SAI_LOG_ERR("Invalid router mask for rif interface\n");
                    MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
                }
            }
            status = mrvl_sai_route_get_rif_nbr_prv(rif_idx, unicast_route_entry, &nbr_idx);
            if (status != SAI_STATUS_SUCCESS) {
            	MRVL_SAI_API_RETURN(SAI_STATUS_FAILURE);
            }       
            entry.data.nh_idx = rif_idx;
            entry.data.nh_type = SAI_OBJECT_TYPE_ROUTER_INTERFACE;
            entry.data.nh_valid = true; 
            status = mrvl_sai_rif_get_vr_id(rif_idx, &rif_vr_id);
            if ((status != SAI_STATUS_SUCCESS) || (rif_vr_id!= entry.key.vr_id)) {
                MRVL_SAI_LOG_ERR("Invalid router interface vr_id %d\n", rif_vr_id);
                MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
            }
            #endif
            
        }               
    }


    if (entry.data.action == SAI_PACKET_ACTION_TRAP){
        nbr_idx = MRVL_SAI_NEIGHBOR_TRAP_ID_CNS;
    } else if (entry.data.action == SAI_PACKET_ACTION_DROP){
        nbr_idx = MRVL_SAI_NEIGHBOR_DROP_ID_CNS;
    } else if (trap_ext == 1 )
    {
    	/* change action for frw only */
    	entry.data.action = SAI_PACKET_ACTION_TRAP;
    	nbr_idx = MRVL_SAI_NEIGHBOR_TRAP_ID_CNS;
    }
    entry.data.valid = true;
    entry.data.nbr_idx = nbr_idx;



    /* add element to hash */
    status = mrvl_sai_utl_InsertHash(mrvl_sai_route_hash_ptr,
                              &entry,
                              (void**)&used_entry_ptr);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("mrvl_sai_create_route_prv can't insert to hash\n");
        MRVL_SAI_API_RETURN(status);
    }
    used_entry_ptr->data.key_ptr = &used_entry_ptr->key;
    used_entry_ptr->data.cookie = (uint32_t)((PTR_TO_INT)used_entry_ptr);
    entry.data.cookie = used_entry_ptr->data.cookie;
    status = mrvl_sai_create_route_prv(unicast_route_entry, &entry);
    if (status != SAI_STATUS_SUCCESS) {
        if (status == SAI_STATUS_ITEM_ALREADY_EXISTS) {
            MRVL_SAI_LOG_ERR("mrvl_sai_create_route_prv failed - ALREADY_EXISTS\n");
            MRVL_SAI_API_RETURN(status);
        }else {
            MRVL_SAI_LOG_ERR("mrvl_sai_create_route_prv failed %d\n", status);
            mrvl_sai_utl_DeleteHash (mrvl_sai_route_hash_ptr, &entry.key);
            MRVL_SAI_API_RETURN(status);
        }
    }

    if ((entry.data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP) &&
        (entry.data.nh_valid == true)){
        status = mrvl_sai_next_hop_add_route(nh_idx, &used_entry_ptr->data.list_elem);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("route_list is invalid\n");
            MRVL_SAI_API_RETURN(status);
        }
    }
    #if 0
    if (add_route == 1){
        status = mrvl_sai_rif_add_route(rif_idx, &used_entry_ptr->data.list_elem);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("route_list is invalid\n");
            MRVL_SAI_API_RETURN(status);
        }
    }     
    #endif
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
static sai_status_t mrvl_sai_get_route_prv(const sai_unicast_route_entry_t* unicast_route_entry,
                                   mrvl_sai_route_hash_entry_t         **route_get_entry)
{
    mrvl_sai_route_hash_entry_t entry;
    sai_status_t                status;
    bool                        vr_is_valid;
    MRVL_SAI_LOG_ENTER();

    if (NULL == unicast_route_entry) {
        MRVL_SAI_LOG_ERR("NULL unicast_route_entry param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&entry,0, sizeof(entry));
    status = mrvl_sai_utl_object_to_type(unicast_route_entry->vr_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &entry.key.vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("vrid is invalid\n");
        return status;
    }
    status = mrvl_sai_virtual_router_is_valid(entry.key.vr_id, &vr_is_valid);
    if ((status != SAI_STATUS_SUCCESS) || (vr_is_valid == false)) {
        MRVL_SAI_LOG_ERR("VIRTUAL_ROUTER %d not supported\n, vr_id");
        return SAI_STATUS_INVALID_PARAMETER;
    }

     if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4){
         memcpy(&entry.key.destination.addr.ip4, &unicast_route_entry->destination.addr.ip4, sizeof(sai_ip4_t));
         memcpy(&entry.key.destination.mask.ip4, &unicast_route_entry->destination.mask.ip4, sizeof(sai_ip4_t));
         entry.key.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
     } else {
         memcpy(&entry.key.destination.addr.ip6, &unicast_route_entry->destination.addr.ip6, sizeof(sai_ip6_t));
         memcpy(&entry.key.destination.mask.ip6, &unicast_route_entry->destination.mask.ip6, sizeof(sai_ip6_t));
         entry.key.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
     }

    *route_get_entry = (mrvl_sai_route_hash_entry_t*)mrvl_sai_utl_SearchHash(mrvl_sai_route_hash_ptr, &entry.key);
    if ((*route_get_entry == NULL) || ((*route_get_entry)->data.valid == false)) {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}
/*
 * Routine Description:
 *    Remove Route
 *
 * Arguments:
 *    [in] unicast_route_entry - route entry
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 * Note: IP prefix/mask expected in Network Byte Order.
 */
sai_status_t mrvl_sai_remove_route(_In_ const sai_unicast_route_entry_t* unicast_route_entry)
{

    char           key_str[MAX_KEY_STR_LEN];
    mrvl_sai_route_hash_entry_t *used_entry_ptr;
    FPA_STATUS                  fpa_status;
    sai_status_t                status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_route_key_to_str(unicast_route_entry, key_str);
    MRVL_SAI_LOG_NTC("Remove route %s\n", key_str);

    status = mrvl_sai_get_route_prv(unicast_route_entry, &used_entry_ptr);
	if ((used_entry_ptr == NULL) || (used_entry_ptr->data.valid == false) || (status != SAI_STATUS_SUCCESS)) {
		MRVL_SAI_API_RETURN(SAI_STATUS_ITEM_NOT_FOUND);
	}

    fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, used_entry_ptr->data.cookie);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to delete entry %x from L3_UNICAST table status = %d\n", used_entry_ptr->data.cookie, fpa_status);
        if (fpa_status == SAI_STATUS_ITEM_NOT_FOUND) {
            mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &used_entry_ptr->key);
        }
        status = mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        MRVL_SAI_API_RETURN(status);
    }
    if (used_entry_ptr->data.nh_valid == true){
        if (used_entry_ptr->data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP){
            status = mrvl_sai_next_hop_del_route(used_entry_ptr->data.nh_idx, &used_entry_ptr->data.list_elem);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("route_list is invalid\n");
                mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &used_entry_ptr->key);
                MRVL_SAI_API_RETURN(status);
            }
        } else if (used_entry_ptr->data.nh_type == SAI_OBJECT_TYPE_ROUTER_INTERFACE){
            status = mrvl_sai_rif_del_route(used_entry_ptr->data.nh_idx, &used_entry_ptr->data.list_elem);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("route_list is invalid\n");
                mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &used_entry_ptr->key);
                MRVL_SAI_API_RETURN(status);
            }
        }
    }
    mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &used_entry_ptr->key);

    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

sai_status_t mrvl_sai_route_remove_all(void)
{

    sai_status_t                status;
    sai_unicast_route_entry_t   unicast_route_entry;
    mrvl_sai_route_hash_entry_t *entry_ptr;
    mrvl_sai_route_hash_key_t   key;


    /* get first entry */
    entry_ptr = (mrvl_sai_route_hash_entry_t*)mrvl_sai_utl_GetNextSort(mrvl_sai_route_hash_ptr, NULL);
    while (entry_ptr != NULL) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, entry_ptr->key.vr_id, &unicast_route_entry.vr_id)) {
            return SAI_STATUS_FAILURE;
        }
        memcpy(&key, &entry_ptr->key, sizeof(mrvl_sai_route_hash_key_t));

        if (((entry_ptr->key.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4) && (entry_ptr->key.destination.mask.ip4 != 0)) ||
            ((entry_ptr->key.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV6) &&
             (entry_ptr->key.destination.mask.ip6[0] != 0) && (entry_ptr->key.destination.mask.ip6[1] != 0) &&
             (entry_ptr->key.destination.mask.ip6[2] != 0) && (entry_ptr->key.destination.mask.ip6[3] != 0) &&
             (entry_ptr->key.destination.mask.ip6[4] != 0) && (entry_ptr->key.destination.mask.ip6[5] != 0) &&
             (entry_ptr->key.destination.mask.ip6[6] != 0) && (entry_ptr->key.destination.mask.ip6[7] != 0) &&
             (entry_ptr->key.destination.mask.ip6[8] != 0) && (entry_ptr->key.destination.mask.ip6[9] != 0) &&
             (entry_ptr->key.destination.mask.ip6[10]!= 0) && (entry_ptr->key.destination.mask.ip6[11]!= 0) &&
             (entry_ptr->key.destination.mask.ip6[12]!= 0) && (entry_ptr->key.destination.mask.ip6[13]!= 0) &&
             (entry_ptr->key.destination.mask.ip6[14]!= 0) && (entry_ptr->key.destination.mask.ip6[15]!= 0))) { /* we can't delete default entry*/
            memcpy(&unicast_route_entry.destination, &entry_ptr->key.destination, sizeof(sai_ip_prefix_t));
            status = mrvl_sai_remove_route(&unicast_route_entry);
            if (status !=SAI_STATUS_SUCCESS) {
                return status;
            }
        }
        entry_ptr = (mrvl_sai_route_hash_entry_t *)mrvl_sai_utl_GetNextSort(mrvl_sai_route_hash_ptr, &key);
    }
    return status;
}


/*
 * Routine Description:
 *    Set route attribute value
 *
 * Arguments:
 *    [in] unicast_route_entry - route entry
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_set_route_attribute(_In_ const sai_unicast_route_entry_t* unicast_route_entry,
                                      _In_ const sai_attribute_t           *attr)
{
    const sai_object_key_t key = { .unicast_route_entry = unicast_route_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == unicast_route_entry) {
        MRVL_SAI_LOG_ERR("NULL unicast_route_entry param\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    mrvl_sai_route_key_to_str(unicast_route_entry, key_str);
    status = mrvl_sai_utl_set_attribute(&key, key_str, mrvl_sai_route_attribs, mrvl_sai_route_vendor_attribs, attr);
    MRVL_SAI_API_RETURN(status);
}

/*
 * Routine Description:
 *    Get route attribute value
 *
 * Arguments:
 *    [in] unicast_route_entry - route entry
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_route_attribute(_In_ const sai_unicast_route_entry_t* unicast_route_entry,
                                      _In_ uint32_t                         attr_count,
                                      _Inout_ sai_attribute_t              *attr_list)
{
    const sai_object_key_t key = { .unicast_route_entry = unicast_route_entry };
    char                   key_str[MAX_KEY_STR_LEN];
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (NULL == unicast_route_entry) {
        MRVL_SAI_LOG_ERR("NULL unicast_route_entry param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    mrvl_sai_route_key_to_str(unicast_route_entry, key_str);
    status = mrvl_sai_utl_get_attributes(&key, key_str, mrvl_sai_route_attribs, mrvl_sai_route_vendor_attribs, attr_count, attr_list);
    MRVL_SAI_API_RETURN(status);
}



/* Packet action [sai_packet_action_t] */
static sai_status_t mrvl_sai_route_packet_action_get_prv(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{

    sai_status_t                     status;
    const sai_unicast_route_entry_t* unicast_route_entry = key->unicast_route_entry;
    mrvl_sai_route_hash_entry_t *route_get_entry;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_route_prv(unicast_route_entry, &route_get_entry))) {
        return status;
    }

    value->s32 = route_get_entry->data.action;
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Packet priority for trap/log actions [uint8_t] */
static sai_status_t mrvl_sai_route_trap_priority_get_prv(_In_ const sai_object_key_t   *key,
                                          _Inout_ sai_attribute_value_t *value,
                                          _In_ uint32_t                  attr_index,
                                          _Inout_ vendor_cache_t        *cache,
                                          void                          *arg)
{
    return SAI_STATUS_NOT_SUPPORTED;
}

/* Next hop id for the packet [sai_next_hop_id_t] */
static sai_status_t mrvl_sai_route_next_hop_id_get_prv(_In_ const sai_object_key_t   *key,
                                        _Inout_ sai_attribute_value_t *value,
                                        _In_ uint32_t                  attr_index,
                                        _Inout_ vendor_cache_t        *cache,
                                        void                          *arg)
{

    sai_status_t                     status;
    const sai_unicast_route_entry_t* unicast_route_entry = key->unicast_route_entry;
    mrvl_sai_route_hash_entry_t *route_get_entry;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_route_prv(unicast_route_entry, &route_get_entry))) {
        return status;
    }

    if (false == route_get_entry->data.nh_valid) {
        MRVL_SAI_LOG_ERR("Can't get next hop ID when no next hop is set\n");
        return SAI_STATUS_INVALID_ATTRIBUTE_0 + attr_index;
    }

    if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, route_get_entry->data.nh_idx, &value->oid))
            return SAI_STATUS_FAILURE;
    } else {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, route_get_entry->data.nh_idx, &value->oid))
            return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


static sai_status_t mrvl_sai_modify_route_prv(const sai_unicast_route_entry_t  *unicast_route_entry,
                                              mrvl_sai_route_hash_entry_t      *route_get_entry)
{

    sai_status_t status;
    FPA_STATUS   fpa_status;

    /* Delete and Add for action/priority, or Set for next hops changes */
    fpa_status = fpaLibFlowTableCookieDelete(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L3_UNICAST_E, route_get_entry->data.cookie);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("mrvl_sai_modify_route_prv: Failed to delete entry %x from L3_UNICAST table status = %d\n", route_get_entry->data.cookie, fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    status = mrvl_sai_create_route_prv(unicast_route_entry, route_get_entry);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("mrvl_sai_modify_route_prv: Failed to add entry %x to L3_UNICAST table status = %d\n", route_get_entry->data.cookie, status);
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Packet action [sai_packet_action_t] */
static sai_status_t mrvl_sai_route_packet_action_set_prv(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{

    sai_status_t                     status;
    const sai_unicast_route_entry_t* unicast_route_entry = key->unicast_route_entry;
    mrvl_sai_route_hash_entry_t *route_get_entry;
    uint32_t    nbr_idx;
    bool        nbr_changed = false;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_route_prv(unicast_route_entry, &route_get_entry))) {
        return status;
    }
    if ((value->s32 != SAI_PACKET_ACTION_FORWARD) &&
        (value->s32 != SAI_PACKET_ACTION_TRAP)    &&
        (value->s32 != SAI_PACKET_ACTION_DROP)) {
        MRVL_SAI_LOG_ERR("route action %d is not supported\n", value->s32);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    if (value->s32 != route_get_entry->data.action) {
        route_get_entry->data.action = value->s32;
        if (route_get_entry->data.nh_valid == true) {
            if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
                status = mrvl_sai_next_hop_get_nbr_id(route_get_entry->data.nh_idx, &nbr_idx);
                if (status != SAI_STATUS_SUCCESS) {
                    MRVL_SAI_LOG_ERR("nh_idx  %d not exist\n", route_get_entry->data.nh_idx);
                    return status;
                }
                /* neighbor index was changed */
                if (nbr_idx != route_get_entry->data.nbr_idx) {
                    nbr_changed = true;
                    mrvl_sai_next_hop_del_route(route_get_entry->data.nh_idx, &route_get_entry->data.list_elem);
                    route_get_entry->data.nbr_idx = nbr_idx;
                }
            } else if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_ROUTER_INTERFACE) {
                status = mrvl_sai_route_get_rif_nbr_prv(route_get_entry->data.nh_idx, unicast_route_entry, &nbr_idx);
                if (status != SAI_STATUS_SUCCESS) {
                    return SAI_STATUS_FAILURE;
                }
                /* neighbor index was changed */
                if (nbr_idx != route_get_entry->data.nbr_idx) {
                    nbr_changed = true;
                    mrvl_sai_rif_del_route(route_get_entry->data.nh_idx, &route_get_entry->data.list_elem);
                    route_get_entry->data.nbr_idx = nbr_idx;
                }
            }
        }
        status = mrvl_sai_modify_route_prv(unicast_route_entry, route_get_entry);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("mrvl_sai_route_packet_action_set_prv: Failed to modify action %d\n", value->s32);
            mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &route_get_entry->key);
            return status;
        }
    }
    if (nbr_changed == true){
        if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP){
            status = mrvl_sai_next_hop_add_route(route_get_entry->data.nh_idx, &route_get_entry->data.list_elem);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("route_list is invalid\n");
                mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &route_get_entry->key);
                return status;
            }
        } else if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_ROUTER_INTERFACE){
            status = mrvl_sai_rif_add_route(route_get_entry->data.nh_idx, &route_get_entry->data.list_elem);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("route_list is invalid\n");
                mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &route_get_entry->key);
                return status;
            }
        }
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* Packet priority for trap/log actions [uint8_t] */
static sai_status_t mrvl_sai_route_trap_priority_set_prv(_In_ const sai_object_key_t      *key,
                                          _In_ const sai_attribute_value_t *value,
                                          void                             *arg)
{
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_NOT_SUPPORTED;
}

/* Next hop id for the packet [sai_next_hop_id_t] */
static sai_status_t mrvl_sai_route_next_hop_id_set_prv(_In_ const sai_object_key_t      *key,
                                        _In_ const sai_attribute_value_t *value,
                                        void                             *arg)
{

    sai_status_t                     status;
    const sai_unicast_route_entry_t* unicast_route_entry = key->unicast_route_entry;
    mrvl_sai_route_hash_entry_t      *route_get_entry;
    uint32_t                         nh_idx, nh_type;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_get_route_prv(unicast_route_entry, &route_get_entry))) {
        return status;
    }

    if (mrvl_sai_utl_is_object_type(value->oid, SAI_OBJECT_TYPE_NEXT_HOP) == SAI_STATUS_SUCCESS){
        status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx);
        nh_type = SAI_OBJECT_TYPE_NEXT_HOP;
    } else if (mrvl_sai_utl_is_object_type(value->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE) == SAI_STATUS_SUCCESS){
        status = mrvl_sai_utl_object_to_type(value->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &nh_idx);
        nh_type = SAI_OBJECT_TYPE_ROUTER_INTERFACE;
    }
    if ((route_get_entry->data.nh_valid == false) ||
        (route_get_entry->data.nh_type != nh_type) ||
        (route_get_entry->data.nh_idx != nh_idx)) {
        if (route_get_entry->data.nh_valid == true) {
            if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
                status = mrvl_sai_next_hop_del_route(route_get_entry->data.nh_idx, &route_get_entry->data.list_elem);
                if (status != SAI_STATUS_SUCCESS) {
                    MRVL_SAI_LOG_ERR("route_list is invalid\n");
                }
            } else if (route_get_entry->data.nh_type == SAI_OBJECT_TYPE_ROUTER_INTERFACE){
                status = mrvl_sai_rif_del_route(route_get_entry->data.nh_idx, &route_get_entry->data.list_elem);
                if (status != SAI_STATUS_SUCCESS) {
                    MRVL_SAI_LOG_ERR("route_list is invalid\n");
                }
            }
            if (nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
                status = mrvl_sai_next_hop_get_nbr_id(nh_idx, &route_get_entry->data.nbr_idx);
            }else {
                status = mrvl_sai_route_get_rif_nbr_prv(nh_idx, unicast_route_entry, &route_get_entry->data.nbr_idx);
                if (status != SAI_STATUS_SUCCESS) {
                    return SAI_STATUS_FAILURE;
                }
            }
        }

        route_get_entry->data.nh_valid = true;
        route_get_entry->data.nh_type = nh_type;
        route_get_entry->data.nh_idx = nh_idx;
        status = mrvl_sai_modify_route_prv(unicast_route_entry, route_get_entry);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("mrvl_sai_route_packet_action_set_prv: Failed to modify action %d\n", value->s32);
            mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &route_get_entry->key);
            return status;
        }
        if (nh_type == SAI_OBJECT_TYPE_NEXT_HOP) {
            status = mrvl_sai_next_hop_add_route(nh_idx, &route_get_entry->data.list_elem);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("route_list is invalid\n");
                return status;
            }
        }else if (nh_type == SAI_OBJECT_TYPE_ROUTER_INTERFACE) {
            status = mrvl_sai_rif_add_route(nh_idx, &route_get_entry->data.list_elem);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("route_list is invalid\n");
                return status;
            }
        }

    }
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/* update route table about the change in neighbor */
sai_status_t mrvl_sai_route_update_nbr_id(mrvl_sai_route_hash_data_t *route_entry_data, uint32_t nbr_idx)
{
    mrvl_sai_route_hash_entry_t *route_entry;
    sai_unicast_route_entry_t   unicast_route_entry;
    sai_status_t                status;

    /* point to route entry */
    /*route_entry = (mrvl_sai_route_hash_entry_t *)((PTR_TO_INT)route_entry_data - sizeof(mrvl_sai_route_hash_key_t));*/
    route_entry = (mrvl_sai_route_hash_entry_t *)route_entry_data->key_ptr;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, route_entry->key.vr_id, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    memcpy(&unicast_route_entry.destination, &route_entry->key.destination, sizeof(sai_ip_prefix_t));
    route_entry->data.nbr_idx = nbr_idx;
    status = mrvl_sai_modify_route_prv(&unicast_route_entry, route_entry);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("mrvl_sai_route_update_nbr_id: Failed to modify route %d\n");
        mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &route_entry->key);
    }
    return status;

}

/* update route table about the change in neighbor only if ip match*/
sai_status_t mrvl_sai_route_update_nbr_id_if_match(mrvl_sai_route_hash_data_t *route_entry_data, sai_ip_address_t *inet_address, uint32_t nbr_idx)
{
    mrvl_sai_route_hash_entry_t *route_entry;
    sai_unicast_route_entry_t   unicast_route_entry;
    sai_status_t                status;

    /* point to route entry */
    /*route_entry = (mrvl_sai_route_hash_entry_t *)((PTR_TO_INT)route_entry_data - sizeof(mrvl_sai_route_hash_key_t));*/
    route_entry = (mrvl_sai_route_hash_entry_t *)route_entry_data->key_ptr;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, route_entry->key.vr_id, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    /* check for match */
    if (((inet_address->addr_family == SAI_IP_ADDR_FAMILY_IPV4) &&
        (memcmp(&route_entry->key.destination.addr.ip4, &inet_address->addr.ip4, sizeof(sai_ip4_t)) == 0) &&
        (route_entry->key.destination.mask.ip4 == 0xFFFFFFFF)) ||
        ((inet_address->addr_family == SAI_IP_ADDR_FAMILY_IPV6) &&
        (memcmp(&route_entry->key.destination.addr.ip6, &inet_address->addr.ip6, sizeof(sai_ip6_t)) == 0) &&
        (route_entry->key.destination.mask.ip6[0] == 0xFF) && (route_entry->key.destination.mask.ip6[1] == 0xFF) &&
        (route_entry->key.destination.mask.ip6[2] == 0xFF) && (route_entry->key.destination.mask.ip6[3] == 0xFF) &&
        (route_entry->key.destination.mask.ip6[4] == 0xFF) && (route_entry->key.destination.mask.ip6[5] == 0xFF) &&
        (route_entry->key.destination.mask.ip6[6] == 0xFF) && (route_entry->key.destination.mask.ip6[7] == 0xFF) &&
        (route_entry->key.destination.mask.ip6[8] == 0xFF) && (route_entry->key.destination.mask.ip6[9] == 0xFF) &&
        (route_entry->key.destination.mask.ip6[10]== 0xFF) && (route_entry->key.destination.mask.ip6[11]== 0xFF) &&
        (route_entry->key.destination.mask.ip6[12]== 0xFF) && (route_entry->key.destination.mask.ip6[13]== 0xFF) &&
        (route_entry->key.destination.mask.ip6[14]== 0xFF) && (route_entry->key.destination.mask.ip6[15]== 0xFF)))
        {
            memcpy(&unicast_route_entry.destination, &route_entry->key.destination, sizeof(sai_ip_prefix_t));
            route_entry->data.nbr_idx = nbr_idx;
            status = mrvl_sai_modify_route_prv(&unicast_route_entry, route_entry);
            if (status != SAI_STATUS_SUCCESS) {
                MRVL_SAI_LOG_ERR("mrvl_sai_route_update_nbr_id: Failed to modify route %d\n");
                mrvl_sai_utl_DeleteHash(mrvl_sai_route_hash_ptr, &route_entry->key);
        }
    }

    return SAI_STATUS_SUCCESS;
}

const sai_route_api_t route_api = {
    mrvl_sai_create_route,
    mrvl_sai_remove_route,
    mrvl_sai_set_route_attribute,
    mrvl_sai_get_route_attribute,
};
