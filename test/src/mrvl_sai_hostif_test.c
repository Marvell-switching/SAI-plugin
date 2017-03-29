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
* mrvl_sai_hostif_test.c
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
#include	"../test/inc/mrvl_sai_test.h"

extern sai_hostif_api_t* sai_hif_api;
extern sai_vlan_api_t* sai_vlan_api;
extern sai_fdb_api_t* sai_fdb_api;

#define vlan_1  1
#define port_0  0
#define port_1  1

sai_object_id_t hif_id;    

#define PKTDUMP_MAX_BYTES       64
#define PKTDUMP_BYTES_PER_LINE  16

#define MRVL_SAI_HOSTIF_LIST_SIZE 10
sai_object_id_t sai_hostif_ports_list[MRVL_SAI_HOSTIF_LIST_SIZE];
sai_object_id_t sai_hostif_vlans_list[MRVL_SAI_HOSTIF_LIST_SIZE];

extern FPA_STATUS fpaSysEnableBufPrint(int enable);

static sai_object_id_t rif_id;
static sai_object_id_t vr_id;

/* test senarios:

1. send SAI_HOSTIF_TRAP_ID_ARP_REQUEST on port 0 action trap
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 0x2000 4 
  
send arp packet and expect trap to cpu 
00000000  FF FF FF FF FF FF 00 00 00 00 00 11 08 06 00 01
00000010  08 00 06 04 00 01 00 00 00 00 00 11 01 01 01 01
00000020  00 00 00 00 00 22 02 02 02 02 00 00 00 00 00 00
00000030  00 00 00 00 00 00 00 00 00 00 00 00 
 
restore configuration: 
mrvl_sai_wrap_hostif_restore_attr_trap_test 0x2000
 
2. send SAI_HOSTIF_TRAP_ID_LLDP on port 1 action trap 
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 4 4
 
send lldp packet and expect trap to cpu 
00000000  01 80 C2 00 00 0E 00 04 96 1F A7 26 88 CC 02 07
00000010  04 00 04 96 1F A7 26 04 04 05 31 2F 33 06 02 00
00000020  78 06 02 00 01 06 02 00 02 06 02 00 03 00 00 FF
00000030  FF FF FF FF FF FF FF FF FF FF FF FF FF FF AA BB 
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 4

 
3. send SAI_HOSTIF_TRAP_ID_STP on port 1 action trap 
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 1 4
 
send stp packet and expect trap to cpu 
00000000  01 80 C2 00 00 00 00 1C 0E 87 85 04 00 00 00 00
00000010  00 80 64 00 1C 0E 87 78 00 00 00 00 04 80 64 00
00000020  1C 0E 87 85 00 80 04 01 00 14 00 02 00 0F 00 00
00000030  00 00 00 00 00 00 00 00 00 00 00 00
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 1 
 
4. send SAI_HOSTIF_TRAP_ID_LACP on port 1 action trap 
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 2 4
 
send lacp packet and expect trap to cpu 
00000000  01 80 C2 00 00 02 00 04 96 1F 50 6A 88 09 01 01
00000010  01 14 91 F4 00 04 96 1F 50 6A 80 00 00 00 00 12
00000020  47 00 00 00 02 14 00 00 00 00 00 00 00 00 00 00
00000030  00 00 00 00 3B 00 00 00 03 10 00 02
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 2
 
5. send SAI_HOSTIF_TRAP_ID_IGMP_TYPE_QUERY on port 0 action trap
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 6 4  
  
send igmp query packet and expect trap to cpu 
00000000  01 00 5E 00 00 01 00 01 63 6F C8 00 08 00 45 C0
00000010  00 1C F7 70 00 00 01 02 D6 B5 0A 3C 00 BD E0 00
00000020  00 01 11 64 EE 9B 00 00 00 00 00 00 00 00 00 00
00000030  00 00 00 00 00 00 00 00 00 00 00 00
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 6 
 
6. send SAI_HOSTIF_TRAP_ID_IGMP_TYPE_LEAVE on port 0 action trap
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 7 4  
  
send igmp query packet and expect trap to cpu 
00000000  01 00 5E 00 00 02 00 02 02 19 51 28 08 00 46 00
00000010  00 20 00 00 40 00 01 02 38 64 C0 A8 0B C9 E0 00
00000020  00 02 94 04 00 00 17 00 06 F1 E1 01 01 04 00 00
00000030  00 00 00 00 00 00 00 00 00 00 00 00 
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 7 
 
7. send SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V1_REPORT on port 0 action trap
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 8 4 
 
send igmp V1 report packet and expect trap to cpu 
00000000  01 00 5E 00 01 3C 00 30 C1 BF 57 55 08 00 45 00
00000010  00 1C 6A 78 00 00 01 02 63 6C 0A 3C 00 84 E0 00
00000020  01 3C 12 00 0C C3 E0 00 01 3C 00 00 00 00 00 00
00000030  FF FF FF FF FF FF 04 52 00 00 00 00 
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 8 
 
8. send SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V2_REPORT on port 0 action trap
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 9 4 
 
send igmp V2 report packet and expect trap to cpu 
00000000  01 00 5E 00 01 3C 00 14 38 E6 47 C6 08 00 46 01
00000010  00 20 20 6D 00 00 01 02 18 DE 0A 3C 00 14 E0 00
00000020  01 3C 94 04 00 00 16 00 08 C3 E0 00 01 3C 0A 0C
00000030  45 3D 54 52 55 45 0D 0A 0C 47 45 53 
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 9 
 
9. send SAI_HOSTIF_TRAP_ID_IGMP_TYPE_V3_REPORT on port 0 action trap
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 10 4 
 
send igmp V3 report packet and expect trap to cpu 
00000000  01 00 5E 00 00 16 00 25 2E 51 C3 81 08 00 46 58
00000010  00 28 00 60 00 00 01 02 82 17 C0 A8 01 42 E0 00
00000020  00 16 94 04 00 00 22 00 E3 38 00 00 00 01 04 00
00000030  00 00 EF C3 07 02 00 00 00 00 00 00 
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 10  
 
10. send SAI_HOSTIF_TRAP_ID_DHCPV6 on vlan 2 action trap --> NOT WORKING WITH SMBSIM
 
set configuration: 
mrvl_sai_wrap_hostif_set_attr_trap_vlan_list_test 0x2007 4 
 
send dhcpv6 packet and expect trap to cpu 
00000000  33 33 00 01 00 02 00 CC 55 EE 11 22 86 DD 60 00
00000010  00 00 00 36 11 40 FE 80 00 00 00 00 00 00 02 CC
00000020  55 FF FE 11 22 FF 02 00 00 00 00 00 00 00 00 00
00000030  00 00 00 01 00 02 02 22 02 23 00 36 0B A5 0B 35
00000040  14 D2 00 01 00 0A 00 03 00 01 00 CC 55 EE 11 22
00000050  00 08 00 02 00 00 00 06 00 12 00 20 00 17 00 18
00000060  00 1F 00 29 00 3B 00 3C 00 07 00 0C 
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 0x2007 
 
11. send SAI_HOSTIF_TRAP_ID_OSPFV6 on vlan 2 action trap --> NEED TO FIND BUG 
 
set configuration: 
cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable 0 1 1 
mrvl_sai_wrap_hostif_set_attr_trap_vlan_list_test 0x2008 4 
 
send ospfv6 packet and expect trap to cpu 
00000000  33 33 00 00 00 05 C2 00 1F FA 00 01 86 DD 6E 00
00000010  00 00 00 24 59 01 FE 80 00 00 00 00 00 00 00 00
00000020  00 00 00 00 00 01 FF 02 00 00 00 00 00 00 00 00
00000030  00 00 00 00 00 05 03 01 00 24 01 01 01 01 00 00
00000040  00 01 FB 86 00 00 00 00 00 05 01 00 00 13 00 0A
00000050  00 28 00 00 00 00 00 00 00 00
 
restore configuration: 
mmrvl_sai_wrap_hostif_restore_attr_trap_test 0x2008  

*/ 

