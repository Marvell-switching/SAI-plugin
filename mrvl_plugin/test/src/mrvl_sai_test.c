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
* mrvl_sai_test.c
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
#include    <time.h>
#include    <sys/time.h> 

#include	"fpaLibApis.h"
#include    "sai.h"
#include    "mrvl_sai.h"

bool sai_test_init_done = false; 
uint32_t waitForReceiveEvent = 1;
uint32_t startTimerRxPackets = 0;
uint32_t countRxPackets = 0;

uint32_t print_au_event = 0;

extern sai_hostif_api_t* sai_hif_api;

sai_status_t mrvl_sai_print_full_au_event(int enable)
{    
   print_au_event = enable;
   return SAI_STATUS_SUCCESS;
}

sai_status_t saiWaitForReceiveEventSet(int enable)
{    
   waitForReceiveEvent = enable;

   return SAI_STATUS_SUCCESS;
}

sai_status_t saiStartTimerRxPacketsSet(int numOfPackets)
{    
   startTimerRxPackets = numOfPackets;
   countRxPackets = 0;

   return SAI_STATUS_SUCCESS;
}

extern uint32_t osTaskCreate
(
    IN  char    *name,
    IN  uint32_t  prio,
    IN  uint32_t  stack,
    IN  unsigned (*start_addr)(void*),
    IN  void    *arglist,
    OUT uint32_t *tid
);

/*--------------------------------------------------------*/
//definition of the api tables
sai_switch_api_t* sai_switch_api;
sai_port_api_t* sai_port_api;
sai_vlan_api_t* sai_vlan_api;
sai_hostif_api_t* sai_hif_api;
sai_fdb_api_t* sai_fdb_api;
sai_router_interface_api_t* sai_rif_api;
sai_acl_api_t* sai_acl_api;
sai_neighbor_api_t* sai_neighbor_api;
sai_next_hop_api_t* sai_nexthop_api;
sai_route_api_t* sai_route_api;
sai_virtual_router_api_t* sai_virtual_router_api;

sai_vlan_port_t sai_vlan_port_list[SAI_MAX_NUM_OF_PORTS];


void mrvl_sai_test_fdb_event_notification(
    _In_ uint32_t count,
    _In_ sai_fdb_event_notification_data_t *data)
{
    uint32_t i, j;
    uint32_t port;
    sai_attribute_t   *fdb_attr;

    if (print_au_event == 1) {
        printf("Got Address Update event. count = %d\n", count);
        for (i = 0; i < count; i++) {
            printf("Message #%d\n", i);
            printf("MAC %02x:%02x:%02x:%02x:%02x:%02x\n", 
                   data->fdb_entry.mac_address[0],
                   data->fdb_entry.mac_address[1],
                   data->fdb_entry.mac_address[2],
                   data->fdb_entry.mac_address[3],
                   data->fdb_entry.mac_address[4],
                   data->fdb_entry.mac_address[5]
                   );
            printf("VLAN %d\n", data->fdb_entry.vlan_id);
            printf("Event %s\n", MRVL_SAI_FDB_EVENT_TO_STR_MAC(data->event_type));
            for (j=0; j<data->attr_count; j++) {
                fdb_attr = &(data->attr[j]);
                if (fdb_attr->id == SAI_FDB_ENTRY_ATTR_PORT_ID) {
                    mrvl_sai_utl_object_to_type(fdb_attr->value.oid, SAI_OBJECT_TYPE_PORT, &port);
                    printf("Port number %d\n", port); 
                } else if (fdb_attr->id == SAI_FDB_ENTRY_ATTR_PACKET_ACTION) {
                    printf("Packet Action %d\n", fdb_attr->value.u32);
                } else if (fdb_attr->id == SAI_FDB_ENTRY_ATTR_TYPE) {
                    printf("Entry Type %d\n", fdb_attr->value.u32);
                }
            }
        }
    }
}

sai_switch_notification_t mrvl_sai_plat_switch_notification_handlers =
{
    NULL,
    mrvl_sai_test_fdb_event_notification,
    NULL,
    NULL,
    NULL,
    NULL
};

/*--------------------------------------------------------*/
//Profile Services

const char* mrvl_sai_test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    UNREFERENCED_PARAMETER(profile_id);
    UNREFERENCED_PARAMETER(variable);

    return NULL;
}

int mrvl_sai_test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    UNREFERENCED_PARAMETER(profile_id);
    UNREFERENCED_PARAMETER(variable);
    UNREFERENCED_PARAMETER(value);

    return -1;
}

const service_method_table_t mrvl_sai_test_services =
{
    mrvl_sai_test_profile_get_value,
    mrvl_sai_test_profile_get_next_value
};

