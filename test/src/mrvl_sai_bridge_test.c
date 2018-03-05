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
* mrvl_sai_bridge_test.c
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

extern sai_fdb_api_t* sai_bridge_api;
static sai_object_id_t switch_id;

/*******************************************************************************
* mrvl_sai_bridge_create_test
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
int mrvl_sai_bridge_port_create_test
(
    void
)
{
    sai_object_id_t bridge_port_oid;
    uint32_t attr_count;
    sai_attribute_t attr_list[5];
    sai_status_t status;

    MRVL_SAI_LOG_INF("mrvl_sai_bridge_port_create_test\n");

    attr_list[0].id = SAI_BRIDGE_PORT_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_BRIDGE_PORT_TYPE_PORT;
    attr_list[1].id = SAI_BRIDGE_PORT_ATTR_PORT_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &attr_list[1].value.oid)) {
        return SAI_STATUS_FAILURE;
    }
    attr_list[2].id = SAI_BRIDGE_PORT_ATTR_VLAN_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, 1, &attr_list[2].value.oid)) {
        return SAI_STATUS_FAILURE;
    }
    attr_list[3].id = SAI_BRIDGE_PORT_ATTR_RIF_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ROUTER_INTERFACE, 1, &attr_list[3].value.oid)) {
        return SAI_STATUS_FAILURE;
    }
    attr_list[4].id = SAI_BRIDGE_PORT_ATTR_TUNNEL_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_TUNNEL, 1, &attr_list[4].value.oid)) {
        return SAI_STATUS_FAILURE;
    }
    attr_list[5].id = SAI_BRIDGE_PORT_ATTR_BRIDGE_ID;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_BRIDGE, 1, &attr_list[5].value.oid)) {
        return SAI_STATUS_FAILURE;
    }

    attr_count = 6;
    MRVL_SAI_LOG_INF("Calling sai_bridge_api->create_bridge_port\n");
    status = sai_bridge_api->create_bridge_port(&bridge_port_oid, SAI_DEFAULT_ETH_SWID_CNS, attr_count, attr_list);

	return SAI_STATUS_SUCCESS;
}