/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_set_attr_trap_port_list 
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
int mrvl_sai_wrap_hostif_set_attr_trap_port_list
(
    IN sai_uint32_t hostif_trapid, 
    IN sai_uint32_t port1,
    IN sai_uint32_t port2        
)
{    
    sai_attribute_t attr_list;    
    
    printf("mrvl_sai_wrap_hostif_set_attr_trap_port_list: hostif_trapid:%d port1:%d port2:%d\n",
           hostif_trapid, port1,port2);
       
    if (port1 == -1 && port2 == -1) {        
        attr_list.value.objlist.count = 0; 
    }
    else if (port1 == -1) {
        sai_hostif_ports_list[0] = port2;
        attr_list.value.objlist.count = 1; 
    }
    else if(port2 == -1) {
        sai_hostif_ports_list[0] = port1;
        attr_list.value.objlist.count = 1; 
    }
    else {
        sai_hostif_ports_list[0] = port1; 
        sai_hostif_ports_list[1] = port2; 
        attr_list.value.objlist.count = 2; 
    }

    attr_list.id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;   
    attr_list.value.objlist.list = sai_hostif_ports_list;

    return sai_hif_api->set_trap_attribute(hostif_trapid, &attr_list);   
}

/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_set_attr_trap_vlan_list 
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
int mrvl_sai_wrap_hostif_set_attr_trap_vlan_list
(
    IN sai_uint32_t hostif_trapid, 
    IN sai_uint32_t vlan1,
    IN sai_uint32_t vlan2        
)
{    
    sai_attribute_t attr_list;    
    
    printf("mrvl_sai_wrap_hostif_set_attr_trap_vlan_list: hostif_trapid:%d vlan1:%d vlan2:%d\n",
           hostif_trapid, vlan1,vlan2);
       
    if (vlan1 == -1 && vlan2 == -1) {        
        attr_list.value.objlist.count = 0; 
    }
    else if (vlan1 == -1) {
        sai_hostif_vlans_list[0] = vlan2;
        attr_list.value.objlist.count = 1; 
    }
    else if(vlan2 == -1) {
        sai_hostif_vlans_list[0] = vlan1;
        attr_list.value.objlist.count = 1; 
    }
    else {
        sai_hostif_vlans_list[0] = vlan1; 
        sai_hostif_vlans_list[1] = vlan2; 
        attr_list.value.objlist.count = 2; 
    }

    attr_list.id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;    
    attr_list.value.objlist.list = sai_hostif_vlans_list;

    return sai_hif_api->set_trap_attribute(hostif_trapid, &attr_list);   
}

