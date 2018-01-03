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
#define mrvl_max_acl_attr_len MAX_LIST_VALUE_STR_LEN

sai_object_id_t acl_table_id1, acl_table_id2, acl_table_id3, acl_table_id4;
sai_object_id_t acl_group_id1, acl_group_id2, acl_group_id3, acl_group_id4;
sai_object_id_t acl_group_member_id1, acl_group_member_id2, acl_group_member_id3, acl_group_member_id4;

static sai_object_id_t switch_id = 0;

/******************ACL TABLE********************/

int mrvl_sai_acl_bind_table_to_port_test(sai_object_id_t acl_table_id, uint32_t port, uint32_t attr_acl, bool bind)
{
	sai_object_id_t port_id;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;

    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[attr_count].id = attr_acl;
    if (bind){
        attr_list[attr_count].value.oid = acl_table_id;
    }
    else {
    	attr_list[attr_count].value.oid = SAI_OBJECT_TYPE_NULL;
    }

    status = mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, port, &port_id);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    if (bind == true){
    	MRVL_SAI_LOG_INF("Calling sai_port_api->set_port_attribute to bind port %d to table\n", port);
    }
    else{
    	MRVL_SAI_LOG_INF("Calling sai_port_api->set_port_attribute to unbind port %d from all tables\n", port);
    }

    status = sai_port_api->set_port_attribute(port_id, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;


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

	printf("type %d\n", attr.id);
	printf("attribute: %s\n",key_str);

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
		printf("type %d\n", attr_list[i].id);
		printf("attribute: %s\n",key_str);
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

    printf("\nacl ta368ble get attribute:\n");
    attr_list[0].id = SAI_ACL_TABLE_ATTR_FIELD_DSCP;
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

int mrvl_sai_acl_add_table_test1(sai_object_id_t *acl_table_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_PORT};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};

    printf("\n\nCreate ACT TABLE: IPCL_0 rule with macSA macDA etherType vlan fields\n");
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


    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table\n");

    status = sai_acl_api->create_acl_table(acl_table_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_table_test2(sai_object_id_t *acl_table_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_VLAN};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};

    printf("\n\nCreate ACT TABLE: IPCL_1 rule with ipv4_SA, ipv4_DA, ip protocol, icmp type, icmp code\n");
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

int mrvl_sai_acl_add_table_test3(sai_object_id_t *acl_table_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_SWITCH};
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};

    printf("\n\nCreate ACT TABLE: IPCL_2 rule with ipv6_SA, ipv6_DA, mac_SA, mac_DA, l4 ports, vlan\n");
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

    attr_list[attr_count].id = SAI_ACL_TABLE_ATTR_FIELD_OUTER_VLAN_ID;
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
    int32_t action_type_list[SAI_ACL_MAX_ACTION_TYPES] = {SAI_ACL_ACTION_TYPE_PACKET_ACTION};

    printf("\n\nCreate ACT TABLE: EPCL rule with ipv4_SA, ipv4_DA, ip protocol, icmp type, icmp code\n");
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
    attr_list[attr_count].value.s32list.count = 1;
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
	default:
		printf("Attribute id %d not supported\n",attr.id);
		break;
	}

	printf("type %d\n", attr.id);
	printf("attribute: %s\n",key_str);

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
		default:
			printf("Attribute id %d not supported\n",attr_list[i].id);
			break;
		}
		printf("type %d\n", attr_list[i].id);
		printf("attribute: %s\n",key_str);
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
    macSa[5] = 0x11;

    macDa[0] = 0;
    macDa[1] = 0;
    macDa[2] = 0;
    macDa[3] = 0x11;
    macDa[4] = 0x00;
    macDa[5] = 0x22;

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
        attr_list[attr_count].value.u32 = SAI_PACKET_ACTION_DROP;
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

