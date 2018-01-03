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

#include    "fpaLibApis.h"
#include    "sai.h"
#include    "mrvl_sai.h"

extern sai_vlan_api_t* sai_vlan_api;
extern sai_switch_api_t* sai_switch_api;

static sai_object_id_t switch_id = 0;

sai_object_id_t sai_vlan_member_list[SAI_MAX_NUM_OF_PORTS];
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
    OUT sai_object_id_t *vlan_oid, uint32_t vlan_id, sai_object_id_t switch_id, uint32_t attr_count, sai_attribute_t *attr_list
)
{
    sai_status_t status;

    MRVL_SAI_LOG_DBG("sai_vlan_api->create_vlan %d\n", vlan_id);
    status = sai_vlan_api->create_vlan(vlan_oid, switch_id, attr_count, attr_list);
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
    int32_t vlan_id,
    OUT sai_object_id_t *vlan_oid

)
{
    uint32_t attr_count;
    sai_attribute_t attr_list[1];

    attr_count = 1;
    attr_list[0].id = SAI_VLAN_ATTR_VLAN_ID;
    attr_list[0].value.u16 = vlan_id;

	printf("mrvl_sai_wrap_vlan_create: vlanId:%d \n", vlan_id);    
    return mrvl_sai_vlan_create_test(vlan_oid, vlan_id, switch_id, attr_count, attr_list);
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
    IN sai_object_id_t vlan_oid
)
{
    sai_status_t status;

    MRVL_SAI_LOG_INF("sai_vlan_api->remove_vlan\n");
    status = sai_vlan_api->remove_vlan(vlan_oid);
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
    IN int32_t vlan_id,
    IN sai_object_id_t vlan_oid
)
{
	printf("mrvl_sai_wrap_vlan_remove: vlanId %d\n",vlan_id);    
    return mrvl_sai_vlan_remove_test(vlan_oid);
}




/*******************************************************************************
* mrvl_sai_vlan_create_member_test
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
int mrvl_sai_vlan_create_member_test
(
    IN sai_object_id_t          vlan_oid,
    IN sai_object_id_t          port_id,
    IN sai_vlan_tagging_mode_t  tagging_mode,
    OUT sai_object_id_t         *vlan_member_id
)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[3];

    attr_list[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID;
    attr_list[attr_count++].value.oid = vlan_oid;
    
    attr_list[attr_count].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID;
    attr_list[attr_count++].value.oid = port_id;
    
    if (tagging_mode < SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED) {
        attr_list[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;
        attr_list[attr_count++].value.s32 = tagging_mode;
    }
    MRVL_SAI_LOG_DBG("sai_vlan_api->create_vlan_member\n"); 
    return sai_vlan_api->create_vlan_member(vlan_member_id, switch_id, attr_count, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_create_member
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
int mrvl_sai_wrap_vlan_create_member
(
    IN sai_object_id_t vlan_oid,
    IN sai_uint32_t port,
    IN sai_vlan_tagging_mode_t tag /* 0=untag, 1= tag */
)
{
    sai_object_id_t  port_id;
    sai_object_id_t  vlan_member_id;
    sai_status_t    status;
    sai_uint32_t    vlan_member, vlan_id;
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_oid, SAI_OBJECT_TYPE_VLAN, &vlan_id))) {
        return status;
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }
    status = mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_wrap_vlan_create_member: vlanId:%d port:%d tag:%d - failed %d\n", vlan_id, port, tag, status);    
    } else {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))) {
            return status;
        }
        printf("mrvl_sai_wrap_vlan_create_member: vlanId:%d port:%d tag:%d --> vlan_member_id:%d (0x%x)\n", vlan_id, port, tag, vlan_member, vlan_member);    
    }
    return status;
}

