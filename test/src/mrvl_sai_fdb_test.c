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
#include    "sai.h"
#include    "mrvl_sai.h"
#include	"../test/inc/mrvl_sai_test.h"

extern sai_fdb_api_t* sai_fdb_api;
static uint32_t sai_fdb_vlan_member_list[10];
static uint32_t sai_fdb_vlan_member_counter = 0;
static sai_object_id_t switch_id = 0;

/*******************************************************************************
* mrvl_sai_fdb_add_test
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
int mrvl_sai_fdb_add_test
(
    IN sai_mac_t mac_addr,
    IN sai_vlan_id_t vlan_id,
    IN sai_uint32_t type,
    IN sai_object_id_t port_id,
    IN sai_uint32_t pkt_action
)
{
    sai_fdb_entry_t fdb_entry;
    uint32_t attr_count;
    sai_attribute_t attr_list[3];
    sai_status_t status;

    attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr_list[0].value.s32 = type;
    attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    attr_list[1].value.oid = port_id;
    attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr_list[2].value.s32 = pkt_action;

    memcpy(fdb_entry.mac_address, mac_addr, sizeof(sai_mac_t));
    fdb_entry.vlan_id = vlan_id;
    attr_count = 3;
    
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->create_fdb_entry\n");
    status = sai_fdb_api->create_fdb_entry(&fdb_entry, attr_count, attr_list);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_fdb_add
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
int mrvl_sai_wrap_fdb_add
(
    IN sai_uint32_t mac0,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5,
    IN sai_uint32_t vlan,
    IN sai_uint32_t type, /*SAI_FDB_ENTRY_DYNAMIC=0,SAI_FDB_ENTRY_STATIC=1*/
    IN sai_uint32_t port,
    IN sai_uint32_t action /*SAI_PACKET_ACTION_DROP=0 ,SAI_PACKET_ACTION_FORWARD=1, SAI_PACKET_ACTION_TRAP=4, SAI_PACKET_ACTION_LOG=5*/  
)
{
    sai_mac_t mac_addr;
    sai_object_id_t port_id;
    
	printf("mrvl_sai_wrap_fdb_add: mac:%02x:%02x:%02x:%02x:%02x:%02x: vlanId:%d type:%d port:%d action:%d\n",
		   mac0, mac1, mac2 ,mac3, mac4, mac5, vlan, type, port, action);
    mac_addr[0] = mac0;
    mac_addr[1] = mac1;
    mac_addr[2] = mac2;
    mac_addr[3] = mac3;
    mac_addr[4] = mac4;
    mac_addr[5] = mac5;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    return mrvl_sai_fdb_add_test(mac_addr, vlan, type, port_id, action);
}

