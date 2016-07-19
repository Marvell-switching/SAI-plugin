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

#include	<unistd.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>
#include    <fcntl.h>
#include    <errno.h> 

#include "sai.h"
#include "mrvl_sai.h"
#include "assert.h"
#ifndef _WIN32
#include <net/if.h>
#endif


#undef  __MODULE__
#define __MODULE__ SAI_HOST_INTERFACE

static const sai_attribute_entry_t host_interface_attribs[] = {
    { SAI_HOSTIF_ATTR_TYPE, true, true, false, true,
      "Host interface type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_HOSTIF_ATTR_RIF_OR_PORT_ID, false, true, false, true,
      "Host interface associated port or router interface", SAI_ATTR_VAL_TYPE_OID },
    { SAI_HOSTIF_ATTR_NAME, true, true, true, true,
      "Host interface name", SAI_ATTR_VAL_TYPE_CHARDATA },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
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

static const sai_vendor_attribute_entry_t host_interface_vendor_attribs[] = {
    { SAI_HOSTIF_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_host_interface_type_get, NULL,
      NULL, NULL },
    { SAI_HOSTIF_ATTR_RIF_OR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      mrvl_sai_host_interface_rif_port_get, NULL,
      NULL, NULL },
    { SAI_HOSTIF_ATTR_NAME,
      { true, false, true, true },
      { true, false, true, true },
      mrvl_sai_host_interface_name_get, NULL,
      mrvl_sai_host_interface_name_set, NULL },
};
static void mrvl_sai_host_interface_key_to_str(_In_ sai_object_id_t hif_id, _Out_ char *key_str)
{
    uint32_t hif_data;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(hif_id, SAI_OBJECT_TYPE_HOST_INTERFACE, &hif_data)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid host interface");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "host interface %u", hif_data);
    }
}