/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_set_attr_trap_action 
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
int mrvl_sai_wrap_hostif_set_attr_trap_action
(
    IN sai_hostif_trap_id_t hostif_trapid, 
    IN sai_packet_action_t action /*SAI_PACKET_ACTION_DROP=0 ,SAI_PACKET_ACTION_FORWARD=1, SAI_PACKET_ACTION_COPY=2, SAI_PACKET_ACTION_TRAP=4 */         
)
{    
    sai_attribute_t attr_list;    
    
    printf("mrvl_sai_wrap_hostif_set_attr_trap_action: hostif_trapid:%d action:%d\n",
           hostif_trapid, action);
        
    attr_list.id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_list.value.s32 = action;

    return sai_hif_api->set_trap_attribute(hostif_trapid, &attr_list);   
}

/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_set_attr_trap_channel 
*
* DESCRIPTION:
*       support only netdev
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
int mrvl_sai_wrap_hostif_set_attr_trap_channel
(
    IN sai_uint32_t hostif_trapid           
)
{    
    sai_attribute_t attr_list;    
    
    printf("mrvl_sai_wrap_hostif_set_attr_trap_channel: hostif_trapid:%d channel:%d\n",
           hostif_trapid, SAI_HOSTIF_TRAP_CHANNEL_NETDEV);            

    attr_list.id = SAI_HOSTIF_TRAP_ATTR_TRAP_CHANNEL;
    attr_list.value.s32 = SAI_HOSTIF_TRAP_CHANNEL_NETDEV;    

    return sai_hif_api->set_trap_attribute(hostif_trapid, &attr_list);       
}