/*******************************************************************************
* mrvl_sai_fdb_remove_test
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
int mrvl_sai_fdb_remove_test
(
    IN sai_mac_t mac_addr,
    IN sai_vlan_id_t vlan_id
)
{
    sai_fdb_entry_t fdb_entry;
    sai_status_t status;

    memcpy(fdb_entry.mac_address, mac_addr, sizeof(sai_mac_t));
    fdb_entry.vlan_id = vlan_id;

    MRVL_SAI_LOG_INF("Calling sai_fdb_api->remove_fdb_entry\n");
    status = sai_fdb_api->remove_fdb_entry(&fdb_entry);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_fdb_remove
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
int mrvl_sai_wrap_fdb_remove
(
    IN sai_uint32_t mac0,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5,
    IN sai_uint32_t vlan
)
{
    sai_mac_t mac_addr;
    
	printf("mrvl_sai_wrap_fdb_remove: mac:%02x:%02x:%02x:%02x:%02x:%02x: vlanId:%d \n",
		   mac0, mac1, mac2 ,mac3, mac4, mac5, vlan);
    mac_addr[0] = mac0;
    mac_addr[1] = mac1;
    mac_addr[2] = mac2;
    mac_addr[3] = mac3;
    mac_addr[4] = mac4;
    mac_addr[5] = mac5;
    
    return mrvl_sai_fdb_remove_test(mac_addr, vlan);
}

/*******************************************************************************
* mrvl_sai_fdb_flush_test
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
int mrvl_sai_fdb_flush_test
(
    IN sai_uint32_t vlan_id,
    IN sai_uint32_t type,
    IN sai_object_id_t port_id
)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[3];
    sai_status_t status;
    if (vlan_id != -1) {
        attr_list[attr_count].id = SAI_FDB_FLUSH_ATTR_VLAN_ID; 
        attr_list[attr_count].value.u16 = vlan_id;
        attr_count++;
    }
    if (type != -1) {
        attr_list[attr_count].id = SAI_FDB_FLUSH_ATTR_ENTRY_TYPE; 
        attr_list[attr_count].value.s32 = type;
        attr_count++;
    }
    if (port_id != -1) {
        attr_list[attr_count].id = SAI_FDB_FLUSH_ATTR_BRIDGE_PORT_ID; 
        attr_list[attr_count].value.oid = port_id;
        attr_count++;
    }

    MRVL_SAI_LOG_INF("Calling sai_fdb_api->flush_fdb_entries\n");
    status = sai_fdb_api->flush_fdb_entries(switch_id, attr_count, attr_list);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_fdb_flush
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
int mrvl_sai_wrap_fdb_flush
(
    IN sai_uint32_t vlan,
    IN sai_uint32_t type,
    IN sai_uint32_t port
)
{
    sai_object_id_t port_id;
    
	printf("mrvl_sai_wrap_fdb_flush: vlanId:%d type:%d port:%d\n",
		   vlan, type, port);
    if (port != -1) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
            return SAI_STATUS_FAILURE;
        }
    }else
        memset(&port_id, 0xFF, sizeof(sai_object_id_t));
    
    return mrvl_sai_fdb_flush_test(vlan, type, port_id);
}

/*******************************************************************************
* mrvl_sai_fdb_get_attr_test
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
int mrvl_sai_fdb_get_attr_test
(
    IN sai_fdb_entry_t *fdb_entry,
    IN uint32_t         attr_count,
    IN sai_attribute_t  *attr_list
)
{
    
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->get_fdb_entry_attribute\n");
    return sai_fdb_api->get_fdb_entry_attribute(fdb_entry,attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_fdb_get_attr
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
int mrvl_sai_wrap_fdb_get_attr
(
    IN sai_uint32_t mac0,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5,
    IN sai_uint32_t vlan
)
{
    sai_mac_t mac_addr;
    sai_fdb_entry_t fdb_entry;
    sai_status_t status;
    uint32_t attr_count = 0, port_id;
    sai_attribute_t attr_list[3];

    printf("mrvl_sai_wrap_fdb_get_attr: mac:%02x:%02x:%02x:%02x:%02x:%02x: vlanId:%d \n",
           mac0, mac1, mac2 ,mac3, mac4, mac5, vlan);
    mac_addr[0] = mac0;
    mac_addr[1] = mac1;
    mac_addr[2] = mac2;
    mac_addr[3] = mac3;
    mac_addr[4] = mac4;
    mac_addr[5] = mac5;
    

    memcpy(fdb_entry.mac_address, mac_addr, sizeof(sai_mac_t));
    fdb_entry.vlan_id = vlan;
    
    attr_list[attr_count].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID; 
    attr_count++;
    
    attr_list[attr_count].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION; 
    attr_count++;
    
    attr_list[attr_count].id = SAI_FDB_ENTRY_ATTR_TYPE; 
    attr_count++;
        
    status = mrvl_sai_fdb_get_attr_test(&fdb_entry, attr_count, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_PORT, &port_id))) {
            return status;
        }

        printf(" get attributes for mac:%02x:%02x:%02x:%02x:%02x:%02x vlan:%d ==> port:%d action:%d \n",
               mac_addr[0],
               mac_addr[1],
               mac_addr[2],
               mac_addr[3],
               mac_addr[4],
               mac_addr[5],
               vlan,
               port_id,
               attr_list[1].value.u32);
    }
    return status; 
}

/*******************************************************************************
* mrvl_sai_fdb_set_attr_test
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
int mrvl_sai_fdb_set_attr_test
(
    IN sai_fdb_entry_t *fdb_entry,
    IN sai_attribute_t *attr_list    
)
{   
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->set_fdb_entry_attribute\n");
    return sai_fdb_api->set_fdb_entry_attribute(fdb_entry, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_fdb_set_attr
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
int mrvl_sai_wrap_fdb_set_attr
(
    IN sai_uint32_t mac0,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5,
    IN sai_uint32_t vlan,
    IN sai_uint32_t attr_id, /* 0=type, 1=port, 2=action */
    IN sai_uint32_t attr_value
)
{
    sai_mac_t mac_addr;
    sai_attribute_value_t at_value ;    
    sai_fdb_entry_t fdb_entry;
    sai_attribute_t attr_list;
    
    printf("mrvl_sai_wrap_fdb_set_attr: mac:%02x:%02x:%02x:%02x:%02x:%02x: vlanId:%d attr_id:%d attr_value:%d\n",
           mac0, mac1, mac2 ,mac3, mac4, mac5, vlan, attr_id, attr_value);
    mac_addr[0] = mac0;
    mac_addr[1] = mac1;
    mac_addr[2] = mac2;
    mac_addr[3] = mac3;
    mac_addr[4] = mac4;
    mac_addr[5] = mac5;
    memset(&at_value, 0, sizeof(sai_attribute_value_t));
    if (attr_id == SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, attr_value, &at_value.oid)) {
            return SAI_STATUS_FAILURE;
        }
    } else {
        at_value.u32 = attr_value;
    }

    memcpy(fdb_entry.mac_address, mac_addr, sizeof(sai_mac_t));
    fdb_entry.vlan_id = vlan;

    attr_list.id = attr_id; 
    attr_list.value = at_value; 
    return mrvl_sai_fdb_set_attr_test(&fdb_entry, &attr_list);
}