/*
 * Routine Description:
 *    Create host interface.
 *
 * Arguments:
 *    [out] hif_id - host interface id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_create_host_interface(_Out_ sai_object_id_t     * hif_id,
                                        _In_ uint32_t               attr_count,
                                        _In_ const sai_attribute_t *attr_list)
{
    sai_status_t                 status;
    const sai_attribute_value_t *type, *rif_port, *name;
    uint32_t                     type_index, rif_port_index, name_index, rif_data;
    char                         key_str[MAX_KEY_STR_LEN];
    char                         list_str[MAX_LIST_VALUE_STR_LEN];
    static uint32_t              next_id = 0;

    MRVL_SAI_LOG_ENTER();
    
    if (NULL == hif_id) {
        MRVL_SAI_LOG_ERR("NULL host interface ID param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, host_interface_attribs, host_interface_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        return status;
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
                 mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_RIF_OR_PORT_ID, &rif_port,
                                     &rif_port_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute rif port id on create of host if netdev type\n");
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }

        if (SAI_OBJECT_TYPE_ROUTER_INTERFACE == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS !=
                (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_data))) {
                return status;
            }
        } else if (SAI_OBJECT_TYPE_PORT == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_PORT, &rif_data))) {
                return status;
            }
        } else {
            MRVL_SAI_LOG_ERR("Invalid rif port object type %s", SAI_TYPE_STR(sai_object_type_query(rif_port->oid)));
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + rif_port_index;
        }
    } else if (SAI_HOSTIF_TYPE_FD == type->s32) {
    } else {
        MRVL_SAI_LOG_ERR("Invalid host interface type %d\n", type->s32);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
    }
   
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HOST_INTERFACE, next_id++, hif_id))) {
        return status;
    }
    mrvl_sai_host_interface_key_to_str(*hif_id, key_str);
    MRVL_SAI_LOG_NTC("Created host interface %s\n", key_str);
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Remove host interface
 *
 * Arguments:
 *    [in] hif_id - host interface id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_remove_host_interface(_In_ sai_object_id_t hif_id)
{
    char         key_str[MAX_KEY_STR_LEN];
    uint32_t     hif_data;
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_host_interface_key_to_str(hif_id, key_str);
    MRVL_SAI_LOG_NTC("Remove host interface %s\n", key_str);

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(hif_id, SAI_OBJECT_TYPE_HOST_INTERFACE, &hif_data))) {
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Set host interface attribute
 *
 * Arguments:
 *    [in] hif_id - host interface id
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_set_host_interface_attribute(_In_ sai_object_id_t hif_id, _In_ const sai_attribute_t *attr)
{
    const sai_object_key_t key = { .object_id = hif_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_host_interface_key_to_str(hif_id, key_str);
    return mrvl_sai_utl_set_attribute(&key, key_str, host_interface_attribs, host_interface_vendor_attribs, attr);
}

/*
 * Routine Description:
 *    Get host interface attribute
 *
 * Arguments:
 *    [in] hif_id - host interface id
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_get_host_interface_attribute(_In_ sai_object_id_t     hif_id,
                                               _In_ uint32_t            attr_count,
                                               _Inout_ sai_attribute_t *attr_list)
{
    const sai_object_key_t key = { .object_id = hif_id };
    char                   key_str[MAX_KEY_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    mrvl_sai_host_interface_key_to_str(hif_id, key_str);
    return mrvl_sai_utl_get_attributes(&key,
                              key_str,
                              host_interface_attribs,
                              host_interface_vendor_attribs,
                              attr_count,
                              attr_list);
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
        (status = mrvl_sai_utl_object_to_type(key->object_id, SAI_OBJECT_TYPE_HOST_INTERFACE, &hif_id))) {
        return status;
    }

    strncpy(value->chardata, "name", HOSTIF_NAME_SIZE);

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
        (status = mrvl_sai_utl_object_to_type(key->object_id, SAI_OBJECT_TYPE_HOST_INTERFACE, &hif_id))) {
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


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
        
    MRVL_SAI_LOG_ENTER();    

    if (hif_id == 0) {
        MRVL_SAI_LOG_ERR("NULL host interface ID param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((NULL == buffer) || (buffer_size == 0)) {
        MRVL_SAI_LOG_ERR("NULL packet buffer or packet size is zero\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }   

    pktPtr.pktDataPtr = malloc(SAI_MAX_MTU_CNS*sizeof(uint8_t));
    pktPtr.pktDataSize = SAI_MAX_MTU_CNS;  

    fpa_status =  fpaLibPktReceive(SAI_DEFAULT_ETH_SWID_CNS,timeout ,&pktPtr);
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to receive host interface packet %d\n", fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    if (*buffer_size < pktPtr.pktDataSize) {
        MRVL_SAI_LOG_ERR("buffer_size is insufficient\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    } 

    memcpy(buffer,pktPtr.pktDataPtr,pktPtr.pktDataSize);  /* packet data */ 
    *buffer_size = pktPtr.pktDataSize;

    attr_list[0].id = SAI_HOSTIF_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
    attr_list[1].id = SAI_HOSTIF_ATTR_RIF_OR_PORT_ID;
    attr_list[1].value.u32 = pktPtr.inPortNum;
    attr_list[2].id = SAI_HOSTIF_ATTR_NAME;
    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_get_host_interface_attribute(hif_id,1,&attr_list[2]))) {
        return status;
    }
    *attr_count = 3;    
    
    free(pktPtr.pktDataPtr);
    
    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
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
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((NULL == buffer) || (buffer_size == 0)) {
        MRVL_SAI_LOG_ERR("NULL packet buffer or packet size is zero\n");
        return SAI_STATUS_INVALID_PARAMETER;
    } 

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, host_interface_attribs, host_interface_vendor_attribs,
                                    SAI_OPERATION_CREATE))) {
        MRVL_SAI_LOG_ERR("Failed attribs check\n");
        return status;
    }

    mrvl_sai_utl_attr_list_to_str(attr_count, attr_list, host_interface_attribs, MAX_LIST_VALUE_STR_LEN, list_str);    

    assert(SAI_STATUS_SUCCESS ==
           mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_TYPE, &type, &type_index));    

    if (SAI_HOSTIF_TYPE_NETDEV == type->s32) {
        if (SAI_STATUS_SUCCESS !=
            (status =
                 mrvl_sai_utl_find_attrib_in_list(attr_count, attr_list, SAI_HOSTIF_ATTR_RIF_OR_PORT_ID, &rif_port,
                                     &rif_port_index))) {
            MRVL_SAI_LOG_ERR("Missing mandatory attribute rif port id on send packet of host with netdev type\n");
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }

        if (SAI_OBJECT_TYPE_ROUTER_INTERFACE == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS !=
                (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_data))) {
                /* TODO need to add support after integration of code for router interface */
                return status;
            }
        } else if (SAI_OBJECT_TYPE_PORT == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_PORT, &rif_data))) {
                return status;
            }
        } else {
            MRVL_SAI_LOG_ERR("Invalid rif port object type %s", SAI_TYPE_STR(sai_object_type_query(rif_port->oid)));
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + rif_port_index;
        }
    } else if (SAI_HOSTIF_TYPE_FD == type->s32) {
    } else {
        MRVL_SAI_LOG_ERR("Invalid host interface type %d\n", type->s32);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
    }

    pktPtr.outPortNum = rif_data;

    /* retreive ingress port */
    /* TODO inPortNum is constant. need to change it by adding support in the get function */
    hif_attr_list.id = SAI_HOSTIF_ATTR_RIF_OR_PORT_ID;    

    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_sai_get_host_interface_attribute(hif_id,1,&hif_attr_list))) {
        MRVL_SAI_LOG_ERR("Failed to get host interface attributes\n");
        return status;
    }    

    if (SAI_STATUS_SUCCESS !=
        (status =
            mrvl_sai_utl_find_attrib_in_list(1, &hif_attr_list, SAI_HOSTIF_ATTR_RIF_OR_PORT_ID, &rif_port,
                                             &rif_port_index))) {
        MRVL_SAI_LOG_ERR("Missing mandatory attribute ingress port id on send packet of host\n");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }   
    
    if (SAI_OBJECT_TYPE_ROUTER_INTERFACE == sai_object_type_query(rif_port->oid)) {
        if (SAI_STATUS_SUCCESS !=
                (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_data))) {
                /* TODO need to add support after integration of code for router interface */
                return status;
            }
        else if (SAI_OBJECT_TYPE_PORT == sai_object_type_query(rif_port->oid)) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_port->oid, SAI_OBJECT_TYPE_PORT, &rif_data))) {
                return status;
            }
        } else {
            MRVL_SAI_LOG_ERR("Invalid rif port object type %s", SAI_TYPE_STR(sai_object_type_query(rif_port->oid)));
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + rif_port_index;
        }
    }    

    pktPtr.pktDataPtr = buffer; 
    pktPtr.pktDataSize = buffer_size; 

    fpa_status =  fpaLibPortPktSend(SAI_DEFAULT_ETH_SWID_CNS, SAI_CPU_PORT_CNS /*inPortNum*/,&pktPtr); /* TODO change const inPortNum*/
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_ERR("Failed to send host interface packet %d\n", fpa_status);
        return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

const sai_hostif_api_t host_interface_api = {
    mrvl_sai_create_host_interface,
    mrvl_sai_remove_host_interface,
    mrvl_sai_set_host_interface_attribute,
    mrvl_sai_get_host_interface_attribute,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    mrvl_sai_recv_host_interface_packet,
    mrvl_sai_send_host_interface_packet
};
