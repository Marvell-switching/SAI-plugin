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
* mrvl_sai_port_test.c
*
* DESCRIPTION:
*       
*
* FILE REVISION NUMBER:
*       $Revision: 01 $
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

extern sai_port_api_t* sai_port_api;
sai_object_id_t switch_id = 0;
extern sai_status_t mrvl_sai_utl_fill_u32list(uint32_t *data, uint32_t count, sai_u32_list_t *list);
/*******************************************************************************
* mrvl_sai_get_port_attribute_wrap
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
int mrvl_sai_get_port_attribute_wrap
(
    IN sai_uint32_t port,
    INOUT sai_attribute_t *port_attr
)
{
    sai_object_id_t port_id;
        
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_INF("sai_port_api->get_port_attribute\n");
    return sai_port_api->get_port_attribute(port_id, 1, port_attr);
}

/*******************************************************************************
* mrvl_sai_set_port_attribute_wrap
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
int mrvl_sai_set_port_attribute_wrap
(
    IN sai_uint32_t port,
    IN sai_attribute_t *port_attr
)
{
    sai_object_id_t port_id;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_INF("sai_port_api->set_port_attribute\n");
    return sai_port_api->set_port_attribute(port_id, port_attr);
}

/*******************************************************************************
* mrvl_sai_get_port_stats_wrap
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
int mrvl_sai_get_port_stats_wrap
(
    IN sai_uint32_t port,
    IN sai_port_stat_t counter_id,
    OUT uint64_t *port_counter
)
{
    sai_object_id_t port_id;
        
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_INF("sai_port_api->get_port_stats\n");
    return sai_port_api->get_port_stats(port_id,1, &counter_id, port_counter);
}

/*******************************************************************************
* mrvl_sai_clear_port_all_stats_wrap
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
int mrvl_sai_clear_port_all_stats_wrap
(
    IN sai_uint32_t port
)
{
    sai_object_id_t port_id;
    
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }

    MRVL_SAI_LOG_INF("sai_port_api->clear_port_all_stats\n");
    return sai_port_api->clear_port_all_stats(port_id);
}

/*******************************************************************************
* mrvl_sai_get_port_operational_status_cli
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
int mrvl_sai_get_port_operational_status_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_OPER_STATUS;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d speed, status %d\n", port, status);
    } else {
        printf("port %d operational status is %s\n", 
               port,
               ((port_attr.value.s32 == SAI_PORT_OPER_STATUS_UP)?"UP":"DOWN"));
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_get_port_admin_state_cli
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
int mrvl_sai_get_port_admin_state_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_ADMIN_STATE;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d speed, status %d\n", port, status);
    } else {
        printf("port %d admin state LINK %s\n", 
               port,
               ((port_attr.value.booldata == true)?"UP":"DOWN"));
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_set_port_admin_state_cli
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
int mrvl_sai_set_port_admin_state_cli
(
    IN uint32_t port,
    IN bool     admin_up
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_ADMIN_STATE;
    port_attr.value.booldata = admin_up;

    status = mrvl_sai_set_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to set port %d admin link, status %d\n", port, status);
    }

    return status;
}

#define PORT_SUPPORTED_SPEED_LIST_LEN 10

/*******************************************************************************
* mrvl_sai_get_port_supported_speed_cli
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
int mrvl_sai_get_port_supported_speed_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;
    uint32_t        port_speed_list[PORT_SUPPORTED_SPEED_LIST_LEN];
    uint32_t ii;

    port_attr.id = SAI_PORT_ATTR_SUPPORTED_SPEED;
    port_attr.value.u32list.count = PORT_SUPPORTED_SPEED_LIST_LEN;
    port_attr.value.u32list.list = &port_speed_list[0];

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d supported speed list, status %d\n", port, status);
    } else {
        printf("port %d, supported speed list:\n", port);
        for ( ii = 0 ; ii < port_attr.value.u32list.count ; ii++ ) {
            printf("%d: %dM\n", ii+1, port_speed_list[ii]);
        }
        if (0 == ii) {
            printf("None\n");
        }
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_get_port_speed_cli
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
int mrvl_sai_get_port_speed_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_SPEED;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d speed, status %d\n", port, status);
    } else {
        printf("port %d, speed %dM\n", port, port_attr.value.u32);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_set_port_speed_cli
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
int mrvl_sai_set_port_speed_cli
(
    IN uint32_t port,
    IN uint32_t speed /*in Mbit*/
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_SPEED;
    port_attr.value.u32 = speed;

    status = mrvl_sai_set_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to set port %d speed, status %d\n", port, status);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_get_port_stats_cli
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
int mrvl_sai_get_port_stats_cli
(
    IN sai_uint32_t port,
    IN sai_port_stat_t counter_id
)
{
    sai_status_t    status;
    uint64_t    port_counter;
    
    status = mrvl_sai_get_port_stats_wrap(port, counter_id, &port_counter);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d, counter id %d, status %d\n", 
               port, counter_id, status);
    } else {
        printf("port %d, counter id %d: %lld\n", port, counter_id, 
               (unsigned long long int)port_counter);
    }

    return status;
}

