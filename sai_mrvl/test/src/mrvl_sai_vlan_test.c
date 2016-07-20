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
******************************************************************************* 
* mrvl_sai_vlan_test.c
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

extern sai_vlan_api_t* sai_vlan_api;
extern sai_switch_api_t* sai_switch_api;
extern sai_vlan_port_t sai_vlan_port_list[SAI_MAX_NUM_OF_PORTS];

/*******************************************************************************
* mrvl_sai_vlan_create_test
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
int mrvl_sai_vlan_create_test
(
    IN sai_vlan_id_t vlan_id
)
{
    sai_status_t status;

    MRVL_SAI_LOG_INF("sai_vlan_api->create_vlan\n");
    status = sai_vlan_api->create_vlan(vlan_id);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_create
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
int mrvl_sai_wrap_vlan_create
(
    IN sai_uint32_t vlan
)
{
	printf("mrvl_sai_wrap_vlan_create: vlanId:%d \n", vlan);    
    return mrvl_sai_vlan_create_test(vlan);
}

/*******************************************************************************
* mrvl_sai_vlan_remove_test
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
int mrvl_sai_vlan_remove_test
(
    IN sai_vlan_id_t vlan_id
)
{
    sai_status_t status;

    MRVL_SAI_LOG_INF("sai_vlan_api->remove_vlan\n");
    status = sai_vlan_api->remove_vlan(vlan_id);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_remove
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
int mrvl_sai_wrap_vlan_remove
(
    IN sai_uint32_t vlan
)
{
	printf("mrvl_sai_wrap_vlan_remove: vlanId:%d \n", vlan);    
    return mrvl_sai_vlan_remove_test(vlan);
}
/*******************************************************************************
* mrvl_sai_vlan_remove_all_test
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
int mrvl_sai_vlan_remove_all_test
(
    void
)
{
    sai_status_t status;

    MRVL_SAI_LOG_INF("sai_vlan_api->remove_all_vlans\n");
    status = sai_vlan_api->remove_all_vlans();
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_remove_all
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
int mrvl_sai_wrap_vlan_remove_all
(
    void
)
{
	printf("mrvl_sai_wrap_vlan_remove_all: \n");    
    return mrvl_sai_vlan_remove_all_test();
}


/*******************************************************************************
* mrvl_sai_vlan_add_ports_test
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
int mrvl_sai_vlan_add_ports_test
(
    IN sai_vlan_id_t vlan_id,
    IN uint32_t port_count,
    IN sai_vlan_port_t *port_list
)
{
    sai_status_t status;

    MRVL_SAI_LOG_INF("sai_vlan_api->add_ports_to_vlan\n");
    status = sai_vlan_api->add_ports_to_vlan(vlan_id, port_count, port_list);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_add_ports
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
int mrvl_sai_wrap_vlan_add_ports
(
    IN sai_uint32_t vlan,
    IN sai_uint32_t num_of_ports,
    IN sai_uint32_t port0,
    IN sai_uint32_t tag0, /* 0=untag, 1= tag */
    IN sai_uint32_t port1,
    IN sai_uint32_t tag1,
    IN sai_uint32_t port2,
    IN sai_uint32_t tag2,
    IN sai_uint32_t port3,
    IN sai_uint32_t tag3
)
{
    sai_vlan_port_t port_list[4];
    if (num_of_ports > 4){
        printf("This wrap function support up to 4 ports\n");
        num_of_ports = 4;
    }
    switch (num_of_ports) {
    case 4:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port3, &port_list[3].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        port_list[3].tagging_mode = tag3;
        
    case 3:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port2, &port_list[2].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        port_list[2].tagging_mode = tag2;
        
    case 2:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port1, &port_list[1].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        port_list[1].tagging_mode = tag1;

    case 1:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port0, &port_list[0].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        port_list[0].tagging_mode = tag0;
        
    default:
        break;
    }
    printf("mrvl_sai_wrap_vlan_add_ports: vlanId:%d num_of_ports:%d\n", vlan, num_of_ports);    
    return mrvl_sai_vlan_add_ports_test(vlan, num_of_ports, port_list);
}