/*******************************************************************************
* mrvl_sai_vlan_remove_member_test
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
int mrvl_sai_vlan_remove_member_test
(
    IN sai_object_id_t  vlan_member_id
)
{
    MRVL_SAI_LOG_INF("sai_vlan_api->remove_vlan_member\n");
    return sai_vlan_api->remove_vlan_member(vlan_member_id);
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_remove_member
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
int mrvl_sai_wrap_vlan_remove_member
(
    IN sai_uint32_t vlan_member
)
{
    sai_object_id_t  vlan_member_id;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, vlan_member, &vlan_member_id)) {
        return SAI_STATUS_FAILURE;
    }
    printf("mrvl_sai_wrap_vlan_remove_member: vlan_member:%d\n", vlan_member);    
    return mrvl_sai_vlan_remove_member_test(vlan_member_id);
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
    IN sai_object_id_t    vlan_oid,
    IN uint32_t         attr_count,
    IN sai_attribute_t  *attr_list
)
{    
    MRVL_SAI_LOG_INF("sai_vlan_api->get_vlan_attribute\n");
    return sai_vlan_api->get_vlan_attribute(vlan_oid, attr_count, attr_list);
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
    IN sai_object_id_t vlan_oid
)
{
    uint32_t            vlan_member, stp_id, i, attr_count = 0, vlan_id;
    sai_attribute_t     attr_list[4];
    sai_status_t        status;
    sai_object_list_t   *objlist;
            
    attr_list[attr_count].id = SAI_VLAN_ATTR_MEMBER_LIST; 
    attr_list[attr_count].value.objlist.count = SAI_MAX_NUM_OF_PORTS;
    attr_list[attr_count++].value.objlist.list  = sai_vlan_member_list;

    attr_list[attr_count++].id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES; 

    attr_list[attr_count++].id = SAI_VLAN_ATTR_STP_INSTANCE; 

    attr_list[attr_count++].id = SAI_VLAN_ATTR_LEARN_DISABLE; 
    

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_oid, SAI_OBJECT_TYPE_VLAN, &vlan_id))) {
        return status;
    }

    printf("mrvl_sai_wrap_vlan_get_attr: vlanId:%d \n", vlan_id);    
    status =  mrvl_sai_vlan_get_attr_test(vlan_oid, attr_count, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[2].value.oid, SAI_OBJECT_TYPE_STP, &stp_id))) {
            return status;
        }
         
        printf(" get attributes for vlan:%d ==> max learn:%d stp instance:%d disable:%d member list:\n",
               vlan_id,
               attr_list[1].value.u32,
               stp_id,
               attr_list[3].value.booldata);
        objlist = &attr_list[0].value.objlist;
        for (i=0; i < objlist->count; i++) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(objlist->list[i], SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))) {
                return status;
            }
            printf("[%d] vlan_member:%d \n", i, vlan_member);
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
    IN sai_object_id_t    vlan_oid,
    IN sai_attribute_t  *attr_list
)
{    
    MRVL_SAI_LOG_INF("sai_vlan_api->set_vlan_attribute\n");
    return sai_vlan_api->set_vlan_attribute(vlan_oid, attr_list);
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
    IN sai_object_id_t vlan_oid,
    IN sai_uint32_t attr_id, /* 1=MAX_LEARNED_ADDRESSES, 2=STP_INSTANCE, 3=LEARN_DISABLE */
    IN sai_uint32_t attr_value
)
{
    sai_attribute_t attr_list;
    sai_status_t status;
    sai_uint32_t vlan_id;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vlan_oid, SAI_OBJECT_TYPE_VLAN, &vlan_id))) {
        return status;
    }
    printf("mrvl_sai_wrap_vlan_set_attr: vlanId:%d \n", vlan_id);    
    attr_list.id = attr_id; 
    switch (attr_id) {
    case SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES:
        attr_list.value.u32 = attr_value;
        break;
    case SAI_VLAN_ATTR_STP_INSTANCE:
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_STP, attr_value, &attr_list.value.oid))) {
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
    status =  mrvl_sai_vlan_set_attr_test(vlan_oid, &attr_list);
    return status; 
}


/*******************************************************************************
* mrvl_sai_vlan_get_member_attr_test
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
int mrvl_sai_vlan_get_member_attr_test
(
    IN sai_object_id_t  vlan_member_id,
    IN uint32_t         attr_count,
    IN sai_attribute_t  *attr_list
)
{    
    MRVL_SAI_LOG_INF("sai_vlan_api->get_vlan_member_attribute\n");
    return sai_vlan_api->get_vlan_member_attribute(vlan_member_id, attr_count, attr_list);
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
int mrvl_sai_wrap_vlan_get_member_attr
(
    IN sai_uint32_t vlan_member
)
{
    uint32_t            port, attr_count = 0;
    sai_attribute_t     attr_list[3];
    sai_status_t        status;
    sai_object_id_t     vlan_member_id;
            
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID; 
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID; 
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;     

    printf("mrvl_sai_wrap_vlan_get_member_attr: vlan_member:%d \n", vlan_member);    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))) {
        return status;
    }
    status =  mrvl_sai_vlan_get_member_attr_test(vlan_member_id, attr_count, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(attr_list[1].value.oid, SAI_OBJECT_TYPE_PORT, &port))) {
            return status;
        }
         
        printf(" get attributes for vlan_member:%d ==> vlan:%d port:%d tag:%s\n",
               vlan_member,
               attr_list[0].value.u16,
               port,
               (attr_list[2].value.s32 == 0)?"untagged":"tagged");
    }
    return status; 
}

/*******************************************************************************
* mrvl_sai_vlan_set_member_attr_test
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
int mrvl_sai_vlan_set_member_attr_test
(
    IN sai_object_id_t  vlan_member_id,
    IN sai_attribute_t  *attr_list
)
{    
    MRVL_SAI_LOG_INF("sai_vlan_api->set_vlan_member_attribute\n");
    return sai_vlan_api->set_vlan_member_attribute(vlan_member_id, attr_list);
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_set_member_attr
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
int mrvl_sai_wrap_vlan_set_member_attr
(
    IN sai_uint32_t vlan_member,
    IN sai_uint32_t attr_id, /* 0=SAI_VLAN_MEMBER_ATTR_VLAN_ID, 1=SAI_VLAN_MEMBER_ATTR_PORT_ID, 2=SAI_VLAN_MEMBER_ATTR_TAGGING_MODE */
    IN sai_uint32_t attr_value
)
{
    sai_attribute_t attr_list;
    sai_status_t status;
    sai_object_id_t  vlan_member_id;

    printf("mrvl_sai_wrap_vlan_set_member_attr: vlan_member:%d \n", vlan_member);    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))) {
        return status;
    }
    attr_list.id = attr_id; 
    switch (attr_id) {
    case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
        attr_list.value.s32 = attr_value;
        break;
    default:
        printf("Not supported param %d\n", attr_id);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    status =  mrvl_sai_vlan_set_member_attr_test(vlan_member_id, &attr_list);
    return status; 
}

