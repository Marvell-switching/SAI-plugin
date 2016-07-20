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

extern sai_router_interface_api_t* sai_rif_api;
/*******************************************************************************
* mrvl_sai_rif_add_test
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
int mrvl_sai_rif_add_test
(
    IN sai_router_interface_type_t interface_type,
    IN sai_uint32_t port_vlan,
    IN bool         use_default_mac,
    IN sai_mac_t    mac_addr,
    IN sai_packet_action_t  nbr_miss_act,
    IN sai_object_id_t  vr_id,
    OUT sai_object_id_t *rif_id
)
{
    uint32_t attr_count=0;
    sai_attribute_t attr_list[9];
    sai_status_t status;

    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr_list[attr_count].value.oid = vr_id;
    attr_count++;
    
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr_list[attr_count].value.s32 = interface_type;
    attr_count++;
    
    if (interface_type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
        attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID; 
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_vlan, &attr_list[attr_count].value.oid)) {
            return SAI_STATUS_FAILURE;
        }
    } else {
        attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID; 
        attr_list[attr_count].value.u16 = port_vlan;
    }
    attr_count++;

    if (use_default_mac == false) {
        attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS; 
        memcpy(&attr_list[attr_count].value.mac, mac_addr, sizeof(sai_mac_t));
        attr_count++;
    }

    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION;
    attr_list[attr_count].value.s32 = nbr_miss_act;
    attr_count++;


    MRVL_SAI_LOG_INF("Calling sai_rif_api->create_router_interface\n");
    status = sai_rif_api->create_router_interface(rif_id, attr_count, attr_list);
	return status;
}


/*******************************************************************************
* mrvl_sai_wrap_rif_add
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
int mrvl_sai_wrap_rif_add
(
    IN sai_uint32_t interface_type, /*SAI_ROUTER_INTERFACE_TYPE_PORT=0,SAI_ROUTER_INTERFACE_TYPE_VLAN=1*/
    IN sai_uint32_t port_vlan,
    IN sai_uint32_t mac0,
    IN sai_uint32_t mac1,
    IN sai_uint32_t mac2,
    IN sai_uint32_t mac3,
    IN sai_uint32_t mac4,
    IN sai_uint32_t mac5,
    IN bool         nbr_miss_act_trap
)
{
    sai_mac_t       mac_addr;
    sai_object_id_t rif_id;
    sai_status_t    status;
    sai_uint32_t    rif_idx;
    sai_packet_action_t nbr_miss_act;
    sai_object_id_t  vr_id;
    
	printf("mrvl_sai_wrap_rif_add: mac:%02x:%02x:%02x:%02x:%02x:%02x: interface type:%d port/vlan:%d \n",
		   mac0, mac1, mac2 ,mac3, mac4, mac5, interface_type, port_vlan);
    mac_addr[0] = mac0;
    mac_addr[1] = mac1;
    mac_addr[2] = mac2;
    mac_addr[3] = mac3;
    mac_addr[4] = mac4;
    mac_addr[5] = mac5;
    
    if (nbr_miss_act_trap == true) {
        nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    } else {
        nbr_miss_act = SAI_PACKET_ACTION_DROP;
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id)) {
        return SAI_STATUS_FAILURE;
    }

    status = mrvl_sai_rif_add_test(interface_type, port_vlan, false, mac_addr, nbr_miss_act, vr_id, &rif_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_fdb_add_test failed to create rif\n");
        return status;
    }else {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
            return status;
        }
        printf("mrvl_sai_wrap_rif_add: rif idx %d was created\n",rif_idx);
    }
    return 0;
}