/*******************************************************************************
* mrvl_sai_vlan_remove_ports_test
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
int mrvl_sai_vlan_remove_ports_test
(
    IN sai_vlan_id_t vlan_id,
    IN uint32_t port_count,
    IN sai_vlan_port_t *port_list
)
{
    sai_status_t status;

    MRVL_SAI_LOG_INF("sai_vlan_api->remove_ports_from_vlan\n");
    status = sai_vlan_api->remove_ports_from_vlan(vlan_id, port_count, port_list);
	return status;
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_remove_ports
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
int mrvl_sai_wrap_vlan_remove_ports
(
    IN sai_uint32_t vlan,
    IN sai_uint32_t num_of_ports,
    IN sai_uint32_t port0,
    IN sai_uint32_t port1,
    IN sai_uint32_t port2,
    IN sai_uint32_t port3
)
{
    sai_vlan_port_t port_list[4];
    if (num_of_ports > 4){
        printf("This wrap function support up to 4 ports\n");
        num_of_ports = 4;
    }
    switch (num_of_ports) {
    case 4:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port3, &port_list[3].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        
    case 3:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port2, &port_list[2].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        
    case 2:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port1, &port_list[1].port_id)) {
            return SAI_STATUS_FAILURE;
        }

    case 1:
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port0, &port_list[0].port_id)) {
            return SAI_STATUS_FAILURE;
        }
        
    default:
        break;
    }
    printf("mrvl_sai_wrap_vlan_remove_ports: vlanId:%d num_of_ports:%d\n", vlan, num_of_ports);    
    return mrvl_sai_vlan_remove_ports_test(vlan, num_of_ports, port_list);
}


/*******************************************************************************
* mrvl_sai_vlan_get_attr_test
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
int mrvl_sai_vlan_get_attr_test
(
    IN sai_vlan_id_t    vlan_id,
    IN uint32_t         attr_count,
    IN sai_attribute_t  *attr_list
)
{    
    MRVL_SAI_LOG_INF("sai_vlan_api->get_vlan_attribute\n");
    return sai_vlan_api->get_vlan_attribute(vlan_id, attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_get_attr
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
int mrvl_sai_wrap_vlan_get_attr
(
    IN sai_uint32_t vlan
)
{
    uint32_t port_id, stp_id, i, attr_count = 0;
    sai_attribute_t attr_list[4];
    sai_status_t status;
    sai_vlan_port_list_t *vlan_port_list;
        
    attr_list[attr_count].id = SAI_VLAN_ATTR_PORT_LIST; 
    attr_list[attr_count].value.vlanportlist.count = SAI_MAX_NUM_OF_PORTS;
    attr_list[attr_count].value.vlanportlist.list  = sai_vlan_port_list;
    attr_count++;

    attr_list[attr_count].id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES; 
    attr_count++;

    attr_list[attr_count].id = SAI_VLAN_ATTR_STP_INSTANCE; 
    attr_count++;

    attr_list[attr_count].id = SAI_VLAN_ATTR_LEARN_DISABLE; 
    attr_count++;
    

    printf("mrvl_sai_wrap_vlan_get_attr: vlanId:%d \n", vlan);    
    status =  mrvl_sai_vlan_get_attr_test(vlan, attr_count, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[2].value.oid, SAI_OBJECT_TYPE_STP_INSTANCE, &stp_id))) {
            return status;
        }
         
        printf(" get attributes for vlan:%d ==> max learn:%d stp instance:%d disable:%d port list:\n",
               vlan,
               attr_list[1].value.u32,
               stp_id,
               attr_list[3].value.booldata);
        vlan_port_list = &attr_list[0].value.vlanportlist;
        for (i=0; i < vlan_port_list->count; i++) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_port_list->list[i].port_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
                return status;
            }
            printf("[%d] port:%d tag mode:%s\n", i, port_id, (vlan_port_list->list[i].tagging_mode==SAI_VLAN_PORT_UNTAGGED)? "UNTAGGED":"TAGGED");
        }
    }
    return status; 
}


/*******************************************************************************
* mrvl_sai_vlan_set_attr_test
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
int mrvl_sai_vlan_set_attr_test
(
    IN sai_vlan_id_t    vlan_id,
    IN sai_attribute_t  *attr_list
)
{    
    MRVL_SAI_LOG_INF("sai_vlan_api->set_vlan_attribute\n");
    return sai_vlan_api->set_vlan_attribute(vlan_id, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_set_attr
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
int mrvl_sai_wrap_vlan_set_attr
(
    IN sai_uint32_t vlan,
    IN sai_uint32_t attr_id, /* 1=MAX_LEARNED_ADDRESSES, 2=STP_INSTANCE, 3=LEARN_DISABLE */
    IN sai_uint32_t attr_value
)
{
    sai_attribute_t attr_list;
    sai_status_t status;

    printf("mrvl_sai_wrap_vlan_set_attr: vlanId:%d \n", vlan);    
    attr_list.id = attr_id; 
    switch (attr_id) {
    case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
        attr_list.value.u32 = attr_value;
        break;
    case SAI_VLAN_ATTR_STP_INSTANCE:
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP_INSTANCE, attr_value, &attr_list.value.oid))) {
            return status;
        }
        break;
    case SAI_VLAN_ATTR_LEARN_DISABLE:
        attr_list.value.booldata = attr_value;
        break;
    default:
        printf("Invalid param %d\n", attr_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }
    status =  mrvl_sai_vlan_set_attr_test(vlan, &attr_list);
    return status; 
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