int mrvl_sai_acl_add_entry_test2(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    sai_ip4_t       ip4Sa, ip4Da, ip4Mask;

    printf("\n\nCreate ACL ENTRY: IPCL_1 rule with ipv4SA 1.2.3.4, ipv4DA 1.2.3.44, ip protocol 1 (icmp) icmp type 8 (echo) icmp code 0 (no code) is_drop = %d\n", is_drop);

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

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.u32 = SAI_PACKET_ACTION_DROP;
        attr_count++;
    }

	MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_entry\n");

	status = sai_acl_api->create_acl_entry(acl_entry_id, switch_id, attr_count, attr_list);
	if (status!= SAI_STATUS_SUCCESS)
		return SAI_STATUS_FAILURE;

	return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_set_attr_entry_test2(sai_object_id_t acl_entry_id, bool admin_state)
{
    sai_attribute_t attr = {0};
    sai_status_t    status;

    printf("\n\nSet attribute ACL ENTRY: IPCL_1 rule admin state %d\n", admin_state);

    printf("\nacl entry set attribute:\n");
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

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[attr_count].value.aclfield.data.u16 = 3;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFF; /* 12 bits */
    attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.u32 = SAI_PACKET_ACTION_DROP;
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

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
	memcpy(&attr_list[attr_count].value.aclfield.data.mac, macSa, sizeof(sai_mac_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.mac, macMask, sizeof(sai_mac_t));
	attr_count++;

	attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
	memcpy(&attr_list[attr_count].value.aclfield.data.mac, macDa, sizeof(sai_mac_t));
	memcpy(&attr_list[attr_count].value.aclfield.mask.mac, macMask, sizeof(sai_mac_t));
	attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    attr_list[attr_count].value.aclfield.data.u16 = 60;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFFF;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_list[attr_count].value.aclfield.data.u16 = 70;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFFF;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    attr_list[attr_count].value.aclfield.data.u16 = 3;
    attr_list[attr_count].value.aclfield.mask.u16 = 0xFFF; /* 12 bits */
    attr_count++;

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.u32 = SAI_PACKET_ACTION_DROP;
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
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl entry get attribute:\n");
    attr_list[0].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
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
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_MAC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_DST_MAC;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT;
    attr_count++;
    attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_FIELD_OUTER_VLAN_ID;
    attr_count++;

    status = mrvl_sai_acl_get_attr_entry_test_prv(acl_entry_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    return status;
}

int mrvl_sai_acl_add_entry_test4(sai_object_id_t *acl_entry_id, sai_object_id_t acl_table_id, bool is_drop)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    sai_ip4_t       ip4Sa, ip4Da, ip4Mask;

    printf("\n\nCreate ACL ENTRY: EPCL rule with ipv4SA 5.5.5.5, ipv4DA 7.7.7.7, ip protocol 1 (icmp) icmp type 8 (echo) icmp code 0 (no code) is_drop = %d\n", is_drop);

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

	if (is_drop){
        attr_list[attr_count].id = SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION;
        attr_list[attr_count].value.u32 = SAI_PACKET_ACTION_DROP;
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

    attr.id = SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP;
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


int mrvl_sai_acl_add_group_test1(sai_object_id_t *acl_group_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES] = {SAI_ACL_BIND_POINT_TYPE_PORT, SAI_ACL_BIND_POINT_TYPE_LAG, SAI_ACL_BIND_POINT_TYPE_VLAN, SAI_ACL_BIND_POINT_TYPE_SWITCH, SAI_ACL_BIND_POINT_TYPE_ROUTER_INTF};

    printf("\n\nCreate ACT GROUP: ingress, port bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = SAI_ACL_MAX_BIND_POINT_TYPES;
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

    printf("\n\nCreate ACT GROUP: ingress, vlan bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_list[attr_count].value.s32list.count = 1;
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

    printf("\n\nCreate ACT GROUP: ingress, switch bind type, sequential \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_INGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.list = bind_point_type_list;
    attr_list[attr_count].value.s32list.count = 1;
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

    printf("\n\nCreate ACT GROUP: egress, port bind type, parallel \n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    attr_list[attr_count].value.u32 = SAI_ACL_STAGE_EGRESS;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[attr_count].value.s32list.count = SAI_ACL_MAX_BIND_POINT_TYPES;
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
		printf("type %d\n", attr_list[i].id);
		printf("attribute: %s\n",key_str);
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


/******************ACL GROUP MEMBER********************/


int mrvl_sai_acl_add_group_member_test1(sai_object_id_t *acl_group_member_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACT GROUP MEMBER1: acl_group_id1 acl_table_id1\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    attr_list[attr_count].value.oid = acl_group_id1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id1;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 100;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group_member\n");

    status = sai_acl_api->create_acl_table_group_member(acl_group_member_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_group_member_test2(sai_object_id_t *acl_group_member_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACT GROUP MEMBER1: acl_group_id2 acl_table_id2\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    attr_list[attr_count].value.oid = acl_group_id2;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id2;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 200;
    attr_count++;

    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group_member\n");

    status = sai_acl_api->create_acl_table_group_member(acl_group_member_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_group_member_test3(sai_object_id_t *acl_group_member_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACT GROUP MEMBER1: acl_group_id3 acl_table_id3\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    attr_list[attr_count].value.oid = acl_group_id3;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id3;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 300;
    attr_count++;


    MRVL_SAI_LOG_INF("Calling sai_acl_api->create_acl_table_group_member\n");

    status = sai_acl_api->create_acl_table_group_member(acl_group_member_id, switch_id, attr_count, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
        return SAI_STATUS_FAILURE;

    return SAI_STATUS_SUCCESS;
}

int mrvl_sai_acl_add_group_member_test4(sai_object_id_t *acl_group_member_id)
{
    uint32_t attr_count = 0;
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;

    printf("\n\nCreate ACT GROUP MEMBER1: acl_group_id4 acl_table_id4\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID;
    attr_list[attr_count].value.oid = acl_group_id4;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID;
    attr_list[attr_count].value.oid = acl_table_id4;
    attr_count++;

    attr_list[attr_count].id = SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY;
    attr_list[attr_count].value.u32 = 400;
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
		printf("type %d\n", attr_list[i].id);
		printf("attribute: %s\n",key_str);
    }

    return status;

}

sai_status_t mrvl_sai_acl_get_attr_group_member_test(sai_object_id_t acl_group_id)
{
    sai_attribute_t attr_list[mrvl_max_acl_attr_len];
    sai_status_t    status;
    uint32_t attr_count = 0;
    int32_t bind_point_type_list[SAI_ACL_MAX_BIND_POINT_TYPES];


    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE;
    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_TYPE;
    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, 1, attr_list);
    if (status!= SAI_STATUS_SUCCESS)
    	return SAI_STATUS_FAILURE;

    printf("\nacl group get attribute:\n");
    memset(attr_list, 0, sizeof(sai_attribute_t)*mrvl_max_acl_attr_len);
    memset(bind_point_type_list, 0, sizeof(int32_t)*SAI_ACL_MAX_BIND_POINT_TYPES);
    attr_list[0].id = SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST;
    attr_list[0].value.s32list.count = 0;
    attr_list[0].value.s32list.list = bind_point_type_list;
    status = mrvl_sai_acl_get_attr_group_member_test_prv(acl_group_id, 1, attr_list);
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

    printf("\nacl counter set attributes:\n");
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

	printf("type %d\n", attr.id);
	printf("attribute: %s\n",key_str);

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


    printf("\n\nCreate ACT COUNTER: \n");
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
* mrvl_sai_acl_test_tables_entries_logic1
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

int mrvl_sai_acl_test_tables_entries_logic1(void)
{
	sai_object_id_t  acl_entry_id1_1, acl_entry_id1_2, acl_entry_id1_3,
					 acl_entry_id2_1, acl_entry_id2_2, acl_entry_id2_3,
					 acl_entry_id3_1, acl_entry_id3_2, acl_entry_id3_3,
					 acl_counter_id1, acl_counter_id2, acl_counter_id3;

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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test1(&acl_group_member_id1))
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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test2(&acl_group_member_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_1, acl_table_id2, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_2, acl_table_id2, true))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_3, acl_table_id2, true))
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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test3(&acl_group_member_id3))
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
	sai_object_id_t  acl_entry_id1_1,
					 acl_entry_id2_1,
					 acl_entry_id3_1;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* TEST4
	   1. add table1
	   1. add entry1_1 action  drop, invalid port
	   1. add table2
	   1. add entry2_1 action  drop, invalid port
	   1. add table3
	   1. add entry3_1 action  drop, invalid port
	   1. traffic passed
	   1. bind table1 to port 1
	   1. bind table2 to port 2
	   1. bind table3 to port 3
	   1. traffic dropped
	   1. unbind table1 from port 1
	   1. unbind table2 from port 2
	   1. unbind table3 from port 3
	   1. traffic passed
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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test1(&acl_group_member_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test1(&acl_entry_id1_1, acl_table_id1, true))
    	return SAI_STATUS_FAILURE;


    /* add ipv4 entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test2(&acl_group_member_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_1, acl_table_id2, true))
    	return SAI_STATUS_FAILURE;


    /* add ipv6 entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test3(&acl_group_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test3(&acl_table_id3))
    	return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test3(&acl_group_member_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3(&acl_entry_id3_1, acl_table_id3, true))
    	return SAI_STATUS_FAILURE;


    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* TODO - send traffic -> must be passed */

    /* bind table 1 to port 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, 1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    /* bind table 2 to port 2 */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id2, 2 /* port 2*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
		return SAI_STATUS_FAILURE;

	/* bind table 3 to port 3 */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id3, 3 /* port 3*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
		return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* TODO - send traffic -> must be dropped */

    /* can't delete acl table with bound port */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id1))
        printf("\n\n can't delete acl table 1 with bound port\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id2))
    	printf("\n\n can't delete acl table 2 with bound port\n");
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_remove_table_test(acl_table_id3))
    	printf("\n\n can't delete acl table 3 with bound port\n");

    /* unbind table 1 from port 1 */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id1, 1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, false))
    	return SAI_STATUS_FAILURE;

    /* unbind table 2 to port 2 */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id2, 2 /* port 2*/,  SAI_PORT_ATTR_INGRESS_ACL, false))
		return SAI_STATUS_FAILURE;

	/* unbind table 3 to port 3 */
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id3, 3 /* port 3*/,  SAI_PORT_ATTR_INGRESS_ACL, false))
		return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL0_E);
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);
    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* TODO - send traffic -> must be passed */

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
	sai_object_id_t  acl_entry_id1_1;

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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1))
        return SAI_STATUS_FAILURE;

    /* add group member 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test1(&acl_group_member_id1))
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
	sai_object_id_t  acl_entry_id2_1;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add group */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test2(&acl_group_id2))
        return SAI_STATUS_FAILURE;

	/* add table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2))
        return SAI_STATUS_FAILURE;

    /* add group member */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test2(&acl_group_member_id2))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test2(&acl_entry_id2_1, acl_table_id2, true))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test2(acl_entry_id2_1, false /* admin state */))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test2(acl_entry_id2_1, true /* admin state */))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test2(acl_entry_id2_1))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* bind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id2, 1 /* port 1*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* invalidate entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test2(acl_entry_id2_1, false /* admin state */))
      	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* validate entry */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test2(acl_entry_id2_1, true /* admin state */))
          	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL1_E);

    /* unbind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id2, 1 /* port 1*/, SAI_PORT_ATTR_INGRESS_ACL, false))
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
	sai_object_id_t  acl_entry_id3_1;

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add group */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_test3(&acl_group_id3))
        return SAI_STATUS_FAILURE;

	/* add table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test3(&acl_table_id3))
        return SAI_STATUS_FAILURE;

    /* add group member */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test3(&acl_group_member_id3))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test3_1(&acl_entry_id3_1, acl_table_id3, true))
       return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test3_1(acl_entry_id3_1))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* bind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id3, 5 /* port 5*/,  SAI_PORT_ATTR_INGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_PCL2_E);

    /* unbind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id3, 5 /* port 5*/, SAI_PORT_ATTR_INGRESS_ACL, false))
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
	sai_object_id_t  acl_entry_id4_1;

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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test4(&acl_group_member_id4))
        return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_entry_test4(&acl_entry_id4_1, acl_table_id4, true))
       return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_EPCL_E);

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test4(acl_entry_id4_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_set_attr_entry_test4(acl_entry_id4_1))
      	return SAI_STATUS_FAILURE;

    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_entry_test4(acl_entry_id4_1))
      	return SAI_STATUS_FAILURE;

    /* bind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id4, 7 /* port 7*/,  SAI_PORT_ATTR_EGRESS_ACL, true))
    	return SAI_STATUS_FAILURE;

    fpaLibFlowTableDumpCpss(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_EPCL_E);

    /* unbind table to port 1*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_bind_table_to_port_test(acl_table_id4, 7 /* port 7*/, SAI_PORT_ATTR_EGRESS_ACL, false))
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
* mrvl_sai_acl_test_groups_tables_logic
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

