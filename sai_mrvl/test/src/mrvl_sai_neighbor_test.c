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
* mrvl_sai_fdb_test.c
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
/*#include    "sai.h"
#include    "mrvl_sai.h"*/

extern sai_neighbor_api_t* sai_neighbor_api;

/*******************************************************************************
* mrvl_sai_nbr_pre_test
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
void mrvl_sai_nbr_pre_test
(
    void
)
{
    sai_vlan_port_t port_list[4];
    sai_mac_t mac_addr;
    uint32_t port_vlan, rif_idx;
    sai_router_interface_type_t interface_type;
    sai_object_id_t rif_id, port_id;
    bool default_mac;
    sai_packet_action_t  nbr_miss_act;
    sai_object_id_t      vr_id;

    mrvl_sai_trace_set_all(1, SEVERITY_LEVEL_ALERT);    
    /**** create rif on vlan 2 */

    printf("Create vlan 2 with port 0-2\n");
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
    /************************************************/

    printf("Create fdb entry mac:00:00:00:01:02:03 vlan 2 --> port 1 FORWARD\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    port_vlan = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_id);
    mrvl_sai_fdb_add_test(mac_addr, port_vlan, SAI_FDB_ENTRY_STATIC, port_id, SAI_PACKET_ACTION_FORWARD);
    /************************************************/
        
    printf("Create fdb entry mac:00:00:00:06:07:08 vlan 2 --> port 2 FORWARD\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x06;
    mac_addr[4] = 0x07;
    mac_addr[5] = 0x08;
    port_vlan = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_id);
    mrvl_sai_fdb_add_test(mac_addr, port_vlan, SAI_FDB_ENTRY_STATIC, port_id, SAI_PACKET_ACTION_FORWARD);
    /************************************************/
    
    printf("Created  virtual router 0 \n");
    mrvl_sai_virtual_router_add_test(&vr_id);


    /**** create rif on vlan 2 */ 
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

    /**** create rif on port 3 */ 
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

    mrvl_sai_trace_set_all(1, SEVERITY_LEVEL_INFO);    

}


