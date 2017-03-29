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
* mrvl_sai_lag_test.c
*
* DESCRIPTION:
*       
*
* FILE REVISION NUMBER:
*       $Revision: 01 $
*
*******************************************************************************/

#include    <unistd.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>
#include    <fcntl.h>
#include    <errno.h>

#include    "fpaLibApis.h"
#include    "sai.h"
#include    "mrvl_sai.h"
#include    "../test/inc/mrvl_sai_test.h"

extern sai_lag_api_t* sai_lag_api;
extern sai_vlan_api_t* sai_vlan_api;

#define MRVL_SAI_LAG_LIST_SIZE 10
sai_object_id_t sai_lag_ports_list[MRVL_SAI_LAG_LIST_SIZE];
sai_object_id_t sai_dummy_list[MRVL_SAI_LAG_LIST_SIZE];

/*******************************************************************************
* NAME:
*       mrvl_sai_lag_pre_test 
*
* DESCRIPTION:
*       system init configurations:
*       1. create vlan 1
*       2. add ports 0,1,2,3 to vlan 1 as tagged       
* 
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       SAI_STATUS_SUCCESS  - on success
*       SAI_STATUS_FAILURE  - on failure
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_lag_pre_test
(
    void
)
{            
    sai_status_t            status;    
    sai_vlan_tagging_mode_t tag;
    sai_object_id_t         vlan_member_oid, port_oid;
    uint32_t                port, vlan, i;    
        
    /* create vlan 1 */
    status = sai_vlan_api->create_vlan(1);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
        
    /* add port i to vlan 1 tagged */ 
    for (i=0; i<4; i++) {
        vlan = 1;
        port = i;
        tag = SAI_VLAN_PORT_TAGGED;
        mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_oid);
        mrvl_sai_vlan_create_member_test(vlan, port_oid, tag, &vlan_member_oid);
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_l2_int_group_wo_vlan(port+1))
            return SAI_STATUS_FAILURE;
    }    

    return SAI_STATUS_SUCCESS; 

}

