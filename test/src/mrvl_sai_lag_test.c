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

static sai_object_id_t switch_id = 0;

#define MRVL_SAI_LAG_LIST_SIZE 10
sai_object_id_t sai_lag_ports_list[MRVL_SAI_LAG_LIST_SIZE];
sai_object_id_t sai_dummy_list[MRVL_SAI_LAG_LIST_SIZE];

/*******************************************************************************
* mrvl_sai_lag_create_test
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
int mrvl_sai_lag_create_test
(
  OUT sai_object_id_t *lag_oid, 
  IN uint32_t lag_id, 
  IN sai_object_id_t switch_id, 
  IN uint32_t attr_count, 
  IN sai_attribute_t *attr_list
)
{
    sai_status_t status;

    MRVL_SAI_LOG_DBG("sai_lag_api->create_lag %d\n", lag_id);
    if (lag_oid == NULL)
    {
        fprintf(stderr, "NULL LAG object\n");
        return(1);
    }
    else if (attr_list == NULL)
    {
        fprintf(stderr, "NULL attribute list\n");
        return(1);
    }
    status = sai_lag_api->create_lag(lag_oid, switch_id, attr_count, attr_list); 
	return status;
}

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
int mrvl_sai_lag_pre_test(void)
{            
    sai_status_t            status;    
    sai_vlan_tagging_mode_t tag;
    sai_object_id_t         vlan_oid,vlan_member_oid, port_oid;
    uint32_t                port, i; 
    uint32_t                vlan_member;
    sai_vlan_id_t           vlan = 1;
    
    /* create vlan 1 */
    uint32_t vlan_attr_count;
    sai_attribute_t vlan_attr_list[1];

    vlan_attr_count = 1;
    vlan_attr_list[0].id = SAI_VLAN_ATTR_VLAN_ID;
    vlan_attr_list[0].value.u16 = vlan;

    printf("Create vlanId:%d \n", vlan);    
    status = mrvl_sai_vlan_create_test(&vlan_oid, vlan, switch_id, vlan_attr_count, vlan_attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
        
    /* add port i to vlan 1 tagged */ 
    for (i=0; i<4; i++) {
        port = i;
        MRVL_SAI_LOG_DBG("Add port %d to vlan %d, tagged\n", port, vlan);
        tag = SAI_VLAN_TAGGING_MODE_TAGGED;
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_oid)) 
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_oid, tag, &vlan_member_oid))
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_oid, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
            return SAI_STATUS_FAILURE;
        printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n", port, vlan, vlan_member, vlan_member);

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
int mrvl_sai_lag_post_test (void)
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
* mrvl_sai_lag_deletion_test
*
* DESCRIPTION: Create lag, add 2 members to it, delete the lag and verify all members are deleted aswell.
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
int mrvl_sai_lag_deletion_test(void)
{       
    sai_object_id_t     lag_oid, port_oid[2], lag_member_oid[2];
    uint32_t            lag_idx, port_idx[2], lag_member_idx[2];
    sai_attribute_t     attr_list[4];
    uint32_t            attr_count, i;   
    sai_status_t        status;    
    uint32_t            lag_attr_count;
    sai_attribute_t     lag_attr_list[2]; 
    
    printf("\n\n---------------------mrvl_sai_lag_deletion_test---------------------\n");
    printf("1. Create empty lag\n");
    printf("2. Add 2 ports to the LAG as lag members\n");
    printf("3. Get LAG attributes\n");
    printf("4. Remove LAG members from LAG\n");
    printf("5. Remove LAG\n");
    printf("Between each step, dump LAG tables from FPA and HW\n");

    /* create empty LAG */    
    lag_attr_count = 0;
    memset(lag_attr_list, 0, sizeof(sai_attribute_t)*2);
    lag_attr_list[0].id = SAI_LAG_ATTR_INGRESS_ACL;
    lag_attr_list[0].value.oid = SAI_NULL_OBJECT_ID;
    lag_attr_list[1].id = SAI_LAG_ATTR_EGRESS_ACL;
    lag_attr_list[1].value.oid = SAI_NULL_OBJECT_ID;
    printf("\nCreate empty LAG\n");
    status = mrvl_sai_lag_create_test(&lag_oid, 1, switch_id, lag_attr_count, lag_attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Create empty LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_idx))) {
            return status;
    }

    /* add ports to LAG */
    sai_lag_ports_list[0] = 1;
    sai_lag_ports_list[1] = 2;
    for (i=0; i<2; i++) 
    {
        /* create port_oid for port i */
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, sai_lag_ports_list[i], &(port_oid[i]))) 
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_oid[i], SAI_OBJECT_TYPE_PORT, &(port_idx[i])))
            return SAI_STATUS_FAILURE;


        printf("\nCreate LAG member #%d: port %d\n",lag_idx, port_idx[i]);
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
        status = sai_lag_api->create_lag_member(&(lag_member_oid[i]), switch_id, attr_count, attr_list);
        if (status!= SAI_STATUS_SUCCESS) {
            printf("%s:%d: Create LAG member returned with status %d\n",
                   __func__, __LINE__, status);
            return SAI_STATUS_FAILURE;
        }
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_member_oid[i], SAI_OBJECT_TYPE_LAG_MEMBER, &(lag_member_idx[i])))
            return SAI_STATUS_FAILURE;
    }

    /* get LAG attributes */
    printf("\nGet LAG #%d attributes\n", lag_idx);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_ATTR_PORT_LIST; 
    attr_list[attr_count].value.objlist.list  = sai_lag_ports_list;  
    attr_count++;     
    status = sai_lag_api->get_lag_attribute(lag_oid, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Get LAG attributes returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    } 

    printf("\n------Before removing LAG %d--------\n", lag_idx);
    printf("------LAG table dump from FPA--------\n");
    fpaWrapDumpAll();
    fpaWrapDumpTrunkDB(switch_id, lag_idx);

    /* Remove LAG group */
    printf("\nRemove LAG %d\n",lag_idx); 
    status = sai_lag_api->remove_lag(lag_oid);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Remove LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
            
    printf("\n------After removing LAG %d--------\n", lag_idx);
    printf("------LAG table dump from FPA--------\n");
    fpaWrapDumpAll();
    fpaWrapDumpTrunkDB(switch_id, lag_idx);

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
    uint32_t            lag_attr_count;
    sai_attribute_t     lag_attr_list[2]; 
    uint32_t            lag_member_lag_idx, lag_member_port;
    
    printf("\n\n---------------------mrvl_sai_lag_group_test---------------------\n");
    printf("1. Create empty lag\n");
    printf("2. Add 4 ports to the LAG as lag members\n");
    printf("3. Get LAG member #1 and LAG attributes\n");
    printf("4. Remove LAG member #1 & get LAG attributes again\n");
    printf("5.Remove other LAG members from LAG\n");
    printf("6. Remove LAG\n");
    printf("Between each step, dump LAG tables from FPA and HW\n");


    /* create empty LAG */    
    lag_attr_count = 0;
    memset(lag_attr_list, 0, sizeof(sai_attribute_t)*2);
    lag_attr_list[0].id = SAI_LAG_ATTR_INGRESS_ACL;
    lag_attr_list[0].value.oid = SAI_NULL_OBJECT_ID;
    lag_attr_list[1].id = SAI_LAG_ATTR_EGRESS_ACL;
    lag_attr_list[1].value.oid = SAI_NULL_OBJECT_ID;
    printf("\nCreate empty LAG\n");
    status = mrvl_sai_lag_create_test(&lag_oid, 1, switch_id, lag_attr_count, lag_attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Create empty LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_idx))) {
            return status;
    }

    /* add ports to LAG */
    sai_lag_ports_list[0] = 1;
    sai_lag_ports_list[1] = 2;
    sai_lag_ports_list[2] = 3;
    sai_lag_ports_list[3] = 4;
    for (i=0; i<4; i++) 
    {
        /* create port_oid for port i */
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, sai_lag_ports_list[i], &(port_oid[i]))) 
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_oid[i], SAI_OBJECT_TYPE_PORT, &(port_idx[i])))
            return SAI_STATUS_FAILURE;

        printf("\nCreate LAG member #%d: port %d\n",lag_idx, port_idx[i]);   
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
        status = sai_lag_api->create_lag_member(&(lag_member_oid[i]), switch_id, attr_count, attr_list);
        if (status!= SAI_STATUS_SUCCESS) {
            printf("%s:%d: Create LAG member returned with status %d\n",
                   __func__, __LINE__, status);
            return SAI_STATUS_FAILURE;
        }
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_member_oid[i], SAI_OBJECT_TYPE_LAG_MEMBER, &(lag_member_idx[i])))
            return SAI_STATUS_FAILURE;
    }
    
    
    /* Get LAG member attributes */
    printf("\nGet LAG member #%d attributes\n",lag_member_idx[0]);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attr_count++;
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
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(attr_list[0].value.oid, SAI_OBJECT_TYPE_LAG, &lag_member_lag_idx))
    {
       printf("mrvl_sai_utl_object_to_type:%d: SAI_OBJECT_TYPE_LAG returned with status %d\n", __LINE__, status);
       return SAI_STATUS_FAILURE;
    }
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(attr_list[1].value.oid, SAI_OBJECT_TYPE_PORT, &lag_member_port))
    {
        printf("mrvl_sai_utl_object_to_type:%d: SAI_OBJECT_TYPE_PORT returned with status %d\n", __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    printf("Lag member %d attributes: lag %d, port %d, ingress/egress disable %s/%s\n", lag_member_idx[0],
          lag_member_lag_idx, lag_member_port, attr_list[2].value.booldata == false ? "false" : "true",
           attr_list[3].value.booldata == false ? "false" : "true");

    /* get LAG attributes */
    printf("\nGet LAG #%d attributes\n", lag_idx);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_ATTR_PORT_LIST; 
    attr_list[attr_count].value.objlist.list  = sai_lag_ports_list;  
    attr_count++;     
    status = sai_lag_api->get_lag_attribute(lag_oid, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Get LAG attributes returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    } 

    printf("\n------Before removing %d LAG members from LAG %d--------\n", attr_list[attr_count-1].value.objlist.count, lag_idx);
    printf("------LAG table dump from FPA--------\n");
    fpaWrapDumpAll();
    
    fpaWrapDumpTrunkDB(switch_id, lag_idx);

    /* remove port '1' from LAG */  
    printf("\nRemove LAG member %d\n",lag_member_idx[0]); 
    status = sai_lag_api->remove_lag_member(lag_member_oid[0]);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Remove LAG member returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    
    /* get LAG attributes */
    printf("\nGet LAG #%d attributes\n",lag_idx);          
    attr_count = 0; 
    attr_list[attr_count].id = SAI_LAG_ATTR_PORT_LIST; 
    /*attr_list[attr_count].value.objlist.count = MRVL_SAI_LAG_LIST_SIZE;*/
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
        printf("\nRemove LAG member %d\n",lag_member_idx[i]); 
        status = sai_lag_api->remove_lag_member(lag_member_oid[i]);
        if (status!= SAI_STATUS_SUCCESS) {
            printf("%s:%d: Remove LAG member returned with status %d\n",
                   __func__, __LINE__, status);
            return SAI_STATUS_FAILURE;
        }
    }

    printf("\n------After removing LAG members from LAG %d--------\n", lag_idx);
    printf("------LAG table dump from FPA--------\n");
    fpaWrapDumpAll();
    fpaWrapDumpTrunkDB(switch_id, lag_idx);

    /* Remove LAG group */
    printf("\nRemove LAG %d\n",lag_idx); 
    status = sai_lag_api->remove_lag(lag_oid);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Remove LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* post test settings */
    /*status = mrvl_sai_lag_post_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: hostif post-test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }*/              
    
    printf("\n------After removing LAG %d--------\n", lag_idx);
    printf("------LAG table dump from FPA--------\n");
    fpaWrapDumpAll();
    fpaWrapDumpTrunkDB(switch_id, lag_idx);
    
    return SAI_STATUS_SUCCESS;     
}
   
/*******************************************************************************
* mrvl_sai_lag_tests
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
int mrvl_sai_lag_tests(void)
{       
    sai_status_t        status;    

    status = mrvl_sai_lag_group_test();
    if (SAI_STATUS_SUCCESS != status)
    {
        fprintf(stderr, "mrvl_sai_lag_group_test failed, status %d\n", status); 
        return(1);
    }

    status = mrvl_sai_lag_deletion_test();
    if (SAI_STATUS_SUCCESS != status)
    {
        fprintf(stderr, "mrvl_sai_lag_deletion_test failed, status %d\n", status); 
        return(1);
    }

    return SAI_STATUS_SUCCESS;
}
