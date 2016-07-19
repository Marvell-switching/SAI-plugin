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
/*
********************************************************************************
* mrvl_sai_route_test.c
*
* DESCRIPTION:
*       
*
* FILE REVISION NUMBER:
*       $Revision: 04 $
*
*******************************************************************************/

#include	<unistd.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>
#include    <fcntl.h>
#include    <errno.h>

#include	"fpaLibApis.h"
#include	"../test/inc/mrvl_sai_test.h"

extern sai_route_api_t* sai_route_api;
/*******************************************************************************
* mrvl_sai_route_pre_test
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void mrvl_sai_route_pre_test
(
    sai_object_id_t *vr_id
)
{
    sai_vlan_port_t port_list[4];
    sai_mac_t mac_addr;
    uint32_t port_vlan, rif_idx, ip_addr;
    sai_router_interface_type_t interface_type;
    sai_object_id_t rif_id, nh_id, port_id;
    bool default_mac;
    sai_packet_action_t  nbr_miss_act, action;
    sai_neighbor_entry_t neighbor_entry;
    
    mrvl_sai_trace_set_all(1, SEVERITY_LEVEL_INFO);    
    printf("Create vlan 2 with port 0-3\n");
    mrvl_sai_vlan_create_test(2);
    /**** add ports to vlan 2 (0=untag, 1= tag)*/
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 0, &port_list[0].port_id);
    port_list[0].tagging_mode = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[1].port_id);
    port_list[1].tagging_mode = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[2].port_id);
    port_list[2].tagging_mode = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 3, &port_list[3].port_id);
    port_list[3].tagging_mode = 0;
    mrvl_sai_vlan_add_ports_test(2, 4, port_list);
    /************************************************/
    printf("Create vlan 3 with port 1-3\n");
    mrvl_sai_vlan_create_test(3);
    /**** add ports to vlan 3 (0=untag, 1= tag)*/
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[0].port_id);
    port_list[0].tagging_mode = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[1].port_id);
    port_list[1].tagging_mode = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 3, &port_list[2].port_id);
    port_list[2].tagging_mode = 0;
    mrvl_sai_vlan_add_ports_test(3, 3, port_list);
    /************************************************/
    
    printf("Create fdb entry mac:00:00:00:01:02:03 vlan 3 --> port 1 FORWARD\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    port_vlan = 3;
    action = SAI_PACKET_ACTION_FORWARD;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_id);
    mrvl_sai_fdb_add_test(mac_addr, port_vlan, SAI_FDB_ENTRY_STATIC, port_id, action);
    /************************************************/
        
    printf("Create fdb entry mac:00:00:00:06:07:08 vlan 2 --> port 2 FORWARD\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x06;
    mac_addr[4] = 0x07;
    mac_addr[5] = 0x08;
    port_vlan = 2;
    action = SAI_PACKET_ACTION_FORWARD;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_id);
    mrvl_sai_fdb_add_test(mac_addr, port_vlan, SAI_FDB_ENTRY_STATIC, port_id, action);
    /************************************************/

    
    printf("Created  virtual router 0 \n");
    mrvl_sai_virtual_router_add_test(vr_id);
    
    /**** create rif 0 on vlan 2 */ 
    printf("Created  rif 0 on vlan 2, src mac:00:00:00:56:45:34, miss - trap\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    port_vlan = 2;
    default_mac = false;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x56;
    mac_addr[4] = 0x45;
    mac_addr[5] = 0x34;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, *vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    /************************************************/

    /**** create rif 1 on port 3 */ 
    printf("Created  rif 1 on port 3, src mac: default, miss - drop\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_PORT;
    port_vlan = 3;
    default_mac = true;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0;
    mac_addr[4] = 0;
    mac_addr[5] = 0;
    nbr_miss_act = SAI_PACKET_ACTION_DROP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act,*vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    /************************************************/
    
    /**** create rif 0 on vlan 2 */ 
    printf("Created  rif 2 on vlan 3, src mac:00:00:00:56:45:34, miss - trap\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    port_vlan = 3;
    default_mac = false;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x56;
    mac_addr[4] = 0x45;
    mac_addr[5] = 0x34;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, *vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    /************************************************/
    printf("Created neighbor on rif 2, ip 1.2.3.4, mac:00:00:00:01:02:03\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    rif_idx = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    /************************************************/
    
    printf("Created neighbor on rif 0, ip 3.4.5.6, mac:00:00:00:06:07:08\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x06;
    mac_addr[4] = 0x07;
    mac_addr[5] = 0x08;
    rif_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((3)<<24)+((4)<<16)+((5)<<8)+((6)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    /************************************************/

    printf("Created neighbor on rif 1, ip 1.2.3.5, mac:00:00:12:34:56:99\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x12;
    mac_addr[3] = 0x34;
    mac_addr[4] = 0x56;
    mac_addr[5] = 0x99;
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((5)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    /************************************************/
    
    
    printf("Created nexthop on rif 2, ip 1.2.3.4 \n");
    rif_idx = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    /************************************************/

    printf("Created nexthop on rif 1, ip 1.2.3.5 \n");
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((5)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    /************************************************/

    printf("Created nexthop on rif 0, ip 3.4.5.6 \n");
    rif_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((3)<<24)+((4)<<16)+((5)<<8)+((6)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    /************************************************/
 
}

/*******************************************************************************
* mrvl_sai_route_add_test
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_route_add_test
(
    _In_ uint32_t ipv4,
    _In_ uint32_t ipv4_mask,
    _In_ sai_packet_action_t action,
    _In_ sai_object_id_t* nh_id,
    _In_ sai_object_id_t vr_id
)
{
    sai_unicast_route_entry_t unicast_route_entry;
    uint32_t attr_count=0;
    sai_attribute_t attr_list[3];
    sai_status_t status;
    memset(attr_list, 0, sizeof(sai_attribute_t)*3);
    attr_list[attr_count].id = SAI_ROUTE_ATTR_PACKET_ACTION;
    attr_list[attr_count].value.s32 = action;
    attr_count++;
    
    if (nh_id != NULL){
        attr_list[attr_count].id = SAI_ROUTE_ATTR_NEXT_HOP_ID;
        attr_list[attr_count].value.oid = *nh_id;
        attr_count++;
    }
    
    memset(&unicast_route_entry, 0, sizeof(unicast_route_entry));
    unicast_route_entry.vr_id = vr_id;
    unicast_route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    unicast_route_entry.destination.addr.ip4 = ipv4;
    unicast_route_entry.destination.mask.ip4 = ipv4_mask;
    
    MRVL_SAI_LOG_INF("Calling sai_route_api->create_route\n");
    status = sai_route_api->create_route(&unicast_route_entry, attr_count, attr_list);
	return status;
}


/*******************************************************************************
* mrvl_sai_wrap_route_add
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_wrap_route_add
(
    IN sai_uint32_t nh_idx,
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t ip_mask_len,
    IN sai_uint32_t action
)
{
    sai_object_id_t     next_hop_id, vr_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask;
    
	printf("mrvl_sai_wrap_route_add: nh_idx: %d  ip:%d.%d.%d.%d action %d\n",
		   nh_idx, ip1, ip2, ip3, ip4, action);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &next_hop_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &next_hop_id, vr_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_route_add_test failed to create route\n");
        return status;
    }
    printf("mrvl_sai_wrap_route_add:route was created\n");
    
    return 0;
}

/*******************************************************************************
* mrvl_sai_wrap_route_add_rif
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_wrap_route_add_rif
(
    IN sai_uint32_t nh_idx,
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t action
)
{
    sai_object_id_t     next_hop_id, vr_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask;
    
	printf("mrvl_sai_wrap_route_add: nh_idx: %d  ip:%d.%d.%d.%d action %d\n",
		   nh_idx, ip1, ip2, ip3, ip4, action);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, nh_idx, &next_hop_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    ip_mask = (0xffffffff);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &next_hop_id, vr_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_route_add_test failed to create route\n");
        return status;
    }
    printf("mrvl_sai_wrap_route_add:route was created\n");
    
    return 0;
}

/*******************************************************************************
* mrvl_sai_route_remove_test
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_route_remove_test
(
    _In_ sai_unicast_route_entry_t *unicast_route_entry
)
{
    MRVL_SAI_LOG_INF("Calling sai_route_api->remove_route\n");
    return sai_route_api->remove_route(unicast_route_entry);
}

/*******************************************************************************
* mrvl_sai_wrap_route_remove
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_wrap_route_remove
(
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t ip_mask_len
)
{
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask;
    sai_unicast_route_entry_t unicast_route_entry;
    
	printf("mrvl_sai_wrap_route_remove:  ip:%d.%d.%d.%d \n",
		   ip1, ip2, ip3, ip4);

    memset(&unicast_route_entry, 0, sizeof(unicast_route_entry));
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    unicast_route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    unicast_route_entry.destination.addr.ip4 = ip_addr;
    unicast_route_entry.destination.mask.ip4 = ip_mask;
    
    status = mrvl_sai_route_remove_test(&unicast_route_entry); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_route_remove_test failed to remove route\n");
        return status;
    } else {
        printf("mrvl_sai_wrap_route_remove: route was deleted\n");
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_route_get_attr_test
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_route_get_attr_test
(
    IN const sai_unicast_route_entry_t* unicast_route_entry,
    IN uint32_t attr_count,
    OUT sai_attribute_t *attr_list
)
{    
    MRVL_SAI_LOG_INF("Calling sai_route_api->get_route_attribute\n");
    return sai_route_api->get_route_attribute(unicast_route_entry, attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_route_get_attr
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_wrap_route_get_attr
(
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t ip_mask_len,
    IN sai_uint32_t attr_id
)
{
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask, nh_idx;
    sai_unicast_route_entry_t unicast_route_entry;
    sai_attribute_t     attr_list[2];
    
	printf("mrvl_sai_wrap_route_get_attr:  ip:%d.%d.%d.%d/%d \n",
		   ip1, ip2, ip3, ip4, ip_mask_len);

    memset(&unicast_route_entry, 0, sizeof(unicast_route_entry));
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    unicast_route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    unicast_route_entry.destination.addr.ip4 = ip_addr;
    unicast_route_entry.destination.mask.ip4 = ip_mask;
    
    attr_list[0].id = attr_id; 
        
    status = mrvl_sai_route_get_attr_test(&unicast_route_entry, 1, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        switch (attr_id) {
        case SAI_ROUTE_ATTR_PACKET_ACTION:
            printf("route get attributes for ACTION:%d \n",attr_list[0].value.s32);
            break;
        case SAI_ROUTE_ATTR_NEXT_HOP_ID:
            if (mrvl_sai_utl_is_object_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_NEXT_HOP) == SAI_STATUS_SUCCESS){
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx))) {
                    printf("mrvl_sai_wrap_route_get_attr: failed to get next hop\n");
                    return status;
                }
            } else {
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &nh_idx))) {
                    printf("mrvl_sai_wrap_route_get_attr: failed to get rif\n");
                    return status;
                }
            }
            printf("route get attributes for NEXT_HOP_ID:%d \n",nh_idx);
            break;              
        default:
            printf("Attribute id %d not supported\n",attr_id);
            break;
        }
    }else{
        printf("mrvl_sai_wrap_route_get_attr:a attribute id %d failed %d \n",attr_id, status);
    }
    return status; 
}
   
/*******************************************************************************
* mrvl_sai_route_set_attr_test
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_route_set_attr_test
(
    IN const sai_unicast_route_entry_t* unicast_route_entry,
    IN sai_attribute_t *attr_list
)
{    
    MRVL_SAI_LOG_INF("Calling sai_route_api->set_route_attribute\n");
    return sai_route_api->set_route_attribute(unicast_route_entry, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_route_set_attr
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_wrap_route_set_attr
(
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t ip_mask_len,
    IN sai_uint32_t attr_id,
    IN sai_uint32_t attr_data
)
{
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask;
    sai_unicast_route_entry_t unicast_route_entry;
    sai_attribute_t     attr_list[2];
    
	printf("mrvl_sai_wrap_route_get_attr:  ip:%d.%d.%d.%d/%d \n",
		   ip1, ip2, ip3, ip4, ip_mask_len);

    memset(&unicast_route_entry, 0, sizeof(unicast_route_entry));
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    unicast_route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    unicast_route_entry.destination.addr.ip4 = ip_addr;
    unicast_route_entry.destination.mask.ip4 = ip_mask;
    
    attr_list[0].id = attr_id; 
        
    switch (attr_id) {
    case SAI_ROUTE_ATTR_PACKET_ACTION:
        attr_list[0].value.s32 = attr_data;
        break;
    case SAI_ROUTE_ATTR_NEXT_HOP_ID:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, attr_data, &attr_list[0].value.oid)) {
            return SAI_STATUS_FAILURE;
        }
        break;              
    default:
        printf("Attribute id %d not supported\n",attr_id);
        return SAI_STATUS_FAILURE;
    }
    status = mrvl_sai_route_set_attr_test(&unicast_route_entry, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_route_set_attr_test failed to set route\n");
    } else {
        printf("mrvl_sai_route_set_attr_test: route was modify\n");
    }
    return status; 
}

/*******************************************************************************
* mrvl_sai_route_test
*
* DESCRIPTION:
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_route_test(void)
{
    
    sai_object_id_t     nh_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask, nh_idx, ip_mask_len;
    sai_packet_action_t action;
    sai_unicast_route_entry_t unicast_route_entry;
    sai_attribute_t     attr_list[2];
    sai_object_id_t     vr_id;
    
    mrvl_sai_route_pre_test(&vr_id);

    printf("Created default route TRAP\n");
    ip_addr = 0;
    ip_mask_len = 0;
    ip_mask = 0;
    action = SAI_PACKET_ACTION_TRAP;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, NULL, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }

    printf("Created route on nh 0 ip 1.2.3.9/32 FRW\n");
    nh_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((9)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    printf("Created route on nh 0 ip 1.2.3.3/32 FRW\n");
    nh_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((3)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    printf("Created route on nh 2 ip 1.1.1.1/32 FRW\n");
    nh_idx = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((1)<<16)+((1)<<8)+((1)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
  
    printf("Created route on nh 1 ip 1.2.3.0/24 FRW\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((0)));
    ip_mask_len = 24;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }

    printf("Created route on nh 1 ip 1.2.4.0/24 FRW\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((4)<<8)+((0)));
    ip_mask_len = 24;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }

    printf("Created route on nh 1 ip 1.2.5.0/24 DROP\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((5)<<8)+((0)));
    ip_mask_len = 24;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_DROP;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }

    printf("Created route on nh 0 ip 1.2.7.0/24 TRAP\n");
    nh_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((7)<<8)+((0)));
    ip_mask_len = 24;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_TRAP;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    printf("Created route on rif 0 ip 1.2.3.8/32 FORWARD\n");
    nh_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((8)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    
    printf("Created route on rif 1 ip 1.2.3.5/32 TRAP\n");
    nh_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((5)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_TRAP;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    printf("Created route on rif 1 ip 1.2.3.4/32 FORWARD\n");
    nh_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    
    printf("Set route attribute ip 1.2.3.9/32 change to nh=1\n");
    memset(&unicast_route_entry, 0, sizeof(unicast_route_entry));
    ip_addr = htonl(((1&0xff)<<24)+((2&0xff)<<16)+((3&0xff)<<8)+((9&0xff)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    unicast_route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    unicast_route_entry.destination.addr.ip4 = ip_addr;
    unicast_route_entry.destination.mask.ip4 = ip_mask;
    nh_idx = 1;
    attr_list[0].id = SAI_ROUTE_ATTR_NEXT_HOP_ID; 
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &attr_list[0].value.oid)) {
        return SAI_STATUS_FAILURE;
    }
    status = mrvl_sai_route_set_attr_test(&unicast_route_entry, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_route_set_attr_test failed to update nexthop id\n");
        return status;
    }
 
    printf("Set route attribute ip 1.2.4.0/24 change to DROP\n");
    memset(&unicast_route_entry, 0, sizeof(unicast_route_entry));
    ip_addr = htonl(((1&0xff)<<24)+((2&0xff)<<16)+((4&0xff)<<8)+((0&0xff)));
    ip_mask_len = 24;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &unicast_route_entry.vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    unicast_route_entry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    unicast_route_entry.destination.addr.ip4 = ip_addr;
    unicast_route_entry.destination.mask.ip4 = ip_mask;
    attr_list[0].id = SAI_ROUTE_ATTR_PACKET_ACTION; 
    action = SAI_PACKET_ACTION_DROP;
    attr_list[0].value.s32 = action;
    status = mrvl_sai_route_set_attr_test(&unicast_route_entry, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_route_set_attr_test failed to update nexthop id\n");
        return status;
    }
    
    return SAI_STATUS_SUCCESS;

}
   

void mrvl_sai_route_pre_demo_test
(
    sai_object_id_t *vr_id
)
{
    sai_vlan_port_t port_list[4];
    sai_mac_t mac_addr;
    uint32_t port_vlan, rif_idx, ip_addr;
    sai_router_interface_type_t interface_type;
    sai_object_id_t rif_id, nh_id, port_id;
    bool default_mac;
    sai_packet_action_t  nbr_miss_act, action;
    sai_neighbor_entry_t neighbor_entry;
    
    mrvl_sai_trace_set_all(1, SEVERITY_LEVEL_INFO);    
    printf("Create vlan 2 with port 0\n");
    mrvl_sai_vlan_create_test(2);
    /**** add ports to vlan 2 (0=untag, 1= tag)*/
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 0, &port_list[0].port_id);
    port_list[0].tagging_mode = 1;
    mrvl_sai_vlan_add_ports_test(2, 1, port_list);
    /************************************************/
    printf("Create vlan 3 with port 1-2\n");
    mrvl_sai_vlan_create_test(3);
    /**** add ports to vlan 3 (0=untag, 1= tag)*/
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[0].port_id);
    port_list[0].tagging_mode = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[1].port_id);
    port_list[1].tagging_mode = 1;
    mrvl_sai_vlan_add_ports_test(3, 2, port_list);
    /************************************************/
    
    printf("Create fdb entry mac:00:00:00:01:02:03 vlan 3 --> port 1 FORWARD\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    port_vlan = 3;
    action = SAI_PACKET_ACTION_FORWARD;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_id);
    mrvl_sai_fdb_add_test(mac_addr, port_vlan, SAI_FDB_ENTRY_STATIC, port_id, action);
    /************************************************/       
    
    printf("Created  virtual router 0 \n");
    mrvl_sai_virtual_router_add_test(vr_id);
    
    /**** create rif 0 on vlan 2 */ 
    printf("Created rif 0 on vlan 2, src mac:00:00:00:56:45:34, miss - trap\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    port_vlan = 2;
    default_mac = false;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x56;
    mac_addr[4] = 0x45;
    mac_addr[5] = 0x34;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, *vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    /************************************************/

    /**** create rif 1 on vlan 3 */ 
    printf("Created rif 1 on vlan 3, src mac:00:00:00:56:45:34, miss - trap\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    port_vlan = 3;
    default_mac = false;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x56;
    mac_addr[4] = 0x45;
    mac_addr[5] = 0x34;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, *vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    /************************************************/
    
    /**** create rif 2 on port 3 */ 
    printf("Created rif 2 on port 3, src mac: default, miss - drop\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_PORT;
    port_vlan = 3;
    default_mac = true;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0;
    mac_addr[4] = 0;
    mac_addr[5] = 0;
    nbr_miss_act = SAI_PACKET_ACTION_DROP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act,*vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    /************************************************/
    
    printf("Created neighbor on rif 1, ip 1.2.3.8, mac:00:00:00:01:02:03\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((8)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    /************************************************/

    
    printf("Created neighbor on rif 2, ip 2.1.1.1, mac:00:00:00:06:07:08\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x06;
    mac_addr[4] = 0x07;
    mac_addr[5] = 0x08;
    rif_idx = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((2)<<24)+((1)<<16)+((1)<<8)+((1)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    /************************************************/
    
    printf("Created nexthop 0 on rif 1, ip 1.2.3.8 \n");
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((8)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    /************************************************/

    printf("Created nexthop 1 on rif 2, ip 2.1.1.1 \n");
    rif_idx = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((2)<<24)+((1)<<16)+((1)<<8)+((1)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    /************************************************/
 
}


int mrvl_sai_route_demo(void)
{
    
    sai_object_id_t     nh_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask, nh_idx, ip_mask_len;
    sai_packet_action_t action;
    sai_object_id_t     vr_id;
    
    mrvl_sai_route_pre_demo_test(&vr_id);

    printf("Created default route TRAP\n");
    ip_addr = 0;
    ip_mask_len = 0;
    ip_mask = 0;
    action = SAI_PACKET_ACTION_TRAP;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, NULL, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }

    
    
    printf("Created route on rif 1 ip 1.2.3.8/32 FORWARD\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((8)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }

    
    
    
    
    printf("Created route on nh 1 ip 2.1.1.0/24 FRW\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((2)<<24)+((1)<<16)+((1)<<8)+((0)));
    ip_mask_len = 24;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_FORWARD;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    
    printf("Created route on nh 1 ip 2.1.1.3/32 DROP\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((2)<<24)+((1)<<16)+((1)<<8)+((3)));
    ip_mask_len = 32;
    ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
    action = SAI_PACKET_ACTION_DROP;
    status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    
    return SAI_STATUS_SUCCESS;

}

int mrvl_sai_route_add_table_size(uint32_t num_entries)
{
    
    sai_object_id_t     nh_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, ip_mask, nh_idx, ip_mask_len, count;
    sai_packet_action_t action;
    sai_object_id_t     vr_id;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    printf("Try to add %d route entries\n",num_entries);
    printf("Created route on nh 1 ip 1.1.1.1/32 FORWARD\n");
    nh_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &nh_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_mask_len = 32;
    action = SAI_PACKET_ACTION_FORWARD;
    count=0;
    while (num_entries> 0) {
        ip_addr = htonl(((1)<<24)+((1)<<16)+((1)<<8)+((1))+count);
        ip_mask = htonl(0xffffffff<<(32-ip_mask_len));
        status = mrvl_sai_route_add_test(ip_addr, ip_mask, action, &nh_id, vr_id);
        if (status != SAI_STATUS_SUCCESS) {
            break;
        }
        count++;
        num_entries--;
    }
    if (num_entries == 0) {
        printf("test finished OK\n");
    } else {
        printf("test failed - added only %d entries\n",count);
    }
    return SAI_STATUS_SUCCESS;
}