/*******************************************************************************
* mrvl_sai_wrap_vlan_add_ports_list
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
int mrvl_sai_wrap_vlan_add_ports_list
(
    IN  sai_object_id_t vlan_oid,
    IN  sai_uint32_t vlan_id,
    IN  sai_uint32_t num_of_ports,
    IN  sai_port_info_t *ports_list,
    OUT sai_uint32_t *vlan_member_list
)
{
    sai_uint32_t            i, port;
    sai_vlan_tagging_mode_t tag;
    sai_object_id_t         port_id, vlan_member_id;
    
    for (i = 0; i < num_of_ports; i++) {
        port = ports_list[i].port;
        tag = ports_list[i].tag;
        printf("Add port %d to vlan %d %s\n", port, vlan_id, (tag == SAI_VLAN_TAGGING_MODE_UNTAGGED)?"untagged":"tagged");
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)){
            printf("Add port %d to vlan %d failed on create object\n", port, vlan_id);
            return SAI_STATUS_FAILURE;
        }
        if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id)){
            printf("Add port %d to vlan %d failed on create member test\n", port, vlan_id);
            return SAI_STATUS_FAILURE;
        }
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member_list[i])){
            printf("Add port %d to vlan %d failed on object to type\n", port, vlan_id);
            return SAI_STATUS_FAILURE;
        }
    }
   return SAI_STATUS_SUCCESS;
}


/*******************************************************************************
* mrvl_sai_vlan_test
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
    sai_vlan_id_t           vlan;
    uint32_t                port, vlan_member, stp, i;
    sai_status_t            status;
    uint32_t                attr_count = 0;
    sai_attribute_t         attr_list[5];
    sai_object_id_t         vlan_member_id, port_id, vlan_oid, my_vlan_oid;
    sai_vlan_tagging_mode_t tag;
    sai_object_list_t       *objlist;
    

    printf("\nCreate vlan 1\n");
    vlan = 1;
    status = mrvl_sai_wrap_vlan_create(vlan, &vlan_oid);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;  
    
    printf("\nAdd ports 0-2 to vlan 1 as tagged\n");
    port = 0;
    tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    port = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    port = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);
      
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
        
    
    printf("\nCreate vlan 2\n");          
    vlan = 2;
    status = mrvl_sai_wrap_vlan_create(vlan, &vlan_oid);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE;    

    printf("\nAdd ports 1-3 to vlan 2 as untagged\n");
    port = 1;
    tag = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    port = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    port = 3;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);

    printf("\nCreate vlan 3\n");
    vlan = 3;
    status = mrvl_sai_wrap_vlan_create(vlan, &vlan_oid);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    
    printf("\nAdd ports 1 and 2 as untagged and port 3 as tagged to vlan 3\n");
    port = 1;
    tag = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    port = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    port = 3;
    tag = SAI_VLAN_TAGGING_MODE_TAGGED;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) 
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_create_member_test(vlan_oid, port_id, tag, &vlan_member_id))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(vlan_member_id, SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
        return SAI_STATUS_FAILURE;
    printf(">>>ports %d, vlan %d --> port member %d (0x%x)\n",port, vlan, vlan_member, vlan_member);

    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    
    
    
        
    printf("\nRemove port 2 from vlan 3\n");
    vlan = 3;
    port = 2;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;

    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
  
    
     
    printf("\nGet vlan 3 attributes \n");
    attr_count=0;
    vlan = 3;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan, &my_vlan_oid))
        return SAI_STATUS_FAILURE;
    attr_list[attr_count].id = SAI_VLAN_ATTR_MEMBER_LIST; 
    attr_list[attr_count].value.objlist.count = SAI_MAX_NUM_OF_PORTS;
    attr_list[attr_count++].value.objlist.list  = sai_vlan_member_list;
    attr_list[attr_count++].id = SAI_VLAN_ATTR_MAX_LEARNED_ADDRESSES; 
    attr_list[attr_count++].id = SAI_VLAN_ATTR_STP_INSTANCE; 
    attr_list[attr_count++].id = SAI_VLAN_ATTR_LEARN_DISABLE; 
    attr_list[attr_count++].id = SAI_VLAN_ATTR_VLAN_ID; 
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_get_attr_test(my_vlan_oid, attr_count, attr_list))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(attr_list[2].value.oid, SAI_OBJECT_TYPE_STP, &stp))
        return SAI_STATUS_FAILURE;
         
    printf(" get attributes for vlan:%d ==> max learn:%d stp instance:%d disable:%d member list:\n",
           attr_list[4].value.u16,
           attr_list[1].value.u32,
           stp,
           attr_list[3].value.booldata);
    objlist = &attr_list[0].value.objlist;
    for (i=0; i < objlist->count; i++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(objlist->list[i], SAI_OBJECT_TYPE_VLAN_MEMBER, &vlan_member))
            return SAI_STATUS_FAILURE;
        printf("[%d] vlan_member:%d (0x%x) \n", i, vlan_member, vlan_member);
    }


    printf("\nGet vlan 3 port 3 attributes \n");
    vlan = 3;
    port = 3;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE; 
    attr_count = 0;  
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_VLAN_ID; 
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID; 
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE;     
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_get_member_attr_test(vlan_member_id, attr_count, attr_list))
        return SAI_STATUS_FAILURE; 
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(attr_list[1].value.oid, SAI_OBJECT_TYPE_PORT, &port)) 
        return SAI_STATUS_FAILURE; 
    printf(" get attributes for vlan_member:%d (0x%x) ==> vlan:%d port:%d tag:%s\n",
           vlan_member, vlan_member,
           attr_list[0].value.u16,
           port,
           (attr_list[2].value.s32 == 0)?"untagged":"tagged");


    printf("\nSet vlan 1 port 0 attributes to untagged\n");
    vlan = 1;
    port = 0;
    tag = SAI_VLAN_TAGGING_MODE_UNTAGGED;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE; 
    attr_count = 0;  
    attr_list[attr_count].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE; 
    attr_list[attr_count++].value.s32 = tag;
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_set_member_attr_test(vlan_member_id, attr_list))
        return SAI_STATUS_FAILURE; 

    printf("\nGet vlan 1 port 0 tag mode \n");
    vlan = 1;
    port = 0;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE; 
    attr_count = 0;  
    attr_list[attr_count++].id = SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE; 
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_get_member_attr_test(vlan_member_id, attr_count, attr_list))
        return SAI_STATUS_FAILURE; 
    printf(" get attributes for vlan_member:%d (0x%x) ==>  tag:%s\n",vlan_member, vlan_member, (attr_list[0].value.s32 == 0)?"untagged":"tagged");
    if (attr_list[0].value.s32 != 0) {
        printf("Invalid tagging mode\n");
        return SAI_STATUS_FAILURE;
    }
    
    printf("\nRemove all ports from vlan 1\n"); 
    vlan = 1;
    port = 0;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    port = 1;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    port = 2;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    
    
    printf("\nRemove  vlan 1\n");
    vlan = 1;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan, &my_vlan_oid))
        return SAI_STATUS_FAILURE;
    status = mrvl_sai_wrap_vlan_remove(vlan, my_vlan_oid);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);
    
        
    printf("\nRemove all ports from vlan 2\n"); 
    vlan = 2;
    port = 1;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    port = 2;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    port = 3;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);


    printf("\nRemove  vlan 2\n");
    vlan = 2;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan, &my_vlan_oid))
        return SAI_STATUS_FAILURE;
    status = mrvl_sai_wrap_vlan_remove(vlan, my_vlan_oid);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);

    printf("\nRemove all ports from vlan 3\n"); 
    vlan = 3;
    port = 1;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    port = 3;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);


    printf("\nRemove  vlan 3\n");
    vlan = 3;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan, &my_vlan_oid))
        return SAI_STATUS_FAILURE;
    status = mrvl_sai_wrap_vlan_remove(vlan, my_vlan_oid);
    if (status!= SAI_STATUS_SUCCESS) 
        return SAI_STATUS_FAILURE; 
    fpaLibFlowTableDump(0, FPA_FLOW_TABLE_TYPE_VLAN_E);

    return SAI_STATUS_SUCCESS;
}