/*******************************************************************************
* mrvl_sai_test_Initialize
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
*       FPA_OK                    - Operation succeeded
*       FPA_FAIL                  - Operation failed
*
* COMMENTS:
*
*******************************************************************************/
int mrvl_sai_test_Initialize
(
    IN void
)
{
	sai_status_t status;
    /*uint32_t attr_count;*/
    sai_attribute_t attr_list[3];
    
    printf("mrvl_sai_test_Initialize\n");
    status = sai_api_initialize(0, (service_method_table_t *)&mrvl_sai_test_services);
    if (status != SAI_STATUS_SUCCESS) {
        fprintf(stderr, "SAI API  initialize failed\n");
        return(1);
    }

    printf("sai_api_query SAI_API_SWITCH, SAI_API_PORT, ...\n"); 
    status = sai_api_query(SAI_API_SWITCH, (void**)&sai_switch_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_switch_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_SWITCH failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_PORT, (void**)&sai_port_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_port_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_PORT failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_FDB, (void**)&sai_fdb_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_fdb_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_FDB failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_VLAN, (void**)&sai_vlan_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_vlan_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_VLAN failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_ROUTER_INTERFACE, (void**)&sai_rif_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_rif_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_ROUTER_INTERFACE failed\n");
        return(1);
    }
    
    status = sai_api_query(SAI_API_ACL, (void**)&sai_acl_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_acl_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_ACL failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_NEIGHBOR, (void**)&sai_neighbor_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_neighbor_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_NEIGHBOR failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_NEXT_HOP, (void**)&sai_nexthop_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_nexthop_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_NEXT_HOP failed\n");
        return(1);
    }
    status = sai_api_query(SAI_API_ROUTE, (void**)&sai_route_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_route_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_ROUTE failed\n");
        return(1);
    }
    
    status = sai_api_query(SAI_API_VIRTUAL_ROUTER, (void**)&sai_virtual_router_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_virtual_router_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_VIRTUAL_ROUTER failed\n");
        return(1);
    }

    status = sai_api_query(SAI_API_HOST_INTERFACE, (void**)&sai_hif_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_hif_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_HOST_INTERFACE failed\n");
        return(1);
    }

    printf("sai_switch_api->initialize_switch \n");
    if(sai_switch_api->initialize_switch == NULL){
        fprintf(stderr, "sai_switch_api->initialize_switch is NULL\n");
        return(1);
    }

    status = sai_switch_api->initialize_switch(0, "0x1234", "", &mrvl_sai_plat_switch_notification_handlers);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "initialize_switch failed %d\n", status);
        return(1);
    }
    
    /* init switch mac address*/
    attr_list[0].id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
    attr_list[0].value.mac[0] = 0;
    attr_list[0].value.mac[1] = 0;
    attr_list[0].value.mac[2] = 0;
    attr_list[0].value.mac[3] = 0x11;
    attr_list[0].value.mac[4] = 0x22;
    attr_list[0].value.mac[5] = 0;
    status = sai_switch_api->set_switch_attribute(attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_SRC_MAC_ADDRESS failed %d\n", status);
        return(1);
    }
    
    /* init aging timeout */
    attr_list[0].id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    attr_list[0].value.u32 = 30;
    status = sai_switch_api->set_switch_attribute(attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_FDB_AGING_TIME failed %d\n", status);
        return(1);
    }
    
	return SAI_STATUS_SUCCESS;
}


static unsigned adaptationTask(void * param)
{
   uint32_t counter = 0;

   /* waiting till fpa init is done */
   sleep(2);

   while (!fpa_init_done) {
      sleep(1);
	  counter++;
   }
   /* System ready */

   mrvl_sai_test_Initialize();
   sai_test_init_done = true;

   return 0;
}

static unsigned waitOnRxTask(void * param)
{
    uint32_t counter = 0;
    sai_attribute_t attr_list[3];    
    uint8_t buffer[SAI_MAX_MTU_CNS];
    sai_size_t buffer_size = SAI_MAX_MTU_CNS;
    uint32_t attr_count = 0;
    sai_status_t status; 
    sai_object_id_t host_id;       
    struct timeval start, end;  /* time for receiving Rx packets from CPU in microseconds */          
   
   /* waiting till sai tests init is done */
   sleep(2);
   while (!sai_test_init_done) {
      sleep(1);
	  counter++;
   }
   if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HOST_INTERFACE, 1, &host_id))) {
       return status;
   }

   /* wait on Rx event */
   while (1) {
       if (waitForReceiveEvent) {  
           buffer_size = SAI_MAX_MTU_CNS;
           attr_count = 0;         
           status =  sai_hif_api->recv_packet(host_id /*hif_id*/, &buffer, &buffer_size, &attr_count, attr_list);
           if (status != SAI_STATUS_SUCCESS) {
               printf("%s:%d: recv_packet returned with status %d \n", 
                      __func__, __LINE__, status);
               
           }
           else if (startTimerRxPackets) {
               if (countRxPackets == 0) {                     
                   gettimeofday(&start, NULL);                                   
                   countRxPackets++;                            
               }
               else if (countRxPackets++ == startTimerRxPackets) {                    
                   gettimeofday(&end, NULL);                                          

                   printf("\n%s:%d: total time receiving %d packets from CPU in microseconds [%ld]\n",
                           __func__, __LINE__, startTimerRxPackets, ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));  

                   startTimerRxPackets = 0;                                     

               }               
           }
       } else {
            sleep(2);
       }
   }
   
   return 0;
}
 
int main(int argc, char *argv[], char *envp[])
{
	uint32_t rc;
	uint32_t task_id;
	
	rc = osTaskCreate("sait", /* Task Name      */
					  250, /* Task Priority  */
					  0x8000, /* Stack Size     */
					  adaptationTask, /* Starting Point */
					  NULL, /* Arguments list */
					  &task_id); /* task ID        */
	if (rc != 0) {
		return rc;
	}

    rc = osTaskCreate("saiRx", /* Task Name      */
					  200, /* Task Priority  */
					  0x8000, /* Stack Size     */
					  waitOnRxTask, /* Starting Point */
					  NULL, /* Arguments list */
					  &task_id); /* task ID        */
	if (rc != 0) {
		return rc;
	}

	fpa_main(argc, argv, envp);
	return 0;
}