/* Counters list definition - each counter to be printed in the port cli,   */
/* should be added only ones to this definition - next definitions will     */
/* extract array of counters IDs, counters names (as string) and place for  */
/* counters values.*/
#define FOREACH_COUNTER(COUNTER_OPER) \
           COUNTER_OPER(SAI_PORT_STAT_IF_IN_OCTETS)                         \
           COUNTER_OPER(SAI_PORT_STAT_IF_IN_UCAST_PKTS)                     \
           COUNTER_OPER(SAI_PORT_STAT_IF_IN_NON_UCAST_PKTS)                 \
           COUNTER_OPER(SAI_PORT_STAT_IF_IN_ERRORS)                         \
           COUNTER_OPER(SAI_PORT_STAT_IF_IN_BROADCAST_PKTS)                 \
           COUNTER_OPER(SAI_PORT_STAT_IF_IN_MULTICAST_PKTS)                 \
           COUNTER_OPER(SAI_PORT_STAT_IF_OUT_OCTETS)                        \
           COUNTER_OPER(SAI_PORT_STAT_IF_OUT_UCAST_PKTS)                    \
           COUNTER_OPER(SAI_PORT_STAT_IF_OUT_NON_UCAST_PKTS)                \
           COUNTER_OPER(SAI_PORT_STAT_IF_OUT_BROADCAST_PKTS)                \
           COUNTER_OPER(SAI_PORT_STAT_IF_OUT_MULTICAST_PKTS)                \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_DROP_EVENTS)              \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_MULTICAST_PKTS)           \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_BROADCAST_PKTS)           \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_UNDERSIZE_PKTS)           \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_FRAGMENTS)                \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_PKTS_64_OCTETS)           \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_PKTS_65_TO_127_OCTETS)    \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_PKTS_128_TO_255_OCTETS)   \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_PKTS_256_TO_511_OCTETS)   \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_PKTS_512_TO_1023_OCTETS)  \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_PKTS_1024_TO_1518_OCTETS) \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_OVERSIZE_PKTS)            \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_RX_OVERSIZE_PKTS)               \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_JABBERS)                  \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_COLLISIONS)               \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_STATS_CRC_ALIGN_ERRORS)         \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_IN_PKTS_64_OCTETS)              \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_IN_PKTS_65_TO_127_OCTETS)       \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_IN_PKTS_128_TO_255_OCTETS)      \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_IN_PKTS_256_TO_511_OCTETS)      \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_IN_PKTS_512_TO_1023_OCTETS)     \
           COUNTER_OPER(SAI_PORT_STAT_ETHER_IN_PKTS_1024_TO_1518_OCTETS)
            

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_ZERO_VAL(HOLDER) 0,

/* Creating counters IDs array */
static const sai_port_stat_t counter_id[] = {
    FOREACH_COUNTER(GENERATE_ENUM)
};

