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
* mrvl_sai_nexthop_test.c
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

extern sai_next_hop_api_t* sai_nexthop_api;

/*******************************************************************************
* mrvl_sai_nh_pre_test
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
void mrvl_sai_nh_pre_test
(
    void
)
{
    sai_vlan_port_t port_list[4];
    sai_mac_t mac_addr;
    uint32_t port_vlan, rif_idx, ip_addr;
    sai_router_interface_type_t interface_type;
    sai_object_id_t rif_id;
    bool default_mac;
    sai_packet_action_t  nbr_miss_act;
    sai_neighbor_entry_t neighbor_entry;
    sai_object_id_t      vr_id;

    mrvl_sai_trace_set_all(1, SEVERITY_LEVEL_ALERT);    
    /**** create rif on vlan 2 */
    mrvl_sai_vlan_create_test(2);


    /**** add ports to vlan 2 */
    /* 0=untag, 1= tag */
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 0, &port_list[0].port_id);
    port_list[0].tagging_mode = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[1].port_id);
    port_list[1].tagging_mode = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[2].port_id);
    port_list[2].tagging_mode = 1;
    mrvl_sai_vlan_add_ports_test(2, 3, port_list);

    printf("Created  virtual router 0 \n");
    mrvl_sai_virtual_router_add_test(&vr_id);

    /**** create rif 0 on vlan 2 */ 
    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    port_vlan = 2;
    default_mac = false;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x11;
    mac_addr[4] = 0x11;
    mac_addr[5] = 0x11;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);

    /**** create rif 1 on port 3 */ 
    interface_type = SAI_ROUTER_INTERFACE_TYPE_PORT;
    port_vlan = 3;
    default_mac = false;
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x11;
    mac_addr[4] = 0x11;
    mac_addr[5] = 0x22;
    nbr_miss_act = SAI_PACKET_ACTION_DROP;
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &rif_id);
    mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);

    
    printf("Created neighbor on rif 0 ip 1.2.3.4 mac:00:00:12:34:56:78\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x12;
    mac_addr[3] = 0x34;
    mac_addr[4] = 0x56;
    mac_addr[5] = 0x78;
    rif_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 

    printf("Created neighbor on rif 1 ip 1.2.3.5 mac:00:00:12:34:56:99\n");
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
    
    mrvl_sai_trace_set_all(1, SEVERITY_LEVEL_INFO);    

}