/*******************************************************************************
* NAME:
*       mrvl_sai_hostif_pre_test 
*
* DESCRIPTION:
*       system init configurations:
*       1. create vlan 1,2
*       2. add ports 0,1,2,3 to 1 as tagged       
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
int mrvl_sai_hostif_pre_test
(
    void
)
{            
    sai_status_t            status;    
    sai_vlan_tagging_mode_t tag;
    sai_object_id_t         vlan_member_id, port_id;
    uint32_t                port, vlan;    
    
    
    /* create vlan 1 */
    status = sai_vlan_api->create_vlan(1);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    /* create vlan 2 */
    status = sai_vlan_api->create_vlan(2);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
        
    /* add port 0 to vlan 1 tagged */   
    vlan = 1;
    port = 0;
    tag = SAI_VLAN_PORT_TAGGED;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id);
    mrvl_sai_vlan_create_member_test(vlan, port_id, tag, &vlan_member_id);

    /* add port 1 to vlan 1 tagged */   
    vlan = 1;
    port = 1;
    tag = SAI_VLAN_PORT_TAGGED;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id);
    mrvl_sai_vlan_create_member_test(vlan, port_id, tag, &vlan_member_id);

    /* add port 2 to vlan 1 tagged */   
    vlan = 1;
    port = 2;
    tag = SAI_VLAN_PORT_TAGGED;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id);
    mrvl_sai_vlan_create_member_test(vlan, port_id, tag, &vlan_member_id);

    /* add port 3 to vlan 1 tagged */   
    vlan = 1;
    port = 3;
    tag = SAI_VLAN_PORT_TAGGED;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id);
    mrvl_sai_vlan_create_member_test(vlan, port_id, tag, &vlan_member_id);

    /* add port 4 to vlan 1 tagged */   
    vlan = 1;
    port = 4;
    tag = SAI_VLAN_PORT_TAGGED;
    mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id);
    mrvl_sai_vlan_create_member_test(vlan, port_id, tag, &vlan_member_id);

    return SAI_STATUS_SUCCESS; 

}

/*******************************************************************************
* NAME:
*       mrvl_sai_hostif_post_test
*
* DESCRIPTION:
*       system restore configurations:
*       1. remove ports 0,1,2,3 from vlan 1
*       2. delete vlan 1,2
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
int mrvl_sai_hostif_post_test 
(
    void
)
{            
    
    sai_status_t    status;    
    uint32_t        vlan, port, vlan_member;
    sai_object_id_t vlan_member_id;
    /* remove ports from vlan 1 */
    vlan = 1;
    port = 0;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;

    vlan = 1;
    port = 1;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;

    vlan = 1;
    port = 2;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;

    vlan = 1;
    port = 3;
    vlan_member = MRVL_SAI_VLAN_CREATE_COOKIE_MAC(vlan, port);
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN_MEMBER, vlan_member, &vlan_member_id))
        return SAI_STATUS_FAILURE;   
    if (SAI_STATUS_SUCCESS != mrvl_sai_vlan_remove_member_test(vlan_member_id))
        return SAI_STATUS_FAILURE;

    /* remove vlan 1 */
    status = sai_vlan_api->remove_vlan(1);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: remove_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* remove vlan 2 */
    status = sai_vlan_api->remove_vlan(2);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: remove_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}


/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_set_attr_trap_vlan_list_test 
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
int mrvl_sai_wrap_hostif_set_attr_trap_vlan_list_test
(
    IN sai_uint32_t hostif_trapid,
    IN sai_packet_action_t action /* SAI_PACKET_ACTION_DROP=0 ,SAI_PACKET_ACTION_FORWARD=1, SAI_PACKET_ACTION_COPY=2, SAI_PACKET_ACTION_TRAP=4 */             
)
{     
    bool isSupported;
       
    printf("mrvl_sai_wrap_hostif_set_attr_trap_test: hostif_trapid:%d action:%d\n",
           hostif_trapid,action);
        
    mrvl_sai_hostif_pre_test();
    mrvl_sai_wrap_hostif_set_attr_trap_channel(hostif_trapid);
    mrvl_sai_wrap_hostif_set_attr_trap_action(hostif_trapid,action);  
    mrvl_sai_host_interface_trap_is_port_vlan_supported(hostif_trapid,&isSupported);
    if (isSupported == true) {
        mrvl_sai_wrap_hostif_set_attr_trap_vlan_list(hostif_trapid,1,2); 
    }
    
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);  

    return SAI_STATUS_SUCCESS;

}

