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

#define TTY_RESTORE_ENABLED

#ifdef TTY_RESTORE_ENABLED
#include <termios.h>
#include <unistd.h>
#endif

#include	"fpaLibApis.h"
#include    "sai.h"
#include    "mrvl_sai.h"

static sai_object_id_t switch_idx = 0;

extern FPA_PORT_MAP_STC bobkMap_12x10G[];
extern FPA_PORT_MAP_STC bc3Map_48x10G_6x100G[];

extern FPA_PORT_CONFIG_STC sai_ports_configuration[];
extern FPA_PORT_CONFIG_STC sai_ports_configuration_BC3[];

extern uint32_t sai_ports_configuration_numOfPorts;
extern uint32_t sai_ports_configuration_numOfPorts_BC3;

bool sai_test_init_done = false; 
uint32_t waitForReceiveEvent = 1;
uint32_t startTimerRxPackets = 0;
uint32_t countRxPackets = 0;

uint32_t print_au_event = 0;

extern sai_hostif_api_t* sai_hif_api;

uint32_t SAI_SYS_TEST_PORT_MAPPING[SAI_MAX_NUM_OF_PORTS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
   48,49,50,51,52,53 /*,54,55,56,57,58,59,64,65,66,67,68,69,70,71,80,81,82,83*/
};
uint32_t  *saiSysTestPortMappingPtr=SAI_SYS_TEST_PORT_MAPPING;

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
sai_next_hop_group_api_t* sai_nexthop_group_api;
sai_route_api_t* sai_route_api;
sai_virtual_router_api_t* sai_virtual_router_api;
sai_lag_api_t* sai_lag_api;
sai_bridge_api_t* sai_bridge_api;


void mrvl_sai_test_fdb_event_notification(
    _In_ uint32_t count,
    _In_ sai_fdb_event_notification_data_t *data)
{
    uint32_t i, j;
    uint32_t port;
    sai_attribute_t   *fdb_attr;

    MRVL_SAI_LOG_ENTER();

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
                if (fdb_attr->id == SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID) {
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
    MRVL_SAI_LOG_EXIT();
}


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

static void mrvl_sai_switch_id_to_str(_In_ sai_object_type_t type, _In_ sai_object_id_t id, _Out_ char *object_id_str)
{
    uint32_t switch_idx;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(id, type, &switch_idx)) {
        snprintf(object_id_str, MAX_KEY_STR_LEN, "invalid switch");
    } else {
        snprintf(object_id_str, MAX_KEY_STR_LEN, "switch %u", switch_idx);
    }
}

static void port_id_to_str(_In_ sai_object_id_t port_id, _Out_ char *key_str)
{
    uint32_t port;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_id, SAI_OBJECT_TYPE_PORT, &port)) {
        snprintf(key_str, MAX_KEY_STR_LEN, "invalid port");
    } else {
        snprintf(key_str, MAX_KEY_STR_LEN, "port %x", port);
    }
}