int mrvl_sai_vlan_test(void)
{
    sai_vlan_id_t vlan;
    uint32_t port_count, port_id;
    sai_vlan_port_t port_list[4];
    sai_status_t status;
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[4];

    printf("\nCreate vlan 1-3\n");
    vlan = 1;
    status = sai_vlan_api->create_vlan(vlan);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;    
    vlan = 2;
    status = sai_vlan_api->create_vlan(vlan);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;    
    vlan = 3;
    status = sai_vlan_api->create_vlan(vlan);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    printf("\nAdd ports 0-2 to vlan 1 as tagged\n");
    vlan = 1;
    port_count = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 0, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;

    status = sai_vlan_api->add_ports_to_vlan(vlan, port_count, port_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    
    printf("\nAdd ports 1-3 to vlan 2 as untagged\n");
    vlan = 2;
    port_count = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 3, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    port_count++;

    status = sai_vlan_api->add_ports_to_vlan(vlan, port_count, port_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    
    printf("\nAdd ports 1 and 2 as untagged and port 3 as tagged to vlan 3\n");
    vlan = 3;
    port_count = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_UNTAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 3, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;

    status = sai_vlan_api->add_ports_to_vlan(vlan, port_count, port_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 

    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    
    printf("\nRemove port 2 from vlan 3\n");
    vlan = 3;
    port_count = 0;
    memset(port_list, 0, sizeof(sai_vlan_port_t)*4);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[port_count].port_id)) {
        return SAI_STATUS_FAILURE;
    }
    port_count++;
    status = sai_vlan_api->remove_ports_from_vlan(vlan, port_count, port_list);
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    
    printf("\nGet vlan 3 port list: \n");
    attr_count = 0;
    vlan = 3;
    attr_list[attr_count].id = SAI_VLAN_ATTR_PORT_LIST; 
    attr_list[attr_count].value.vlanportlist.count = 4;
    attr_list[attr_count].value.vlanportlist.list  = port_list;
    attr_count++;
    status = sai_vlan_api->get_vlan_attribute(vlan, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    for (port_count=0; port_count < attr_list[0].value.vlanportlist.count; port_count++) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_list[port_count].port_id, SAI_OBJECT_TYPE_PORT, &port_id))) {
            return status;
        }
        printf("[%d] port:%d tag mode:%s\n", port_count, port_id, (port_list[port_count].tagging_mode==SAI_VLAN_PORT_UNTAGGED)? "UNTAGGED":"TAGGED");
    }
    printf("\nRemove all ports from vlan 1\n");
    vlan = 1;
    port_count = 0;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 0, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_list[port_count].port_id)) 
        return SAI_STATUS_FAILURE;
    port_list[port_count].tagging_mode = SAI_VLAN_PORT_TAGGED;
    port_count++;
    status = sai_vlan_api->remove_ports_from_vlan(vlan, port_count, port_list);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    
    
    printf("\nRemove  vlan 1\n");
    vlan = 1;
    status = sai_vlan_api->remove_vlan(vlan);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);

    return SAI_STATUS_SUCCESS;
}