/*******************************************************************************
* mrvl_sai_nbr_add_test
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
int mrvl_sai_nbr_add_test
(
    IN sai_neighbor_entry_t *neighbor_entry,
    IN sai_mac_t            mac_addr
)
{
    uint32_t attr_count=0;
    sai_attribute_t attr_list[3];
    sai_status_t status;

    attr_list[attr_count].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    memcpy(&attr_list[attr_count].value.mac, mac_addr, sizeof(sai_mac_t));
    attr_count++;
    
    MRVL_SAI_LOG_INF("Calling sai_neighbor_api->create_neighbor_entry\n");
    status = sai_neighbor_api->create_neighbor_entry(neighbor_entry, attr_count, attr_list);
	return status;
}


/*******************************************************************************
* mrvl_sai_wrap_nbr_add
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
int mrvl_sai_wrap_nbr_add
(
    IN sai_uint32_t rif_idx, 
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5
)
{
    sai_mac_t           mac_addr;
    sai_object_id_t     rif_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr;
    sai_neighbor_entry_t neighbor_entry;
    
	printf("mrvl_sai_wrap_nbr_add: rif: %d mac:00:%02x:%02x:%02x:%02x:%02x: ip:%d.%d.%d.%d \n",
		   rif_idx, mac1, mac2 ,mac3, mac4, mac5, ip1, ip2, ip3, ip4);
    mac_addr[0] = 0;
    mac_addr[1] = mac1;
    mac_addr[2] = mac2;
    mac_addr[3] = mac3;
    mac_addr[4] = mac4;
    mac_addr[5] = mac5;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    memset(&neighbor_entry, 0, sizeof(neighbor_entry));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    
    status = mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_add_test failed to create neighbor\n");
        return status;
    }
    return 0;
}

/*******************************************************************************
* mrvl_sai_nbr_remove_test
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
int mrvl_sai_nbr_remove_test
(
    IN sai_neighbor_entry_t* neighbor_entry
)
{
    MRVL_SAI_LOG_INF("Calling sai_neighbor_api->remove_neighbor_entry\n");
    return sai_neighbor_api->remove_neighbor_entry(neighbor_entry);
}

/*******************************************************************************
* mrvl_sai_wrap_nbr_remove
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
int mrvl_sai_wrap_nbr_remove
(
    IN sai_uint32_t rif_idx,
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4
)
{
    sai_object_id_t rif_id;
    sai_status_t    status;
    sai_neighbor_entry_t neighbor_entry;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    memset(&neighbor_entry, 0, sizeof(neighbor_entry));    
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));

    
    status = mrvl_sai_nbr_remove_test(&neighbor_entry);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_remove_test failed to delete neighbor\n");
    } else {
        printf("mrvl_sai_wrap_nbr_remove: neighbor was deleted\n");
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_nbr_get_attr_test
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
int mrvl_sai_nbr_get_attr_test
(
    IN sai_neighbor_entry_t* neighbor_entry,
    IN uint32_t attr_count,
    OUT sai_attribute_t *attr_list
)
{
    
    MRVL_SAI_LOG_INF("Calling sai_neighbor_api->get_neighbor_attribute\n");
    return sai_neighbor_api->get_neighbor_attribute(neighbor_entry, attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_nbr_get_attr
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
int mrvl_sai_wrap_nbr_get_attr
(
    IN sai_uint32_t rif_idx,
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t attr_id
)
{
    sai_attribute_t attr_list[1];
    sai_object_id_t rif_id;
    sai_status_t    status;
    sai_neighbor_entry_t neighbor_entry;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    memset(&neighbor_entry, 0, sizeof(neighbor_entry));    
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    
    attr_list[0].id = attr_id; 
        
    status = mrvl_sai_nbr_get_attr_test(&neighbor_entry, 1, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        switch (attr_id) {
        case SAI_NEIGHBOR_ATTR_PACKET_ACTION:
            printf("neighbor get attributes for ACTION:%d \n",attr_list[0].value.s32);
            break;

        case SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS:
            printf("neighbor get attributes for MAC_ADDRESS:%02x:%02x:%02x:%02x:%02x:%02x\n",
                attr_list[0].value.mac[0],
                attr_list[0].value.mac[1],
                attr_list[0].value.mac[2],
                attr_list[0].value.mac[3],
                attr_list[0].value.mac[4],
                attr_list[0].value.mac[5]);
            break;
        default:
            printf("Attribute id %d not supported\n",attr_id);
            break;
        }
    }else{
        printf("mrvl_sai_wrap_nbr_get_attr:a attribute id %d failed %d \n",attr_id, status);
    }
    return status; 
}

/*******************************************************************************
* mrvl_sai_nbr_set_attr_test
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
int mrvl_sai_nbr_set_attr_test
(
    IN sai_neighbor_entry_t* neighbor_entry,
    IN sai_attribute_t *attr_list    
)
{   
    MRVL_SAI_LOG_INF("Calling sai_neighbor_api->set_neighbor_attribute\n");
    return sai_neighbor_api->set_neighbor_attribute(neighbor_entry, attr_list);

}

/*******************************************************************************
* mrvl_sai_wrap_nbr_set_attr
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
int mrvl_sai_wrap_nbr_set_mac_attr
(
    IN sai_uint32_t rif_idx,
    IN sai_uint32_t ip1,
    IN sai_uint32_t ip2,
    IN sai_uint32_t ip3,
    IN sai_uint32_t ip4,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5
)
{
    sai_attribute_t attr_list;
    sai_object_id_t rif_id;
    sai_neighbor_entry_t neighbor_entry;
    
    printf("mrvl_sai_wrap_nbr_set: rif: %d mac:00:%02x:%02x:%02x:%02x:%02x: ip:%d.%d.%d.%d \n",
           rif_idx, mac1, mac2 ,mac3, mac4, mac5, ip1, ip2, ip3, ip4);
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    memset(&neighbor_entry, 0, sizeof(neighbor_entry));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = htonl(((ip1&0xff)<<24)+((ip2&0xff)<<16)+((ip3&0xff)<<8)+((ip4&0xff)));
    
    attr_list.id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS; 
    attr_list.value.mac[0] = 0;
    attr_list.value.mac[1] = mac1;
    attr_list.value.mac[2] = mac2;
    attr_list.value.mac[3] = mac3;
    attr_list.value.mac[4] = mac4;
    attr_list.value.mac[5] = mac5;
    return mrvl_sai_nbr_set_attr_test(&neighbor_entry, &attr_list);
}


/*******************************************************************************
* mrvl_sai_nbr_set_remove_all
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
int mrvl_sai_nbr_set_remove_all
(
    void
)
{   
    MRVL_SAI_LOG_INF("Calling sai_neighbor_api->remove_all_neighbor_entries\n");
    return sai_neighbor_api->remove_all_neighbor_entries();

}

/*******************************************************************************
* mrvl_sai_wrap_nbr_remove_all
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
int mrvl_sai_wrap_nbr_remove_all
(
    void
)
{
    sai_status_t        status;
    
    printf("mrvl_sai_wrap_nbr_remove_all: \n");
    
    status = mrvl_sai_nbr_set_remove_all();
    if (status == SAI_STATUS_SUCCESS) {
        printf("neighbor remove all finished ok\n");
    } else {
        printf("neighbor remove all failed %d \n", status);        
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_neighbor_test
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
int mrvl_sai_neighbor_test(void)
{
    
    sai_mac_t           mac_addr;
    sai_object_id_t     rif_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, rif_idx;
    sai_neighbor_entry_t neighbor_entry;
    sai_attribute_t     attr_list[2];
    
    mrvl_sai_nbr_pre_test();

    printf("Created neighbor on rif 0 ip 1.2.3.4 mac:00:00:00:01:02:03\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    status = mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_add_test failed to create neighbor\n");
        return status;
    }

    
    printf("Created neighbor on rif 1 ip 1.2.3.5 mac:00:00:12:34:56:99\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x12;
    mac_addr[3] = 0x34;
    mac_addr[4] = 0x56;
    mac_addr[5] = 0x99;
    rif_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((5)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    status = mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_add_test failed to create neighbor\n");
        return status;
    }
    
    printf("Created neighbor on rif 0 ip 3.4.5.6 mac:00:00:00:99:98:97 - should fail - no entry in fdb\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x99;
    mac_addr[4] = 0x98;
    mac_addr[5] = 0x97;
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    status = mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    if (status == SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_add_test failed \n");
        return status;
    }
    
        
    printf("Set attr for neighbor on rif 0 ip 1.2.3.4 ==> new mac:00:00:01:02:03:04\n");
    memset(&neighbor_entry, 0, sizeof(neighbor_entry));
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x01;
    mac_addr[3] = 0x02;
    mac_addr[4] = 0x03;
    mac_addr[5] = 0x04;
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;

    attr_list[0].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    memcpy(attr_list[0].value.mac, mac_addr, 6); 
    status = mrvl_sai_nbr_set_attr_test(&neighbor_entry, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_set_attr_test failed to update mac\n");
        return status;
    }

    
    printf("Get attr for neighbor on rif 0 ip 1.2.3.4\n");
    memset(&attr_list[0], 0, sizeof(sai_attribute_t)); 
    attr_list[0].id = SAI_NEIGHBOR_ATTR_DST_MAC_ADDRESS;
    status = mrvl_sai_nbr_get_attr_test(&neighbor_entry, 1, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nbr_get_attr_test failed to get mac\n");
        return status;
    }
    printf("neighbor get attributes for MAC_ADDRESS:%02x:%02x:%02x:%02x:%02x:%02x\n",
            attr_list[0].value.mac[0],
            attr_list[0].value.mac[1],
            attr_list[0].value.mac[2],
            attr_list[0].value.mac[3],
            attr_list[0].value.mac[4],
            attr_list[0].value.mac[5]);
    if (memcmp(mac_addr, attr_list[0].value.mac, 6) != 0){
        printf("mrvl_sai_nbr_get_attr_test failed to get mac\n");
        return SAI_STATUS_FAILURE;
    }
    return status;
}
    
    
int mrvl_sai_neighbor_add_table_size(uint32_t   num_entries)
{
    
    sai_mac_t           mac_addr;
    sai_object_id_t     rif_id;
    sai_status_t        status;
    sai_uint32_t        ip_addr, rif_idx;
    sai_neighbor_entry_t neighbor_entry;
    uint32_t            count;

    printf("Created neighbor on rif 0 ip 1.1.1.1 mac:00:00:00:01:02:03\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x00;
    mac_addr[3] = 0x01;
    mac_addr[4] = 0x02;
    mac_addr[5] = 0x03;
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    
    printf("Try to add %d neighbors \n",num_entries);
    count = 0;
    while (num_entries > count) {
        ip_addr = htonl(((1)<<24)+((1)<<16)+((1)<<8)+((1))+count);
        neighbor_entry.ip_address.addr.ip4 = ip_addr;
        status = mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
        if (status != SAI_STATUS_SUCCESS) {
            break;
        }
        count++;
    }
    if (num_entries == count) {
        printf("test finished OK\n");
    } else {
        printf("test failed - added only %d entries\n",count);
    }
    return SAI_STATUS_SUCCESS;
}
