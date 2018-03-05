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
* mrvl_sai_acl_test.c
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

extern sai_acl_api_t* sai_acl_api;
extern sai_port_api_t* sai_port_api;
extern sai_lag_api_t* sai_lag_api;
extern sai_vlan_api_t* sai_vlan_api;
extern sai_switch_api_t* sai_switch_api;

#define mrvl_max_acl_attr_len MAX_LIST_VALUE_STR_LEN

static sai_object_id_t switch_id = 0;

#if 0
sai_status_t mrvl_sai_acl_print_attr(sai_object_type_t acl_object_type, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    for (i = 0; i < attr_count; i++){
        if (acl_object_type == SAI_OBJECT_TYPE_ACL_TABLE) {
            switch (attr_list[i].id) {
            case SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE:
                mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
                break;
            case SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST:
                mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_S32LIST , MAX_VALUE_STR_LEN, key_str);
                break;
            case SAI_ACL_TABLE_GROUP_ATTR_TYPE:
                mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
                break;
            default:
                printf("Attribute id %d not supported\n",attr_list[i].id);
                break;
            }
            printf("id %d\n", attr_list[i].id);
            printf("value: %s\n",key_str);
        }
    }

    return status;

}

sai_status_t mrvl_sai_acl_verify_attr(uint32_t attr_count, sai_attribute_t *set_attr_list, sai_attribute_t *get_attr_list)
{
    sai_status_t    status = SAI_STATUS_SUCCESS;
    uint32_t i;

    for (i = 0; i < attr_count; i++){
		if (set_attr_list[i].id != get_attr_list[i].id) {
            printf("attr_count %d, id set_attr_list %d, id get_attr_list %d\n", i, set_attr_list[i].id, get_attr_list[i].id);
            status = SAI_STATUS_FAILURE;
            break;
        }
        if (set_attr_list[i].value != get_attr_list[i].value) {
            printf("attr_count %d, value set_attr_list %d, value get_attr_list %d\n", i, set_attr_list[i].value, get_attr_list[i].value);
            status = SAI_STATUS_FAILURE;
            break;
        }
    }

    return status;

}
#endif

/******************ACL RANGE********************/

