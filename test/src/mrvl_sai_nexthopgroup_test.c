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
* mrvl_sai_nexthopgroup_test.c
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

#define MRVL_SAI_WRAP_NHG_LIST_SIZE 10
extern sai_next_hop_group_api_t* sai_nexthop_group_api;

static sai_object_id_t sai_nhg_nh_list[10];
static uint32_t sai_nhg_nh_counter = 0;
static sai_object_id_t sai_nhg_rif_list[10];
static uint32_t sai_nhg_rif_counter = 0;
static uint32_t sai_nhg_vlan_member_list[10];
static uint32_t sai_nhg_vlan_member_counter = 0;

static sai_object_id_t sai_nhg_add_list[MRVL_SAI_WRAP_NHG_LIST_SIZE];
/*******************************************************************************
* mrvl_sai_nhg_pre_test
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
void mrvl_sai_nhg_pre_test
(
    void
)
{
    sai_mac_t               mac_addr;
    uint32_t                port_vlan, rif_idx, ip_addr, vlan;
    sai_router_interface_type_t interface_type;
    sai_object_id_t         rif_id, port_id;
    bool                    default_mac;
    sai_packet_action_t     nbr_miss_act;
    sai_neighbor_entry_t    neighbor_entry;
    sai_object_id_t         vr_id;
    sai_uint32_t num_of_ports = 0;
    sai_port_info_t ports_list[3];
    
    /**** create rif on vlan 2 */
    vlan = 2;
    mrvl_sai_vlan_create_test(vlan);


    /**** add ports to vlan 2 */
    /* 0=untag, 1= tag */
    ports_list[num_of_ports].port = 0;
    ports_list[num_of_ports++].tag = SAI_VLAN_PORT_UNTAGGED;
    ports_list[num_of_ports].port = 1;
    ports_list[num_of_ports++].tag = SAI_VLAN_PORT_TAGGED;
    ports_list[num_of_ports].port = 2;
    ports_list[num_of_ports++].tag = SAI_VLAN_PORT_TAGGED;
    mrvl_sai_wrap_vlan_add_ports_list(vlan, num_of_ports, ports_list, &sai_nhg_vlan_member_list[sai_nhg_vlan_member_counter]);
    sai_nhg_vlan_member_counter = 3;
    
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
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &sai_nhg_rif_list[sai_nhg_rif_counter]);
    mrvl_sai_utl_object_to_type(sai_nhg_rif_list[sai_nhg_rif_counter], SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    sai_nhg_rif_counter++;

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
    mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &sai_nhg_rif_list[sai_nhg_rif_counter]);
    mrvl_sai_utl_object_to_type(sai_nhg_rif_list[sai_nhg_rif_counter], SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx);
    sai_nhg_rif_counter++;
    
    printf("Create fdb entry mac:00:00:00:01:02:03 vlan 2 --> port 1 FORWARD\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0x12;
    mac_addr[3] = 0x34;
    mac_addr[4] = 0x56;
    mac_addr[5] = 0x78;
    port_vlan = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_id);
    mrvl_sai_fdb_add_test(mac_addr, port_vlan, SAI_FDB_ENTRY_STATIC, port_id, SAI_PACKET_ACTION_FORWARD);
    /************************************************/
    
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
    /************************************************/
    printf("Created neighbor on rif 1 ip 2.2.2.1 mac:00:00:00:22:22:11\n");
    mac_addr[0] = 0;
    mac_addr[1] = 0;
    mac_addr[2] = 0;
    mac_addr[3] = 0x22;
    mac_addr[4] = 0x22;
    mac_addr[5] = 0x11;
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((2)<<24)+((2)<<16)+((2)<<8)+((1)));
    neighbor_entry.rif_id = rif_id;
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 = ip_addr;
    mrvl_sai_nbr_add_test(&neighbor_entry, mac_addr); 
    /************************************************/

    printf("Created nexthop 0 on rif 0, ip 1.2.3.4 \n");
    rif_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((4)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &sai_nhg_nh_list[sai_nhg_nh_counter]); 
    sai_nhg_nh_counter++;
    /************************************************/

    printf("Created nexthop 1 on rif 1, ip 1.2.3.5 \n");
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((1)<<24)+((2)<<16)+((3)<<8)+((5)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &sai_nhg_nh_list[sai_nhg_nh_counter]); 
    sai_nhg_nh_counter++;
    /************************************************/
    printf("Created nexthop 2 on rif 1, ip 2.2.2.1 \n");
    rif_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id);
    ip_addr = htonl(((2)<<24)+((2)<<16)+((2)<<8)+((1)));
    mrvl_sai_nh_add_test(ip_addr, &rif_id, &sai_nhg_nh_list[sai_nhg_nh_counter]); 
    sai_nhg_nh_counter++;
    /************************************************/

}