/*******************************************************************************
* mrvl_sai_switch_test
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
int mrvl_sai_switch_test(void)
{
    sai_status_t status;
    uint32_t attr_count = 0, mode;
    sai_attribute_t attr_list[10];
    sai_object_id_t switch_id;
    char          switch_str[MAX_LIST_VALUE_STR_LEN];
    char          port_str[MAX_LIST_VALUE_STR_LEN];
    char           value_str[MAX_VALUE_STR_LEN];
    uint32_t     i;
    FPA_MAC_ADDRESS_STC src_mac;
    FPA_STATUS          fpa_status;

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, switch_idx, &switch_id))) {
        fprintf(stderr, "Failed to create SAI_OBJECT_TYPE_SWITCH %d\n", SAI_DEFAULT_ETH_SWID_CNS);
        return status;
    }

    mrvl_sai_switch_id_to_str(SAI_OBJECT_TYPE_SWITCH, switch_id, switch_str);
    MRVL_SAI_LOG_NTC("Testing %s\n", switch_str);
    /*MRVL_SAI_LOG_DBG("Get SAI_SWITCH_ATTR_INIT_SWITCH, attr_count %d\n", attr_count); 
    attr_list[attr_count].id = SAI_SWITCH_ATTR_INIT_SWITCH;
    attr_count++;
    attr_list[attr_count].id = SAI_SWITCH_ATTR_SWITCH_PROFILE_ID;
    status = sai_switch_api->get_switch_attribute(switch_id, 2, attr_list);
    if (status != SAI_STATUS_SUCCESS)
    {
        fprintf(stderr, "get SAI_SWITCH_ATTR_INIT_SWITCH failed %d\n", status);
        return(1);
    }
    if (true == attr_list[0].value.booldata)
    {
        MRVL_SAI_LOG_DBG("%s already exists with profile id %d. Removing switch\n", switch_str, attr_list[1].value.u32);
        status = sai_switch_api->remove_switch(switch_id);
        if (status != SAI_STATUS_SUCCESS)
        {
            fprintf(stderr, "Remove switch failed, status %d\n", status);
            return(1);
        }
        MRVL_SAI_LOG_DBG("%s removed. Initializing again\n", switch_str);
    }

    MRVL_SAI_LOG_DBG("Set SAI_SWITCH_ATTR_INIT_SWITCH\n"); 
    status = sai_switch_api->set_switch_attribute(switch_id, attr_list); 
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_INIT_SWITCH failed %d\n", status);
        return(1);
    }
    attr_count++;*/

    fpa_status = fpaLibSwitchSrcMacAddressGet(switch_idx, &mode, &src_mac);
    memcpy(attr_list[attr_count].value.mac, src_mac.addr, FPA_MAC_ADDRESS_SIZE);
    mrvl_sai_utl_value_to_str(attr_list[attr_count].value, SAI_ATTR_VAL_TYPE_MAC, MAX_VALUE_STR_LEN, value_str);
    MRVL_SAI_LOG_DBG("fpaLibSwitchSrcMacAddressGet: %s\n", value_str);
    /*MRVL_SAI_LOG_DBG("Get SAI_SWITCH_ATTR_SRC_MAC_ADDRESS\n"); */
    attr_list[attr_count].id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
    status = sai_switch_api->get_switch_attribute(switch_id, 1, attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "get SAI_SWITCH_ATTR_SRC_MAC_ADDRESS failed %d\n", status);
        return(1);
    }
    mrvl_sai_utl_value_to_str(attr_list[attr_count].value, SAI_ATTR_VAL_TYPE_MAC, MAX_VALUE_STR_LEN, value_str);
    MRVL_SAI_LOG_DBG("Got SAI_SWITCH_ATTR_SRC_MAC_ADDRESS: %s\n", value_str); 
    attr_count++;

    MRVL_SAI_LOG_DBG("Get SAI_SWITCH_ATTR_PORT_LIST, attr_count %d\n", attr_count); 
    attr_list[attr_count].id = SAI_SWITCH_ATTR_PORT_LIST;
    status = sai_switch_api->get_switch_attribute(switch_id, 1, attr_list);
    if (status != SAI_STATUS_SUCCESS)
    {
        fprintf(stderr, "get SAI_SWITCH_ATTR_PORT_LIST failed %d\n", status);
        return(1);
    }
    if (0 == attr_list[attr_count].value.objlist.count)
    {
        attr_list[attr_count].value.objlist.count = SAI_MAX_NUM_OF_PORTS;
        attr_list[attr_count].value.objlist.list = (sai_object_id_t*)calloc(SAI_MAX_NUM_OF_PORTS, sizeof(sai_object_id_t));

        MRVL_SAI_LOG_DBG("%s contains no ports. Generating new port list for %d ports\n", switch_str, SAI_MAX_NUM_OF_PORTS); 
        for (i=0; i < SAI_MAX_NUM_OF_PORTS; i++)
        {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, saiSysTestPortMappingPtr[i], &attr_list[attr_count].value.objlist.list[i]))) 
            {
                fprintf(stderr, "Failed to create port list, for port %d\n", saiSysTestPortMappingPtr[i]);
                return(1);
            }
        }
        /*mrvl_sai_utl_fill_objlist(attr_list[attr_count].value.objlist.list, SAI_MAX_NUM_OF_PORTS, &attr_list[attr_count].value.objlist);*/
    }
    else if (NULL == attr_list[attr_count].value.objlist.list) 
    {
        if (0 == attr_list[attr_count].value.objlist.count)
            attr_list[attr_count].value.objlist.count = SAI_MAX_NUM_OF_PORTS;
        
        MRVL_SAI_LOG_DBG("Port list is empty. Generating list for %d ports\n", attr_list[attr_count].value.objlist.count); 
        for (i=0; i < attr_list[attr_count].value.objlist.count; i++) {
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, saiSysTestPortMappingPtr[i], &attr_list[attr_count].value.objlist.list[i])))
            {
                fprintf(stderr, "Failed to create port list, for port %d\n", saiSysTestPortMappingPtr[i]);
            	return(1);
            }
        }
    }
    MRVL_SAI_LOG_DBG("The switch contains %d ports\n", attr_list[attr_count].value.objlist.count);
    MRVL_SAI_LOG_DBG("Port list created:\n"); 
    for (i=0; i < attr_list[attr_count].value.objlist.count; i++)
    {
        port_id_to_str(attr_list[attr_count].value.objlist.list[i], port_str);
        MRVL_SAI_LOG_DBG("Sys port %d mapped to switch port %s\n", saiSysTestPortMappingPtr[i], port_str); 
    }

    attr_count = 0;
    memset(&attr_list, 0, 10*sizeof(attr_list));
    MRVL_SAI_LOG_DBG("Set SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY, attr_count %d\n", attr_count); 
    attr_list[attr_count].id = SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY;
    attr_list[attr_count].value.ptr = mrvl_sai_test_fdb_event_notification;
    status = sai_switch_api->set_switch_attribute(switch_id, attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY failed %d\n", status); 
        return(1);
    }
    attr_count++;

    attr_count = 1;
    /* init switch mac address*/
    attr_list[attr_count].id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
    attr_list[attr_count].value.mac[0] = 0;
    attr_list[attr_count].value.mac[1] = 0;
    attr_list[attr_count].value.mac[2] = 0;
    attr_list[attr_count].value.mac[3] = 0x11;
    attr_list[attr_count].value.mac[4] = 0x22;
    attr_list[attr_count].value.mac[5] = 0x33;
    mrvl_sai_utl_value_to_str(attr_list[attr_count].value, SAI_ATTR_VAL_TYPE_MAC, MAX_VALUE_STR_LEN, value_str);
    MRVL_SAI_LOG_DBG("Set SAI_SWITCH_ATTR_SRC_MAC_ADDRESS: %s\n", value_str); 
    /*status = sai_switch_api->set_switch_attribute(switch_id, attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_SRC_MAC_ADDRESS failed %d\n", status);
        return(1);
    }*/
    memcpy(src_mac.addr, attr_list[attr_count].value.mac, FPA_MAC_ADDRESS_SIZE);
    fpa_status = fpaLibSwitchSrcMacAddressSet(switch_idx, 0, &src_mac);
    MRVL_SAI_LOG_DBG("Set SAI_SWITCH_ATTR_SRC_MAC_ADDRESS: %d:%d:%d:%x:%x:%x\n", src_mac.addr[0], src_mac.addr[1], src_mac.addr[2],
                     src_mac.addr[3], src_mac.addr[4], src_mac.addr[5]); 
    if (fpa_status != FPA_OK) {
        MRVL_SAI_LOG_DBG("fpaLibSwitchSrcMacAddressSet failed, mode %d, status %d\n", SAI_SWITCH_DEFAULT_MAC_MODE_CNS, fpa_status);
    	MRVL_SAI_LOG_EXIT();
        return SAI_STATUS_FAILURE;
    }
    attr_count++;

    /* init aging timeout */
    attr_list[attr_count].id = SAI_SWITCH_ATTR_FDB_AGING_TIME;
    attr_list[attr_count].value.u32 = 30;
    MRVL_SAI_LOG_DBG("Set SAI_SWITCH_ATTR_FDB_AGING_TIME, attr_count %d\n", attr_count); 
    status = sai_switch_api->set_switch_attribute(switch_id, attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_FDB_AGING_TIME failed %d\n", status);
        return(1);
    }
  
    MRVL_SAI_LOG_DBG("Get SAI_SWITCH_ATTR_SRC_MAC_ADDRESS\n"); 
    attr_count = 0;
    attr_list[attr_count].id = SAI_SWITCH_ATTR_SRC_MAC_ADDRESS;
    status = sai_switch_api->get_switch_attribute(switch_id, 1, attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "get SAI_SWITCH_ATTR_SRC_MAC_ADDRESS failed %d\n", status);
        return(1);
    }
    mrvl_sai_utl_value_to_str(attr_list[attr_count].value, SAI_ATTR_VAL_TYPE_MAC, MAX_VALUE_STR_LEN, value_str);
    MRVL_SAI_LOG_DBG("Got SAI_SWITCH_ATTR_SRC_MAC_ADDRESS: %s\n", value_str); 

    return SAI_STATUS_SUCCESS;
}
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
    sai_object_id_t switch_id;
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[1];

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
    status = sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&sai_nexthop_group_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_nexthop_group_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_NEXT_HOP_GROUP failed\n");
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

    status = sai_api_query(SAI_API_HOSTIF, (void**)&sai_hif_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_hif_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_HOSTIF failed\n");
        return(1);
    }

    status = sai_api_query(SAI_API_LAG, (void**)&sai_lag_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_lag_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_LAG failed, status %d\n", status); 
    }

    status = sai_api_query(SAI_API_BRIDGE, (void**)&sai_bridge_api);
    if ((status != SAI_STATUS_SUCCESS) || (sai_bridge_api == NULL)){
        fprintf(stderr, "sai_api_query SAI_API_BRIDGE failed\n");
        return(1);
    }

    if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))) {
        return status;
    }

    attr_list[attr_count].id = SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY;
    attr_list[attr_count].value.ptr = mrvl_sai_test_fdb_event_notification;
    status = sai_switch_api->set_switch_attribute(switch_id, attr_list);
    if (status != SAI_STATUS_SUCCESS){
        fprintf(stderr, "set SAI_SWITCH_ATTR_FDB_EVENT_NOTIFY failed %d\n", status);
        return(1);
    }

	return SAI_STATUS_SUCCESS;
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
   if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_HOSTIF, 1, &host_id))) {
       return status;
   }

   /* wait on Rx event */
   while (1) {
       if (waitForReceiveEvent) {  
           buffer_size = SAI_MAX_MTU_CNS;
           attr_count = 0;         
           status =  sai_hif_api->recv_hostif_packet(host_id /*hif_id*/, &buffer, &buffer_size, &attr_count, attr_list);
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
 
typedef enum
{
    SAI_BC3 	= 1,    
    SAI_BOBK 	= 2,    

    SAI_NA        		

}SAI_BOARDS_ENT;
 

int main_test(int argc, char *argv[], char *envp[])
{
	uint32_t rc;
	uint32_t task_id;
	int counter = 0;
#ifdef ASIC_SIMULATION
	SAI_BOARDS_ENT dev_type;
#endif

#ifdef TTY_RESTORE_ENABLED
    struct termios tc_saved;

#endif

#ifdef TTY_RESTORE_ENABLED
	tcgetattr(0, &tc_saved);
#endif

#ifdef ASIC_SIMULATION
	if (argc != 1) {
		dev_type = atoi(argv[1]);
	} else {
		dev_type = SAI_BC3;
	}

	if (dev_type == SAI_BOBK) {
		setenv("INIFILE","bobk_cetus_pss_wm.ini", 1);
	} 
#endif

	mrvl_sai_test_Initialize();
	rc = osTaskCreate("saiRx", /* Task Name      */
					  200, /* Task Priority  */
					  0x8000, /* Stack Size     */
					  waitOnRxTask, /* Starting Point */
					  NULL, /* Arguments list */
					  &task_id); /* task ID        */

	if (rc != 0) {
		return rc;
	}
	sai_test_init_done = true;

#ifdef TTY_RESTORE_ENABLED
    tcsetattr(0, TCSAFLUSH, &tc_saved);
    printf("tty settings restored\n");
	while (1) {
		counter++;
        sleep(10);
	}

	printf("Counter is %d\n", counter);
#endif

	return 0;
}