int mrvl_sai_acl_create_range_test(sai_object_id_t *acl_range_id, bool is_src, uint32_t start, uint32_t end)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACL RANGE: acl_range_id \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_RANGE_ATTR_TYPE;
    if (is_src) {
        attr_list[attr_count].value.u32 = SAI_ACL_RANGE_TYPE_L4_SRC_PORT_RANGE;
    }
    else {
        attr_list[attr_count].value.u32 = SAI_ACL_RANGE_TYPE_L4_DST_PORT_RANGE;
    }
    
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_RANGE_ATTR_LIMIT;
    attr_list[attr_count].value.u32range.min = start;
    attr_list[attr_count].value.u32range.max = end;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_range\n");

    status = sai_acl_api->create_acl_range(acl_range_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_acl_get_attr_range_test_prv(sai_object_id_t acl_range_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    status = sai_acl_api->get_acl_range_attribute(acl_range_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    for (i = 0; i < attr_count; i++){
		switch (attr_list[i].id) {
		case SAI_ACL_RANGE_ATTR_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_RANGE_ATTR_LIMIT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32RANGE , MAX_VALUE_STR_LEN, key_str);
			break;
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("attr %d\n", attr_list[i].id);
		printf("value: %s\n",key_str);
    }

    return status;

}

sai_status_t mrvl_sai_acl_get_attr_range_test(sai_object_id_t acl_range_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_RANGE_ATTR_TYPE;
    status = mrvl_sai_acl_get_attr_range_test_prv(acl_range_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_RANGE_ATTR_LIMIT;
    status = mrvl_sai_acl_get_attr_range_test_prv(acl_range_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all table attributes */
    printf("\nacl group get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_RANGE_ATTR_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_RANGE_ATTR_LIMIT;
    attr_count++;

    status = mrvl_sai_acl_get_attr_range_test_prv(acl_range_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_remove_range_test(sai_object_id_t acl_range_id)
{
	sai_status_t    status;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->remove_acl_range\n");

    status = sai_acl_api->remove_acl_range(acl_range_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    return SAI_STATUS_SUCCESS;
}


/******************ACL TABLE********************/

int mrvl_sai_acl_bind_table_to_port_test(sai_object_id_t acl_table_id, sai_object_id_t port_or_lag_id, uint32_t attr_acl, bool bind)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    char            key_str[MAX_VALUE_STR_LEN];
    sai_uint32_t    port_or_lag;

    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = attr_acl;
    if (bind){
        attr_list[0].value.oid = acl_table_id;
    }
    else {
    	attr_list[0].value.oid = SAI_OBJECT_TYPE_NULL;
    }

    if (bind == true){
        printf("\n\nBIND ACL TABLE to port/lag\n");
    }
    else{
        printf("\n\nUNBIND ACL TABLE from port/lag\n");
    }

    if ((attr_acl == SAI_PORT_ATTR_INGRESS_ACL) || (attr_acl == SAI_PORT_ATTR_EGRESS_ACL)){
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_or_lag_id, SAI_OBJECT_TYPE_PORT, &port_or_lag))
            return SAI_STATUS_FAILURE;

        MRVL_SAI_LOG_INF("Calling sai_port_api->set_port_attribute to %s port %d to table\n", (bind == true) ? "bind" : "unbind", port_or_lag);
        status = sai_port_api->set_port_attribute(port_or_lag_id, attr_list);
        if (status!= SAI_STATUS_SUCCESS)
            return SAI_STATUS_FAILURE;

        MRVL_SAI_LOG_INF("Calling sai_port_api->get_port_attribute get table bound to port %d\n", port_or_lag);
        memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
        attr_list[0].id = attr_acl;
        status = sai_port_api->get_port_attribute(port_or_lag_id, 1, attr_list);
        if (status!= SAI_STATUS_SUCCESS)
            return SAI_STATUS_FAILURE;
    }
    else { /* SAI_LAG_ATTR_INGRESS_ACL or SAI_LAG_ATTR_INGRESS_ACL*/
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(port_or_lag_id, SAI_OBJECT_TYPE_LAG, &port_or_lag))
            return SAI_STATUS_FAILURE;

        MRVL_SAI_LOG_INF("Calling sai_lag_api->set_lag_attribute to %s lag %d to table\n", (bind == true) ? "bind" : "unbind", port_or_lag);
        status = sai_lag_api->set_lag_attribute(port_or_lag_id, attr_list);
        if (status!= SAI_STATUS_SUCCESS)
            return SAI_STATUS_FAILURE;

        MRVL_SAI_LOG_INF("Calling sai_lag_api->get_lag_attribute get table bound to lag %d\n", port_or_lag);
        memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
        attr_list[0].id = attr_acl;
        status = sai_lag_api->get_lag_attribute(port_or_lag_id, 1, attr_list);
        if (status!= SAI_STATUS_SUCCESS)
            return SAI_STATUS_FAILURE;
    }

    mrvl_sai_utl_value_to_str(attr_list[0].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
    printf("attr %d\n", attr_list[0].id);
    printf("value: %s\n",key_str);

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_bind_table_to_vlan_test(sai_object_id_t acl_table_id, uint32_t vlan, uint32_t attr_acl, bool bind)
{
	sai_object_id_t vlan_id;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    char            key_str[MAX_VALUE_STR_LEN];


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = attr_acl;
    if (bind){
        attr_list[0].value.oid = acl_table_id;
    }
    else {
    	attr_list[0].value.oid = SAI_OBJECT_TYPE_NULL;
    }

    status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_VLAN, vlan, &vlan_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    if (bind == true){
        printf("\n\nBIND ACL TABLE to vlan %d\n", vlan);
    	MRVL_SAI_LOG_INF("Calling sai_vlan_api->set_vlan_attribute to bind vlan %d to table\n", vlan);
    }
    else{
        printf("\n\nUNBIND ACL TABLE from vlan %d\n", vlan);
    	MRVL_SAI_LOG_INF("Calling sai_vlan_api->set_vlan_attribute to unbind vlan %d from all tables\n", vlan);
    }

    status = sai_vlan_api->set_vlan_attribute(vlan_id, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    MRVL_SAI_LOG_INF("Calling sai_vlan_api->get_vlan_attribute get table bound to vlan %d\n", vlan);
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_VLAN_ATTR_INGRESS_ACL;
    status = sai_vlan_api->get_vlan_attribute(vlan_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;
    mrvl_sai_utl_value_to_str(attr_list[0].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
    printf("attr %d\n", attr_list[0].id);
    printf("value: %s\n",key_str);

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_bind_table_to_switch_test(sai_object_id_t acl_table_id, uint32_t attr_acl, bool bind)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    char            key_str[MAX_VALUE_STR_LEN];

    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = attr_acl;
    if (bind){
        attr_list[0].value.oid = acl_table_id;
    }
    else {
    	attr_list[0].value.oid = SAI_OBJECT_TYPE_NULL;
    }

    if (bind == true){
        printf("\n\nBIND ACL TABLE to switch\n");
    	MRVL_SAI_LOG_INF("Calling sai_switch_api->set_switch_attribute to bind switch to table\n");
    }
    else{
        printf("\n\nUNBIND ACL TABLE from switch\n");
    	MRVL_SAI_LOG_INF("Calling sai_switch_api->set_switch_attribute to unbind switch from all tables\n");
    }

    status = sai_switch_api->set_switch_attribute(switch_id, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    MRVL_SAI_LOG_INF("Calling sai_switch_api->get_switch_attribute get table bound to switch\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_SWITCH_ATTR_INGRESS_ACL;
    status = sai_switch_api->get_switch_attribute(switch_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;
    mrvl_sai_utl_value_to_str(attr_list[0].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
    printf("attr %d\n", attr_list[0].id);
    printf("value: %s\n",key_str);

    return SAI_STATUS_SUCCESS;
}

/* SET ACL TABLE SET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_set_attr_table_test_prv(sai_object_id_t acl_table_id, sai_attribute_t attr)
{
	char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;

    printf("\nacl table set attributes:\n");
	switch (attr.id) {
	case SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_DST_MAC:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_SRC_IP:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_DST_IP:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_DSCP:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
    case SAI_ACL_TABLE_ATTR_FIELD_ECN:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	default:
		printf("Attribute id %d not supported\n",attr.id);
		break;
	}

	printf("attr %d\n", attr.id);
	printf("value: %s\n",key_str);

    status = sai_acl_api->set_acl_table_attribute(acl_table_id, &attr);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return status;
}

/* SET ACL TABLE GET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_get_attr_table_test_prv(sai_object_id_t acl_table_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    status = sai_acl_api->get_acl_table_attribute(acl_table_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    for (i = 0; i < attr_count; i++){
		switch (attr_list[i].id) {
		case SAI_ACL_TABLE_ATTR_ACL_STAGE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_S32LIST , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_S32LIST , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_SIZE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_DST_MAC:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_SRC_IP:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_DST_IP:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_IN_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_SRC_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_OUT_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_DSCP:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("attr %d\n", attr_list[i].id);
		printf("value: %s\n",key_str);
    }

    return status;

}


sai_status_t mrvl_sai_acl_set_attr_table_test1(sai_object_id_t acl_table_id)
{
    sai_attribute_t attr = {0};
    sai_status_t    status;


    attr.id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr.value.booldata = false;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr.value.booldata = false;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr.value.booldata = false;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr.value.booldata = false;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
    attr.value.booldata = true;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
    attr.value.booldata = true;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
    attr.value.booldata = true;
    status = mrvl_sai_acl_set_attr_table_test_prv(acl_table_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}


sai_status_t mrvl_sai_acl_get_attr_table_test(sai_object_id_t acl_table_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_PORT};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    memset(action_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_ACTION_TYPES);

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[0].value.s32list.count = 0;
    attr_list[0].value.s32list.list = bind_point_type_list;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST;
    attr_list[0].value.s32list.count = 0;
    attr_list[0].value.s32list.list = action_type_list;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_SIZE;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_ECN;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl table get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    /* get all table attributes */
    printf("\nacl table get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    memset(action_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_ACTION_TYPES);

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 0;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 0;
    attr_list[attr_count].value.s32list.list = action_type_list;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_SIZE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_IN_PORT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_PRI;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
    attr_count++;

    status = mrvl_sai_acl_get_attr_table_test_prv(acl_table_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_table_test1(sai_object_id_t *acl_table_id, uint32_t size)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[2] = {SAI_ACL_BIND_POINT_TYPE_PORT, SAI_ACL_BIND_POINT_TYPE_LAG};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};

    printf("\n\nCreate ACL TABLE: IPCL_0 rule with macSA macDA etherType vlan fields\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_list[attr_count].value.s32list.count = 2;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = action_type_list;
    attr_list[attr_count].value.s32list.count = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_SIZE;
    attr_list[attr_count].value.u32 = size;
    attr_count++;

    /* fill minimal match fields for non ip acl */
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ETHER_TYPE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_FRAG;
    attr_list[attr_count].value.booldata = true;
    attr_count++;


    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table\n");

    status = sai_acl_api->create_acl_table(acl_table_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_table_test2(sai_object_id_t *acl_table_id, uint32_t size)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_VLAN};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = 
    {SAI_ACL_ACTION_TYPE_PACKET_ACTION, 
    SAI_ACL_ACTION_TYPE_SET_TC, 
    SAI_ACL_ACTION_TYPE_SET_DSCP, 
    SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_ID, 
    SAI_ACL_ACTION_TYPE_SET_OUTER_VLAN_PRI};

    printf("\n\nCreate ACL TABLE: IPCL_1 rule with ipv4_SA, ipv4_DA, ip protocol, icmp type, icmp code\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_list[attr_count].value.s32list.count = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = action_type_list;
    attr_list[attr_count].value.s32list.count = 5;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_SIZE;
    attr_list[attr_count].value.u32 = size;
    attr_count++;

    /* fill minimal match fields for ip acl */
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ECN;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ACL_IP_TYPE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table\n");

    status = sai_acl_api->create_acl_table(acl_table_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_table_test3(sai_object_id_t *acl_table_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_SWITCH};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};

    printf("\n\nCreate ACL TABLE: IPCL_2 rule with ipv6_SA, ipv6_DA, mac_SA, mac_DA, l4 ports, vlan\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_list[attr_count].value.s32list.count = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = action_type_list;
    attr_list[attr_count].value.s32list.count = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_SIZE;
    attr_list[attr_count].value.u32 = 10;
    attr_count++;

    /* fill minimal match fields for ipv6 acl */
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IPV6;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IPV6;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_MAC;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_MAC;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_SRC_PORT;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table\n");

    status = sai_acl_api->create_acl_table(acl_table_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_table_test4(sai_object_id_t *acl_table_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_PORT};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION, SAI_ACL_ACTION_TYPE_SET_TC, SAI_ACL_ACTION_TYPE_SET_DSCP};

    printf("\n\nCreate ACL TABLE: EPCL rule with ipv4_SA, ipv4_DA, ip protocol, icmp type, icmp code\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_EGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_list[attr_count].value.s32list.count = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = action_type_list;
    attr_list[attr_count].value.s32list.count = 3;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_SIZE;
    attr_list[attr_count].value.u32 = 50;
    attr_count++;

    /* fill minimal match fields for ip acl */
    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_SRC_IP;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_DST_IP;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_IP_PROTOCOL;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_TYPE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_ICMP_CODE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table\n");

    status = sai_acl_api->create_acl_table(acl_table_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_remove_table_test(sai_object_id_t acl_table_id)
{
	sai_status_t    status;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->remove_acl_table\n");

    status = sai_acl_api->remove_acl_table(acl_table_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    return SAI_STATUS_SUCCESS;
}



/******************ACL ENTRY********************/

/* SET ACL ENTRY SET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_set_attr_entry_test_prv(sai_object_id_t acl_entry_id, sai_attribute_t attr)
{
	char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;

	switch (attr.id) {
	case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV6 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV6 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_MAC , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_MAC , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV4 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV4 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
		break;
    case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
        mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACLACTION_U32 , MAX_VALUE_STR_LEN, key_str);
        break;
    case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
        mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACLACTION_U8 , MAX_VALUE_STR_LEN, key_str);
        break;
    case SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP:
        mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_ACLACTION_U8 , MAX_VALUE_STR_LEN, key_str);
        break;
	default:
		printf("Attribute id %d not supported\n",attr.id);
		break;
	}

	printf("attr %d\n", attr.id);
	printf("value: %s\n",key_str);

    status = sai_acl_api->set_acl_entry_attribute(acl_entry_id, &attr);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return status;
}

/* GET ACL ENTRY GET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_get_attr_entry_test_prv(sai_object_id_t acl_entry_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    status = sai_acl_api->get_acl_entry_attribute(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    for (i = 0; i < attr_count; i++){
		switch (attr_list[i].id) {
		case SAI_ACL_ENTRY_ATTR_TABLE_ID:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_PRIORITY:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_ADMIN_STATE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV6 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV6 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_MAC , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_MAC , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV4 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_DST_IP:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_IPV4 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_IN_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_SRC_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_PRI:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U16 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_DSCP:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_ECN:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACL_FIELD_DATA_U8 , MAX_VALUE_STR_LEN, key_str);
			break;
        case SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION:
            mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACLACTION_U32 , MAX_VALUE_STR_LEN, key_str);
            break;
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_TC:
            mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACLACTION_U8 , MAX_VALUE_STR_LEN, key_str);
            break;
        case SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP:
            mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_ACLACTION_U8 , MAX_VALUE_STR_LEN, key_str);
            break;
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("attr %d\n", attr_list[i].id);
		printf("value: %s\n",key_str);
    }

    return status;

}

int mrvl_sai_acl_add_entry_test1(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    sai_mac_t       macSa, macDa, macMask;

    printf("\n\nCreate ACL ENTRY: IPCL_0 rule macSA 00:00:00:11:00:11, macDA 00:00:00:11:00:22, etherType 0600, vlan 2, is_drop = %d\n", is_drop);

    macSa[0] = 0;
    macSa[1] = 0;
    macSa[2] = 0;
    macSa[3] = 0x11;
    macSa[4] = 0x00;
    macSa[5] = 0x33;

    macDa[0] = 0;
    macDa[1] = 0;
    macDa[2] = 0;
    macDa[3] = 0x22;
    macDa[4] = 0x00;
    macDa[5] = 0x44;

    macMask[0] = 0xFF;
    macMask[1] = 0xFF;
    macMask[2] = 0xFF;
    macMask[3] = 0xFF;
    macMask[4] = 0xFF;
    macMask[5] = 0xFF;

    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    /* fill minimal match fields for non ip acl */
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
    memcpy(&attr_list[attr_count].value.aclfield.data.mac, macSa, sizeof(sai_mac_t));
    memcpy(&attr_list[attr_count].value.aclfield.mask.mac, macMask, sizeof(sai_mac_t));
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    memcpy(&attr_list[attr_count].value.aclfield.data.mac, macDa, sizeof(sai_mac_t));
    memcpy(&attr_list[attr_count].value.aclfield.mask.mac, macMask, sizeof(sai_mac_t));
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[attr_count].value.aclfield.data.u16 = 2;
    attr_list[attr_count].value.aclfield.mask.u16 = 0x0fff;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
	attr_list[attr_count].value.aclfield.data.u16 = 0x0600;
	attr_list[attr_count].value.aclfield.mask.u16 = 0xffff;
	attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = true;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    else {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = false;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_FORWARD;
        attr_count++;
    }

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

    status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_set_attr_entry_test1(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr = {0};
    sai_status_t    status;
    sai_mac_t       macSa, macDa, macMask;

    printf("\n\nSet attribute ACL ENTRY: IPCL_0 rule macSA 00:00:00:11:00:33, macDA 00:00:00:11:00:44, etherType 0800 and vlan 5\n");

    macSa[0] = 0;
    macSa[1] = 0;
    macSa[2] = 0;
    macSa[3] = 0x11;
    macSa[4] = 0x00;
    macSa[5] = 0x33;

    macDa[0] = 0;
    macDa[1] = 0;
    macDa[2] = 0;
    macDa[3] = 0x11;
    macDa[4] = 0x00;
    macDa[5] = 0x44;

    macMask[0] = 0xFF;
    macMask[1] = 0xFF;
    macMask[2] = 0xFF;
    macMask[3] = 0xFF;
    macMask[4] = 0xFF;
    macMask[5] = 0xFF;

    printf("\nacl entry set attribute:\n");
    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
    memcpy(&attr.value.aclfield.data.mac, macSa, sizeof(sai_mac_t));
    memcpy(&attr.value.aclfield.mask.mac, macMask, sizeof(sai_mac_t));
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry set attribute:\n");
    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    memcpy(&attr.value.aclfield.data.mac, macDa, sizeof(sai_mac_t));
    memcpy(&attr.value.aclfield.mask.mac, macMask, sizeof(sai_mac_t));
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry set attribute:\n");
    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    attr.value.aclfield.data.u16 = 5;
    attr.value.aclfield.mask.u16 = 0x0fff;
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry set attribute:\n");
    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr.value.aclfield.data.u16 = 0x0800;
    attr.value.aclfield.mask.u16 = 0xffff;
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;

}

sai_status_t mrvl_sai_acl_get_attr_entry_test1(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all entry attributes */
    printf("\nacl entry get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_entry_test_iptype(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop, uint32_t iptype)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACL ENTRY: IPCL_0 ip type = %d, is_drop = %d\n", iptype, is_drop);

    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE;
	attr_list[attr_count].value.aclfield.data.u32 = iptype;
	attr_list[attr_count].value.aclfield.mask.u32 = 0xffffffff;
	attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = true;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    else {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = false;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_FORWARD;
        attr_count++;
    }

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

    status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_acl_get_attr_entry_test_iptype(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all entry attributes */
    printf("\nacl entry get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_entry_test_ipfrag(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop, uint32_t ipfrag)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACL ENTRY: IPCL_0 ip ipfrag = %d, is_drop = %d\n", ipfrag, is_drop);

    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG;
	attr_list[attr_count].value.aclfield.data.u32 = ipfrag;
	attr_list[attr_count].value.aclfield.mask.u32 = 0xffffffff;
	attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = true;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    else {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = false;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_FORWARD;
        attr_count++;
    }

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

    status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_acl_get_attr_entry_test_ipfrag(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all entry attributes */
    printf("\nacl entry get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_FRAG;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_entry_test2(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, sai_uint32_t acl_action, sai_acl_action_data_t *acl_action_data)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_ip4_t       ip4Sa, ip4Da, ip4Mask;
    sai_status_t    status;

    printf("\n\nCreate ACL ENTRY: IPCL_1 rule with ipv4SA 1.2.3.4, ipv4DA 1.2.3.44, ip protocol 1 (icmp) icmp type 8 (echo) icmp code 0 (no code)\n");
    printf("ACL ACTIONS:\n");

    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION)){
        printf("packet action is %s\n", (acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable)? "enabled":"disabled");
        printf("packet action value is %s\n", (acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u8)?"forward":"drop");
    } 
    else 
        printf("packet action is not set\n");

    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC)){
        printf("tc is %s\n", (acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable)? "enabled":"disabled");
        printf("tc value is %d\n", acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8);
    } 
    else 
        printf("tc is not set\n");

    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP)){
        printf("dscp is %s\n", (acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable)? "enabled":"disabled");
        printf("dscp value is %d\n", acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8);
    } 
    else 
        printf("dscp is not set\n");

    ip4Sa = 0x01020304;
    ip4Da = 0x0102032C;
    ip4Mask = 0xFFFFFFFF;

	memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
	attr_list[attr_count].value.oid = acl_table_id;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
	attr_list[attr_count].value.u32 = 1;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
	attr_list[attr_count].value.booldata = true;
	attr_count++;

	/* fill minimal match fields for ip acl */
	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &ip4Sa, sizeof(sai_ip4_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &ip4Mask, sizeof(sai_ip4_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
	memcpy(&attr_list[attr_count].value.aclfield.data.mac, &ip4Da, sizeof(sai_ip4_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.mac, &ip4Mask, sizeof(sai_ip4_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
	attr_list[attr_count].value.aclfield.data.u8 = 1;
	attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
	attr_list[attr_count].value.aclfield.data.u8 = 8;
	attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
	attr_list[attr_count].value.aclfield.data.u8 = 0;
	attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
	attr_count++;

    	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
	attr_list[attr_count].value.aclfield.data.u16 = 6;
	attr_list[attr_count].value.aclfield.mask.u16 = 0xfff;
	attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DSCP;
	attr_list[attr_count].value.aclfield.data.u8 = 6;
	attr_list[attr_count].value.aclfield.mask.u8 = 0x3f;
	attr_count++;

        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ECN;
	attr_list[attr_count].value.aclfield.data.u8 = 3;
	attr_list[attr_count].value.aclfield.mask.u8 = 0x3;
	attr_count++;

    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable;
        attr_list[attr_count].value.aclaction.parameter.u8 = acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8;
        attr_count++;
    }
    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable;
        attr_list[attr_count].value.aclaction.parameter.u8 = acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8;
        attr_count++;
    }

	MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

	status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
	if (status!= SAI_STATUS_SUCCESS)
		return SAI_STATUS_FAILURE;

	return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_entry_test2_simple(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, sai_uint32_t acl_action, sai_acl_action_data_t *acl_action_data)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

	memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
	attr_list[attr_count].value.oid = acl_table_id;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
	attr_list[attr_count].value.u32 = 1;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
	attr_list[attr_count].value.booldata = true;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ACL_IP_TYPE;
	attr_list[attr_count].value.aclfield.data.u32 = SAI_ACL_IP_TYPE_IPV6ANY;
	attr_count++;

	if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable;
        attr_list[attr_count].value.aclaction.parameter.u8 = acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8;
        attr_count++;
    }
    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable;
        attr_list[attr_count].value.aclaction.parameter.u8 = acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8;
        attr_count++;
    }

	MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

	status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
	if (status!= SAI_STATUS_SUCCESS)
		return SAI_STATUS_FAILURE;

	return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_set_attr_entry_admin_state_test(sai_object_id_t acl_entry_id, bool admin_state)
{
    sai_attribute_t attr = {0};
    sai_status_t    status;

    printf("\nacl entry set attribute: admin state %d\n", admin_state);
    attr.id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr.value.booldata = admin_state;
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;

}

sai_status_t mrvl_sai_acl_get_attr_entry_test2(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute: \n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute: \n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute: \n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute: \n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute: \n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ECN;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute: \n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_DSCP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all entry attributes */
    printf("\nacl entry get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DSCP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ECN;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}
int mrvl_sai_acl_add_entry_test3(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    sai_ip6_t       ip6Sa = {0}, ip6Da = {0}, ip6Mask = {0};
    printf("\n\nCreate ACL ENTRY: IPCL_2 rule with ipv6_SA, ipv6_DA, l4 src port - 60, l4 dst port - 70, vlan - 3, is_drop = %d\n", is_drop);

    ip6Sa[0] = 11;
    ip6Sa[1] = 22;
    ip6Sa[2] = 33;
    ip6Sa[15] = 44;

    ip6Da[0] = 66;
    ip6Da[1] = 77;
    ip6Da[2] = 88;
    ip6Da[15] = 55;

    memset(ip6Mask, 0xFF, 16);

	memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
	attr_list[attr_count].value.oid = acl_table_id;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
	attr_list[attr_count].value.u32 = 1;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
	attr_list[attr_count].value.booldata = true;
	attr_count++;

	/* fill minimal match fields for ipv6 acl */
	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip6, ip6Sa, sizeof(sai_ip6_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, ip6Mask, sizeof(sai_ip6_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip6, ip6Da, sizeof(sai_ip6_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, ip6Mask, sizeof(sai_ip6_t));
	attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    attr_list[attr_count].value.aclfield.data.u16 = 60;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFFF;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.data.u16 = 70;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFFF;
    attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = true;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    else {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = false;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_FORWARD;
        attr_count++;
    }

	MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

	status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
	if (status!= SAI_STATUS_SUCCESS)
		return SAI_STATUS_FAILURE;

return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_entry_test3_1(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    sai_ip6_t       ip6Sa = {0}, ip6Da = {0}, ip6Mask = {0};
    sai_mac_t       macSa = {0}, macDa = {0}, macMask = {0};
    printf("\n\nCreate ACL ENTRY: IPCL_2 rule with ipv6_SA, ipv6_DA, macSa, macDa, l4 src port - 60, l4 dst port - 70, vlan - 3, is_drop = %d\n", is_drop);

    memset(ip6Sa, 0x11, sizeof(ip6Sa));
    memset(ip6Da, 0x22, sizeof(ip6Da));
    memset(ip6Mask, 0xFF, sizeof(ip6Mask));

    memset(macSa, 0x55, sizeof(macSa));
    memset(macDa, 0x66, sizeof(macDa));
    memset(macMask, 0xFF, sizeof(macMask));

	memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
	attr_list[attr_count].value.oid = acl_table_id;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
	attr_list[attr_count].value.u32 = 1;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
	attr_list[attr_count].value.booldata = true;
	attr_count++;

	/* fill match fields for ipv6 acl */
	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip6, ip6Sa, sizeof(sai_ip6_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, ip6Mask, sizeof(sai_ip6_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip6, ip6Da, sizeof(sai_ip6_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip6, ip6Mask, sizeof(sai_ip6_t));
	attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    attr_list[attr_count].value.aclfield.data.u16 = 60;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFFF;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.data.u16 = 70;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFFF;
    attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = true;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    else {
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = false;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_FORWARD;
        attr_count++;
    }

	MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

	status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
	if (status!= SAI_STATUS_SUCCESS)
		return SAI_STATUS_FAILURE;

return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_acl_get_attr_entry_test3_1(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all entry attributes */
    printf("\nacl entry get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IPV6;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IPV6;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_entry_test4(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, sai_uint32_t acl_action, sai_acl_action_data_t *acl_action_data)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    sai_ip4_t       ip4Sa, ip4Da, ip4Mask;

    printf("\n\nCreate ACL ENTRY: EPCL rule with ipv4SA 5.5.5.5, ipv4DA 7.7.7.7, ip protocol 1 (icmp) icmp type 8 (echo) icmp code 0 (no code) \n");
    printf("ACL Actions\n");
    printf("packet action: %s\n",mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION)?"drop":"forward");

    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC)){
        printf("tc is %s\n", (acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable)? "enabled":"disabled");
        printf("tc value is %d\n", acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8);
    } 
    else 
        printf("tc is not set\n");

    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP)){
        printf("dscp is %s\n", (acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable)? "enabled":"disabled");
        printf("dscp value is %d\n", acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8);
    } 
    else 
        printf("dscp is not set\n");

    ip4Sa = 0x05050505;
    ip4Da = 0x07070707;
    ip4Mask = 0xFFFFFFFF;

	memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
	attr_list[attr_count].value.oid = acl_table_id;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
	attr_list[attr_count].value.u32 = 1;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
	attr_list[attr_count].value.booldata = true;
	attr_count++;

	/* fill minimal match fields for ip acl */
	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &ip4Sa, sizeof(sai_ip4_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &ip4Mask, sizeof(sai_ip4_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
	memcpy(&attr_list[attr_count].value.aclfield.data.ip4, &ip4Da, sizeof(sai_ip4_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.ip4, &ip4Mask, sizeof(sai_ip4_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
	attr_list[attr_count].value.aclfield.data.u8 = 1;
	attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
	attr_list[attr_count].value.aclfield.data.u8 = 8;
	attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
	attr_list[attr_count].value.aclfield.data.u8 = 0;
	attr_list[attr_count].value.aclfield.mask.u8 = 0xff;
	attr_count++;

	if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.aclaction.enable = SAI_PACKET_ACTION_DROP;
        attr_list[attr_count].value.aclaction.parameter.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }
    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable;
        attr_list[attr_count].value.aclaction.parameter.u8 = acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8;
        attr_count++;
    }
    if (mrvl_acl_is_bit_set_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP)){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
        attr_list[attr_count].value.aclaction.enable = acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable;
        attr_list[attr_count].value.aclaction.parameter.u8 = acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8;
        attr_count++;
    }

	MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

	status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
	if (status!= SAI_STATUS_SUCCESS)
		return SAI_STATUS_FAILURE;

	return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_set_attr_entry_test4(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr = {0};
    sai_status_t    status;
    sai_ip4_t       ip4Sa, ip4Da, ip4Mask;

    printf("\n\nSET ACL ENTRY: EPCL rule with ipv4SA 6.6.6.6, ipv4DA 8.8.8.8, ip protocol 1 (icmp) icmp type 0 (echo reply) icmp code 3 (dest unreachable)\n");

    ip4Sa = 0x06060606;
    ip4Da = 0x08080808;
    ip4Mask = 0xFFFFFFFF;

    printf("\nacl entry set attribute:\n");

    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
	memcpy(&attr.value.aclfield.data.ip4 , &ip4Sa, sizeof(sai_ip4_t));
	memcpy(&attr.value.aclfield.mask.ip4, &ip4Mask, sizeof(sai_ip4_t));
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
	memcpy(&attr.value.aclfield.data.ip4 , &ip4Da, sizeof(sai_ip4_t));
	memcpy(&attr.value.aclfield.mask.ip4, &ip4Mask, sizeof(sai_ip4_t));
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry set attribute:\n");
    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
    attr.value.aclfield.data.u8 = 0;
    attr.value.aclfield.mask.u8 = 0xff;
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry set attribute:\n");
    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
    attr.value.aclfield.data.u8 = 3;
    attr.value.aclfield.mask.u8 = 0xff;
    status = mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

sai_status_t mrvl_sai_acl_get_attr_entry_test4(sai_object_id_t acl_entry_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    /* get all entry attributes */
    printf("\nacl entry get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_PRIORITY;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ADMIN_STATE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_IP;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_ICMP_CODE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_remove_entry_test(sai_object_id_t acl_entry_id)
{
	sai_status_t    status;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->remove_acl_entry\n");

    status = sai_acl_api->remove_acl_entry(acl_entry_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/******************ACL GROUP********************/

int mrvl_sai_acl_add_group_test1_auto(sai_object_id_t *acl_group_id, sai_attribute_t *attr_list)
{
    uint32_t attr_count = 0;
    /*sai_attribute_t attr_list[mrvl_max_acl_attr_len];*/
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_PORT};

    printf("\n\nCreate ACL GROUP: ingress, port bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 1;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.u32 = SAI_ACL_TABLE_GROUP_TYPE_PARALLEL;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group\n");

    status = sai_acl_api->create_acl_table_group(acl_group_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}


int mrvl_sai_acl_add_group_test1(sai_object_id_t *acl_group_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[2] = {SAI_ACL_BIND_POINT_TYPE_PORT, SAI_ACL_BIND_POINT_TYPE_LAG};

    printf("\n\nCreate ACL GROUP: ingress, port bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 2;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.u32 = SAI_ACL_TABLE_GROUP_TYPE_PARALLEL;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group\n");

    status = sai_acl_api->create_acl_table_group(acl_group_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_group_test2(sai_object_id_t *acl_group_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_VLAN};

    printf("\n\nCreate ACL GROUP: ingress, vlan bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 1;    
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.u32 = SAI_ACL_TABLE_GROUP_TYPE_PARALLEL;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group\n");

    status = sai_acl_api->create_acl_table_group(acl_group_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_group_test3(sai_object_id_t *acl_group_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_SWITCH};

    printf("\n\nCreate ACL GROUP: ingress, switch bind type, sequential \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 1;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.u32 = SAI_ACL_TABLE_GROUP_TYPE_SEQUENTIAL;


    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group\n");

    status = sai_acl_api->create_acl_table_group(acl_group_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_group_test4(sai_object_id_t *acl_group_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_PORT};

    printf("\n\nCreate ACL GROUP: egress, port bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_EGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 1;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_list[attr_count].value.u32 = SAI_ACL_TABLE_GROUP_TYPE_PARALLEL;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group\n");

    status = sai_acl_api->create_acl_table_group(acl_group_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}


int mrvl_sai_acl_remove_group_test(sai_object_id_t acl_group_id)
{
	sai_status_t    status;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->remove_acl_table_group\n");

    status = sai_acl_api->remove_acl_table_group(acl_group_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    return SAI_STATUS_SUCCESS;
}



/* GET ACL GROUP GET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_get_attr_group_test_prv(sai_object_id_t acl_group_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    status = sai_acl_api->get_acl_table_group_attribute(acl_group_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    for (i = 0; i < attr_count; i++){
		switch (attr_list[i].id) {
		case SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_S32LIST , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_GROUP_ATTR_TYPE:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("attr %d\n", attr_list[i].id);
		printf("value: %s\n",key_str);
    }

    return status;

}

sai_status_t mrvl_sai_acl_get_attr_group_test(sai_object_id_t acl_group_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES];


    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[0].value.s32list.count = 0;
    attr_list[0].value.s32list.list = bind_point_type_list;
    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all table attributes */
    printf("\nacl group get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 0;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}
#if 0
sai_status_t mrvl_sai_acl_get_and_verify_attr_group_test(sai_object_id_t acl_group_id, sai_attribute_t *set_attr_list)
{
    sai_attribute_t get_attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES];


    printf("\nacl group get attribute:\n");
    memset(get_attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    get_attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, 1, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    status = mrvl_sai_acl_verify_attr(1, set_attr_list, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    printf("\nacl group get attribute:\n");
    memset(get_attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    get_attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, 1, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    status = mrvl_sai_acl_verify_attr(1, set_attr_list, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    printf("\nacl group get attribute:\n");
    memset(get_attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    get_attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    get_attr_list[0].value.s32list.count = 0;
    get_attr_list[0].value.s32list.list = bind_point_type_list;
    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, 1, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    status = mrvl_sai_acl_verify_attr(1, set_attr_list, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    /* get all table attributes */
    printf("\nacl group get attributes:\n");
    memset(get_attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    get_attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_count++;
    get_attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_count++;
    get_attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    get_attr_list[attr_count].value.s32list.count = 0;
    get_attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    status = mrvl_sai_acl_get_attr_group_test_prv(acl_group_id, attr_count, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    status = mrvl_sai_acl_verify_attr(attr_count, set_attr_list, get_attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_group_api_test1(sai_object_id_t *acl_group_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1_auto(acl_group_id, attr_list))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_and_verify_attr_group_test(*acl_group_id, attr_list))
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}
#endif
/******************ACL GROUP MEMBER********************/


int mrvl_sai_acl_add_group_member_test(sai_object_id_t *acl_group_member_id, sai_object_id_t acl_group_id, sai_object_id_t acl_table_id, sai_uint32_t prio)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACL GROUP MEMBER: acl_group_id acl_table_id\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    attr_list[attr_count].value.oid = acl_group_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = prio;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group_member\n");

    status = sai_acl_api->create_acl_table_group_member(acl_group_member_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_remove_group_member_test(sai_object_id_t acl_group_member_id)
{
	sai_status_t    status;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->remove_acl_table_group_member\n");

    status = sai_acl_api->remove_acl_table_group_member(acl_group_member_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    return SAI_STATUS_SUCCESS;
}



/* GET ACL GROUP MEMBER GET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_get_attr_group_member_test_prv(sai_object_id_t acl_group_member_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    status = sai_acl_api->get_acl_table_group_member_attribute(acl_group_member_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    for (i = 0; i < attr_count; i++){
		switch (attr_list[i].id) {
		case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U32 , MAX_VALUE_STR_LEN, key_str);
			break;
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("attr %d\n", attr_list[i].id);
		printf("value: %s\n",key_str);
    }

    return status;

}

sai_status_t mrvl_sai_acl_get_attr_group_member_test(sai_object_id_t acl_group_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES];


    printf("\nacl group member get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group member get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group member get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[0].value.s32list.count = 0;
    attr_list[0].value.s32list.list = bind_point_type_list;
    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    /* get all table attributes */
    printf("\nacl group member get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = 0;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_count++;

    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

/******************ACL COUNTER*******************/

sai_status_t mrvl_sai_acl_get_attr_counter_test_prv(sai_object_id_t acl_counter_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;
    uint32_t i;

    status = sai_acl_api->get_acl_counter_attribute(acl_counter_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    for (i = 0; i < attr_count; i++){
		switch (attr_list[i].id) {
		case SAI_ACL_COUNTER_ATTR_TABLE_ID:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_COUNTER_ATTR_PACKETS:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U64 , MAX_VALUE_STR_LEN, key_str);
			break;
		case SAI_ACL_COUNTER_ATTR_BYTES:
			mrvl_sai_utl_value_to_str(attr_list[i].value, SAI_ATTR_VAL_TYPE_U64 , MAX_VALUE_STR_LEN, key_str);
			break;
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("type %d\n", attr_list[i].id);
		printf("attribute: %s\n",key_str);
    }

    return status;

}

/* SET ACL COUNTER SET ATTRIBUTE TEST FUNCTION */
sai_status_t mrvl_sai_acl_set_attr_counter_test_prv(sai_object_id_t acl_table_id, sai_attribute_t attr)
{
	char         key_str[MAX_KEY_STR_LEN];
    sai_status_t    status;

	switch (attr.id) {
	case SAI_ACL_COUNTER_ATTR_TABLE_ID:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_OID , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_BOOL , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_COUNTER_ATTR_PACKETS:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_U64 , MAX_VALUE_STR_LEN, key_str);
		break;
	case SAI_ACL_COUNTER_ATTR_BYTES:
		mrvl_sai_utl_value_to_str(attr.value, SAI_ATTR_VAL_TYPE_U64 , MAX_VALUE_STR_LEN, key_str);
		break;
	default:
		printf("Attribute id %d not supported\n",attr.id);
		break;
	}

	printf("attr %d\n", attr.id);
	printf("value: %s\n",key_str);

    status = sai_acl_api->set_acl_counter_attribute(acl_table_id, &attr);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_counter_test(sai_object_id_t *acl_counter_id, sai_object_id_t acl_table_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;


    printf("\n\nCreate ACL COUNTER: \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT;
    attr_list[attr_count].value.booldata = true;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_counter\n");

    status = sai_acl_api->create_acl_counter(acl_counter_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_acl_get_attr_counter_test(sai_object_id_t acl_counter_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;


    printf("\nacl counter get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    status = mrvl_sai_acl_get_attr_counter_test_prv(acl_counter_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl counter get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT;
    status = mrvl_sai_acl_get_attr_counter_test_prv(acl_counter_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl counter get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT;
    status = mrvl_sai_acl_get_attr_counter_test_prv(acl_counter_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl counter get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_COUNTER_ATTR_PACKETS;
    status = mrvl_sai_acl_get_attr_counter_test_prv(acl_counter_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl counter get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_COUNTER_ATTR_BYTES;
    status = mrvl_sai_acl_get_attr_counter_test_prv(acl_counter_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;


    /* get all table attributes */
    printf("\nacl counter get attributes:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_TABLE_ID;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_PACKETS;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr_count++;

    status = mrvl_sai_acl_get_attr_counter_test_prv(acl_counter_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_set_attr_counter_test(sai_object_id_t acl_counter_id)
{
    sai_attribute_t attr = {0};
    sai_status_t    status;

    printf("\n\nSET ACL COUNTER:\n");

    printf("\nacl counter set attribute:\n");
    attr.id = SAI_ACL_COUNTER_ATTR_PACKETS;
    attr.value.u64 = 0;
    status = mrvl_sai_acl_set_attr_counter_test_prv(acl_counter_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl counter set attribute:\n");
    attr.id = SAI_ACL_COUNTER_ATTR_BYTES;
    attr.value.u64 = 0;
    status = mrvl_sai_acl_set_attr_counter_test_prv(acl_counter_id, attr);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;


    return status;
}

int mrvl_sai_acl_remove_counter_test(sai_object_id_t acl_counter_id)
{
	sai_status_t    status;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->remove_acl_counter\n");

    status = sai_acl_api->remove_acl_counter(acl_counter_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_tables_3entries_logic1
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

int mrvl_sai_acl_test_tables_3entries_logic1(void)
{
	sai_object_id_t  acl_group_id1, acl_group_id2, acl_group_id3,
                     acl_table_id1, acl_table_id2, acl_table_id3,
                     acl_group_member_id1, acl_group_member_id2, acl_group_member_id3,
                     acl_entry_id1_1, acl_entry_id1_2, acl_entry_id1_3,
					 acl_entry_id2_1, acl_entry_id2_2, acl_entry_id2_3,
					 acl_entry_id3_1, acl_entry_id3_2, acl_entry_id3_3,
					 acl_counter_id1, acl_counter_id2, acl_counter_id3;

    sai_uint32_t acl_action = 0;
    sai_acl_action_data_t acl_action_data[SAI_ACL_MAX_ACTION_TYPES];

    memset(acl_action_data, 0, sizeof(acl_action_data));

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;


    /* TEST2
	   1. add table1
	   1. add entry1_1
	   1. add entry1_2
	   1. add entry1_3
	   1. add table2
	   1. add entry2_1
	   1. add entry2_2
	   1. add entry2_3
	   1. add table3
	   1. add entry3_1
	   1. add entry3_2
	   1. add entry3_3
	   1. del entry1_1
	   1. del entry1_2
	   1. del entry1_3
	   1. del entry2_1
	   1. del entry2_2
	   1. del entry2_3
	   1. del entry3_1
	   1. del entry3_2
	   1. del entry3_3
	   1. del table1
	   1. del table2
	   1. del table3
	 */
	/* add non ip entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_1, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_2, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_3, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_counter_test(&acl_counter_id1, acl_table_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_counter_test(acl_counter_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_counter_test(acl_counter_id1))
    	return SAI_STATUS_FAILURE;


    /* add ipv4 entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id2, acl_group_id2, acl_table_id2, 200))
        return SAI_STATUS_FAILURE;

    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION);
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 = SAI_PACKET_ACTION_DROP;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_1, acl_table_id2, acl_action, acl_action_data))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_2, acl_table_id2, acl_action, acl_action_data))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_3, acl_table_id2, acl_action, acl_action_data))
    	return SAI_STATUS_FAILURE;


    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_counter_test(&acl_counter_id2, acl_table_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_counter_test(acl_counter_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_counter_test(acl_counter_id2))
    	return SAI_STATUS_FAILURE;

    /* add ipv6 entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test3(&acl_group_id3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test3(&acl_table_id3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id3, acl_group_id3, acl_table_id3, 300))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3(&acl_entry_id3_1, acl_table_id3, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3(&acl_entry_id3_2, acl_table_id3, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3(&acl_entry_id3_3, acl_table_id3, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_counter_test(&acl_counter_id3, acl_table_id3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_counter_test(acl_counter_id3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_counter_test(acl_counter_id3))
    	return SAI_STATUS_FAILURE;


    /* remove all entries */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id2_1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id3_1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id2_2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id3_2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id2_3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id3_3))
    	return SAI_STATUS_FAILURE;

    /* remove all group_members */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id3))
    	return SAI_STATUS_FAILURE;

    /* remove all counters */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_counter_test(acl_counter_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_counter_test(acl_counter_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_counter_test(acl_counter_id3))
    	return SAI_STATUS_FAILURE;

    /* remove all tables */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id3))
    	return SAI_STATUS_FAILURE;

    /* remove all groups */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id3))
    	return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_bind_unbind_3tables_3entries
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

int mrvl_sai_acl_test_bind_unbind_3tables_3entries(void)
{
	sai_object_id_t  acl_group_id1, acl_group_id2, acl_group_id3,
                     acl_table_id1, acl_table_id2, acl_table_id3,
                     acl_group_member_id1, acl_group_member_id2, acl_group_member_id3,
                     acl_entry_id1_1,
					 acl_entry_id2_1,
					 acl_entry_id3_1,
                     port_id1;
    sai_uint32_t     acl_action = 0;
    sai_acl_action_data_t acl_action_data[SAI_ACL_MAX_ACTION_TYPES];

    memset(acl_action_data, 0, sizeof(acl_action_data));

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* TEST4
	   1. add table1
	   1. add entry1_1 action  drop, invalid port
	   1. add table2
	   1. add entry2_1 action  drop, invalid port
	   1. add table3
	   1. add entry3_1 action  drop, invalid port
	   1. bind table1 to port 1
	   1. bind table2 to vlan 2
	   1. bind table3 to switch
	   1. unbind table1 from port 1
	   1. unbind table2 from vlan 2
	   1. unbind table3 from switch
	   1. del group_member1
	   1. del group_member2
	   1. del group_member3
	   1. del table1
	   1. del table2
	   1. del table3
	   1. del group1
	   1. del group2
	   1. del group3
	 */

    /* add non ip entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_1, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;


    /* add ipv4 entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id2, acl_group_id2, acl_table_id2, 200))
        return SAI_STATUS_FAILURE;

    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION);
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 = SAI_PACKET_ACTION_DROP;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_1, acl_table_id2, acl_action, acl_action_data))
    	return SAI_STATUS_FAILURE;


    /* add ipv6 entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test3(&acl_group_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test3(&acl_table_id3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id3, acl_group_id3, acl_table_id3, 300))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3(&acl_entry_id3_1, acl_table_id3, true))
    	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_id1))
        return SAI_STATUS_FAILURE;

    /* bind table 1 to port 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    /* bind table 2 to vlan 6 */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_vlan_test(acl_table_id2, 6 /* vlan 6*/,  SAI_VLAN_ATTR_INGRESS_ACL, true))
		return SAI_STATUS_FAILURE;

	/* bind table 3 to switch */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_switch_test(acl_table_id3, SAI_SWITCH_ATTR_INGRESS_ACL, true))
		return SAI_STATUS_FAILURE;

    /* unbind table 1 from port 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;

    /* unbind table 2 from vlan 2 */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_vlan_test(acl_table_id2, 6 /* vlan 6*/,  SAI_VLAN_ATTR_INGRESS_ACL, false))
		return SAI_STATUS_FAILURE;

	/* unbind table 3 from switch */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_switch_test(acl_table_id3,  SAI_SWITCH_ATTR_INGRESS_ACL, false))
		return SAI_STATUS_FAILURE;

    /* cleanup */
    printf("\n\n cleanup acl entries\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id2_1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id3_1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_members\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id3))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl tables\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id3))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl groups\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id3))
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_one_group_table_entry
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

int mrvl_sai_acl_test_one_group_table_entry(void)
{
	sai_object_id_t  acl_group_id1, acl_table_id1, acl_entry_id1_1, acl_group_member_id1;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

	 /* TEST5
	   1. add table1
	   1. get table1 attributes
	   1. add entry1_1
	   1. get entry1_1 attributes
	   1. set entry1_1 attributes
	   1. get entry1_1 attributes
	   1. del entry1_1
	   1. add table1
	 */

    /* add group 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;

	/* add non ip entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
        return SAI_STATUS_FAILURE;

    /* add group member 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_1, acl_table_id1, true))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test1(acl_entry_id1_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test1(acl_entry_id1_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test1(acl_entry_id1_1))
      	return SAI_STATUS_FAILURE;

    /* cleanup */
    printf("\n\n cleanup acl entry\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_member\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl table\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;

 return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_ingress_l2_port
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

int mrvl_sai_acl_test_ingress_l2_port(void)
{
	sai_object_id_t  acl_entry_id1_1, port_id1_1, port_id1_2, acl_group_member_id1, acl_table_id1, acl_group_id1;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add non ip entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_1, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test1(acl_entry_id1_1))
        return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 1, &port_id1_1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 2, &port_id1_2))
        return SAI_STATUS_FAILURE;


    /* bind table 1 to port 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1_1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* bind table 1 to port 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1_2 /* port 2*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
        return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* unbind table 1 from port 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1_1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* unbind table 1 from port 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1_2 /* port 2*/,  SAI_PORT_ATTR_INGRESS_ACL, false))
        return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);


    /* cleanup */
    printf("\n\n cleanup acl entries\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_1))
    	return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_members\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl tables\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
    	return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl groups\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}


/*******************************************************************************
* mrvl_sai_acl_test_ingress_l2_28port_28tables
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

int mrvl_sai_acl_test_ingress_l2_28port_2tables(void)
{
	sai_object_id_t  acl_entry_id1,
                     acl_table_id1,
					 acl_group_id1,
                     acl_group_member_id1,
                     port_id1;

    sai_uint32_t table_idx = 0, entry_idx = 0, port_idx = 0, entry_count = 0, port_count = 0;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* TEST2
    adding 1 group, 2 table each of 10 size, 10 entries, each table to 14 ports
	 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;
   
    for (entry_idx=0, port_idx=0, table_idx=0;table_idx<2;table_idx++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
        	return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 200))
            return SAI_STATUS_FAILURE;

        for (entry_count=0;entry_count<10;entry_count++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1, acl_table_id1, true))
                return SAI_STATUS_FAILURE;
            entry_idx++;
        }
        /* bind table 0 to port 0-13 */
        /* bind table 1 to port 14-27 */
        for (port_count=0;port_count<14;port_count++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_idx, &port_id1))
                return SAI_STATUS_FAILURE;

            if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1,  SAI_PORT_ATTR_INGRESS_ACL, true))
                return SAI_STATUS_FAILURE;
            port_idx++;
        }
    }

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    for (entry_idx=0, port_idx=0, table_idx=0;table_idx<2;table_idx++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, table_idx, &acl_table_id1))
            return SAI_STATUS_FAILURE;

        /* unbind table 0 from port 0-13 */
        /* unbind table 1 from port 14-27 */
        for (port_count=0;port_count<14;port_count++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port_idx, &port_id1))
                return SAI_STATUS_FAILURE;

            if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, port_id1,  SAI_PORT_ATTR_INGRESS_ACL, false))
                return SAI_STATUS_FAILURE;
            port_idx++;
        }

        for (entry_count =0;entry_count<10;entry_count++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_ENTRY, entry_idx, &acl_entry_id1))
                return SAI_STATUS_FAILURE;

            if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1))
                return SAI_STATUS_FAILURE;
            entry_idx++;
        }

        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, 0, table_idx, &acl_group_member_id1))
            return SAI_STATUS_FAILURE;

        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
            return SAI_STATUS_FAILURE;

        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, table_idx, &acl_table_id1))
            return SAI_STATUS_FAILURE;

        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
            return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
    	return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}



int mrvl_sai_acl_change_port_to_lag_test(sai_object_id_t lag_oid, uint32_t lag_ports_list_count, uint32_t *lag_ports_list, bool is_added)
{       
    sai_object_id_t     port_oid[SAI_MAX_NUM_OF_PORTS] = {0}, lag_member_oid = {0};
    uint32_t            lag_idx;
    uint32_t            attr_count, i;   
    sai_status_t        status;  
    sai_attribute_t     attr_list[4]; 

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_idx))
        return SAI_STATUS_FAILURE;

    /* add ports to LAG */
    for (i=0; i<lag_ports_list_count; i++) 
    {
        if (is_added) {
            /* create port_oid for port i */
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, lag_ports_list[i], &(port_oid[i]))) 
                return SAI_STATUS_FAILURE;

            printf("\nCreate LAG member #%d: port %d\n",lag_idx, lag_ports_list[i]);   
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
            status = sai_lag_api->create_lag_member(&lag_member_oid, switch_id, attr_count, attr_list);
            if (status!= SAI_STATUS_SUCCESS) {
                printf("%s:%d: Create LAG member returned with status %d\n",
                       __func__, __LINE__, status);
                return SAI_STATUS_FAILURE;
            }
        }
        else {

            /* create LAG member for port_idx[i] and lag_idx */
            if (SAI_STATUS_SUCCESS != (status = mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_LAG_MEMBER, lag_ports_list[i], lag_idx, &lag_member_oid))) {
                MRVL_SAI_API_RETURN(status);
            }

            printf("\nRemove LAG member #%d: port %d\n",lag_idx, lag_ports_list[i]);   
            status = sai_lag_api->remove_lag_member(lag_member_oid);
            if (status!= SAI_STATUS_SUCCESS) {
                printf("%s:%d: REMOVE LAG member returned with status %d\n",
                       __func__, __LINE__, status);
                return SAI_STATUS_FAILURE;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}


int mrvl_sai_acl_create_lag_test(sai_object_id_t *lag_oid, uint32_t lag_ports_list_count, uint32_t *lag_ports_list, sai_object_id_t acl_table_id)
{       
    sai_attribute_t     lag_attr_list[2];
    uint32_t            attr_count;   
    sai_status_t        status;    

    attr_count = 2;
    memset(lag_attr_list, 0, sizeof(sai_attribute_t)*2);
    lag_attr_list[0].id = SAI_LAG_ATTR_INGRESS_ACL;
    lag_attr_list[0].value.oid = acl_table_id;
    lag_attr_list[1].id = SAI_LAG_ATTR_EGRESS_ACL;
    lag_attr_list[1].value.oid = SAI_NULL_OBJECT_ID;

    printf("\nCreate empty LAG\n");
    status = sai_lag_api->create_lag(lag_oid, switch_id, attr_count, lag_attr_list);     
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Create empty LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }

    mrvl_sai_acl_change_port_to_lag_test(*lag_oid, lag_ports_list_count, lag_ports_list, true);

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_remove_lag_test(sai_object_id_t lag_oid)
{         
    sai_status_t        status;    

    printf("\nRemove LAG\n");
    status = sai_lag_api->remove_lag(lag_oid);     
    if (status!= SAI_STATUS_SUCCESS) {
        printf("%s:%d: Remove LAG returned with status %d\n",
               __func__, __LINE__, status);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}
/*******************************************************************************
* mrvl_sai_acl_test_ingress_l2_lag
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

int mrvl_sai_acl_test_ingress_l2_lag(void)
{
	sai_object_id_t  acl_group_id1, acl_table_id1, acl_group_member_id1,
	                 acl_entry_id1_1, lag_id1, lag_id2;
    sai_uint32_t lag_ports_list[4], lag1, lag2;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add non ip entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_1, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test1(acl_entry_id1_1))
        return SAI_STATUS_FAILURE;

    /* create lag without acl */
    lag_ports_list[0] = 1;
    lag_ports_list[1] = 2;
    lag_ports_list[2] = 3;
    lag_ports_list[3] = 4;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_lag_test(&lag_id1, 4, lag_ports_list, SAI_NULL_OBJECT_ID))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_id1, SAI_OBJECT_TYPE_LAG, &lag1))
        return SAI_STATUS_FAILURE;
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);

    lag_ports_list[0] = 5;
    lag_ports_list[1] = 6;
    lag_ports_list[2] = 7;
    lag_ports_list[3] = 8;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_lag_test(&lag_id2, 4, lag_ports_list, SAI_NULL_OBJECT_ID))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_object_to_type(lag_id2, SAI_OBJECT_TYPE_LAG, &lag2))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);

    /* bind table 1 to lag 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, lag_id1 /* lag 1*/,  SAI_LAG_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* bind table 1 to lag 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, lag_id2 /* lag 2*/,  SAI_LAG_ATTR_INGRESS_ACL, true))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* add port 9 to lag 1*/
    lag_ports_list[0] = 9;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id1, 1, lag_ports_list, true))
        return SAI_STATUS_FAILURE;
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);

    /* add port 10 to lag 2*/
    lag_ports_list[0] = 10;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id2, 1, lag_ports_list, true))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);

    /* remove port 9 from lag 1*/
    lag_ports_list[0] = 9;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id1, 1, lag_ports_list, false))
        return SAI_STATUS_FAILURE;
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);

    /* remove port 10 from lag 2*/
    lag_ports_list[0] = 10;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id2, 1, lag_ports_list, false))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);

    /* unbind table 1 from lag 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, lag_id1 /* lag 1*/,  SAI_LAG_ATTR_INGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* unbind table 1 from lag 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, lag_id2 /* lag 2*/,  SAI_LAG_ATTR_INGRESS_ACL, false))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* remove lag 1 and lag 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_lag_test(lag_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_lag_test(lag_id2))
        return SAI_STATUS_FAILURE;
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag1);
    fpaWrapDumpTrunkDB(SAI_DEFAULT_ETH_SWID_CNS, lag2);


    /* create lag with acl */
    lag_ports_list[0] = 11;
    lag_ports_list[1] = 12;
    lag_ports_list[2] = 13;
    lag_ports_list[3] = 14;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_lag_test(&lag_id1, 4, lag_ports_list, acl_table_id1))
        return SAI_STATUS_FAILURE;

    lag_ports_list[0] = 15;
    lag_ports_list[1] = 16;
    lag_ports_list[2] = 17;
    lag_ports_list[3] = 18;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_lag_test(&lag_id2, 4, lag_ports_list, acl_table_id1))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* add port 19 to lag 1*/
    lag_ports_list[0] = 19;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id1, 1, lag_ports_list, true))
        return SAI_STATUS_FAILURE;

    /* add port 20 to lag 2*/
    lag_ports_list[0] = 20;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id2, 1, lag_ports_list, true))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* remove port 19 from lag 1*/
    lag_ports_list[0] = 19;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id1, 1, lag_ports_list, false))
        return SAI_STATUS_FAILURE;

    /* remove port 20 from lag 2*/
    lag_ports_list[0] = 20;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_change_port_to_lag_test(lag_id2, 1, lag_ports_list, false))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* unbind table 1 from lag 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, lag_id1 /* lag 1*/,  SAI_LAG_ATTR_INGRESS_ACL, false))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* unbind table 1 from lag 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, lag_id2 /* lag 2*/,  SAI_LAG_ATTR_INGRESS_ACL, false))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* remove lag 1 and lag 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_lag_test(lag_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_lag_test(lag_id2))
        return SAI_STATUS_FAILURE;

    /* cleanup */
    printf("\n\n cleanup acl entries\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_1))
    	return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_members\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl tables\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
    	return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl groups\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_ingress_ipv4
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

int mrvl_sai_acl_test_ingress_ipv4(void)
{
	sai_object_id_t        acl_group_id2, acl_table_id2, acl_group_member_id2, acl_entry_id2_1;
    sai_uint32_t           acl_action = 0;
    sai_acl_action_data_t  acl_action_data[SAI_ACL_MAX_ACTION_TYPES];
    sai_attribute_t attr = {0};

    memset(acl_action_data, 0, sizeof(acl_action_data));

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add group */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
        return SAI_STATUS_FAILURE;

	/* add table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2, 10))
        return SAI_STATUS_FAILURE;

    /* add group member */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id2, acl_group_id2, acl_table_id2, 200))
        return SAI_STATUS_FAILURE;

    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC);
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8 = 2;
    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP);
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8 = 3;
    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION);
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable = true; /* drop */
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 = SAI_PACKET_ACTION_DROP;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_1, acl_table_id2, acl_action, acl_action_data))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaHalQosDump(0, 30);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
    attr.value.aclaction.enable = false;
    if (SAI_STATUS_SUCCESS != (mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id2_1, attr)))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaHalQosDump(0, 30);

    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_TC;
    attr.value.aclaction.enable = true;
    attr.value.aclaction.parameter.u8 = 7;
    if (SAI_STATUS_SUCCESS != (mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id2_1, attr)))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaHalQosDump(0, 30);

    attr.id = SAI_ACL_ENTRY_ATTR_ACTION_SET_DSCP;
    attr.value.aclaction.enable = true;
    attr.value.aclaction.parameter.u8 = 22;
    if (SAI_STATUS_SUCCESS != (mrvl_sai_acl_set_attr_entry_test_prv(acl_entry_id2_1, attr)))
        return SAI_STATUS_FAILURE;
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaHalQosDump(0, 30);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    /* invalidate entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_admin_state_test(acl_entry_id2_1, false))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* validate entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_admin_state_test(acl_entry_id2_1, true /* admin state */))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* bind table to vlan 6 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_vlan_test(acl_table_id2, 6 /* vlan 6 */,  SAI_VLAN_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* invalidate entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_admin_state_test(acl_entry_id2_1, false /* admin state */))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* validate entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_admin_state_test(acl_entry_id2_1, true /* admin state */))
          	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* unbind table to vlan 6 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_vlan_test(acl_table_id2, 6 /* vlan 6 */, SAI_VLAN_ATTR_INGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* cleanup */
    printf("\n\n cleanup acl entry\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id2_1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_member\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id2))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl table\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id2))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id2))
        return SAI_STATUS_FAILURE;

 return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_ingress_ipv6
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

int mrvl_sai_acl_test_ingress_ipv6(void)
{
	sai_object_id_t        acl_group_id3, acl_table_id3, acl_group_member_id3, acl_entry_id3_1;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add group */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test3(&acl_group_id3))
        return SAI_STATUS_FAILURE;

	/* add table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test3(&acl_table_id3))
        return SAI_STATUS_FAILURE;

    /* add group member */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id3, acl_group_id3, acl_table_id3, 300))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3_1(&acl_entry_id3_1, acl_table_id3, true))
       return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test3_1(acl_entry_id3_1))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* bind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_switch_test(acl_table_id3, SAI_SWITCH_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* unbind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_switch_test(acl_table_id3, SAI_SWITCH_ATTR_INGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* cleanup */
    printf("\n\n cleanup acl entry\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id3_1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_member\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id3))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl table\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id3))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id3))
        return SAI_STATUS_FAILURE;

 return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_egress_ipv4
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

int mrvl_sai_acl_test_egress_ipv4(void)
{
	sai_object_id_t        acl_group_id4, acl_table_id4, acl_group_member_id4, acl_entry_id4_1, port_id4;
    sai_uint32_t           acl_action = 0;
    sai_acl_action_data_t acl_action_data[SAI_ACL_MAX_ACTION_TYPES];

    memset(acl_action_data, 0, sizeof(acl_action_data));

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* EPCL test */
    /* add group egress */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test4(&acl_group_id4))
        return SAI_STATUS_FAILURE;

	/* add egress entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test4(&acl_table_id4))
        return SAI_STATUS_FAILURE;

    /* add group member egress */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id4, acl_group_id4, acl_table_id4, 400))
        return SAI_STATUS_FAILURE;

    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_TC);
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_TC].parameter.u8 = 5;
    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_SET_DSCP);
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_SET_DSCP].parameter.u8 = 6;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test4(&acl_entry_id4_1, acl_table_id4, acl_action, acl_action_data))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_EPCL_E);
    fpaHalQosDump(0, 30);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test4(acl_entry_id4_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test4(acl_entry_id4_1))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_EPCL_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test4(acl_entry_id4_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, 7, &port_id4)) 
        return SAI_STATUS_FAILURE;

    /* bind table to port 7*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id4, port_id4 /* port 7*/,  SAI_PORT_ATTR_EGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_EPCL_E);

    /* unbind table to port 7*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id4, port_id4 /* port 7*/, SAI_PORT_ATTR_EGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_EPCL_E);

    /* cleanup */
    printf("\n\n cleanup acl entry\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id4_1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group_member\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id4))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl table\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id4))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl group\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id4))
        return SAI_STATUS_FAILURE;

 return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_groups_tables_members_apis_auto
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
#if 0
int mrvl_sai_acl_test_groups_tables_members_apis_auto(void)
{

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* api group 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_group_api_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl groups\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;


    return SAI_STATUS_SUCCESS;
}
#endif

/*******************************************************************************
* mrvl_sai_acl_test_groups_tables_members_apis
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

int mrvl_sai_acl_test_groups_tables_members_apis(void)
{
	sai_object_id_t  acl_group_id1, acl_group_id2, acl_group_id3, acl_group_id4,
                     acl_table_id1, acl_table_id2, acl_table_id3, acl_table_id4,
                     acl_group_member_id1, acl_group_member_id2, acl_group_member_id3, acl_group_member_id4;


    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* api group 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;

    /* add group 2 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_test(acl_group_id2))
        return SAI_STATUS_FAILURE;

    /* add group 3 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test3(&acl_group_id3))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_test(acl_group_id3))
        return SAI_STATUS_FAILURE;

    /* add group 4 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test4(&acl_group_id4))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_test(acl_group_id4))
        return SAI_STATUS_FAILURE;

    /* add non ip table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_table_test(acl_table_id1))
        return SAI_STATUS_FAILURE;


    /* add ipv4 table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2, 10))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_table_test(acl_table_id2))
        return SAI_STATUS_FAILURE;


    /* add ipv6 table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test3(&acl_table_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_table_test(acl_table_id3))
        return SAI_STATUS_FAILURE;

    /* add egress ipv4 table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test4(&acl_table_id4))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_table_test(acl_table_id4))
        return SAI_STATUS_FAILURE;

    /* add group member 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;


    /* add group member 2*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id2, acl_group_id2, acl_table_id2, 200))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id2))
        return SAI_STATUS_FAILURE;


    /* add group member 3*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id3, acl_group_id3, acl_table_id3, 300))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id3))
        return SAI_STATUS_FAILURE;

    /* add group member 4*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id4, acl_group_id4, acl_table_id4, 400))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id4))
        return SAI_STATUS_FAILURE;


    /* cleanup */
    printf("\n\n cleanup acl group_members\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id4))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl tables\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id4))
        return SAI_STATUS_FAILURE;

    printf("\n\n cleanup acl groups\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id4))
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_range
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

int mrvl_sai_acl_test_range(void)
{
	sai_object_id_t  acl_range_id1, acl_range_id2, acl_range_id3, acl_range_id4;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_range_test(&acl_range_id1, 1, 200, 300))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_range_test(acl_range_id1))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_range_test(&acl_range_id2, 0, 400, 500))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_range_test(acl_range_id2))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_range_test(&acl_range_id3, 1, 10, 20))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_range_test(acl_range_id3))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_create_range_test(&acl_range_id4, 0, 1000, 1020))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_range_test(acl_range_id4))
        return SAI_STATUS_FAILURE;


    printf("\n\n cleanup acl group\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_range_test(acl_range_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_range_test(acl_range_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_range_test(acl_range_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_range_test(acl_range_id4))
        return SAI_STATUS_FAILURE;

 return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_1group_4tables_250entries_logic
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

int mrvl_sai_acl_test_1group_4tables_250entries_logic(void)
{
	sai_object_id_t  acl_entry_id2,
                     acl_table_id2,
					 acl_group_id2,
                     acl_group_member_id2;

    sai_uint32_t acl_action = 0, table_idx = 0, entry_idx = 0, entry_count = 0;
    sai_acl_action_data_t acl_action_data[SAI_ACL_MAX_ACTION_TYPES];

    memset(acl_action_data, 0, sizeof(acl_action_data));
    mrvl_acl_set_bit_MAC(acl_action, SAI_ACL_ACTION_TYPE_PACKET_ACTION);
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].enable = true;
    acl_action_data[SAI_ACL_ACTION_TYPE_PACKET_ACTION].parameter.u32 = SAI_PACKET_ACTION_DROP;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* TEST2
    adding 1 group, 4 table each of 250 size, 250 entries
	 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
        return SAI_STATUS_FAILURE;

    for (table_idx=0;table_idx<4;table_idx++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2, 250))
        	return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id2, acl_group_id2, acl_table_id2, 200))
            return SAI_STATUS_FAILURE;
        for (entry_idx=0;entry_idx<250;entry_idx++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2_simple(&acl_entry_id2, acl_table_id2, acl_action, acl_action_data))
                return SAI_STATUS_FAILURE;
        }
    }

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    
    for (entry_idx = 0, table_idx=0;table_idx<4;table_idx++) {
        for (entry_count=0;entry_count<250;entry_count++) {
            if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_ENTRY, entry_idx, &acl_entry_id2))
                return SAI_STATUS_FAILURE;

            if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id2))
                return SAI_STATUS_FAILURE;
            entry_idx++;
        }
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_ext_object(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER, 0, table_idx, &acl_group_member_id2))
            return SAI_STATUS_FAILURE;

        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id2))
            return SAI_STATUS_FAILURE;

        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_TABLE, table_idx, &acl_table_id2))
            return SAI_STATUS_FAILURE;

        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id2))
            return SAI_STATUS_FAILURE;
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id2))
    	return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_ip_type
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