/*******************************************************************************
* mrvl_sai_nh_add_test
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
int mrvl_sai_nh_add_test
(
    _In_ uint32_t ipv4,
    _In_ sai_object_id_t *rif_id,
    _Out_ sai_object_id_t* next_hop_id
)
{
    uint32_t attr_count=0;
    sai_attribute_t attr_list[3];
    sai_status_t status;
    memset(attr_list, 0, sizeof(sai_attribute_t)*3);
    attr_list[attr_count].id = SAI_NEXT_HOP_ATTR_TYPE;
    attr_list[attr_count].value.s32 = SAI_NEXT_HOP_IP;
    attr_count++;
    
    attr_list[attr_count].id = SAI_NEXT_HOP_ATTR_IP;
    attr_list[attr_count].value.ipaddr.addr.ip4 = ipv4;
    attr_list[attr_count].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr_count++;

    attr_list[attr_count].id = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID;
    attr_list[attr_count].value.oid = *rif_id;
    attr_count++;
    MRVL_SAI_LOG_INF("Calling sai_nexthop_api->create_next_hop\n");
    status = sai_nexthop_api->create_next_hop(next_hop_id, attr_count, attr_list);
	return status;
}


/*******************************************************************************
* mrvl_sai_wrap_nh_add
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
int mrvl_sai_wrap_nh_add
(
    IN sai_uint32_t rif_idx,
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4
)
{
    sai_object_id_t     next_hop_id, rif_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, next_hop_idx;
    
	printf("mrvl_sai_wrap_nh_add: rif: %d  ip:%d.%d.%d.%d \n",
		   rif_idx, ip1, ip2, ip3, ip4);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    
    status = mrvl_sai_nh_add_test(ip_addr, &rif_id, &next_hop_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP, &next_hop_idx))) {
        printf("mrvl_sai_nh_add_test failed to create nexthop object\n");
        return status;
    }
    printf("mrvl_sai_wrap_nh_add: rif idx %d was created\n",rif_idx);
    
    return 0;
}

/*******************************************************************************
* mrvl_sai_nh_remove_test
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
int mrvl_sai_nh_remove_test
(
    _In_ sai_object_id_t next_hop_id
)
{
    MRVL_SAI_LOG_INF("Calling sai_nexthop_api->remove_next_hop\n");
    return sai_nexthop_api->remove_next_hop(next_hop_id);
}

/*******************************************************************************
* mrvl_sai_wrap_nh_remove
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
int mrvl_sai_wrap_nh_remove
(
    IN sai_uint32_t nh_idx
)
{
    sai_status_t    status;
    sai_object_id_t next_hop_id;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &next_hop_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    status = mrvl_sai_nh_remove_test(next_hop_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_remove_test failed to delete next hop\n");
    } else {
        printf("mrvl_sai_wrap_nh_remove: next hop %d was deleted\n", nh_idx);
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_nh_get_attr_test
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
int mrvl_sai_nh_get_attr_test
(
    IN sai_object_id_t next_hop_id,
    IN uint32_t attr_count,
    OUT sai_attribute_t *attr_list
)
{    
    MRVL_SAI_LOG_INF("Calling sai_nexthop_api->get_next_hop_attribute\n");
    return sai_nexthop_api->get_next_hop_attribute(next_hop_id, attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_nh_get_attr
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
int mrvl_sai_wrap_nh_get_attr
(
    IN sai_uint32_t nh_idx,
    IN sai_uint32_t attr_id
)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_attribute_t attr_list[1];
    sai_object_id_t next_hop_id;
    sai_status_t    status, temp;
    sai_uint32_t rif_idx;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &next_hop_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    attr_list[0].id = attr_id; 
        
    status = mrvl_sai_nh_get_attr_test(next_hop_id, 1, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        switch (attr_id) {
        case SAI_NEXT_HOP_ATTR_TYPE:
            printf("nexthop get attributes for TYPE:%d \n",attr_list[0].value.s32);
            break;
        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
                printf("mrvl_sai_wrap_nbr_get_attr: failed to get rif\n");
                return status;
            }
            printf("nexthop get attributes for ROUTER_INTERFACE:%d \n",rif_idx);
            break;              
        case SAI_NEXT_HOP_ATTR_IP:
            mrvl_sai_utl_ipaddr_to_str(attr_list[0].value.ipaddr, MAX_KEY_STR_LEN, key_str, &temp);                           
            printf("nexthop get attributes for IP:family %d, addr %x %s\n",
                attr_list[0].value.ipaddr.addr_family,
                attr_list[0].value.ipaddr.addr.ip4, key_str);  
            break;
        default:
            printf("Attribute id %d not supported\n",attr_id);
            break;
        }
    }else{
        printf("mrvl_sai_wrap_nh_get_attr:a attribute id %d failed %d \n",attr_id, status);
    }
    return status; 
}
    

/*******************************************************************************
* mrvl_sai_nexthop_test
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
int mrvl_sai_nexthop_test(void)
{
    
    sai_object_id_t     rif_id, nh_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, rif_idx, nh_idx;
    
    mrvl_sai_nh_pre_test();
    printf("Created nexthop on rif 0 ip 1.2.3.4 \n");
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));


    status = mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(nh_id, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx))) {
        printf("mrvl_sai_nh_add_test: failed to create next hop\n");
        return status;
    }
    printf("mrvl_sai_nh_add_test:Next hop %d was created\n", nh_idx);

    printf("Created nexthop on rif 1 ip 1.2.3.5 \n");
    rif_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((5)));


    status = mrvl_sai_nh_add_test(ip_addr, &rif_id, &nh_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nh_add_test failed to create nexthop\n");
        return status;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(nh_id, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx))) {
        printf("mrvl_sai_nh_add_test: failed to create next hop\n");
        return status;
    }
    printf("mrvl_sai_nh_add_test:Next hop %d was created\n", nh_idx);
    return SAI_STATUS_SUCCESS;

}
   
 
    