/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_set_attr_trap_port_list_test 
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
int mrvl_sai_wrap_hostif_set_attr_trap_port_list_test
(
    IN sai_uint32_t hostif_trapid,
    IN sai_packet_action_t action /* SAI_PACKET_ACTION_DROP=0 ,SAI_PACKET_ACTION_FORWARD=1, SAI_PACKET_ACTION_COPY=2, SAI_PACKET_ACTION_TRAP=4 */             
)
{     
    bool isSupported;
       
    printf("mrvl_sai_wrap_hostif_set_attr_trap_test: hostif_trapid:%d action:%d\n",
           hostif_trapid,action);
    
    fpaSysEnableBufPrint(1);
    mrvl_sai_hostif_pre_test();
    mrvl_sai_wrap_hostif_set_attr_trap_channel(hostif_trapid);
    mrvl_sai_wrap_hostif_set_attr_trap_action(hostif_trapid,action);  
    mrvl_sai_host_interface_trap_is_port_vlan_supported(hostif_trapid,&isSupported);
    if (isSupported == true) {
        mrvl_sai_wrap_hostif_set_attr_trap_port_list(hostif_trapid,0,1); 
    }
    
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);  

    return SAI_STATUS_SUCCESS;

}

/*******************************************************************************
* NAME:
*       mrvl_sai_wrap_hostif_restore_attr_trap_test 
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
int mrvl_sai_wrap_hostif_restore_attr_trap_test
(
    IN sai_uint32_t hostif_trapid                 
)
{    
    sai_packet_action_t action;
    bool isSupported;
        
    printf("mrvl_sai_wrap_hostif_restore_attr_trap_test: hostif_trapid:%d\n",
           hostif_trapid);
    
    mrvl_sai_host_interface_trap_get_default_action(hostif_trapid,&action);
    mrvl_sai_hostif_post_test();    
    mrvl_sai_wrap_hostif_set_attr_trap_action(hostif_trapid,action); 
     mrvl_sai_host_interface_trap_is_port_vlan_supported(hostif_trapid,&isSupported);
    if (isSupported == true) {  
        mrvl_sai_wrap_hostif_set_attr_trap_port_list(hostif_trapid,-1,-1);
        mrvl_sai_wrap_hostif_set_attr_trap_vlan_list(hostif_trapid,-1,-1);
    }
     
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);  
    fpaSysEnableBufPrint(0);

    return SAI_STATUS_SUCCESS;

}

/*******************************************************************************
* NAME:
*       mrvl_sai_hostif_test 
*
* DESCRIPTION:
*       system init configurations:
*       1. create vlan 1
*       2. add ports 0 & 1 to 1 as tagged
*       3. configure ARP REQUEST trap attributes
*       4. configure STP trap attributes
*       5. restore
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
int mrvl_sai_hostif_test
(
    void
)
{        
    sai_attribute_t         attr_list[4];    
    sai_status_t            status;        
    uint32_t                attr_count;    
    
    /* pre test settings */
    status = mrvl_sai_hostif_pre_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: hostif pre-test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }           
    
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);   

    /* add trap attributes for ARP_REQUEST */    
    memset(attr_list, 0, sizeof(sai_attribute_t)*4);
    printf("\nSet port list [ports 0,1] trap attributes for ARP REQUEST\n");
    sai_hostif_ports_list[0] = 0; /* port 0 */
    sai_hostif_ports_list[1] = 1; /* port 1 */    
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[0].value.objlist.count = 2;
    attr_list[0].value.objlist.list = sai_hostif_ports_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set port list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E); 

    sai_hostif_ports_list[0] = 2; /* port 0 */
    sai_hostif_ports_list[1] = 3; /* port 1 */
    sai_hostif_ports_list[2] = 4;    
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[0].value.objlist.count = 3;
    attr_list[0].value.objlist.list = sai_hostif_ports_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set port list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E); 

    printf("\nSet channel [NETDEV] trap attributes for ARP REQUEST\n");
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_TRAP_CHANNEL;
    attr_list[0].value.s32 = SAI_HOSTIF_TRAP_CHANNEL_NETDEV;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set trap channel for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E); 

    printf("\nSet action [TRAP] trap attributes for ARP REQUEST\n");
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_list[0].value.s32 = SAI_PACKET_ACTION_TRAP;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set action attribute for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);     

    printf("\nSet vlan list [vlan 2] trap attributes for ARP REQUEST\n");
    sai_hostif_vlans_list[0] = 2; /* vlan 2 */    
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;
    attr_list[0].value.objlist.count = 1;
    attr_list[0].value.objlist.list = sai_hostif_vlans_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set vlan list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);   
        

    /* get trap attributes for ARP_REQUEST */
    printf("\nGet trap attributes for ARP REQUEST\n");
    attr_count = 0;
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[attr_count].value.objlist.count = 3;
    attr_list[attr_count].value.objlist.list = sai_hostif_ports_list;
    attr_count++; 
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_CHANNEL_NETDEV;
    attr_count++; 
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_count++;
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;    
    attr_list[attr_count].value.objlist.count = 1;
    attr_list[attr_count].value.objlist.list = sai_hostif_vlans_list;   
    attr_count++;

    status = sai_hif_api->get_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST,attr_count,attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: get trap attributes for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }     

    /* change trap attrinutes for ARP REQUEST */    
    printf("\nSet action [FORWARD] trap attributes for ARP REQUEST\n");
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_list[0].value.s32 = SAI_PACKET_ACTION_FORWARD;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set action attribute for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);    


    printf("\nSet vlan list [vlans: 1,2] trap attributes for ARP REQUEST\n");
    sai_hostif_vlans_list[0] = 1; /* vlan 1 */   
    sai_hostif_vlans_list[1] = 2; /* vlan 2 */       
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;
    attr_list[0].value.objlist.count = 2;
    attr_list[0].value.objlist.list = sai_hostif_vlans_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set vlan list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);     

    printf("\nSet port list [port 0] trap attributes for ARP REQUEST\n");
    sai_hostif_ports_list[0] = 0; /* port 0 */    
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[0].value.objlist.count = 1;
    attr_list[0].value.objlist.list = sai_hostif_ports_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set port list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }            

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);     

    /* delete trap attrinutes for ARP REQUEST */
    printf("\nDelete VLAN list for ARP REQUEST\n");  
    memset(sai_hostif_vlans_list, 0, MRVL_SAI_HOSTIF_LIST_SIZE);      
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;
    attr_list[0].value.objlist.count = 0;
    attr_list[0].value.objlist.list = sai_hostif_vlans_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: delete vlan list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);  

    printf("\nDelete PORT list for ARP REQUEST\n");  
    memset(sai_hostif_ports_list, 0, MRVL_SAI_HOSTIF_LIST_SIZE);      
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[0].value.objlist.count = 0;
    attr_list[0].value.objlist.list = sai_hostif_ports_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: delete port list for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }            

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);  

    /* get trap attributes for ARP_REQUEST */
    printf("\nGet trap attributes for ARP REQUEST\n");
    attr_count = 0;
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[attr_count].value.objlist.count = 0;
    attr_list[attr_count].value.objlist.list = sai_hostif_ports_list;
    attr_count++; 
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_CHANNEL_NETDEV;
    attr_count++; 
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_count++;
    attr_list[attr_count].id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;    
    attr_list[attr_count].value.objlist.count = 0;
    attr_list[attr_count].value.objlist.list = sai_hostif_vlans_list;   
    attr_count++;

    status = sai_hif_api->get_trap_attribute(SAI_HOSTIF_TRAP_ID_ARP_REQUEST,attr_count,attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: get trap attributes for SAI_HOSTIF_TRAP_ID_ARP_REQUEST returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }         

    printf("\nSet action [TRAP] trap attributes for STP\n");
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PACKET_ACTION;
    attr_list[0].value.s32 = SAI_PACKET_ACTION_TRAP;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_STP, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: set action attribute for SAI_HOSTIF_TRAP_ID_STP returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);   

    printf("\nSet vlan list [vlans: 1,2] trap attributes for ARP REQUEST\n");
    sai_hostif_vlans_list[0] = 1; /* vlan 1 */   
    sai_hostif_vlans_list[1] = 2; /* vlan 2 */       
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_VLAN_LIST;
    attr_list[0].value.objlist.count = 2;
    attr_list[0].value.objlist.list = sai_hostif_vlans_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_STP, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        printf("%s:%d: set vlan list for SAI_HOSTIF_TRAP_ID_STP returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E); 

    printf("\nDelete PORT list for STP\n");  
    memset(sai_hostif_ports_list, 0, MRVL_SAI_HOSTIF_LIST_SIZE);      
    attr_list[0].id = SAI_HOSTIF_TRAP_ATTR_PORT_LIST;
    attr_list[0].value.objlist.count = 0;
    attr_list[0].value.objlist.list = sai_hostif_ports_list;
    status = sai_hif_api->set_trap_attribute(SAI_HOSTIF_TRAP_ID_STP, attr_list);
    if (status == SAI_STATUS_SUCCESS) {
        printf("%s:%d: delete port list for SAI_HOSTIF_TRAP_ID_STP returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }            

    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_CONTROL_PKT_E);  

    /* post test settings */
    status = mrvl_sai_hostif_post_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: hostif post-test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }               

    return SAI_STATUS_SUCCESS; 

}