int mrvl_sai_fdb_address_update_vlan_add_pre_test(void)
{
    sai_vlan_id_t vlan;
    sai_uint32_t num_of_ports = 0;
    sai_port_info_t ports_list[5];
    uint32_t attr_count;
    sai_attribute_t attr_list[3];
    sai_object_id_t vlan_oid;


    printf("\nCreate vlan 1\n");
    vlan = 1;
    attr_count = 1;
    attr_list[0].id = SAI_VLAN_ATTR_VLAN_ID;
    attr_list[0].value.u16 = 1;

    mrvl_sai_vlan_create_test(&vlan_oid, vlan, switch_id, attr_count, attr_list);
    printf("\nAdd ports 0-3 to vlan 1 as tagged\n");
    vlan = 1;
    ports_list[num_of_ports].port = 0;
    ports_list[num_of_ports++].tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    ports_list[num_of_ports].port = 1;
    ports_list[num_of_ports++].tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    ports_list[num_of_ports].port = 2;
    ports_list[num_of_ports++].tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    ports_list[num_of_ports].port = 3;
    ports_list[num_of_ports++].tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    ports_list[num_of_ports].port = 4;
    ports_list[num_of_ports++].tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    mrvl_sai_wrap_vlan_add_ports_list(vlan_oid, vlan, num_of_ports, ports_list, &sai_fdb_vlan_member_list[sai_fdb_vlan_member_counter]);
    sai_fdb_vlan_member_counter +=num_of_ports;
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_fdb_address_update_vlan_rem_post_test(void)
{
    sai_vlan_id_t vlan;
    uint32_t i;
    sai_object_id_t  vlan_member_id;
    printf("\nRemove all ports from vlan 1\n");
    vlan = 1;
    for (i = 0; i < sai_fdb_vlan_member_counter; i++) {
        
        mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, sai_fdb_vlan_member_list[i], &vlan_member_id);
        mrvl_sai_vlan_remove_member_test(vlan_member_id);
    }
    sai_fdb_vlan_member_counter = 0;
    mrvl_sai_vlan_remove_test(vlan);
    
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_fdb_test
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
int mrvl_sai_fdb_test(void)
{
    sai_fdb_entry_t fdb_entry;
    uint32_t attr_count, vlan, port;
    sai_attribute_t attr_list[3];
    sai_status_t status;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))) {
        return status;
    }

    mrvl_sai_fdb_address_update_vlan_add_pre_test();

    /**** add mac 00:00:00:11:22:33 vlan 1 port 3 forward*/ 
    printf("\nAdd mac address 00:00:00:11:22:33 vlan 1 forward to port 3\n");
    vlan = 1;
    port = 3;
    attr_count = 3;
    attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_FDB_ENTRY_TYPE_STATIC;
    attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &attr_list[1].value.oid)) 
        return SAI_STATUS_FAILURE;
    attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr_list[2].value.s32 = SAI_PACKET_ACTION_FORWARD;
    fdb_entry.vlan_id = vlan;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x33;
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->create_fdb_entry\n");
    status = sai_fdb_api->create_fdb_entry(&fdb_entry, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;    
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);
    
    /**** add mac 00:00:00:11:22:44 vlan 1 port 4 mirror*/ 
    printf("\nAdd mac address 00:00:00:11:22:44 vlan 1 mirror to port 4\n");
    vlan = 1;
    port = 4;
    attr_count = 3;
    attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_FDB_ENTRY_TYPE_STATIC;
    attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &attr_list[1].value.oid)) 
        return SAI_STATUS_FAILURE;
    attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr_list[2].value.s32 = SAI_PACKET_ACTION_LOG;
    fdb_entry.vlan_id = vlan;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x44;
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->create_fdb_entry\n");
    status = sai_fdb_api->create_fdb_entry(&fdb_entry, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;  
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);
    
    /**** add mac 00:00:00:11:22:55 vlan 1 port 4 drop*/ 
    printf("\nAdd mac address 00:00:00:11:22:55 vlan 1 drop\n");
    vlan = 1;
    port = 4;
    attr_count = 3;
    attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_FDB_ENTRY_TYPE_STATIC;
    attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &attr_list[1].value.oid)) 
        return SAI_STATUS_FAILURE;
    attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr_list[2].value.s32 = SAI_PACKET_ACTION_DROP;
    fdb_entry.vlan_id = vlan;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x55;
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->create_fdb_entry\n");
    status = sai_fdb_api->create_fdb_entry(&fdb_entry, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;   
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);
    
    /*** get mac 00:00:00:11:22:44 vlan 1 all attributes */
    printf("\nGet attributes for mac address 00:00:00:11:22:44 vlan 1\n");
    vlan =1;
    fdb_entry.vlan_id = vlan;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x44;
    attr_count = 0;
    attr_list[attr_count].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID; 
    attr_count++;  
    attr_list[attr_count].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION; 
    attr_count++;    
    attr_list[attr_count].id = SAI_FDB_ENTRY_ATTR_TYPE; 
    attr_count++;        
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->get_fdb_entry_attribute\n");
    status = sai_fdb_api->get_fdb_entry_attribute(&fdb_entry,attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_PORT, &port))) {
        return status;
    }
    printf("Attributes ==> port:%d action:%d \n", port, attr_list[1].value.u32);

    /**** change mac 00:00:00:11:22:33 vlan 1 port 3 to drop*/ 
    printf("\nChange mac address 00:00:00:11:22:33 vlan 1  to drop\n");
    vlan =1;
    fdb_entry.vlan_id = vlan;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x33;
    attr_list[0].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION; 
    attr_list[0].value.s32 = SAI_PACKET_ACTION_DROP; 
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->set_fdb_entry_attribute\n");
    status = sai_fdb_api->set_fdb_entry_attribute(&fdb_entry, &attr_list[0]);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);
    
    /**** delete mac 00:00:00:11:22:33 vlan 1 */ 
    printf("\nDelete mac address 00:00:00:11:22:33 vlan 1 \n");
    vlan =1;
    fdb_entry.vlan_id = vlan;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x33;
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->remove_fdb_entry\n");
    status = sai_fdb_api->remove_fdb_entry(&fdb_entry);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);
    
    /**** flush all other macs on vlan 1*/
    printf("\nFlush mac address on vlan 1 \n");
    vlan =1;
    attr_count = 0;
    attr_list[attr_count].id = SAI_FDB_FLUSH_ATTR_VLAN_ID; 
    attr_list[attr_count].value.u16 = vlan;
    attr_count++;
    MRVL_SAI_LOG_INF("Calling sai_fdb_api->flush_fdb_entries\n");
    status = sai_fdb_api->flush_fdb_entries(switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;
    
    mrvl_sai_fdb_address_update_vlan_rem_post_test();
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);

    return SAI_STATUS_SUCCESS;
}