int mrvl_sai_acl_test_ip_type(void)
{
	sai_object_id_t  acl_group_id1,
                     acl_table_id1,
                     acl_group_member_id1,
                     acl_entry_id1;

    sai_uint32_t entry_idx = 0, iptype_idx = 0, entry_count = 0;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;

    /* create entries */
    for (iptype_idx=0; iptype_idx<=SAI_ACL_IP_TYPE_NON_IPV6; iptype_idx++, entry_count++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test_iptype(&acl_entry_id1, acl_table_id1, true, SAI_ACL_IP_TYPE_ANY+iptype_idx))
        	return SAI_STATUS_FAILURE;
        
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test_iptype(acl_entry_id1))
          	return SAI_STATUS_FAILURE;
    }

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    /* remove all entries */
    for (entry_idx=0; entry_idx<entry_count; entry_idx++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_ACL_ENTRY, entry_idx, &acl_entry_id1))
            return SAI_STATUS_FAILURE;
        if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1))
            return SAI_STATUS_FAILURE;
    }

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
    	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
    	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
    	return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

/*******************************************************************************
* mrvl_sai_acl_test_ip_frag
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

int mrvl_sai_acl_test_ip_frag(void)
{
	sai_object_id_t  acl_group_id1,
                     acl_table_id1,
                     acl_group_member_id1,
                     acl_entry_id1_1, acl_entry_id1_2;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test1(&acl_group_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1, 10))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test(&acl_group_member_id1, acl_group_id1, acl_table_id1, 100))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test_ipfrag(&acl_entry_id1_1, acl_table_id1, true, SAI_ACL_IP_FRAG_NON_FRAG))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test_ipfrag(&acl_entry_id1_2, acl_table_id1, true, SAI_ACL_IP_FRAG_ANY))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test_ipfrag(acl_entry_id1_1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test_ipfrag(acl_entry_id1_2))
        return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_entry_test(acl_entry_id1_2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_member_test(acl_group_member_id1))
    	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
    	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_group_test(acl_group_id1))
    	return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}



/*******************************************************************************
* mrvl_sai_acl_test
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

int mrvl_sai_acl_test(void)
{
    /*if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_groups_tables_members_apis_auto())
        return SAI_STATUS_FAILURE;*/
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_groups_tables_members_apis())
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_tables_3entries_logic1())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_bind_unbind_3tables_3entries())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_one_group_table_entry())
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_l2_port())
		return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_l2_lag())
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_l2_28port_2tables())
        return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_ipv4())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_ipv6())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_egress_ipv4())
		return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_range())
		return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_1group_4tables_250entries_logic())
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ip_type())
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ip_frag())
        return SAI_STATUS_FAILURE;
    return SAI_STATUS_SUCCESS;
}
/*******************************************************************************
* mrvl_sai_acl_dump
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

int mrvl_sai_acl_dump(sai_uint32_t dev, sai_uint32_t startRuleIndex, sai_uint32_t endRuleIndex)
{   

    fpaLibRulesDumpCpss(dev, startRuleIndex, endRuleIndex);
    return SAI_STATUS_SUCCESS;
}