/*******************************************************************************
* NAME:
*       mrvl_sai_hostif_arp_response_pre_test 
*
* DESCRIPTION:
*       1. run fpaSysEnableBufPrint 1
*       2. run mrvl_sai_hostif_pre_test
*       3. run mrvl_sai_hostif_arp_response_pre_test 
*       4. send traffic expect traffic is trapped to CPU with CPU code CPSS_NET_ARP_REPLY_TO_ME_E
* 
*       0x0000 : 00 00 00 11 11 11 00 00 00 00 00 01 08 06 00 01
*       0x0010 : 08 00 06 04 00 02 00 00 00 00 00 01 01 01 01 01
*       0x0020 : 00 00 00 11 11 11 02 02 02 02 00 00 00 00 00 00
*       0x0030 : 00 00 00 00 00 00 00 00 00 00 00 00 9c 35 51 81 
* 
*       5. run mrvl_sai_hostif_arp_response_post_test
*       5. run mrvl_sai_hostif_post_test
*       6. run fpaSysEnableBufPrint 0
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
int mrvl_sai_hostif_arp_response_pre_test
(
    void
)
{        
        
    sai_status_t status;            
    sai_mac_t mac_addr;
    uint32_t port_vlan, rif_idx;    
    sai_router_interface_type_t interface_type;   
    bool default_mac;
    sai_packet_action_t  nbr_miss_act;    
    
    printf("Created  virtual router 0 \n");
    mrvl_sai_virtual_router_add_test(&vr_id);

    /**** create rif on vlan 1 */ 
    printf("\nCreate rif on vlan 1 mac 00:00:00:11:11:11 nbr miss trap\n");
    interface_type = SAI_ROUTER_INTERFACE_TYPE_VLAN;
    port_vlan = 1;
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

    /* send traffic and expect trap to CPU */
    /* 00000000  00 00 00 11 11 11 00 00 00 00 00 01 08 06 00 01
       00000010  08 00 06 04 00 02 00 00 00 00 00 01 01 01 01 01
       00000020  00 00 00 11 11 11 02 02 02 02 00 00 00 00 00 00
       00000030  00 00 00 00 00 00 00 00 00 00 00 00
    */           

    return SAI_STATUS_SUCCESS; 

}