void mrvl_sai_nhg_pre_test_clean
(
    void
)
{
    uint32_t i;
    sai_object_id_t vr_id;
    sai_object_id_t vlan_member_id;
    

    for (i = 0; i < sai_nhg_nh_counter; i++) {
        mrvl_sai_nh_remove_test(sai_nhg_nh_list[i]); 
    }
    sai_nhg_nh_counter = 0;
/************************************************/
    mrvl_sai_nbr_set_remove_all();
    
    for (i = 0; i < sai_nhg_rif_counter; i++) {
        mrvl_sai_rif_remove_test(sai_nhg_rif_list[i]); 
    }
    sai_nhg_rif_counter = 0;
    
    mrvl_sai_fdb_flush_test(-1,SAI_FDB_FLUSH_ENTRY_STATIC,-1);
    
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id);
    mrvl_sai_virtual_router_remove_test(vr_id); 

    for (i = 0; i < sai_nhg_vlan_member_counter; i++) {
        mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, sai_nhg_vlan_member_list[i], &vlan_member_id);
        mrvl_sai_vlan_remove_member_test(vlan_member_id);
    }
    sai_nhg_vlan_member_counter = 0;
    mrvl_sai_vlan_remove_test(2);
        
}
/*******************************************************************************
* mrvl_sai_nhg_create_test
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
int mrvl_sai_nhg_create_test
(
    _In_ sai_object_list_t next_hop_group_list,
    _Out_ sai_object_id_t *next_hop_group_id
)
{
    uint32_t attr_count=0;
    sai_attribute_t attr_list[3];
    sai_status_t status;
    memset(attr_list, 0, sizeof(sai_attribute_t)*3);
    attr_list[attr_count].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    attr_list[attr_count++].value.s32 = SAI_NEXT_HOP_GROUP_ECMP;
    
    attr_list[attr_count].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST;
    attr_list[attr_count++].value.objlist = next_hop_group_list;

    MRVL_SAI_LOG_INF("Calling sai_nexthop_group_api->create_next_hop_group\n");
    status = sai_nexthop_group_api->create_next_hop_group(next_hop_group_id, attr_count, attr_list);
	return status;
}


/*******************************************************************************
* mrvl_sai_wrap_nhg_create
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
int mrvl_sai_wrap_nhg_create
(
    IN sai_uint32_t nh_idx_1,
    IN sai_uint32_t nh_idx_2
)
{
    sai_object_id_t     next_hop_group_id;
    sai_status_t        status;
    sai_uint32_t        next_hop_group_idx, counter = 0;
    sai_object_list_t   nhg_list;
    
	printf("mrvl_sai_wrap_nhg_create: nexthop idx1:%d  nexthop idx2: %d \n",
		   nh_idx_1, nh_idx_2);

    if (nh_idx_1 != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx_1, &sai_nhg_add_list[counter])) {
            return SAI_STATUS_FAILURE;
        }
        counter++;
    }
    if (nh_idx_2 != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx_2, &sai_nhg_add_list[counter])) {
            return SAI_STATUS_FAILURE;
        }
        counter++;
    }
    nhg_list.count = counter;
    nhg_list.list  =  sai_nhg_add_list;
     
    status = mrvl_sai_nhg_create_test(nhg_list, &next_hop_group_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nhg_create_test failed to create nexthop\n");
        return status;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(next_hop_group_id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP, &next_hop_group_idx))) {
        printf("mrvl_sai_nhg_create_test failed to create nexthop object\n");
        return status;
    }
    printf("mrvl_sai_wrap_nhg_create: next hop group idx %d was created\n",next_hop_group_idx);
    
    return 0;
}

/*******************************************************************************
* mrvl_sai_nhg_remove_test
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
int mrvl_sai_nhg_remove_test
(
    _In_ sai_object_id_t next_hop_group_id
)
{
    MRVL_SAI_LOG_INF("Calling sai_nexthop_group_api->remove_next_hop_group\n");
    return sai_nexthop_group_api->remove_next_hop_group(next_hop_group_id);
}

/*******************************************************************************
* mrvl_sai_wrap_nhg_remove
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
int mrvl_sai_wrap_nhg_remove
(
    IN sai_uint32_t nhg_idx
)
{
    sai_status_t    status;
    sai_object_id_t next_hop_group_id;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &next_hop_group_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    status = mrvl_sai_nhg_remove_test(next_hop_group_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_nhg_remove_test failed to delete next hop\n");
    } else {
        printf("mrvl_sai_wrap_nhg_remove: next hop group %d was deleted\n", nhg_idx);
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_nhg_get_attr_test
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
int mrvl_sai_nhg_get_attr_test
(
    IN sai_object_id_t next_hop_group_id,
    IN uint32_t attr_count,
    OUT sai_attribute_t *attr_list
)
{    
    MRVL_SAI_LOG_INF("Calling sai_nexthop_group_api->get_next_hop_group_attribute\n");
    return sai_nexthop_group_api->get_next_hop_group_attribute(next_hop_group_id, attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_nhg_get_attr
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
int mrvl_sai_wrap_nhg_get_attr
(
    IN sai_uint32_t nhg_idx,
    IN sai_uint32_t attr_id
)
{
    sai_attribute_t     attr_list[1];
    sai_object_id_t     next_hop_group_id;
    sai_status_t        status;
    sai_object_list_t   *objlist;
    uint32_t            nh_idx, i;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &next_hop_group_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    attr_list[0].id = attr_id; 
    if (attr_id == SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST) {
        attr_list[0].value.objlist.count = MRVL_SAI_WRAP_NHG_LIST_SIZE; 
        attr_list[0].value.objlist.list  =  sai_nhg_add_list;
    }
        
    status = mrvl_sai_nhg_get_attr_test(next_hop_group_id, 1, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        switch (attr_id) {
        case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
            printf("nexthop group get attributes for TYPE:%d \n",attr_list[0].value.s32);
            break;
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
            printf("nexthop group get attributes for NEXT_HOP_COUNT:%d \n",attr_list[0].value.u32);
            break;
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST:
            objlist = &attr_list[0].value.objlist;
            printf("nexthop group get attributes for NEXT_HOP_LIST count:%d \n",objlist->count);
            for (i=0; i < objlist->count; i++) {
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(objlist->list[i], SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx))) {
                    return status;
                }
                printf("[%d] next hop index:%d \n", i, nh_idx);
            }
            break;              
        default:
            printf("Attribute id %d not supported\n",attr_id);
            break;
        }
    }else{
        printf("mrvl_sai_wrap_nhg_get_attr: attribute id %d failed %d \n",attr_id, status);
    }
    return status; 
}
 
 
/*******************************************************************************
* mrvl_sai_nhg_set_attr_test
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
int mrvl_sai_nhg_set_attr_test
(
    IN sai_object_id_t next_hop_group_id,
    IN sai_attribute_t *attr_list
)
{    
    MRVL_SAI_LOG_INF("Calling sai_nexthop_group_api->set_next_hop_group_attribute\n");
    return sai_nexthop_group_api->set_next_hop_group_attribute(next_hop_group_id, attr_list);
}

  
/*******************************************************************************
* mrvl_sai_wrap_nhg_set_attr
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
int mrvl_sai_wrap_nhg_set_attr
(
    IN sai_uint32_t nhg_idx,
    IN sai_uint32_t nh_idx_1,
    IN sai_uint32_t nh_idx_2
    
)
{
    sai_attribute_t     attr_list[1];
    sai_object_id_t     next_hop_group_id;
    sai_status_t        status;
    uint32_t            counter = 0;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &next_hop_group_id)) {
        return SAI_STATUS_FAILURE;
    }
 
    if (nh_idx_1 != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx_1, &sai_nhg_add_list[counter])) {
            return SAI_STATUS_FAILURE;
        }
        counter++;
    }
    if (nh_idx_2 != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx_2, &sai_nhg_add_list[counter])) {
            return SAI_STATUS_FAILURE;
        }
        counter++;
    }
    attr_list[0].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST;
    attr_list[0].value.objlist.count = counter; 
    attr_list[0].value.objlist.list  =  sai_nhg_add_list;
        
    status = mrvl_sai_nhg_set_attr_test(next_hop_group_id, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        printf("nexthop group set attributes for NEXT_HOP_LIST ok\n");
    }
    return status; 
}
    
/*******************************************************************************
* mrvl_sai_nhg_add_to_group_test
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
int mrvl_sai_nhg_add_to_group_test
(
    IN sai_object_id_t next_hop_group_id,
    IN uint32_t next_hop_count,
    IN const sai_object_id_t* nexthops
)
{    
    MRVL_SAI_LOG_INF("Calling sai_nexthop_group_api->add_next_hop_to_group\n");
    return sai_nexthop_group_api->add_next_hop_to_group(next_hop_group_id, next_hop_count, nexthops);
}

  
/*******************************************************************************
* mrvl_sai_wrap_nhg_add_to_group
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
int mrvl_sai_wrap_nhg_add_to_group
(
    IN sai_uint32_t nhg_idx,
    IN sai_uint32_t nh_idx_1
    
)
{
    sai_object_id_t     next_hop_group_id;
    sai_object_id_t     nexthops[MRVL_SAI_WRAP_NHG_LIST_SIZE];
    sai_status_t        status;
    uint32_t            counter = 0;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &next_hop_group_id)) {
        return SAI_STATUS_FAILURE;
    }
 
    if (nh_idx_1 != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx_1, &nexthops[counter])) {
            return SAI_STATUS_FAILURE;
        }
        counter++;
    }
        
    status = mrvl_sai_nhg_add_to_group_test(next_hop_group_id, counter, nexthops);
    if (status == SAI_STATUS_SUCCESS) {
        printf("nexthop group add to grop %d ok\n", nhg_idx);
    } else {
        printf("nexthop group add to grop %d failed\n", nhg_idx);
    }
    return status; 
}
    
/*******************************************************************************
* mrvl_sai_nhg_remove_from_group_test
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
int mrvl_sai_nhg_remove_from_group_test
(
    IN sai_object_id_t next_hop_group_id,
    IN uint32_t next_hop_count,
    IN const sai_object_id_t* nexthops
)
{    
    MRVL_SAI_LOG_INF("Calling sai_nexthop_group_api->remove_next_hop_from_group\n");
    return sai_nexthop_group_api->remove_next_hop_from_group(next_hop_group_id, next_hop_count, nexthops);
}

  
/*******************************************************************************
* mrvl_sai_wrap_nhg_remove_from_group
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
int mrvl_sai_wrap_nhg_remove_from_group
(
    IN sai_uint32_t nhg_idx,
    IN sai_uint32_t nh_idx_1
    
)
{
    sai_object_id_t     next_hop_group_id;
    sai_object_id_t     nexthops[MRVL_SAI_WRAP_NHG_LIST_SIZE];
    sai_status_t        status;
    uint32_t            counter = 0;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &next_hop_group_id)) {
        return SAI_STATUS_FAILURE;
    }
 
    if (nh_idx_1 != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx_1, &nexthops[counter])) {
            return SAI_STATUS_FAILURE;
        }
        counter++;
    }
        
    status = mrvl_sai_nhg_remove_from_group_test(next_hop_group_id, counter, nexthops);
    if (status == SAI_STATUS_SUCCESS) {
        printf("nexthop group remove from group %d ok\n", nhg_idx);
    } else {
        printf("nexthop group remove from group %d failed\n", nhg_idx);
    }
    return status;
}
    
   

/*******************************************************************************
* mrvl_sai_nexthop_group_test
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
int mrvl_sai_nexthop_group_test(void)
{
    
    sai_object_id_t      nhg_id;
    sai_uint32_t        i, nh_idx, nhg_idx, counter=0;
    sai_object_list_t   next_hop_group_list;
    sai_attribute_t     attr_list[4];

    mrvl_sai_nhg_pre_test();
    next_hop_group_list.list  = sai_nhg_add_list;
    printf("Created nexthop group 0 with nexthop 0\n");
    nh_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    next_hop_group_list.count = counter;
    next_hop_group_list.list  = sai_nhg_add_list;
    mrvl_sai_nhg_create_test(next_hop_group_list, &nhg_id);

    printf("Created nexthop group 1 with nexthop 0,1\n");
    nh_idx = 0;
    counter = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    nh_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    next_hop_group_list.count = counter;
    mrvl_sai_nhg_create_test(next_hop_group_list, &nhg_id);

    printf("Set attr for nexthop group 0 with nexthop 1,2\n");
    nh_idx = 1;
    counter = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    nh_idx = 2;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;    
    nhg_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    attr_list[0].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST;
    attr_list[0].value.objlist.count = counter; 
    attr_list[0].value.objlist.list  =  sai_nhg_add_list;
    mrvl_sai_nhg_set_attr_test(nhg_id, attr_list);

    printf("Add group 2 to nexthop group 1 \n");
    nhg_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    nh_idx = 2;
    counter = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    mrvl_sai_nhg_add_to_group_test(nhg_id,1,sai_nhg_add_list);
    
    printf("Remove group 1 from nexthop group 0 \n");
    nhg_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    nh_idx = 1;
    counter = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    mrvl_sai_nhg_remove_from_group_test(nhg_id,1,sai_nhg_add_list);
    
    printf("Add group 1 to nexthop group 0 \n");
    nhg_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    nh_idx = 1;
    counter = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    mrvl_sai_nhg_add_to_group_test(nhg_id, 1, sai_nhg_add_list);

    printf("Add group 0 to nexthop group 0 \n");
    nhg_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    nh_idx = 0;
    counter = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    mrvl_sai_nhg_add_to_group_test(nhg_id, 1, sai_nhg_add_list);

    printf("Remove nexthop group 1 \n");
    nhg_idx = 1;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    mrvl_sai_nhg_remove_test(nhg_id);

    
    printf("Get attr for nexthop group 0 \n");
    nhg_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    attr_list[0].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_LIST; 
    attr_list[0].value.objlist.count = MRVL_SAI_WRAP_NHG_LIST_SIZE; 
    attr_list[0].value.objlist.list  =  sai_nhg_add_list;
    attr_list[1].id = SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT; 
    attr_list[2].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE; 

    mrvl_sai_nhg_get_attr_test(nhg_id, 3, attr_list);
    printf(">>>nexthop group get attributes for TYPE:%d \n",attr_list[2].value.s32);
    printf(">>>nexthop group get attributes for NEXT_HOP_COUNT:%d \n",attr_list[1].value.u32);
    printf(">>>nexthop group get attributes for NEXT_HOP_LIST count:%d \n",attr_list[0].value.objlist.count);
    for (i=0; i < attr_list[0].value.objlist.count; i++) {
        mrvl_sai_utl_object_to_type(attr_list[0].value.objlist.list[i], SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx);
        printf("    [%d] next hop index:%d \n", i, nh_idx);
    }
    if (attr_list[0].value.objlist.count != 3) {
        printf("Invalid num of groups in list %d\n",attr_list[0].value.objlist.count);
        return SAI_STATUS_FAILURE;
    }
    
    printf("Remove nexthop group 0 \n");
    nhg_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, nhg_idx, &nhg_id);
    mrvl_sai_nhg_remove_test(nhg_id);
    
    mrvl_sai_nhg_pre_test_clean();
    
    return SAI_STATUS_SUCCESS;
}
   

int mrvl_sai_nhg_add_table_size(uint32_t num_entries)
{
    
    sai_object_id_t     nhg_id;
    sai_uint32_t        count, nh_idx, counter=0;
    sai_object_list_t   next_hop_group_list;
    sai_status_t        status;
    
    printf("Try to add %d next hop group entries\n",num_entries);

    mrvl_sai_nhg_pre_test();
    next_hop_group_list.list  = sai_nhg_add_list;
    nh_idx = 0;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &sai_nhg_add_list[counter]); 
    counter++;
    next_hop_group_list.count = counter;
    next_hop_group_list.list  = sai_nhg_add_list;
    count=0;
    while (num_entries> 0) {
        status = mrvl_sai_nhg_create_test(next_hop_group_list, &nhg_id);
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