/* Creating counters names array */
static const char *counter_str[] = {
    FOREACH_COUNTER(GENERATE_STRING)
};

/* Creating counters data array */
uint64_t port_counter[] = {
    FOREACH_COUNTER(GENERATE_ZERO_VAL)
};

/*******************************************************************************
* mrvl_sai_get_port_counters_cli
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
int mrvl_sai_get_port_counters_cli
(
    IN sai_uint32_t port
)
{
    sai_status_t    status;
    sai_object_id_t port_id;
    uint32_t        num_of_counters;
    uint32_t        ii;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id)) {
        return SAI_STATUS_FAILURE;
    }

    num_of_counters = sizeof(counter_id)/sizeof(sai_port_stat_t);
    
    status = sai_port_api->get_port_stats(port_id, num_of_counters, counter_id, port_counter);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d counters, status %d\n", port, status);
        return status;
    }

    printf("port %2d counters:\n", port);
    printf("==================\n");
    for ( ii = 0 ; ii < num_of_counters ; ii++ ) {
        printf("%-60s %10lld\n", counter_str[ii], 
               (unsigned long long int)port_counter[ii]);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_clear_port_all_stats_cli
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
int mrvl_sai_clear_port_all_stats_cli
(
    IN sai_uint32_t port
)
{
    sai_status_t    status;
    
    status = mrvl_sai_clear_port_all_stats_wrap(port);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to clear port %d statistics, status %d\n", port, status);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_get_port_fc_cli
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
int mrvl_sai_get_port_fc_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d fc, status %d\n", port, status);
    } else {
        printf("port %d, flow control state %d "
               "(%d-disable/%d-only tx/%d-only rx/%d-enable(rx&tx)\n",
               port, port_attr.value.s32,
               SAI_PORT_FLOW_CONTROL_MODE_DISABLE,
               SAI_PORT_FLOW_CONTROL_MODE_TX_ONLY,
               SAI_PORT_FLOW_CONTROL_MODE_RX_ONLY,
               SAI_PORT_FLOW_CONTROL_MODE_BOTH_ENABLE);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_set_port_fc_cli
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
int mrvl_sai_set_port_fc_cli
(
    IN uint32_t port,
    IN int32_t  fc
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_GLOBAL_FLOW_CONTROL_MODE;
    port_attr.value.s32 = fc;

    status = mrvl_sai_set_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to set port %d fc, status %d\n", port, status);
    }

    return status;
}
#if 0
/*******************************************************************************
* mrvl_sai_get_port_fdb_learning_cli
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
int mrvl_sai_get_port_fdb_learning_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_FDB_LEARNING;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d FDB learning mode, status %d\n", port, status);
    } else {
        printf("port %d, FDB learning mode %d "
               "(%d-drop/%d-fwd/%d-hw/%d-cpu_trap/%d-cpu_log\n",
               port, port_attr.value.s32,
               SAI_PORT_LEARN_MODE_DROP,
               SAI_PORT_LEARN_MODE_DISABLE,
               SAI_PORT_LEARN_MODE_HW,
               SAI_PORT_LEARN_MODE_CPU_TRAP,
               SAI_PORT_LEARN_MODE_CPU_LOG);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_set_port_fdb_learning_cli
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
int mrvl_sai_set_port_fdb_learning_cli
(
    IN uint32_t port,
    IN int32_t  learn_mode
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_FDB_LEARNING;
    port_attr.value.s32 = learn_mode;

    status = mrvl_sai_set_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to set port %d FDB learning mode, status %d\n", port, status);
    }

    return status;
}
#endif
/*******************************************************************************
* mrvl_sai_get_port_default_vlan_cli
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
int mrvl_sai_get_port_default_vlan_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_PORT_VLAN_ID;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d default VLAN, status %d\n", port, status);
    } else {
        printf("port %d, default VLAN %d \n", port, port_attr.value.u16);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_set_port_default_vlan_cli
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
int mrvl_sai_set_port_default_vlan_cli
(
    IN uint32_t port,
    IN uint16_t vlan
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_PORT_VLAN_ID;
    port_attr.value.u16 = vlan;

    status = mrvl_sai_set_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to set port %d default VLAN, status %d\n", port, status);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_get_port_deafult_vlan_priority_cli
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
int mrvl_sai_get_port_deafult_priority_vlan_cli
(
    IN uint32_t port
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;

    status = mrvl_sai_get_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to get port %d default VLAN priority, status %d\n", port, status);
    } else {
        printf("port %d, default VLAN priority %d\n", port, port_attr.value.u8);
    }

    return status;
}

/*******************************************************************************
* mrvl_sai_set_port_default_vlan_priority_cli
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
int mrvl_sai_set_port_default_vlan_priority_cli
(
    IN uint32_t port,
    IN uint8_t  up
)
{
    sai_status_t    status;
    sai_attribute_t port_attr;

    port_attr.id = SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY;
    port_attr.value.u8 = up;

    status = mrvl_sai_set_port_attribute_wrap(port, &port_attr);
    if (SAI_STATUS_SUCCESS != status) {
        printf("failed to set port %d default VLAN priority, status %d\n", port, status);
    }

    return status;
}


/*******************************************************************************
* mrvl_sai_port_tests
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
int mrvl_sai_port_tests (void)
{
    sai_object_id_t     port_oid;
    uint32_t            port_idx;
    sai_attribute_t     attr_list[4];
    uint32_t            attr_count, i;   
    sai_status_t        status = SAI_STATUS_SUCCESS;
    uint32_t            port_speed_ARR[2] = {10, 100};

    /*printf("\n\n---------------------mrvl_sai_lag_deletion_test---------------------\n");
    printf("1. Create empty lag\n");
    printf("2. Add 2 ports to the LAG as lag members\n");
    printf("3. Get LAG attributes\n");
    printf("4. Remove LAG members from LAG\n");
    printf("5. Remove LAG\n");
    printf("Between each step, dump LAG tables from FPA and HW\n");*/

    /* create ports */
    for (i = 0; i < 2; i++)
    {
        attr_count = 0; 
        memset(attr_list, 0, sizeof(sai_attribute_t)*4);
        /*attr_list[0].id = SAI_PORT_ATTR_HW_LANE_LIST;
        attr_list[0].value.u32list.count = 1;
        attr_list[0].value.u32list.list = calloc(1, sizeof(uint32_t));
        attr_count++;*/
        attr_list[attr_count].id = SAI_PORT_ATTR_SPEED;
        attr_list[attr_count].value.u32 = port_speed_ARR[i];
        attr_count++;
        attr_list[attr_count].id = SAI_PORT_ATTR_INGRESS_ACL;
        attr_list[attr_count].value.oid = SAI_NULL_OBJECT_ID;
        attr_count++;
        attr_list[attr_count].id = SAI_PORT_ATTR_EGRESS_ACL;
        attr_list[attr_count].value.oid = SAI_NULL_OBJECT_ID;
        attr_count++;
        status = sai_port_api->create_port(&port_oid, switch_id, attr_count, attr_list);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Create port failed\n");
            return status;
        }
    
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_object_to_type(port_oid, SAI_OBJECT_TYPE_PORT, &port_idx))) {
            MRVL_SAI_LOG_ERR("Convert object PORT to type failed\n");
            return status;
        }

        MRVL_SAI_LOG_NTC("Created port %d\n", port_idx);
        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_set_port_admin_state_cli(port_idx, true))) {
            MRVL_SAI_LOG_ERR("Failed to set port %d admin state UP\n", port_idx);
            return status;
        }

        if (SAI_STATUS_SUCCESS != (status = mrvl_sai_set_port_speed_cli(port_idx, 1000))) {
            MRVL_SAI_LOG_ERR("Failed to set port %d speed to 1G\n", port_idx);
            return status;
        }
    }
    return status;
}