int mrvl_sai_acl_test_groups_tables_logic(void)
{

    if (SAI_STATUS_SUCCESS != mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_SWITCH, SAI_DEFAULT_ETH_SWID_CNS, &switch_id))
        return SAI_STATUS_FAILURE;

    /* add group 1 */
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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test1(&acl_table_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_table_test(acl_table_id1))
        return SAI_STATUS_FAILURE;


    /* add ipv4 table */
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_table_test2(&acl_table_id2))
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
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test1(&acl_group_member_id1))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id1))
        return SAI_STATUS_FAILURE;


    /* add group member 2*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test2(&acl_group_member_id2))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id2))
        return SAI_STATUS_FAILURE;


    /* add group member 3*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test3(&acl_group_member_id3))
        return SAI_STATUS_FAILURE;
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_get_attr_group_member_test(acl_group_member_id3))
        return SAI_STATUS_FAILURE;

    /* add group member 4*/
    if (SAI_STATUS_SUCCESS != mrvl_sai_acl_add_group_member_test4(&acl_group_member_id4))
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
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_tables_entries_logic1())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_bind_unbind_3tables_3entries())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_one_group_table_entry())
    	return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_ipv4())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_ingress_ipv6())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_egress_ipv4())
		return SAI_STATUS_FAILURE;
	if (SAI_STATUS_SUCCESS != mrvl_sai_acl_test_groups_tables_logic())
    	return SAI_STATUS_FAILURE;
    return SAI_STATUS_SUCCESS;
}
