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
* mrvl_sai_RxTx_test.c
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

extern sai_hostif_api_t* sai_hif_api;
extern sai_vlan_api_t* sai_vlan_api;
extern sai_vlan_port_t sai_vlan_port_list[SAI_MAX_NUM_OF_PORTS];
extern sai_fdb_api_t* sai_fdb_api;

#define vlan_1  1
#define port_0  0
#define port_1  1

sai_vlan_port_t port_list[2];
sai_object_id_t hif_id;    

#define PKTDUMP_MAX_BYTES       64
#define PKTDUMP_BYTES_PER_LINE  16
/*******************************************************************************
* NAME:
*       mrvl_sai_pre_test
*
* DESCRIPTION:
*       system init configurations:
*       1. create vlan_1
*       2. add ports port_0 & port_1 to vlan_1 as tagged
*       3. add fdb entry with macDA 00:00:11:22:33 port_1 vlan_1 command trap_to_cpu
*       4. create host interface
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
int mrvl_sai_pre_test 
(
    void
)
{        
    sai_attribute_t attr_list[3];
    sai_fdb_entry_t fdb_entry;
    sai_status_t status;    

    /* create vlan 1 */
    status = sai_vlan_api->create_vlan(vlan_1);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
        
    /* add port 0 to vlan 1 tagged */   
    status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_0, &port_list[0].port_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: mrvl_sai_utl_create_object returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    port_list[0].tagging_mode = SAI_VLAN_PORT_TAGGED;

    /* add port 1 to vlan 1 tagged */   
    status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_1, &port_list[1].port_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: mrvl_sai_utl_create_object returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    port_list[1].tagging_mode = SAI_VLAN_PORT_TAGGED;

    status = sai_vlan_api->add_ports_to_vlan(vlan_1, 2, port_list);    
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: add_ports_to_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    

    /* add mac 00:00:00:11:22:33 vlan 1 port 1 trap */     
    attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_FDB_ENTRY_STATIC;
    attr_list[1].id = SAI_FDB_ENTRY_ATTR_PORT_ID;
    attr_list[1].value.oid = port_list[0].port_id;    
    attr_list[2].id = SAI_FDB_ENTRY_ATTR_PACKET_ACTION;
    attr_list[2].value.s32 = SAI_PACKET_ACTION_TRAP;
    fdb_entry.vlan_id = vlan_1;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x33;
    status = sai_fdb_api->create_fdb_entry(&fdb_entry, 3, attr_list);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_fdb_entry returned with status %d \n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
            
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);         

    attr_list[0].id = SAI_HOSTIF_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
    attr_list[1].id = SAI_HOSTIF_ATTR_NAME;
    attr_list[1].value.s32 = SAI_DEFAULT_ETH_SWID_CNS;
    attr_list[2].id = SAI_HOSTIF_ATTR_RIF_OR_PORT_ID;
    attr_list[2].value.oid = port_list[0].port_id;     
    
    /* create host interface */
    status = sai_hif_api->create_hostif(&hif_id, 3, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_hostif returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS; 

}

/*******************************************************************************
* NAME:
*       mrvl_sai_post_test
*
* DESCRIPTION:
*       system restore configurations:
*       1. remove ports port_0 & port_0 from vlan_1
*       2. delete vlan_1
*       3. remove fdb entry with macDA 00:00:11:22:33 port_0 vlan_1 command trap_to_cpu
*       4. delete host interface
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
int mrvl_sai_post_test 
(
    void
)
{            
    sai_fdb_entry_t fdb_entry;
    sai_status_t status;    

    /* remove ports from vlan 1 */
    status = sai_vlan_api->remove_ports_from_vlan(vlan_1, 2, port_list);    
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: remove_ports_from_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    

    /* remove vlan 1 */
    status = sai_vlan_api->remove_vlan(vlan_1);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: remove_vlan returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    /* remove mac 00:00:00:11:22:33 vlan 1 port 0 trap */         
    fdb_entry.vlan_id = vlan_1;
    fdb_entry.mac_address[0] = 0;
    fdb_entry.mac_address[1] = 0;
    fdb_entry.mac_address[2] = 0;
    fdb_entry.mac_address[3] = 0x11;
    fdb_entry.mac_address[4] = 0x22;
    fdb_entry.mac_address[5] = 0x33;
    status = sai_fdb_api->remove_fdb_entry(&fdb_entry);
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: remove_fdb_entry returned with status %d \n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
            
    fpaLibFlowTableDump(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E);     
    
    /* remove host interface */
    status = sai_hif_api->remove_hostif(hif_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: create_hostif returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS; 
}