/*******************************************************************************
* mrvl_sai_rif_remove_test
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
int mrvl_sai_rif_remove_test
(
    IN sai_object_id_t rif_id
)
{
    MRVL_SAI_LOG_INF("Calling sai_rif_api->remove_router_interface\n");
    return sai_rif_api->remove_router_interface(rif_id);
}

/*******************************************************************************
* mrvl_sai_wrap_rif_remove
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
int mrvl_sai_wrap_rif_remove
(
    IN sai_uint32_t rif_idx
)
{
    sai_object_id_t rif_id;
    sai_status_t    status;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    status = mrvl_sai_rif_remove_test(rif_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_rif_remove_test failed to delete rif idx %d\n", rif_idx);
    } else {
        printf("mrvl_sai_wrap_rif_remove: rif idx %d was deleted\n",rif_idx);
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_rif_get_attr_test
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
int mrvl_sai_rif_get_attr_test
(
    IN sai_object_id_t  rif_id,
    IN uint32_t         attr_count,
    IN sai_attribute_t  *attr_list
)
{
    
    MRVL_SAI_LOG_INF("Calling sai_rif_api->get_router_interface_attribute\n");
    return sai_rif_api->get_router_interface_attribute(rif_id,attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_rif_get_attr
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
int mrvl_sai_wrap_rif_get_attr
(
    IN sai_uint32_t rif_idx,
    IN sai_uint32_t attr_id
)
{
    uint32_t data;
    sai_attribute_t attr_list[1];
    sai_object_id_t rif_id;
    sai_status_t    status;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    
    attr_list[0].id = attr_id; 
        
    status = mrvl_sai_rif_get_attr_test(rif_id, 1, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        switch (attr_id) {
        case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &data))) {
                return status;
            }
            printf("rif get attributes for VIRTUAL_ROUTER_ID:%d \n",data);
            break;

        case SAI_ROUTER_INTERFACE_ATTR_TYPE:
            printf("rif get attributes for TYPE:%d \n",attr_list[0].value.s32);
            break;

        case SAI_ROUTER_INTERFACE_ATTR_PORT_ID:     
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_PORT, &data))) {
                return status;
            }
            printf("rif get attributes for PORT_ID:%d \n",data);
            break;

        case SAI_ROUTER_INTERFACE_ATTR_VLAN_ID:
            printf("rif get attributes for VLAN_ID:%d \n",attr_list[0].value.u16);
            break;

        case SAI_ROUTER_INTERFACE_ATTR_MTU:
            printf("rif get attributes for MTU:%d \n",attr_list[0].value.u32);
            break;

        case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
            printf("rif get attributes for SRC_MAC_ADDRESS:%02x:%02x:%02x:%02x:%02x:%02x\n",
                attr_list[0].value.mac[0],
                attr_list[0].value.mac[1],
                attr_list[0].value.mac[2],
                attr_list[0].value.mac[3],
                attr_list[0].value.mac[4],
                attr_list[0].value.mac[5]);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
            printf("rif get attributes for ADMIN_V4_STATE:%d \n",attr_list[0].value.booldata);
            break;
        case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
            printf("rif get attributes for MISS_PACKET_ACTION:%s \n",(attr_list[0].value.s32 == SAI_PACKET_ACTION_TRAP)?"TRAP":"DROP");
            break;
        default:
            printf("Attribute id %d not supported\n",attr_id);
            break;
        }
    }else{
        printf("mrvl_sai_wrap_rif_get_attr:a ttribute id %d failed %d \n",attr_id, status);
    }
    return status; 
}

/*******************************************************************************
* mrvl_sai_rif_set_attr_test
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
int mrvl_sai_rif_set_attr_test
(
    IN sai_object_id_t  rif_id,
    IN sai_attribute_t *attr_list    
)
{   
    MRVL_SAI_LOG_INF("Calling sai_rif_api->set_router_interface_attribute\n");
    return sai_rif_api->set_router_interface_attribute(rif_id, attr_list);

}

/*******************************************************************************
* mrvl_sai_wrap_rif_set_attr
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
int mrvl_sai_wrap_rif_set_attr
(
    IN sai_uint32_t rif_idx,
    IN sai_uint32_t attr_id,
    IN sai_uint32_t attr_value
)
{
    sai_attribute_t attr_list;
    sai_object_id_t rif_id;

    printf("mrvl_sai_wrap_rif_set_attr: rif id:%d attr_id:%d attr_value:%d\n",
           rif_idx, attr_id, attr_value);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    if ((attr_id != SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS) &&

        (attr_id !=  SAI_ROUTER_INTERFACE_ATTR_MTU)){
        printf("mrvl_sai_wrap_rif_set_attr: invalid attribute %d\n",attr_id);
        return SAI_STATUS_FAILURE;
    }
    attr_list.id = attr_id; 
    attr_list.value.u32 = attr_value; 
    return mrvl_sai_rif_set_attr_test(rif_id, &attr_list);
}

/*******************************************************************************
* mrvl_sai_rif_get_all_test_prv
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
static int mrvl_sai_rif_get_all_test_prv(
    IN sai_object_id_t rif_id
)
{
    uint32_t attr_count, rif_idx, i,intr_type, data;
    sai_attribute_t attr_list[10];
    sai_status_t status;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
        return status;
    }
    attr_count = 0;
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_MTU;
    attr_count++;
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS;
    attr_count++;
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION;
    attr_count++;
    
    status = mrvl_sai_rif_get_attr_test(rif_id, attr_count, attr_list);

    if (status == SAI_STATUS_SUCCESS) {

        for (i = 0; i < attr_count; i++){
            switch (attr_list[i].id) {
            case SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID:
                if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[i].value.oid, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &data))) {
                    return status;
                }
                printf("rif id %d get attributes for VIRTUAL_ROUTER_ID:%d \n",rif_idx, data);
                break;
            
            case SAI_ROUTER_INTERFACE_ATTR_TYPE:
                printf("rif id %d get attributes for TYPE:%d \n",rif_idx, attr_list[i].value.s32);
                intr_type = attr_list[i].value.s32;
                break;
            
            
            case SAI_ROUTER_INTERFACE_ATTR_MTU:
                printf("rif id %d get attributes for MTU:%d \n",rif_idx, attr_list[i].value.u32);
                break;
            
            case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:
                printf("rif id %d get attributes for SRC_MAC_ADDRESS:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    rif_idx, attr_list[i].value.mac[0],
                    attr_list[i].value.mac[1],
                    attr_list[i].value.mac[2],
                    attr_list[i].value.mac[3],
                    attr_list[i].value.mac[4],
                    attr_list[i].value.mac[5]);
                break;
            
            case SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE:
                printf("rif id %d get attributes for ADMIN_V4_STATE:%d \n",rif_idx, attr_list[i].value.booldata);
                break;
                
            case SAI_ROUTER_INTERFACE_ATTR_NEIGHBOR_MISS_PACKET_ACTION:
                printf("rif id %d get attributes for MISS_PACKET_ACTION:%s \n",rif_idx, (attr_list[i].value.s32 == SAI_PACKET_ACTION_TRAP)?"TRAP":"DROP");
                break;
            }
        }
        switch (intr_type) {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            attr_list[0].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID;
            status = mrvl_sai_rif_get_attr_test(rif_id, 1, attr_list);
            if (status!= SAI_STATUS_SUCCESS){
                printf("test failed %d\n", status);
                return status;    
            }
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_PORT, &data))) {
                return status;
            }
            printf("rif id %d get attributes for PORT_ID:%d \n",rif_idx, data);
            break;
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            attr_list[0].id = SAI_ROUTER_INTERFACE_ATTR_VLAN_ID;
            status = mrvl_sai_rif_get_attr_test(rif_id, 1, attr_list);
            if (status!= SAI_STATUS_SUCCESS){
                printf("test failed %d\n", status);
                return status;    
            }
            printf("rif id %d get attributes for VLAN_ID:%d \n",rif_idx, attr_list[0].value.u16);
            break;
        }
    }
    return status;
}
        

/*******************************************************************************
* mrvl_sai_rif_test
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
int mrvl_sai_rif_test(void)
{
    sai_mac_t mac_addr;
    uint32_t port_vlan, rif_idx;
    sai_status_t status;
    sai_router_interface_type_t interface_type;
    sai_object_id_t rif_id;
    bool default_mac;
    sai_packet_action_t  nbr_miss_act;
    sai_object_id_t      vr_id;

    printf("Created  virtual router 0 \n");
    mrvl_sai_virtual_router_add_test(&vr_id);

    /**** create rif on vlan 2 */ 
    printf("\nCreate rif on vlan 2 mac 00:00:00:11:11:11 nbr miss trap\n");
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
    status = mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &rif_id);
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    } else {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
            return status;
        }
        printf("rif created ok, rif idx %d \n",rif_idx);
    }
    
    /**** create rif on port 3 */ 
    printf("\nCreate rif on port 3 mac 00:00:00:11:11:22 nbr miss drop\n");
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
    status = mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &rif_id);
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    } else {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
            return status;
        }
        printf("rif created ok, rif idx %d \n",rif_idx);
    }
    mrvl_sai_rif_dump();    
    /**** create rif on port 3 default mac address*/ 
    printf("\nCreate rif on port 3 with default mac address nbr miss trap\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_PORT;
    port_vlan = 3;
    default_mac = true;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    status = mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &rif_id);
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    } else {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(rif_id, SAI_OBJECT_TYPE_ROUTER_INTERFACE, &rif_idx))) {
            return status;
        }
        printf("rif created ok, rif idx %d \n",rif_idx);
    }
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E);
    mrvl_sai_rif_dump();
    /*** get rif id 0 attributes */
    printf("\nGet attributes for rif id 0\n");
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }    
    status = mrvl_sai_rif_get_all_test_prv(rif_id) ;   
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    }

    /*** get rif id 1 attributes */
    printf("\nGet attributes for rif id 1\n");
    rif_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }
    status = mrvl_sai_rif_get_all_test_prv(rif_id);    
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    }
               
    /**** delete rif 0 */ 
    printf("\nDelete rif 0 \n");
    rif_idx = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }    
    status = mrvl_sai_rif_remove_test(rif_id);
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    }
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E);
    mrvl_sai_rif_dump();
    
    /**** delete rif 1 */ 
    printf("\nDelete rif 1 \n");
    rif_idx = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }    
    status = mrvl_sai_rif_remove_test(rif_id);
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    }
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E);
    mrvl_sai_rif_dump();
    
        /**** delete rif 1 */ 
    printf("\nDelete rif 2 \n");
    rif_idx = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }    
    status = mrvl_sai_rif_remove_test(rif_id);
    if (status!= SAI_STATUS_SUCCESS){
        printf("test failed %d\n", status);
        return status;    
    }
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_TERMINATION_E);
    mrvl_sai_rif_dump();

    printf("\nTry to Delete rif 2 again\n");
    rif_idx = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, rif_idx, &rif_id)) {
        return SAI_STATUS_FAILURE;
    }    
    status = mrvl_sai_rif_remove_test(rif_id);
    if (status!= SAI_STATUS_ITEM_NOT_FOUND){
        printf("test failed %d\n", status);
        return status;    
    }
    return SAI_STATUS_SUCCESS;
}
    
 
    
   
int mrvl_sai_rif_add_table_size(uint32_t   num_entries)
{
    
    sai_mac_t           mac_addr;
    uint32_t            port_vlan;
    sai_status_t        status;
    sai_router_interface_type_t interface_type;
    sai_object_id_t     rif_id;
    bool                default_mac;
    sai_packet_action_t nbr_miss_act;
    sai_object_id_t     vr_id;
    uint32_t            count;

    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    default_mac = true;
    nbr_miss_act = SAI_PACKET_ACTION_TRAP;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, SAI_DEFAULT_VRID_CNS, &vr_id)) {
        return SAI_STATUS_FAILURE;
    }
    
    printf("Try to add %d \n",num_entries);
    count = 0;
    while (num_entries > count) {
        port_vlan = 1+ count;
        status = mrvl_sai_rif_add_test(interface_type, port_vlan, default_mac, mac_addr, nbr_miss_act, vr_id, &rif_id);
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
    
