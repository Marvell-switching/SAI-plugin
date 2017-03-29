/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
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

extern sai_virtual_router_api_t* sai_virtual_router_api;

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
int mrvl_sai_virtual_router_add_test
(
    _Out_ sai_object_id_t* vr_id
)
{
    uint32_t attr_count=0;
    sai_attribute_t attr_list[3];
    sai_status_t status;
    memset(attr_list, 0, sizeof(sai_attribute_t)*3);
    attr_list[attr_count].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;    
    attr_list[attr_count].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;
    
    MRVL_SAI_LOG_INF("Calling sai_virtual_router_api->create_virtual_router\n");
    status = sai_virtual_router_api->create_virtual_router(vr_id, attr_count, attr_list);
	return status;
}


/*******************************************************************************
* mrvl_sai_wrap_virtual_router_add
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
int mrvl_sai_wrap_virtual_router_add
(
    void
)
{
    sai_object_id_t     vr_id;
    sai_uint32_t        vr_idx;
    sai_status_t        status;
    
	printf("mrvl_sai_wrap_virtual_router_add: \n");
    
    status = mrvl_sai_virtual_router_add_test(&vr_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_virtual_router_add_test failed to create virtual_router\n");
        return status;
    }
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(vr_id, SAI_OBJECT_TYPE_VIRTUAL_ROUTER, &vr_idx))) {
        printf("mrvl_sai_wrap_virtual_router_add: failed to get virtual_router\n");
        return status;
    }
    printf("mrvl_sai_virtual_router_add_test:virtual_router %d was created\n", vr_idx);
    
    return 0;
}
/*******************************************************************************
* mrvl_sai_virtual_router_remove_test
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
int mrvl_sai_virtual_router_remove_test
(
    _In_ sai_object_id_t  vr_id
)
{
    sai_status_t        status;
    
    MRVL_SAI_LOG_INF("Calling sai_virtual_router_api->remove_virtual_router\n");
    status = sai_virtual_router_api->remove_virtual_router(vr_id);
	return status;
}
/*******************************************************************************
* mrvl_sai_wrap_virtual_router_remove
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
int mrvl_sai_wrap_virtual_router_remove
(
    sai_uint32_t        vr_idx
)
{
    sai_object_id_t     vr_id;
    sai_status_t        status;
    
	printf("mrvl_sai_wrap_virtual_router_remove: \n");
    
    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, vr_idx, &vr_id))) {
        printf("mrvl_sai_wrap_virtual_router_remove: failed to get virtual_router\n");
        return status;
    }
    status = mrvl_sai_virtual_router_remove_test(vr_id); 
    if (status != SAI_STATUS_SUCCESS) {
        printf("mrvl_sai_wrap_virtual_router_remove failed to delete virtual_router\n");
        return status;
    }
    printf("mrvl_sai_wrap_virtual_router_remove:virtual_router %d was deleted\n", vr_idx);
    
    return 0;
}