/*******************************************************************************
* NAME:
*       mrvl_sai_showPacket
*
* DESCRIPTION:
*       Show received/Transmitted packet
*
* INPUTS:
*      packetBuffs  - The received packet buffers list
*      buffLenArr   - List of buffer lengths for packetBuffs
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
static void mrvl_sai_showPacket
(         
    IN uint8_t                               **packetBuffs,
    IN uint32_t                              *buffLenArr    
)
{
    int numOfBuff = 1;
    int bytesShown = 0;
    int i;
    
    printf("\n Packet Received/Transmitted:\n");

    for (;numOfBuff && bytesShown < PKTDUMP_MAX_BYTES; numOfBuff--,packetBuffs++,buffLenArr++)
    {
        for (i = 0; i < *buffLenArr && bytesShown < PKTDUMP_MAX_BYTES; i++, bytesShown++)
        {
            if ((bytesShown % PKTDUMP_BYTES_PER_LINE) == 0 && bytesShown != 0)
                printf("\n");
            printf(" %02x",(*packetBuffs)[i]);
        }
    }
    if (bytesShown != 0)
        printf("\n");
}

/*******************************************************************************
* NAME:
*       mrvl_sai_Rx_test
*
* DESCRIPTION:
*       Send packet with macDA 00:00:00:11:22:33 vlan_1 port_0 from Smbsim.
*   	Packet is matched in FDB with command trap to cpu.
*   	receive RX event and print the packet.
* 
*   	scenario:
*       1. init configuration
*       2. Wait (blocked) on new RX Event
*   	3. print received packet
*       4. restore configuration
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
int mrvl_sai_Rx_test(void)
{        
    sai_attribute_t attr_list[3];    
    uint8_t buffer[SAI_MAX_MTU_CNS];
    sai_size_t buffer_size = SAI_MAX_MTU_CNS;
    uint32_t attr_count = 0; 
    sai_status_t status;    
    uint8_t*      packetBuffs[1];
    uint32_t      buffLen[1];      

    /* create init configuration */
    status = mrvl_sai_pre_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: mrvl_sai_pre_test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    
                                                
    /* wait for receive packet then print it
       need to send packet from smbsim with macDa 00:00:00:11:22:33 */
    status =  sai_hif_api->recv_packet(hif_id, &buffer, &buffer_size, &attr_count, attr_list);
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: recv_packet returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
      
    *packetBuffs = buffer;
    *buffLen = (uint32_t)buffer_size;

    /* print the received packet */
    mrvl_sai_showPacket(packetBuffs,buffLen);
   
    /* restore configuration */
    status = mrvl_sai_post_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: mrvl_sai_post_test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    

    return SAI_STATUS_SUCCESS;
}


/*******************************************************************************
* NAME:
*       mrvl_sai_Tx_test
*
* DESCRIPTION:
*       Send single packet from CPU with synchronize access and macDA 00:00:00:11:22:33
*   	vlan_1 to egress port_0. 		
*   	print the transmitted packet.
* 
*   	scenario:
*       1. init configuration
*   	2. print received packet
*   	3. transmit packet
*       4. restore configuration
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
int mrvl_sai_Tx_test(void)
{        
    sai_attribute_t attr_list[3];     
    uint8_t *buffer;   
    sai_size_t buffer_size = 64;
    uint32_t attr_count = 3; 
    sai_status_t status;    
    uint8_t*      packetBuffs[1];
    uint32_t      buffLen[1];      

    /* create init configuration */
    status = mrvl_sai_pre_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: mrvl_sai_pre_test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }       
                
    buffer = malloc(buffer_size*sizeof(uint8_t));
    if(buffer == NULL)
    {
        printf("%s:%d: malloc returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
                  
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0x11;
    buffer[4] = 0x22;
    buffer[5] = 0x33;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10]= 0;
    buffer[11]= 0x11;
    buffer[12]= 0x81;
    buffer[13]= 0;
    buffer[14]= 0;
    buffer[15]= 1;
    buffer[16]= 0x66;
    buffer[17]= 0x66; 

    *packetBuffs = buffer;
    *buffLen = (uint32_t)buffer_size;

    /* print the transmit packet */
    mrvl_sai_showPacket(packetBuffs,buffLen);

    attr_list[0].id = SAI_HOSTIF_ATTR_TYPE;
    attr_list[0].value.s32 = SAI_HOSTIF_TYPE_NETDEV;
    attr_list[1].id = SAI_HOSTIF_ATTR_NAME;
    attr_list[1].value.s32 = SAI_DEFAULT_ETH_SWID_CNS;
    attr_list[2].id = SAI_HOSTIF_ATTR_RIF_OR_PORT_ID;
    attr_list[2].value.oid = port_list[0].port_id;     

    /* set capture on port 0 smbsim */
    status = sai_hif_api->send_packet(hif_id, buffer, buffer_size, attr_count, attr_list);   
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: send_packet returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }                  

    /* restore configuration */
    status = mrvl_sai_post_test();
    if (status != SAI_STATUS_SUCCESS) {
        printf("%s:%d: mrvl_sai_post_test returned with status %d \n", 
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }    
    
    free(buffer);

    return SAI_STATUS_SUCCESS;
}