/*******************************************************************************
* NAME:
*       mrvl_sai_hostif_arp_response_post_test 
*
* DESCRIPTION:
*       1. run fpaSysEnableBufPrint 1
*       2. run mrvl_sai_hostif_pre_test
*       3. run mrvl_sai_hostif_arp_response_pre_test 
*       4. send traffic expect traffic is trapped to CPU with CPU code CPSS_NET_ARP_REPLY_TO_ME_E
* 
*       0x0000 : 00 00 00 11 11 11 00 00 00 00 00 01 08 06 00 01
*       0x0010 : 08 00 06 04 00 02 00 00 00 00 00 01 01 01 01 01
*       0x0020 : 00 00 00 11 11 11 02 02 02 02 00 00 00 00 00 00
*       0x0030 : 00 00 00 00 00 00 00 00 00 00 00 00 9c 35 51 81 
* 
*       5. run mrvl_sai_hostif_arp_response_post_test
*       5. run mrvl_sai_hostif_post_test
*       6. run fpaSysEnableBufPrint 0
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
int mrvl_sai_hostif_arp_response_post_test
(
    void
)
{        
        
    sai_status_t status;             
    
    status = mrvl_sai_rif_remove_test(rif_id);
    if (status!= SAI_STATUS_ITEM_NOT_FOUND){
        printf("test failed %d\n", status);
        return status;    
    }
    printf("Delete virtual router 0 \n");
    mrvl_sai_virtual_router_remove_test(vr_id);

    return SAI_STATUS_SUCCESS; 

}