/*******************************************************************************
* NAME:
*       mrvl_sai_lag_post_test
*
* DESCRIPTION:
*       system restore configurations:
*       1. remove ports 0,1,2,3 from vlan 1
*       2. delete vlan 1
* 
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       SAI_STATUS_SUCCESS  - on success
*       SAI_STATUS_FAILURE  - on failure
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_lag_post_test 
(
    void
)
{            
    
    sai_status_t    status;    
    uint32_t        vlan, port, vlan_member, i;
    sai_object_id_t vlan_member_id;
    /* remove ports from vlan 1 */
    for (i=0; i<4; i++) {
        vlan = 1;
        port = i;
        vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
            return SAI_STATUS_FAILURE;   
        if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_delete_l2_int_group(port+1,0))
            return SAI_STATUS_FAILURE;
    }    

    /* remove vlan 1 */
    status = sai_vlan_api->remove_vlan(1);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: remove_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_lag_group_test
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
int mrvl_sai_lag_group_test(void)
{       
    sai_object_id_t     lag_oid, port_oid[4], lag_member_oid[4];
    uint32_t            lag_idx, port_idx[4], lag_member_idx[4];
    sai_attribute_t     attr_list[4];
    uint32_t            attr_count, i;   
    sai_status_t        status;     

    /* pre test settings */
    status = mrvl_sai_lag_pre_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: lag pre-test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }                  

    /* create empty LAG */    
    memset(attr_list, 0, sizeof(sai_attribute_t)*4);
    printf("\nCreate empty LAG\n");   
    attr_count = 0;     
    status = sai_lag_api->create_lag(&lag_oid, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Create empty LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    sai_lag_ports_list[0] = 1;
    sai_lag_ports_list[1] = 2;
    sai_lag_ports_list[2] = 3;
    sai_lag_ports_list[3] = 4;

    mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_idx);    

    /* add ports to LAG */
    for (i=0; i<4; i++) {
        /* create port_oid for port i */
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, sai_lag_ports_list[i], &(port_oid[i]))) 
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_oid[i], SAI_OBJECT_TYPE_PORT, &(port_idx[i])))
            return SAI_STATUS_FAILURE;

        printf("\nCreate LAG member lag_idx %d port_idx %d\n",lag_idx,port_idx[i]);   
        attr_count = 0; 
        attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
        attr_list[attr_count].value.oid = lag_oid;    
        attr_count++;
        attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
        attr_list[attr_count].value.oid = port_oid[i];    
        attr_count++;
        attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE;
        attr_list[attr_count].value.booldata = false;    
        attr_count++;
        attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;
        attr_list[attr_count].value.booldata = false;    
        attr_count++;
        status = sai_lag_api->create_lag_member(&(lag_member_oid[i]), attr_count, attr_list);
        if (status!= SAI_STATUS_SUCCESS) {
            printf("%s:%d: Create LAG member returned with status %d\n",
                   __func__, __LINE__, status);
            return SAI_STATUS_FAILURE;
        }
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_member_oid[i], SAI_OBJECT_TYPE_LAG_MEMBER, &(lag_member_idx[i])))
            return SAI_STATUS_FAILURE;
    }
    
    /* set LAG member attributes  */        
    printf("\nSet LAG member lag_member_idx %d attributes\n",lag_member_idx[0]);           
    attr_list[0].id = SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE;
    attr_list[0].value.booldata = false;          
    status = sai_lag_api->set_lag_member_attribute(lag_member_oid[0], &attr_list[0]);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Set LAG member attributes returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* Get LAG member attributes */
    printf("\nGet LAG member lag_member_idx %d attributes\n",lag_member_idx[0]);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_INGRESS_DISABLE;   
    attr_count++; 
    attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;    
    attr_count++;
    status = sai_lag_api->get_lag_member_attribute(lag_member_oid[0], attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Get LAG member attributes returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* get LAG attributes */
    printf("\nGet LAG lag_idx %d attributes\n",lag_idx);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_ATTR_PORT_LIST;   
    attr_list[attr_count].value.objlist.count = MRVL_SAI_LAG_LIST_SIZE;
    attr_list[attr_count].value.objlist.list  = sai_lag_ports_list;
    attr_count++;     
    status = sai_lag_api->get_lag_attribute(lag_oid, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Get LAG attributes returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* remove port '1' from LAG */  
    printf("\nRemove LAG member lag_member_idx %d\n",lag_member_idx[0]); 
    status = sai_lag_api->remove_lag_member(lag_member_oid[0]);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Remove LAG member returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    
    /* get LAG attributes */
    printf("\nGet LAG lag_idx %d attributes\n",lag_idx);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_ATTR_PORT_LIST; 
    attr_list[attr_count].value.objlist.count = MRVL_SAI_LAG_LIST_SIZE;
    attr_list[attr_count].value.objlist.list  = sai_lag_ports_list;  
    attr_count++;     
    status = sai_lag_api->get_lag_attribute(lag_oid, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Get LAG attributes returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    

    /* remove other ports from LAG */
    for (i=1; i<4; i++) {
        printf("\nRemove LAG member lag_member_idx %d\n",lag_member_idx[i]); 
        status = sai_lag_api->remove_lag_member(lag_member_oid[i]);
        if (status!= SAI_STATUS_SUCCESS) {
            printf("%s:%d: Remove LAG member returned with status %d\n",
                   __func__, __LINE__, status);
            return SAI_STATUS_FAILURE;
        }
    }    

    /* Remove LAG group */
    printf("\nRemove LAG lag_idx %d\n",lag_idx); 
    status = sai_lag_api->remove_lag(lag_oid);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Remove LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* post test settings */
    status = mrvl_sai_lag_post_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: hostif post-test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }               

    return SAI_STATUS_SUCCESS;     
}
   

