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

#include "sai.h"
#include "mrvl_sai.h"
#include "assert.h"
#include "inttypes.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#ifndef WIN32
#include <arpa/inet.h>
#else
#include <Ws2tcpip.h>
#endif

#undef  __MODULE__
#define __MODULE__ SAI_UTILS

#define MRVL_SAI_TRACE_MAX_LEN 512
#define MRVL_SAI_TRACE_APP_NAME_LEN 16
static time_t mrvl_sai_trace_start_time = 0;

static sai_status_t mrvl_sai_utl_fill_genericlist_prv(size_t element_size, void *data, uint32_t count, void *list)
{
    /* all list objects have same field count in the beginning of the object, and then different data,
     * so can be casted to one type */
    sai_object_list_t *objlist = list;

    if (NULL == data) {
        MRVL_SAI_LOG_ERR("NULL data value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == list) {
        MRVL_SAI_LOG_ERR("NULL list value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (0 == element_size) {
        MRVL_SAI_LOG_ERR("Zero element size\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (count > objlist->count) {
        MRVL_SAI_LOG_ERR("Insufficient list buffer size. Allocated %u needed %u\n",
                     objlist->count, count);
        objlist->count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    objlist->count = count;
    memcpy(objlist->list, data, count * element_size);

    return SAI_STATUS_SUCCESS;
}


static sai_status_t mrvl_sai_utl_find_functionality_attrib_index_prv(_In_ const sai_attr_id_t          id,
                                                    _In_ const sai_attribute_entry_t *functionality_attr,
                                                    _Out_ uint32_t                   *index)
{
    uint32_t curr_index;

    MRVL_SAI_LOG_ENTER();

    if (NULL == functionality_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == index) {
        MRVL_SAI_LOG_ERR("NULL value index\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (curr_index = 0; END_FUNCTIONALITY_ATTRIBS_ID != functionality_attr[curr_index].id; curr_index++) {
        if (id == functionality_attr[curr_index].id) {
            *index = curr_index;
            MRVL_SAI_LOG_EXIT();
            return SAI_STATUS_SUCCESS;
        }
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_ITEM_NOT_FOUND;
}

static sai_status_t mrvl_sai_utl_set_dispatch_attrib_handler_prv(_In_ const sai_attribute_t              *attr,
                                                _In_ const sai_attribute_entry_t        *functionality_attr,
                                                _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
                                                _In_ const sai_object_key_t             *key,
                                                _In_ const char                         *key_str)
{
    uint32_t     index;
    sai_status_t err;
    char         value_str[MAX_VALUE_STR_LEN];

    MRVL_SAI_LOG_ENTER();

    if (NULL == attr) {
        MRVL_SAI_LOG_ERR("NULL value attr\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_vendor_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality vendor attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_functionality_attrib_index_prv(attr->id, functionality_attr, &index));

    if (!functionality_vendor_attr[index].setter) {
        MRVL_SAI_LOG_ERR("Attribute %s not implemented on set and defined incorrectly\n",
                     functionality_attr[index].attrib_name);
        return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0;
    }

    mrvl_sai_utl_value_to_str(attr->value, functionality_attr[index].type, MAX_VALUE_STR_LEN, value_str);
    MRVL_SAI_LOG_NTC("Set %s, key:%s, val:%s\n", functionality_attr[index].attrib_name, key_str, value_str);
    err = functionality_vendor_attr[index].setter(key, &(attr->value), functionality_vendor_attr[index].setter_arg);

    MRVL_SAI_LOG_EXIT();
    return err;
}

static sai_status_t mrvl_sai_utl_get_dispatch_attribs_handler_prv(_In_ uint32_t                            attr_count,
                                                 _Inout_ sai_attribute_t                 *attr_list,
                                                 _In_ const sai_attribute_entry_t        *functionality_attr,
                                                 _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
                                                 _In_ const sai_object_key_t             *key,
                                                 _In_ const char                         *key_str)
{
    uint32_t       ii, index;
    vendor_cache_t cache;
    sai_status_t   status;
    char           value_str[MAX_VALUE_STR_LEN];

    if ((attr_count) && (NULL == attr_list)) {
        MRVL_SAI_LOG_ERR("NULL value attr list\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_vendor_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality vendor attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&cache, 0, sizeof(cache));

    for (ii = 0; ii < attr_count; ii++) {
        assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_functionality_attrib_index_prv(attr_list[ii].id, functionality_attr, &index));

        if (!functionality_vendor_attr[index].getter) {
            MRVL_SAI_LOG_ERR("Attribute %s not implemented on get and defined incorrectly\n",
                         functionality_attr[index].attrib_name);
            return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0 + ii;
        }

        if (SAI_STATUS_SUCCESS !=
            (status =
                 functionality_vendor_attr[index].getter(key, &(attr_list[ii].value), ii, &cache,
                                                         functionality_vendor_attr[index].getter_arg))) {
            MRVL_SAI_LOG_ERR("Failed getting attrib %s\n", functionality_attr[index].attrib_name);
            return status;
        }
        mrvl_sai_utl_value_to_str(attr_list[ii].value, functionality_attr[index].type, MAX_VALUE_STR_LEN, value_str);
        MRVL_SAI_LOG_NTC("Got #%u, %s, key:%s, val:%s\n", ii, functionality_attr[index].attrib_name, key_str, value_str);
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_utl_ipv4_to_str_prv(_In_ sai_ip4_t value,
                                    _In_ uint32_t  max_length,
                                    _Out_ char    *value_str,
                                    _Out_ int     *chars_written)
{
    inet_ntop(AF_INET, &value, value_str, max_length);

    if (NULL != chars_written) {
        *chars_written = (int)strlen(value_str);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t mrvl_sai_utl_ipv6_to_str_prv(_In_ sai_ip6_t value,
                                    _In_ uint32_t  max_length,
                                    _Out_ char    *value_str,
                                    _Out_ int     *chars_written)
{
    inet_ntop(AF_INET6, value, value_str, max_length);

    if (NULL != chars_written) {
        *chars_written = (int)strlen(value_str);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_fpa_to_sai_status(int32_t fpa_status)
{
    switch (fpa_status) {
    case FPA_OK:
        return SAI_STATUS_SUCCESS;


    case FPA_ALREADY_EXIST:
        return SAI_STATUS_ITEM_ALREADY_EXISTS;

    case FPA_NOT_INITIALIZED:
        return SAI_STATUS_UNINITIALIZED;

    case FPA_NO_RESOURCE:
        return SAI_STATUS_INSUFFICIENT_RESOURCES;

    case FPA_OUT_OF_CPU_MEM:
        return SAI_STATUS_NO_MEMORY;

    case FPA_BAD_PARAM:
        return SAI_STATUS_INVALID_PARAMETER;

    case FPA_NOT_FOUND:
        return SAI_STATUS_ITEM_NOT_FOUND;
        
    case FPA_FULL:
        return SAI_STATUS_TABLE_FULL;

    case FPA_NOT_SUPPORTED:
    case FPA_NOT_IMPLEMENTED:
        return SAI_STATUS_NOT_SUPPORTED;


    case FPA_ERROR:
    case FPA_FAIL:
    default:
        return SAI_STATUS_FAILURE;
    }
}

sai_status_t mrvl_sai_utl_check_attribs_metadata(_In_ uint32_t                            attr_count,
                                    _In_ const sai_attribute_t              *attr_list,
                                    _In_ const sai_attribute_entry_t        *functionality_attr,
                                    _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
                                    _In_ sai_operation_t                     oper)
{
    uint32_t functionality_attr_count, ii, index;
    bool    attr_present[MAX_ATTRIBS_NUMBUR];

    MRVL_SAI_LOG_ENTER();
    memset(attr_present, 0, sizeof(bool)*MAX_ATTRIBS_NUMBUR);
    
    if ((attr_count) && (NULL == attr_list)) {
        MRVL_SAI_LOG_ERR("NULL value attr list\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_vendor_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality vendor attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_OPERATION_MAX <= oper) {
        MRVL_SAI_LOG_ERR("Invalid operation %d\n", oper);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_OPERATION_REMOVE == oper) {
        /* No attributes expected for remove at this point */
        return SAI_STATUS_NOT_IMPLEMENTED;
    }

    if (SAI_OPERATION_SET == oper) {
        if (1 != attr_count) {
            MRVL_SAI_LOG_ERR("Set operation supports only single attribute\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    for (functionality_attr_count = 0;
         END_FUNCTIONALITY_ATTRIBS_ID != functionality_attr[functionality_attr_count].id;
         functionality_attr_count++) {
        if (functionality_attr[functionality_attr_count].id !=
            functionality_vendor_attr[functionality_attr_count].id) {
            MRVL_SAI_LOG_ERR("Mismatch between functionality attribute and vendor attribute index %u %u %u\n",
                         functionality_attr_count, functionality_attr[functionality_attr_count].id,
                         functionality_vendor_attr[functionality_attr_count].id);
            return SAI_STATUS_FAILURE;
        }
    }

    /*attr_present = (bool*)calloc(functionality_attr_count, sizeof(bool));*/
    if ((NULL == attr_present) || (functionality_attr_count >= MAX_ATTRIBS_NUMBUR)) {
        MRVL_SAI_LOG_ERR("Can't allocate memory\n");
        return SAI_STATUS_NO_MEMORY;
    }

    for (ii = 0; ii < attr_count; ii++) {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_find_functionality_attrib_index_prv(attr_list[ii].id, functionality_attr, &index)) {
            MRVL_SAI_LOG_ERR("Invalid attribute %d\n", attr_list[ii].id);
            /*free(attr_present);*/
            return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + ii;
        }

        if ((SAI_OPERATION_CREATE == oper) &&
            (!(functionality_attr[index].valid_for_create))) {
            MRVL_SAI_LOG_ERR("Invalid attribute %s for create\n", functionality_attr[index].attrib_name);
            /*free(attr_present);*/
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + ii;
        }

        if ((SAI_OPERATION_SET == oper) &&
            (!(functionality_attr[index].valid_for_set))) {
            MRVL_SAI_LOG_ERR("Invalid attribute %s for set\n", functionality_attr[index].attrib_name);
            /*free(attr_present);*/
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + ii;
        }

        if ((SAI_OPERATION_GET == oper) &&
            (!(functionality_attr[index].valid_for_get))) {
            MRVL_SAI_LOG_ERR("Invalid attribute %s for get\n", functionality_attr[index].attrib_name);
            /*free(attr_present);*/
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + ii;
        }

        if (!(functionality_vendor_attr[index].is_supported[oper])) {
            MRVL_SAI_LOG_ERR("Not supported attribute %s\n", functionality_attr[index].attrib_name);
            /*free(attr_present);*/
            return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + ii;
        }

        if (!(functionality_vendor_attr[index].is_implemented[oper])) {
            MRVL_SAI_LOG_ERR("Not implemented attribute %s\n", functionality_attr[index].attrib_name);
            /*free(attr_present);*/
            return SAI_STATUS_ATTR_NOT_IMPLEMENTED_0 + ii;
        }

        if (attr_present[index]) {
            MRVL_SAI_LOG_ERR("Attribute %s appears twice in attribute list at index %d\n",
                         functionality_attr[index].attrib_name,
                         ii);
            /*free(attr_present);*/
            return SAI_STATUS_INVALID_ATTRIBUTE_0 + ii;
        }

        if (((SAI_ATTR_VAL_TYPE_OBJLIST == functionality_attr[index].type) &&
             (NULL == attr_list[ii].value.objlist.list)) ||
            ((SAI_ATTR_VAL_TYPE_U32LIST == functionality_attr[index].type) &&
             (NULL == attr_list[ii].value.u32list.list)) ||
            ((SAI_ATTR_VAL_TYPE_S32LIST == functionality_attr[index].type) &&
             (NULL == attr_list[ii].value.s32list.list)) ||
            ((SAI_ATTR_VAL_TYPE_VLANLIST == functionality_attr[index].type) &&
             (NULL == attr_list[ii].value.vlanlist.list))) {
            MRVL_SAI_LOG_ERR("Null list attribute %s at index %d\n",
                         functionality_attr[index].attrib_name,
                         ii);
            /*free(attr_present);*/
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + ii;
        }

        attr_present[index] = true;
    }

    if (SAI_OPERATION_CREATE == oper) {
        for (ii = 0; ii < functionality_attr_count; ii++) {
            if ((functionality_attr[ii].mandatory_on_create) &&
                (!attr_present[ii])) {
                MRVL_SAI_LOG_ERR("Missing mandatory attribute %s on create\n", functionality_attr[ii].attrib_name);
                /*free(attr_present);*/
                return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
            }
        }
    }

    /*free(attr_present);*/

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}



sai_status_t mrvl_sai_utl_find_attrib_in_list(_In_ uint32_t                       attr_count,
                                             _In_ const sai_attribute_t         *attr_list,
                                             _In_ sai_attr_id_t                  attrib_id,
                                             _Out_ const sai_attribute_value_t **attr_value,
                                             _Out_ uint32_t                     *index)
{
    uint32_t ii;

    MRVL_SAI_LOG_ENTER();

    if ((attr_count) && (NULL == attr_list)) {
        MRVL_SAI_LOG_ERR("NULL value attr list\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == attr_value) {
        MRVL_SAI_LOG_ERR("NULL value attr value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == index) {
        MRVL_SAI_LOG_ERR("NULL value index\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (ii = 0; ii < attr_count; ii++) {
        if (attr_list[ii].id == attrib_id) {
            *attr_value = &(attr_list[ii].value);
            *index      = ii;
            MRVL_SAI_LOG_EXIT();
            return SAI_STATUS_SUCCESS;
        }
    }
    MRVL_SAI_LOG_DBG("item not found %d\n",attrib_id);
    return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t mrvl_sai_utl_set_attribute(_In_ const sai_object_key_t             *key,
                                       _In_ const char                         *key_str,
                                       _In_ const sai_attribute_entry_t        *functionality_attr,
                                       _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
                                       _In_ const sai_attribute_t              *attr)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_check_attribs_metadata(1, attr, functionality_attr, functionality_vendor_attr, SAI_OPERATION_SET))) {
        MRVL_SAI_LOG_ERR("Failed attribs check, key:%s\n", key_str);
        return status;
    }

    if (SAI_STATUS_SUCCESS !=
        (status = mrvl_sai_utl_set_dispatch_attrib_handler_prv(attr, functionality_attr, functionality_vendor_attr, key, key_str))) {
        MRVL_SAI_LOG_ERR("Failed set attrib dispatch\n");
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_get_attributes(_In_ const sai_object_key_t             *key,
                                        _In_ const char                         *key_str,
                                        _In_ const sai_attribute_entry_t        *functionality_attr,
                                        _In_ const sai_vendor_attribute_entry_t *functionality_vendor_attr,
                                        _In_ uint32_t                            attr_count,
                                        _Inout_ sai_attribute_t                 *attr_list)
{
    sai_status_t status;

    MRVL_SAI_LOG_ENTER();

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_check_attribs_metadata(attr_count, attr_list, functionality_attr, functionality_vendor_attr,
                                    SAI_OPERATION_GET))) {
        MRVL_SAI_LOG_ERR("Failed attribs check, key:%s\n", key_str);
        return status;
    }

    if (SAI_STATUS_SUCCESS !=
        (status =
             mrvl_sai_utl_get_dispatch_attribs_handler_prv(attr_count, attr_list, functionality_attr, functionality_vendor_attr, key,
                                          key_str))) {
        MRVL_SAI_LOG_ERR("Failed attribs dispatch\n");
        return status;
    }

    MRVL_SAI_LOG_EXIT();
    return SAI_STATUS_SUCCESS;
}


sai_status_t mrvl_sai_utl_ipaddr_to_str(_In_ sai_ip_address_t value,
                               _In_ uint32_t         max_length,
                               _Out_ char           *value_str,
                               _Out_ int            *chars_written)
{
    int res;

    if (SAI_IP_ADDR_FAMILY_IPV4 == value.addr_family) {
        mrvl_sai_utl_ipv4_to_str_prv(value.addr.ip4, max_length, value_str, chars_written);
    } else if (SAI_IP_ADDR_FAMILY_IPV6 == value.addr_family) {
        mrvl_sai_utl_ipv6_to_str_prv(value.addr.ip6, max_length, value_str, chars_written);
    } else {
        res = snprintf(value_str, max_length, "Invalid ipaddr family %d", value.addr_family);
        if (NULL != chars_written) {
            *chars_written = res;
        }
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_ipprefix_to_str(_In_ sai_ip_prefix_t value, _In_ uint32_t max_length, _Out_ char *value_str)
{
    int      chars_written;
    uint32_t pos = 0;

    if (SAI_IP_ADDR_FAMILY_IPV4 == value.addr_family) {
        mrvl_sai_utl_ipv4_to_str_prv(value.addr.ip4, max_length, value_str, &chars_written);
        pos += chars_written;
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }
        pos += snprintf(value_str + pos, max_length - pos, " ");
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }
        mrvl_sai_utl_ipv4_to_str_prv(value.mask.ip4, max_length - pos, value_str + pos, &chars_written);
    } else if (SAI_IP_ADDR_FAMILY_IPV6 == value.addr_family) {
        mrvl_sai_utl_ipv6_to_str_prv(value.addr.ip6, max_length, value_str, &chars_written);
        pos += chars_written;
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }
        pos += snprintf(value_str + pos, max_length - pos, " ");
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }
        mrvl_sai_utl_ipv6_to_str_prv(value.mask.ip6, max_length - pos, value_str + pos, &chars_written);
    } else {
        snprintf(value_str, max_length, "Invalid addr family %d", value.addr_family);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_nexthops_to_str(_In_ uint32_t               next_hop_count,
                                 _In_ const sai_object_id_t* nexthops,
                                 _In_ uint32_t               max_length,
                                 _Out_ char                 *str)
{
    uint32_t     ii;
    uint32_t     pos = 0;
    uint32_t     nexthop_id;
    sai_status_t status;

    pos += snprintf(str, max_length, "%u hops : [", next_hop_count);
    if (pos > max_length) {
        return SAI_STATUS_SUCCESS;
    }
    for (ii = 0; ii < next_hop_count; ii++) {
        if (SAI_STATUS_SUCCESS !=
            (status = mrvl_sai_utl_object_to_type(nexthops[ii], SAI_OBJECT_TYPE_NEXT_HOP, &nexthop_id))) {
            snprintf(str + pos, max_length - pos, " invalid next hop]");
            return status;
        }

        pos += snprintf(str + pos, max_length - pos, " %u", nexthop_id);
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }
    }
    snprintf(str + pos, max_length - pos, "]");

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_value_to_str(_In_ sai_attribute_value_t      value,
                              _In_ sai_attribute_value_type_t type,
                              _In_ uint32_t                   max_length,
                              _Out_ char                     *value_str)
{
    uint32_t          ii;
    uint32_t          pos = 0;
    uint32_t          count;
    mrvl_object_id_t *mrvl_object_id;

    if (NULL == value_str) {
        MRVL_SAI_LOG_ERR("NULL value str");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *value_str = '\0';

    switch (type) {
    case SAI_ATTR_VAL_TYPE_BOOL:
        snprintf(value_str, max_length, "%u", value.booldata);
        break;

    case SAI_ATTR_VAL_TYPE_CHARDATA:
        snprintf(value_str, max_length, "%s", value.chardata);
        break;

    case SAI_ATTR_VAL_TYPE_U8:
        snprintf(value_str, max_length, "%u", value.u8);
        break;

    case SAI_ATTR_VAL_TYPE_S8:
        snprintf(value_str, max_length, "%d", value.s8);
        break;

    case SAI_ATTR_VAL_TYPE_U16:
        snprintf(value_str, max_length, "%u", value.u16);
        break;

    case SAI_ATTR_VAL_TYPE_S16:
        snprintf(value_str, max_length, "%d", value.s16);
        break;

    case SAI_ATTR_VAL_TYPE_U32:
        snprintf(value_str, max_length, "%u", value.u32);
        break;

    case SAI_ATTR_VAL_TYPE_S32:
        snprintf(value_str, max_length, "%d", value.s32);
        break;

    case SAI_ATTR_VAL_TYPE_U64:
        snprintf(value_str, max_length, "%" PRIu64, value.u64);
        break;

    case SAI_ATTR_VAL_TYPE_S64:
        snprintf(value_str, max_length, "%" PRId64, value.s64);
        break;

    case SAI_ATTR_VAL_TYPE_MAC:
        snprintf(value_str, max_length, "[%02x:%02x:%02x:%02x:%02x:%02x]",
                 value.mac[0],
                 value.mac[1],
                 value.mac[2],
                 value.mac[3],
                 value.mac[4],
                 value.mac[5]);
        break;

    /* IP is in network order */
    case SAI_ATTR_VAL_TYPE_IPV4:
        mrvl_sai_utl_ipv4_to_str_prv(value.ip4, max_length, value_str, NULL);
        break;

    case SAI_ATTR_VAL_TYPE_IPV6:
        mrvl_sai_utl_ipv6_to_str_prv(value.ip6, max_length, value_str, NULL);
        break;

    case SAI_ATTR_VAL_TYPE_IPADDR:
        mrvl_sai_utl_ipaddr_to_str(value.ipaddr, max_length, value_str, NULL);
        break;

    case SAI_ATTR_VAL_TYPE_OID:
        mrvl_object_id = (mrvl_object_id_t*)&value.oid;
        snprintf(value_str, max_length, "%s %x",
                 SAI_TYPE_STR(sai_object_type_query(value.oid)), mrvl_object_id->data);
        break;

    case SAI_ATTR_VAL_TYPE_OBJLIST:
    case SAI_ATTR_VAL_TYPE_U32LIST:
    case SAI_ATTR_VAL_TYPE_S32LIST:
    case SAI_ATTR_VAL_TYPE_VLANLIST:
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }

        count = (SAI_ATTR_VAL_TYPE_OBJLIST == type) ? value.objlist.count :
                (SAI_ATTR_VAL_TYPE_U32LIST == type) ? value.u32list.count :
                (SAI_ATTR_VAL_TYPE_S32LIST == type) ? value.s32list.count :
                value.vlanlist.count;
        pos += snprintf(value_str + pos, max_length - pos, "%u : [", count);
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }

        for (ii = 0; ii < count; ii++) {
            if (SAI_ATTR_VAL_TYPE_OBJLIST == type) {
                pos += snprintf(value_str + pos, max_length - pos, " %" PRIx64, value.objlist.list[ii]);
            } else if (SAI_ATTR_VAL_TYPE_U32LIST == type) {
                pos += snprintf(value_str + pos, max_length - pos, " %u", value.u32list.list[ii]);
            } else if (SAI_ATTR_VAL_TYPE_S32LIST == type) {
                pos += snprintf(value_str + pos, max_length - pos, " %d", value.s32list.list[ii]);
            } else if (SAI_ATTR_VAL_TYPE_VLANLIST == type) {
                pos += snprintf(value_str + pos, max_length - pos, " %u", value.vlanlist.list[ii]);
            } else {
                snprintf(value_str, max_length, "Not implemented value type %d", type);
                return SAI_STATUS_NOT_IMPLEMENTED;
            }
            if (pos > max_length) {
                return SAI_STATUS_SUCCESS;
            }
        }
        snprintf(value_str + pos, max_length - pos, "]");
        break;


    case SAI_ATTR_VAL_TYPE_ACLFIELD:
    case SAI_ATTR_VAL_TYPE_ACLACTION:
        /* TODO : implement if in case it is used */
        snprintf(value_str, max_length, "Not implemented value type %d", type);
        return SAI_STATUS_NOT_IMPLEMENTED;

    case SAI_ATTR_VAL_TYPE_UNDETERMINED:
    default:
        snprintf(value_str, max_length, "Invalid/Unsupported value type %d", type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_attr_list_to_str(_In_ uint32_t                     attr_count,
                                  _In_ const sai_attribute_t       *attr_list,
                                  _In_ const sai_attribute_entry_t *functionality_attr,
                                  _In_ uint32_t                     max_length,
                                  _Out_ char                       *list_str)
{
    uint32_t ii, index, pos = 0;
    char     value_str[MAX_VALUE_STR_LEN];

    if ((attr_count) && (NULL == attr_list)) {
        MRVL_SAI_LOG_ERR("NULL value attr list\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == functionality_attr) {
        MRVL_SAI_LOG_ERR("NULL value functionality attrib\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == list_str) {
        MRVL_SAI_LOG_ERR("NULL value str");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (0 == attr_count) {
        snprintf(list_str, max_length, "empty list");
        return SAI_STATUS_SUCCESS;
    }

    for (ii = 0; ii < attr_count; ii++) {
        assert(SAI_STATUS_SUCCESS == mrvl_sai_utl_find_functionality_attrib_index_prv(attr_list[ii].id, functionality_attr, &index));

        mrvl_sai_utl_value_to_str(attr_list[ii].value, functionality_attr[index].type, MAX_VALUE_STR_LEN, value_str);
        pos += snprintf(list_str + pos,
                        max_length - pos,
                        " #%u %s val:%s,",
                        ii,
                        functionality_attr[index].attrib_name,
                        value_str);
        if (pos > max_length) {
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_is_object_type(sai_object_id_t object_id, sai_object_type_t type)
{
    mrvl_object_id_t *mrvl_object_id = (mrvl_object_id_t*)&object_id;


    if (type != mrvl_object_id->object_type) {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_object_to_type(sai_object_id_t object_id, sai_object_type_t type, uint32_t *data)
{
    mrvl_object_id_t *mrvl_object_id = (mrvl_object_id_t*)&object_id;

    if (NULL == data) {
        MRVL_SAI_LOG_ERR("NULL data value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (type != mrvl_object_id->object_type) {
        MRVL_SAI_LOG_ERR("Expected object %s got %s\n", SAI_TYPE_STR(type), SAI_TYPE_STR(mrvl_object_id->object_type));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *data = mrvl_object_id->data;
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_object_to_ext_type(sai_object_id_t object_id, sai_object_type_t type, uint32_t *port_id, uint32_t *lag_id)
{
    mrvl_object_id_t *mrvl_object_id = (mrvl_object_id_t*)&object_id;

    if ((NULL == port_id) || (NULL == lag_id)) {
        MRVL_SAI_LOG_ERR("NULL value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (type != mrvl_object_id->object_type) {
        MRVL_SAI_LOG_ERR("Expected object %s got %s\n", SAI_TYPE_STR(type), SAI_TYPE_STR(mrvl_object_id->object_type));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *port_id = mrvl_object_id->data;
    *lag_id = (mrvl_object_id->reserved[1] << 8) || (mrvl_object_id->reserved[0]);
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_create_object(sai_object_type_t type, uint32_t data, sai_object_id_t *object_id)
{
    mrvl_object_id_t *mrvl_object_id = (mrvl_object_id_t*)object_id;

    if (NULL == object_id) {
        MRVL_SAI_LOG_ERR("NULL object id value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (type >= SAI_OBJECT_TYPE_MAX) {
        MRVL_SAI_LOG_ERR("Unknown object type %d\n", type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(mrvl_object_id, 0, sizeof(*mrvl_object_id));
    mrvl_object_id->data        = data;
    mrvl_object_id->object_type = type;
    return SAI_STATUS_SUCCESS;
}

sai_status_t mrvl_sai_utl_create_ext_object(sai_object_type_t type, uint32_t port_id, uint32_t lag_id, sai_object_id_t *object_id)
{
    mrvl_object_id_t *mrvl_object_id = (mrvl_object_id_t*)object_id;

    if (NULL == object_id) {
        MRVL_SAI_LOG_ERR("NULL object id value\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (type >= SAI_OBJECT_TYPE_MAX) {
        MRVL_SAI_LOG_ERR("Unknown object type %d\n", type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(mrvl_object_id, 0, sizeof(*mrvl_object_id));
    mrvl_object_id->data        = port_id;
    mrvl_object_id->reserved[0] = lag_id && 0xFF;
    mrvl_object_id->reserved[1] = (lag_id >> 8) && 0xFF;
    mrvl_object_id->object_type = type;
    return SAI_STATUS_SUCCESS;
}


sai_status_t mrvl_sai_utl_fill_objlist(sai_object_id_t *data, uint32_t count, sai_object_list_t *list)
{
    return mrvl_sai_utl_fill_genericlist_prv(sizeof(sai_object_id_t), (void*)data, count, (void*)list);
}

sai_status_t mrvl_sai_utl_fill_u32list(uint32_t *data, uint32_t count, sai_u32_list_t *list)
{
    return mrvl_sai_utl_fill_genericlist_prv(sizeof(uint32_t), (void*)data, count, (void*)list);
}

sai_status_t mrvl_sai_utl_fill_s32list(int32_t *data, uint32_t count, sai_s32_list_t *list)
{
    return mrvl_sai_utl_fill_genericlist_prv(sizeof(int32_t), (void*)data, count, (void*)list);
}

sai_status_t mrvl_sai_utl_fill_vlanlist(sai_vlan_id_t *data, uint32_t count, sai_vlan_list_t *list)
{
    return mrvl_sai_utl_fill_genericlist_prv(sizeof(sai_vlan_id_t), (void*)data, count, (void*)list);
}

#define LOG_ENTRY_SIZE_MAX 1024

/*
 * Routine Description:
 *  Create new l2 interface group entry.
 *  l2 interface key must have vlan + port
 *      
 * Arguments:
 *    [in] port - l2 interface port
 *    [in] vlan - l2 interface vlan
 *    [in] tag - should the port output be tagged or not
 *    [in] tag_overwrite - if group already exist - tag should be changed or not
 *    [inout] group - the l3 group entry 
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_create_l2_int_group(_In_ uint32_t port, _In_ uint32_t vlan, _In_ uint32_t tag, _In_ bool tag_overwrite, _Inout_ uint32_t *group)
{
    FPA_GROUP_TABLE_ENTRY_STC  group_entry;
    FPA_GROUP_BUCKET_ENTRY_STC bucket;
    FPA_STATUS fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
    parsed_group_identifier.portNum = port;
    parsed_group_identifier.vlanId = vlan;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, group);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    group_entry.groupIdentifier = *group;
    group_entry.groupTypeSemantics = FPA_GROUP_INDIRECT;
    group_entry.selectionAlgorithm = 0;
    fpa_status = fpaLibGroupTableEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, &group_entry);
    if (fpa_status == FPA_OK) {
        /* new group crated - create with default values */
        bucket.groupIdentifier = *group;
        bucket.index = 0;
        bucket.type = FPA_GROUP_BUCKET_L2_INTERFACE_E;
        bucket.data.l2Interface.outputPort = port;
        bucket.data.l2Interface.popVlanTagAction = (tag == SAI_VLAN_PORT_TAGGED)? false: true ; 
        fpa_status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
    }
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add group %d entry status = %d\n", *group, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            if (tag_overwrite == false) { 
                /* when group is created from fdb */
                return SAI_STATUS_SUCCESS;
            } else { 
                /* group eas created from fdb and now it is updated with the real tagging mode */
                fpa_status = fpaLibGroupEntryBucketGet(SAI_DEFAULT_ETH_SWID_CNS, *group, 0, &bucket);
                if (fpa_status != FPA_OK){
                    MRVL_SAI_LOG_ERR("Failed to add group %d entry status = %d\n", *group, fpa_status); 
                    return SAI_STATUS_FAILURE;
                } 
                if (bucket.data.l2Interface.popVlanTagAction != ((tag == SAI_VLAN_PORT_TAGGED)? false: true)) {
                    bucket.data.l2Interface.popVlanTagAction = (tag == SAI_VLAN_PORT_TAGGED)? false: true; 
                    fpa_status = fpaLibGroupEntryBucketModify(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
                }
                if (fpa_status != FPA_OK){
                    return SAI_STATUS_FAILURE;
                } else {
                    return SAI_STATUS_SUCCESS; 
                }
            }
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

/*
 * Routine Description:
 *  delete  l2 interface group entry.
 *  l2 interface key must have vlan + port
 *      
 * Arguments:
 *    [in] port - l2 interface port
 *    [in] vlan - l2 interface vlan
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_delete_l2_int_group(_In_ uint32_t port,
                                              _In_ uint32_t vlan)
{
    uint32_t                    groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
    parsed_group_identifier.portNum = port;
    parsed_group_identifier.vlanId = vlan;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to delete group identifier vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  Create new l2 interface group entry.
 *  l2 interface key must have port + vlan '0'
 *      
 * Arguments:
 *    [in] port - l2 interface port
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_create_l2_int_group_wo_vlan(_In_ uint32_t port)
{	
	uint16_t                     vlanId = 0;
	uint16_t                     egressVlanIsTag = 0;	
    FPA_GROUP_TABLE_ENTRY_STC    groupEntry;
    FPA_GROUP_BUCKET_ENTRY_STC   bucket;    
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsedGroupIdentifier;	
    uint32_t                     groupIdentifier;
    FPA_STATUS                   status; 	    
    
    memset(&groupEntry, 0, sizeof(groupEntry));
    memset(&bucket, 0, sizeof(bucket));
    memset(&parsedGroupIdentifier, 0, sizeof(parsedGroupIdentifier));

    parsedGroupIdentifier.groupType = FPA_GROUP_L2_INTERFACE_E;
    parsedGroupIdentifier.portNum = port;
    parsedGroupIdentifier.vlanId = vlanId;
    /* build the 32 bit opaque group identifier */
    status = fpaLibGroupIdentifierBuild(&parsedGroupIdentifier, &groupIdentifier); 
    if (status != FPA_OK){
        MRVL_SAI_LOG_ERR("fpaLibGroupIdentifierBuild: fail vlan %d port %d status %d\n", 
                         vlanId, port, (int)status);		        
        return SAI_STATUS_FAILURE;
    }
   
    groupEntry.groupIdentifier = groupIdentifier; 
    groupEntry.groupTypeSemantics = FPA_GROUP_INDIRECT; /* This group supports only a single bucket */
    groupEntry.selectionAlgorithm = 0;
    status = fpaLibGroupTableEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, &groupEntry);
    if (status != FPA_OK){
        MRVL_SAI_LOG_ERR("fpaLibGroupTableEntryAdd: fail groupIdentifier %d status %d\n", 
                         groupIdentifier, (int)status);		        
        return SAI_STATUS_FAILURE;
    }
        
    /* set egress packet */ 
    bucket.groupIdentifier = groupIdentifier;
    bucket.index = 0; /* index of the group bucket in the group */
    bucket.type = FPA_GROUP_BUCKET_L2_INTERFACE_E;
    bucket.data.l2Interface.outputPort = port;
    if (egressVlanIsTag) {
        bucket.data.l2Interface.popVlanTagAction = false; /* output port is tagged */ 
    }
    else
    {
        bucket.data.l2Interface.popVlanTagAction = true; /* output port is untagged */ 
    }
    /* add entry action bucket to the new group */
    status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket); 
    if (status != FPA_OK){
        MRVL_SAI_LOG_ERR("fpaLibGroupEntryBucketAdd: fail groupIdentifier %d status %d\n", 
                         groupIdentifier, (int)status);		
        return SAI_STATUS_FAILURE;              
    }
    
    return SAI_STATUS_SUCCESS;    
    
}

sai_status_t mrvl_sai_utl_l2_int_group_get_tagging_mode(_In_ uint32_t port, _In_ uint32_t vlan, _Out_ sai_vlan_tagging_mode_t *tag_mode)
{
    FPA_GROUP_BUCKET_ENTRY_STC bucket;
    FPA_STATUS fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    uint32_t                   groupIdentifier;
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
    parsed_group_identifier.portNum = port;
    parsed_group_identifier.vlanId = vlan;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    /* read group id to get tagging mode */
    fpa_status = fpaLibGroupEntryBucketGet(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, 0, &bucket);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to found group vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    *tag_mode = (bucket.data.l2Interface.popVlanTagAction == true)? SAI_VLAN_PORT_UNTAGGED: SAI_VLAN_PORT_TAGGED;
    return SAI_STATUS_SUCCESS; 
}


sai_status_t mrvl_sai_utl_l2_int_group_set_tagging_mode(_In_ uint32_t port, _In_ uint32_t vlan, _In_ sai_vlan_tagging_mode_t tag_mode)
{
    FPA_GROUP_BUCKET_ENTRY_STC bucket;
    FPA_STATUS fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    uint32_t                   groupIdentifier;
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
    parsed_group_identifier.portNum = port;
    parsed_group_identifier.vlanId = vlan;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    /* read group id to get tagging mode */
    fpa_status = fpaLibGroupEntryBucketGet(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, 0, &bucket);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to found group vlan %d port %d \n", vlan, port);
        return SAI_STATUS_FAILURE;
    }
    if (tag_mode >= SAI_VLAN_PORT_PRIORITY_TAGGED) {
        MRVL_SAI_LOG_ERR("Failed to invalid tagging mode %d \n", tag_mode);
        return SAI_STATUS_FAILURE;
    }
    bucket.data.l2Interface.popVlanTagAction = (tag_mode == SAI_VLAN_PORT_TAGGED)? false: true; 
    fpa_status = fpaLibGroupEntryBucketModify(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
    if (fpa_status != FPA_OK){
        return SAI_STATUS_FAILURE;
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

/*
 * Routine Description:
 *  Create new l3 group entry.
 *  the FPA group must be created on port -  so if the group is created on vlan
 *  we read the fdb table to get the port on which the cmac was learned
 *     
 * Arguments:
 *    [in] index - the nbr index used to create the group key
 *    [in] dst_mac - the entry dst mac address
 *    [in] rif_idx - the rif on which the group wll be crated
 *    [in] action - currently not use (in the future will be frw or mirror)
 *    [inout] group - the l3 group entry 
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_create_l3_unicast_group(_In_ uint32_t     index, 
                                                  _In_ sai_mac_t    dst_mac, 
                                                  _In_ uint32_t     rif_idx, 
                                                  _In_ sai_packet_action_t action,
                                                  _Inout_ uint32_t  *group)
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_GROUP_TABLE_ENTRY_STC   group_entry;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;
    FPA_FLOW_TABLE_ENTRY_STC    flow_entry;
    sai_status_t                status;
    mrvl_sai_rif_table_t        rif_entry;
    uint32_t                    ref_group;
    uint64_t                    cookie;
    
    memset(&group_entry, 0, sizeof(group_entry));
    memset(&bucket, 0, sizeof(bucket));
    memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
    
    parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, group);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    status = mrvl_sai_rif_get_entry(rif_idx, &rif_entry);
    if (status != SAI_STATUS_SUCCESS) {
        MRVL_SAI_LOG_ERR("Can't get rif %d\n", rif_idx);
        return status;
    }
    if (rif_entry.intf_type == SAI_ROUTER_INTERFACE_TYPE_VLAN) {
        /* get port number from fdb table */
        cookie = MRVL_SAI_FDB_CREATE_COOKIE_MAC(rif_entry.port_vlan_id, dst_mac);
        flow_entry.cookie = cookie;
        fpa_status = fpaLibFlowTableGetByCookie(SAI_DEFAULT_ETH_SWID_CNS, FPA_FLOW_TABLE_TYPE_L2_BRIDGING_E, &flow_entry);
        if ((fpa_status != FPA_OK) || (flow_entry.data.l2_bridging.groupId == 0xFFFFFFFF)) {
            MRVL_SAI_LOG_ERR("Can't find mac and vlan in FDB table\n");
            return mrvl_sai_utl_fpa_to_sai_status(fpa_status);
        }
        ref_group = flow_entry.data.l2_bridging.groupId;
    }
    
    group_entry.groupIdentifier = *group;
    group_entry.groupTypeSemantics = FPA_GROUP_INDIRECT;
    group_entry.selectionAlgorithm = 0;
    fpa_status = fpaLibGroupTableEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, &group_entry);
    if (fpa_status == FPA_OK) {
        /* new group crated - create with default values */
        bucket.groupIdentifier = *group;
        bucket.index = 0;
        bucket.type = FPA_GROUP_BUCKET_L3_UNICAST_E;
        memcpy(&bucket.data.l3Unicast.dstMac, dst_mac, 6);;
        mrvl_sai_rif_get_mtu_size(rif_idx, &bucket.data.l3Unicast.mtu);
        memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
        parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
        if (rif_entry.intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
            parsed_group_identifier.portNum = rif_entry.port_vlan_id; 
            fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &ref_group);
            if (fpa_status != FPA_OK){
                MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", 
                                 parsed_group_identifier.vlanId, 
                                 parsed_group_identifier.portNum);
                fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, *group);
                return SAI_STATUS_FAILURE;
            }
            memset(&group_entry, 0, sizeof(group_entry));
            fpa_status = fpaLibGroupTableGetEntry(SAI_DEFAULT_ETH_SWID_CNS, ref_group, &group_entry);
            if (fpa_status != FPA_OK){
                MRVL_SAI_LOG_ERR("Failed - referance group 0x%x does not exist\n", ref_group);
                fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, *group);
                return SAI_STATUS_FAILURE;
            }        
        } else {
            bucket.data.l3Unicast.vlanId = rif_entry.port_vlan_id;
        }
        bucket.data.l3Unicast.refGroupId = ref_group;        
        memcpy(&bucket.data.l3Unicast.srcMac, rif_entry.src_mac, 6);
        fpa_status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed - to add bucket\n");
            fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, *group);
            return SAI_STATUS_FAILURE;
        }        
    }
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add group 0x%x entry status = %d\n", *group, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

/*
 * Routine Description:
 *  delete l3 group entry.
 *  l3 key must have index
 *      
 * Arguments:
 *    [in] index - l3 group index
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_delete_l3_unicast_group(_In_ uint32_t index)
{
    uint32_t                    groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    
    parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to delete group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  modify l3 group entry mac address.
 *  l3 key must have index
 *      
 * Arguments:
 *    [in] index - l3 group index
 *    [in] dst_mac - new mac address
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_modify_l3_unicast_group(_In_ uint32_t     index, 
                                                  _In_ sai_mac_t    dst_mac)
{
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    uint32_t                    group;
    
    memset(&bucket, 0, sizeof(bucket));
    memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
    
    parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &group);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupEntryBucketGet(SAI_DEFAULT_ETH_SWID_CNS, group,0, &bucket);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed - to get group 0x%x does not exist\n", group);
        return SAI_STATUS_FAILURE;
    }        
    memcpy(&bucket.data.l3Unicast.dstMac, dst_mac, 6); 
    
    fpa_status = fpaLibGroupEntryBucketModify(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
    if (fpa_status == FPA_NOT_FOUND){
        MRVL_SAI_LOG_ERR("Failed - to modify group 0x%x not found\n", group);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }        
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed - to modify group 0x%x \n", group);
        return SAI_STATUS_FAILURE;
    }        
    return SAI_STATUS_SUCCESS; 
}

sai_status_t mrvl_sai_utl_l3_ecmp_group_add_bucket(_In_ uint32_t     group_identifier,
                                                   _In_ uint32_t     bucket_index, 
                                                   _In_ sai_object_id_t next_hop)
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;
    sai_status_t                status;
    uint32_t                    nh_idx, nbr_idx;
    
    bucket.groupIdentifier = group_identifier; 
    bucket.type = FPA_GROUP_BUCKET_L2_REFERENCE_E;
    bucket.index = bucket_index;
    if (mrvl_sai_utl_is_object_type(next_hop, SAI_OBJECT_TYPE_NEXT_HOP) == SAI_STATUS_SUCCESS){
        mrvl_sai_utl_object_to_type(next_hop, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx);
        status = mrvl_sai_next_hop_get_nbr_id(nh_idx, &nbr_idx);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("nh_idx  %d not exist\n", nh_idx);
            fpa_status = FPA_FAIL;
        }
        parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
        parsed_group_identifier.index = nbr_idx;
        fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &bucket.data.l2Reference.referenceGroupId);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", nbr_idx);
        }
        fpa_status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", bucket_index);
        }        
    } else {
        fpa_status = FPA_FAIL;
    }
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add bucket 0x%x entry status = %d\n", bucket_index, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

sai_status_t mrvl_sai_utl_l3_ecmp_group_bucket_get_index(_In_ uint32_t     group_identifier,
                                                         _In_ sai_object_id_t next_hop,
                                                         _Out_ uint32_t    *bucket_index)
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;
    sai_status_t                status;
    uint32_t                    nh_idx, nbr_idx;

    if (mrvl_sai_utl_is_object_type(next_hop, SAI_OBJECT_TYPE_NEXT_HOP) == SAI_STATUS_SUCCESS) {
        mrvl_sai_utl_object_to_type(next_hop, SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx);
        status = mrvl_sai_next_hop_get_nbr_id(nh_idx, &nbr_idx);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("nh_idx  %d not exist\n", nh_idx);
            fpa_status = FPA_FAIL;
        }
        parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
        parsed_group_identifier.index = nbr_idx;
        fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &bucket.data.l2Reference.referenceGroupId);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", nbr_idx);
        }
        bucket.groupIdentifier = group_identifier; 
        bucket.type = FPA_GROUP_BUCKET_L2_REFERENCE_E;
        fpa_status = fpaLibGroupEntryBucketGetIndexByEntry(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
        if (fpa_status == FPA_OK){
            *bucket_index = bucket.index; 
                return SAI_STATUS_SUCCESS; 
        } else if (fpa_status == FPA_NOT_FOUND) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        } 
    }
    return SAI_STATUS_FAILURE; 
}

sai_status_t mrvl_sai_utl_l3_ecmp_group_del_bucket(_In_ uint32_t     group_identifier,
                                                   _In_ sai_object_id_t next_hop)
{
    sai_status_t                status;
    uint32_t                    bucket_index;
    
    status = mrvl_sai_utl_l3_ecmp_group_bucket_get_index(group_identifier, next_hop, &bucket_index);
    if (status == SAI_STATUS_SUCCESS) {
        fpaLibGroupEntryBucketDelete(SAI_DEFAULT_ETH_SWID_CNS, group_identifier, bucket_index);
        return SAI_STATUS_SUCCESS; 
    }
    return status;
}


/*
 * Routine Description:
 *  Create new l3 ecmp group entry.
 * 
 * Arguments:
 *    [in] index - the ecmp index used to create the group key
 *    [in] algorithm - the ecmp algorithm (hash/random)
 *    [in] ecmp_objlist - next hop list
 *    [inout] group - the l3 ecmp group entry 
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_create_l3_ecmp_group(_In_ uint32_t     index, 
                                               _In_ uint32_t     algorithm,
                                               _In_ const sai_object_list_t *ecmp_objlist, 
                                               _Out_ uint32_t  *group)
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_GROUP_TABLE_ENTRY_STC   group_entry;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;
    sai_status_t                status;
    uint32_t                    member_idx;
    
    memset(&group_entry, 0, sizeof(group_entry));
    memset(&bucket, 0, sizeof(bucket));
    memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
    
    parsed_group_identifier.groupType = FPA_GROUP_L3_ECMP_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, group);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group ecmp identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    group_entry.groupIdentifier = *group;
    group_entry.groupTypeSemantics = FPA_GROUP_SELECT;
    group_entry.selectionAlgorithm = algorithm;
    fpa_status = fpaLibGroupTableEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, &group_entry);
    if (fpa_status == FPA_OK) {
      /*  bucket.groupIdentifier = *group; 
        bucket.type = FPA_GROUP_BUCKET_L2_REFERENCE_E;*/
        for (member_idx = 0; member_idx < ecmp_objlist->count; member_idx++) {
            status = mrvl_sai_utl_l3_ecmp_group_add_bucket(*group, member_idx, ecmp_objlist->list[member_idx]);
            if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_ALREADY_EXISTS)){
                MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", member_idx);
                fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, *group);
                return SAI_STATUS_FAILURE;
            }
        }
/*            bucket.index = member_idx;
            if (mrvl_sai_utl_is_object_type(ecmp_objlist->list[member_idx], SAI_OBJECT_TYPE_NEXT_HOP) == SAI_STATUS_SUCCESS){
                mrvl_sai_utl_object_to_type(ecmp_objlist->list[member_idx], SAI_OBJECT_TYPE_NEXT_HOP, &nh_idx);
                status = mrvl_sai_next_hop_get_nbr_id(nh_idx, &nbr_idx);
                if (status != SAI_STATUS_SUCCESS) {
                    MRVL_SAI_LOG_ERR("nh_idx  %d not exist\n", nh_idx);
                    fpa_status = FPA_FAIL;
                    break;
                }
                parsed_group_identifier.groupType = FPA_GROUP_L3_UNICAST_E;
                parsed_group_identifier.index = nbr_idx;
                fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &bucket.data.l2Reference.referenceGroupId);
                if (fpa_status != FPA_OK){
                    MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", nbr_idx);
                    break;
                }
                fpa_status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
                if (fpa_status != FPA_OK){
                    MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", member_idx);
                    break;
                }        
            } else {
                fpa_status = FPA_FAIL;
                break;
            }
        }
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", member_idx);
            fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, *group);
            return SAI_STATUS_FAILURE;
        }*/
    }
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add group 0x%x entry status = %d\n", *group, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

/*
 * Routine Description:
 *  delete l3 ecmp group entry.
 *  l3 key must have index
 *      
 * Arguments:
 *    [in] index - l3 ecmp group index
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_delete_l3_ecmp_group(_In_ uint32_t index)
{
    uint32_t                    groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    
    parsed_group_identifier.groupType = FPA_GROUP_L3_ECMP_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to delete group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  get l3 ecmp group bucket list.
 *  l3 key must have index
 *      
 * Arguments:
 *    [in] index - l3 gecmp roup index
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_get_l3_ecmp_group_bucket_list(_In_ uint32_t index, 
                                                        _Inout_ sai_object_list_t *ecmp_objlist)
{
    uint32_t                    counter, ecmp_size, bucket_index, groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    uint32_t                    nh_idx;
    sai_status_t                status;
    
    ecmp_size = ecmp_objlist->count;
    if ((ecmp_size == 0) || (ecmp_objlist == NULL)){
        MRVL_SAI_LOG_ERR("Invalid ecmp list\n");
        return SAI_STATUS_FAILURE;
    }
    parsed_group_identifier.groupType = FPA_GROUP_L3_ECMP_E; 
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    bucket_index = 0xFFFFFFFF;
    counter = 0;
    while (fpa_status == FPA_OK) {
        fpa_status = fpaLibGroupEntryBucketGetNext(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, bucket_index, &bucket); 
        if (fpa_status != FPA_OK) {
            break;
        }
        fpaLibGroupIdentifierParse(bucket.data.l2Reference.referenceGroupId, &parsed_group_identifier);
        status = mrvl_sai_neighbor_get_next_hop(parsed_group_identifier.index ,&nh_idx);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Failed to get group bucket index %d \n", bucket_index);
            return SAI_STATUS_FAILURE;
        }
        
        if (counter < ecmp_size) {
            mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_NEXT_HOP, nh_idx, &ecmp_objlist->list[counter]);
        }
        counter++;
        bucket_index = bucket.index;
    }
    ecmp_objlist->count = counter;
    
    if (counter > ecmp_size) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    } else {
        return SAI_STATUS_SUCCESS;
    }
}

sai_status_t mrvl_sai_utl_update_l3_ecmp_group_bucket_list(_In_ uint32_t index, 
                                                           _In_ sai_object_list_t *ecmp_objlist,
                                                           _In_ uint32_t operation,
                                                           _Out_ uint32_t *numChanged)
{
    uint32_t                    ecmp_size, bucket_index, groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    FPA_GROUP_TABLE_ENTRY_STC   groupEntry;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    uint32_t                    member_idx, i, empty_index;
    sai_status_t                status;
    
    ecmp_size = ecmp_objlist->count;
    if ((ecmp_size == 0) || (ecmp_objlist == NULL)){
        MRVL_SAI_LOG_ERR("Invalid ecmp list\n");
        return SAI_STATUS_FAILURE;
    }
    parsed_group_identifier.groupType = FPA_GROUP_L3_ECMP_E; 
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupTableGetEntry(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, &groupEntry);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to find group identifier index %d \n", groupIdentifier);
        return SAI_STATUS_FAILURE;
    }
    *numChanged = 0;
    switch (operation) {
    case MRVL_SAI_UTL_MODIFY:
       bucket_index = 0xFFFFFFFF;
        fpa_status =  fpaLibGroupEntryBucketDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, bucket_index);
        if (fpa_status != FPA_OK) {
            fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
            return SAI_STATUS_FAILURE;
        }
        for (member_idx = 0; member_idx < ecmp_objlist->count; member_idx++) {
            status = mrvl_sai_utl_l3_ecmp_group_add_bucket(groupIdentifier, member_idx, ecmp_objlist->list[member_idx]);
            if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_ALREADY_EXISTS)){
                MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", member_idx);
                fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
                return SAI_STATUS_FAILURE;
            }
        }
        *numChanged = ecmp_objlist->count;
        break;
    case MRVL_SAI_UTL_ADD:
        empty_index = 0;
        i = 0xFFFFFFFF;
        for (member_idx = 0; member_idx < ecmp_objlist->count; member_idx++) {
            status = mrvl_sai_utl_l3_ecmp_group_bucket_get_index(groupIdentifier, ecmp_objlist->list[member_idx], &bucket_index);
            if (status == SAI_STATUS_SUCCESS){ /* bucket already inside*/
                continue;
            } else if (status != SAI_STATUS_ITEM_NOT_FOUND) {
                MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", member_idx);
                fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
                return SAI_STATUS_FAILURE;
            } else {
                /* find index to add bucket */
                while (fpa_status == FPA_OK) {
                    fpa_status = fpaLibGroupEntryBucketGetNext(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, i, &bucket); 
                    if ((fpa_status != FPA_OK) && (fpa_status != FPA_NO_MORE)) {
                        MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", index);
                        /*fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);*/
                        return SAI_STATUS_FAILURE;
                    } else if ((fpa_status == FPA_NO_MORE) || (empty_index < bucket.index)) {
                        status = mrvl_sai_utl_l3_ecmp_group_add_bucket(groupIdentifier, empty_index, ecmp_objlist->list[member_idx]);
                        if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_ALREADY_EXISTS)){
                            MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", member_idx);
                            /*fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);*/
                            return SAI_STATUS_FAILURE;
                        } else {
                            *numChanged +=1;
                            i = empty_index;
                            empty_index++;
                            break;
                        }
                    } else {
                        i = bucket.index; 
                        empty_index++;
                    }
                }
                
            }
        }
        break;
    case MRVL_SAI_UTL_DEL:                
        for (member_idx = 0; member_idx < ecmp_objlist->count; member_idx++) {
            status = mrvl_sai_utl_l3_ecmp_group_del_bucket(groupIdentifier, ecmp_objlist->list[member_idx]);
            if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_NOT_FOUND)){
                MRVL_SAI_LOG_ERR("Failed - to del bucket idx %d\n", member_idx);
                /*fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);*/
                return SAI_STATUS_FAILURE;
            }
            *numChanged +=1;
        }
        break;
    default:
        MRVL_SAI_LOG_ERR("Failed operation %d not supported\n", operation);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    return SAI_STATUS_SUCCESS; 
}
  
/*
 * Routine Description:
 *  Create new empty l2 lag group entry.
 * 
 * Arguments:
 *    [in] index - the l2 lag index used to create the group key 
 *    [out] group - the l2 lag group entry 
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_create_l2_lag_group(_In_ uint32_t    index,                                                                                               
                                              _Out_ uint32_t  *group)
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier = {0};
    FPA_GROUP_TABLE_ENTRY_STC   group_entry;    
    FPA_STATUS                  fpa_status;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;      
    
    memset(&group_entry, 0, sizeof(group_entry));
    memset(&bucket, 0, sizeof(bucket));
    memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_LAG_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, group);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to create group lag identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    group_entry.groupIdentifier = *group;
    group_entry.groupTypeSemantics = FPA_GROUP_SELECT;
    group_entry.selectionAlgorithm = 0; /* hash */
    fpa_status = fpaLibGroupTableEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, &group_entry);
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add group 0x%x entry status = %d\n", *group, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            MRVL_SAI_LOG_ERR("group 0x%x already exists entry status = %d\n", *group, fpa_status); 
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

/*
 * Routine Description:
 *  delete l2 lag group entry.
 *      
 * Arguments:
 *    [in] index - l2 lag group index
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_delete_l2_lag_group(_In_ uint32_t index)
{
    uint32_t                    groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier = {0};
    FPA_STATUS                  fpa_status;
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_LAG_E;
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to delete group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *  add port to l2 lag group entry.
 *      
 * Arguments:
 *    [in] group_identifier - l2 reference group identifier
 *    [in] bucket_index - bucket index 
 *    [in] port_oid - port object id
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t mrvl_sai_utl_l2_lag_group_add_bucket(_In_ uint32_t        group_identifier,
                                                         _In_ uint32_t        bucket_index, 
                                                         _In_ sai_object_id_t port_oid) 
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier = {0};
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;    
    uint32_t                    port_idx;
    
    bucket.groupIdentifier = group_identifier; 
    bucket.type = FPA_GROUP_BUCKET_L2_REFERENCE_E;
    bucket.index = bucket_index;
    if (mrvl_sai_utl_is_object_type(port_oid, SAI_OBJECT_TYPE_PORT) == SAI_STATUS_SUCCESS){
        mrvl_sai_utl_object_to_type(port_oid, SAI_OBJECT_TYPE_PORT, &port_idx);
        
        parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
        parsed_group_identifier.portNum = port_idx;
        fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &bucket.data.l2Reference.referenceGroupId);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed to create group identifier port_idx %d \n", port_idx);
        }
        fpa_status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed - to add bucket idx %d\n", bucket_index);
        }        
    } else {
        fpa_status = FPA_FAIL;
    }
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add bucket 0x%x entry status = %d\n", bucket_index, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}

/*
 * Routine Description:
 *  get l2 lag group bucket list.
 *  l2 key must have index
 *      
 * Arguments:
 *    [in] index - l2 lag roup index
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_get_l2_lag_group_bucket_list(_In_ uint32_t             lag_idx, 
                                                       _Inout_ sai_object_list_t *port_objlist)
{
    uint32_t                    counter, lag_size, bucket_index, groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier = {0};
    FPA_STATUS                  fpa_status;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;    
    
    lag_size = port_objlist->count;
  
    if(lag_size == 0) {
        MRVL_SAI_LOG_NTC("\nport list count is zero for lag_idx %d",lag_idx);
    }
    parsed_group_identifier.groupType = FPA_GROUP_L2_LAG_E; 
    parsed_group_identifier.index = lag_idx;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", lag_idx);
        return SAI_STATUS_FAILURE;
    }
    bucket_index = 0xFFFFFFFF;
    counter = 0;
    while (fpa_status == FPA_OK) {
        fpa_status = fpaLibGroupEntryBucketGetNext(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, bucket_index, &bucket); 
        if (fpa_status != FPA_OK) {
            break;
        }
        fpaLibGroupIdentifierParse(bucket.data.l2Reference.referenceGroupId, &parsed_group_identifier);        
        
        if (counter < lag_size) {
            mrvl_sai_utl_create_object(SAI_OBJECT_TYPE_PORT, parsed_group_identifier.portNum, &port_objlist->list[counter]);
        }
        else
        {
             break;
        }
        counter++;
        bucket_index = bucket.index;
    }
    port_objlist->count = counter;
    
    if (counter > lag_size) {
        return SAI_STATUS_BUFFER_OVERFLOW;
    } else {
        return SAI_STATUS_SUCCESS;
    }
}

/*
 * Routine Description:
 *  get port's bucket index from l2 lag group entry.
 *      
 * Arguments:
 *    [in] group_identifier - l2 reference group identifier
 *    [in] port_oid - port object id   
 *    [out] bucket_index - bucket index 
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t mrvl_sai_utl_l2_lag_group_bucket_get_index(_In_ uint32_t        group_identifier,
                                                               _In_ sai_object_id_t port_oid,
                                                               _Out_ uint32_t       *bucket_index)
{
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier = {0};
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;    
    uint32_t                    port_idx;

    if (mrvl_sai_utl_is_object_type(port_oid, SAI_OBJECT_TYPE_PORT) == SAI_STATUS_SUCCESS) {
        mrvl_sai_utl_object_to_type(port_oid, SAI_OBJECT_TYPE_PORT, &port_idx);        
        parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
        parsed_group_identifier.portNum = port_idx;
        fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &bucket.data.l2Reference.referenceGroupId);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed to create group identifier index %d \n", port_idx);
            return SAI_STATUS_FAILURE;
        }
        bucket.groupIdentifier = group_identifier; 
        bucket.type = FPA_GROUP_BUCKET_L2_REFERENCE_E;
        fpa_status = fpaLibGroupEntryBucketGetIndexByEntry(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
        if (fpa_status == FPA_OK){
            *bucket_index = bucket.index; 
                return SAI_STATUS_SUCCESS; 
        } else if (fpa_status == FPA_NOT_FOUND) {
            return SAI_STATUS_ITEM_NOT_FOUND;
        } 
    }
    return SAI_STATUS_FAILURE; 
}

/*
 * Routine Description:
 *  delete port's bucket from l2 lag group entry.
 *      
 * Arguments:
 *    [in] group_identifier - l2 reference group identifier
 *    [in] port_oid - port object id   
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t mrvl_sai_utl_l2_lag_group_del_bucket(_In_ uint32_t        group_identifier,
                                                         _In_ sai_object_id_t port_oid)
{
    sai_status_t                status;
    uint32_t                    bucket_index;
    
    status = mrvl_sai_utl_l2_lag_group_bucket_get_index(group_identifier, port_oid, &bucket_index);
    if (status == SAI_STATUS_SUCCESS) {
        fpaLibGroupEntryBucketDelete(SAI_DEFAULT_ETH_SWID_CNS, group_identifier, bucket_index);
        return SAI_STATUS_SUCCESS; 
    }
    return status;
}

/*
 * Routine Description:
 *  update l2 lag group member attributes entry.
 *      
 * Arguments:
 *    [in] index - l2 lag group index
 *    [in] port_oid - port object id
 *    [in] operation - the operation to do on lag member
 *    [out] numChanged - number of changed ports
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t mrvl_sai_utl_update_l2_lag_group_bucket(_In_  uint32_t        index, 
                                                     _In_  sai_object_id_t port_oid,
                                                     _In_  uint32_t        operation,
                                                     _Out_ uint32_t       *numChanged)
{
    uint32_t                    bucket_index, groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier = {0};
    FPA_STATUS                  fpa_status;
    FPA_GROUP_TABLE_ENTRY_STC   groupEntry;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    uint32_t                    i, empty_index;
    sai_status_t                status;
            
    parsed_group_identifier.groupType = FPA_GROUP_L2_LAG_E; 
    parsed_group_identifier.index = index;
    fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &groupIdentifier);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to build group identifier index %d \n", index);
        return SAI_STATUS_FAILURE;
    }
    fpa_status = fpaLibGroupTableGetEntry(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, &groupEntry);
    if (fpa_status != FPA_OK){
        MRVL_SAI_LOG_ERR("Failed to find group identifier index %d \n", groupIdentifier);
        return SAI_STATUS_FAILURE;
    }

    *numChanged = 0;
    switch (operation) {    
    case MRVL_SAI_UTL_ADD:
        empty_index = 0;
        i = 0xFFFFFFFF;        
        status = mrvl_sai_utl_l2_lag_group_bucket_get_index(groupIdentifier, port_oid, &bucket_index);
        if (status == SAI_STATUS_SUCCESS){ /* bucket already inside*/           
           return SAI_STATUS_SUCCESS; 
        } else if (status != SAI_STATUS_ITEM_NOT_FOUND) {
            MRVL_SAI_LOG_ERR("Failed - to add bucket to lag group index %d\n", index);
            fpaLibGroupTableEntryDelete(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier);
            return SAI_STATUS_FAILURE;
        } else {
            /* find index to add bucket */
            while (fpa_status == FPA_OK) {
                fpa_status = fpaLibGroupEntryBucketGetNext(SAI_DEFAULT_ETH_SWID_CNS, groupIdentifier, i, &bucket); 
                if ((fpa_status != FPA_OK) && (fpa_status != FPA_NO_MORE)) {
                    MRVL_SAI_LOG_ERR("Failed - to add bucket to lag group index %d\n", index);                       
                    return SAI_STATUS_FAILURE;
                } else if ((fpa_status == FPA_NO_MORE) || (empty_index < bucket.index)) {
                    status = mrvl_sai_utl_l2_lag_group_add_bucket(groupIdentifier, empty_index, port_oid);
                    if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_ALREADY_EXISTS)){
                        MRVL_SAI_LOG_ERR("Failed - to add bucket to lag group index %d\n", index);                            
                        return SAI_STATUS_FAILURE;
                    } else {  
                        *numChanged = 1;                                              
                        break;
                    }
                } else {
                    i = bucket.index; 
                    empty_index++;
                }
            }            
        }        
        break;
    case MRVL_SAI_UTL_DEL:                        
        status = mrvl_sai_utl_l2_lag_group_del_bucket(groupIdentifier, port_oid);
        if ((status != SAI_STATUS_SUCCESS) && (status != SAI_STATUS_ITEM_NOT_FOUND)){
            MRVL_SAI_LOG_ERR("Failed - to del bucket from lag group index %d\n", index);                
            return SAI_STATUS_FAILURE;
        }   
        *numChanged = 1;             
        break;
    default:
        MRVL_SAI_LOG_ERR("Failed operation %d not supported\n", operation);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    return SAI_STATUS_SUCCESS; 
}

/******** Trace *********/

typedef struct MSDK_LOG_TRACE_PARAMS_STCT {
   MRVL_SAI_APP_ID         appId;
   char               appName[MRVL_SAI_TRACE_APP_NAME_LEN];
   bool	              enabled;
   MRVL_SAI_LOG_LEVEL      level;
} MRVL_SAI_TRACE_PARAMS_STC;

static MRVL_SAI_TRACE_PARAMS_STC mrvl_sai_trace_params[] =
{
	{ APPID_GENERAL,   	 	 "GENERAL   ",      true,   SEVERITY_LEVEL_DEFAULT   },
};

/*******************************************************************************
* fpaTraceIsEnable
*
* DESCRIPTION:
*
********************************************************************************/
bool mrvl_sai_trace_is_enable(unsigned int appId, unsigned int level)
{
	 return ((mrvl_sai_trace_params[appId].enabled == true) && (level <= mrvl_sai_trace_params[appId].level)); 
}
/*******************************************************************************
* halTrace
*
* DESCRIPTION:
*
********************************************************************************/


#include <stdio.h>
#include <unistd.h>
#ifdef DEMO
#ifdef LINUX
#include <syslog.h>
#endif
#endif

void mrvl_sai_trace(unsigned int appId, unsigned int level, char *format, ...)
{
	va_list vaList;
    char    tempBuffer[MRVL_SAI_TRACE_MAX_LEN];
	size_t len;
	time_t sec;

	if ((mrvl_sai_trace_params[appId].enabled == false) ||
		(level > mrvl_sai_trace_params[appId].level))
      return;

    /* Store in a string */
    va_start(vaList, format);
    if (vsnprintf(tempBuffer, MRVL_SAI_TRACE_MAX_LEN-2, format, vaList) > MRVL_SAI_TRACE_MAX_LEN)
        return;
    va_end(vaList);
	len = strlen(tempBuffer);
	/*tempBuffer[len] = '\n';*/
	tempBuffer[len ] = '\0';

	sec = time(0);
    if (level <= SEVERITY_LEVEL_ALERT)
        printf("%s", COLOR_RED);
    else
        printf("%s", COLOR_CYN);

	printf("%ld    | %s | %s",
      sec - mrvl_sai_trace_start_time,
      mrvl_sai_trace_params[appId].appName,
      tempBuffer);

    printf("%s", COLOR_NRM);

    if ( level == SEVERITY_LEVEL_ERROR || level == SEVERITY_LEVEL_ALARM )
    {
		fprintf(stderr,"%ld    | %s | %s",
				  sec - mrvl_sai_trace_start_time,
				  mrvl_sai_trace_params[appId].appName,
				  tempBuffer);
		if (level == SEVERITY_LEVEL_ALARM )
		{
			sleep(1);
		}
    }

#ifdef DEMO
#ifdef LINUX
	syslog(LOG_NOTICE, "MRVLSAI: %ld    | %s | %s",
		      sec - mrvl_sai_trace_start_time,
		      mrvl_sai_trace_params[appId].appName,
		      tempBuffer);
#endif
#endif


	return;
}

void mrvl_sai_trace_init(void)
{
   mrvl_sai_trace_start_time = time(0);
#ifdef DEMO
#ifdef LINUX
	openlog("mrvlSai", LOG_PID|LOG_CONS, LOG_USER);
#endif
#endif
/*
#ifndef _WIN32
	openlog("SAI", 0, LOG_USER);
#endif
*/


}

void mrvl_sai_trace_close(void)
{
#ifdef DEMO
#ifdef LINUX
	closelog();
#endif
#endif
}


sai_status_t mrvl_sai_trace_set_module_level (
   MRVL_SAI_APP_ID      moduleId,
   uint32_t             enable,
   MRVL_SAI_LOG_LEVEL   level
)
{
	uint32_t i;
   if (moduleId < APPID_LAST) {
	   mrvl_sai_trace_params[moduleId].enabled    = enable;
	   mrvl_sai_trace_params[moduleId].level    = level;
      return SAI_STATUS_SUCCESS;
   } else if (moduleId == 255) {
		for (i=0; i < APPID_LAST; i++) {
			mrvl_sai_trace_params[i].enabled    = enable;
			mrvl_sai_trace_params[i].level    = level;
		}
		return SAI_STATUS_SUCCESS;
	}

   return SAI_STATUS_FAILURE;
}

sai_status_t mrvl_sai_trace_show_module_id()
{
    int i;
    for (i = 0 ;i < APPID_LAST; i++){
        printf(" ID:%2d  enable:%d  lvl:%d  name:%s \n",i,mrvl_sai_trace_params[i].enabled, mrvl_sai_trace_params[i].level, mrvl_sai_trace_params[i].appName);
    }
    return SAI_STATUS_SUCCESS;
}

void mrvl_sai_trace_set_all (
   uint32_t             enable,
   MRVL_SAI_LOG_LEVEL   level
)
{
    int i;
    for (i = 0 ;i < APPID_LAST; i++){
        mrvl_sai_trace_set_module_level (i,enable,level);
    }
}
sai_status_t  mrvl_sai_return
(
		const char		*func_name,
        int             line,
        sai_status_t	status
)
{
	sai_status_t new_status;
	int ver=5;

	MRVL_SAI_LOG_ERR("%s:%d - Ver[%d] try to return status: 0x%x\n", func_name, line, ver, status);
#ifdef DEMO
	switch (status)
	{
	case SAI_STATUS_NO_MEMORY:
	case SAI_STATUS_INSUFFICIENT_RESOURCES:
	case SAI_STATUS_ITEM_NOT_FOUND:
	case SAI_STATUS_NOT_IMPLEMENTED:
		new_status = status;
	default:
		new_status = SAI_STATUS_SUCCESS;
	}
#else
	new_status = status;
#endif
	MRVL_SAI_LOG_ERR("%s:%d - Ver[%d], Acctually Returned new status: 0x%x\n", func_name, line, ver, new_status);
    return new_status;
}

#ifdef LINUX
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include    <unistd.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>
#include    <fcntl.h>
#include    <errno.h> 
#endif

#ifndef _WIN32
#include <net/if.h>
#endif

int mrvl_sai_netdev_alloc(char *dev) {
#ifdef LINUX
  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";
   int flags = (IFF_TAP | IFF_NO_PI);

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if ( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
#else
  return -1;
#endif

}

#include <net/if_arp.h>

int mrvl_sai_netdev_set_mac(char * name, unsigned char * mac_char)
{
#ifdef LINUX
	struct ifreq ifr;
	int s;
	ifr.ifr_hwaddr.sa_data[0]= mac_char[0];
	ifr.ifr_hwaddr.sa_data[1]= mac_char[1];
	ifr.ifr_hwaddr.sa_data[2]= mac_char[2];
	ifr.ifr_hwaddr.sa_data[3]= mac_char[3];
	ifr.ifr_hwaddr.sa_data[4]= mac_char[4];
	ifr.ifr_hwaddr.sa_data[5]= mac_char[5];

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s == -1)
		return -1;

	strcpy(ifr.ifr_name, name);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	if (ioctl(s, SIOCSIFHWADDR, &ifr) == -1)
		return -1;
#endif
	return 0;
}

int mrvl_sai_netdev_get_mac(char * name, unsigned char * mac_char)
{
#ifdef LINUX
	struct ifreq ifr;
	int s;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s == -1)
		return -1;

	strcpy(ifr.ifr_name, name);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	if (ioctl(s, SIOCGIFHWADDR, &ifr) == -1)
		return -1;
        
    mac_char[0] = ifr.ifr_hwaddr.sa_data[0];
    mac_char[1] = ifr.ifr_hwaddr.sa_data[1];
    mac_char[2] = ifr.ifr_hwaddr.sa_data[2];
    mac_char[3] = ifr.ifr_hwaddr.sa_data[3];
    mac_char[4] = ifr.ifr_hwaddr.sa_data[4];
    mac_char[5] = ifr.ifr_hwaddr.sa_data[5];

	return 0;
#else
	return -1;
#endif
}

void fpautilsPortDump
(
    IN   uint8_t                devId,
    IN   uint32_t               portNum,
	IN	 int					fd
);


/*
 * simple_server.c
 *
 *  Created on: Feb 14, 2017
 *      Author: gain
 */
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void signal_callback_handler(int signum){
	return;
}

void * mrvl_sai_simple_server(void)
{
	 struct sockaddr_un address;
	 int socket_fd, connection_fd;
	 socklen_t address_length;
	 char address_str[] = "/tmp/console_socket";
	 int nbytes;
	 char buffer[256];
	 unsigned int cmd;

	 socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	 if(socket_fd < 0)
	 {
		 return NULL;
	 }
	 unlink(address_str);
	 /* start with a clean address structure */
	 memset(&address, 0, sizeof(struct sockaddr_un));
	 address.sun_family = AF_UNIX;
	 memset(address.sun_path, 0, 108);
	 sprintf(address.sun_path, "%s", address_str);
	 if(bind(socket_fd,
			 (struct sockaddr *) &address,
			 sizeof(struct sockaddr_un)) != 0)
	 {
		 return NULL;
	 }
	 if(listen(socket_fd, 5) != 0)
	 {
		 return NULL;
	 }
	 while((connection_fd = accept(socket_fd,
								   (struct sockaddr *) &address,
								   &address_length)) > -1)
	 {
		 do {
			 nbytes = read(connection_fd, buffer, 256);
			 buffer[nbytes] = 0;
			 if ( strcmp(buffer,"help")==0 || strcmp(buffer,"?")==0) {
				 dprintf(connection_fd, "Commands: help | exit | Ethernet<0..53>\n");
			 } else if ( strcmp(buffer,"exit")==0 || strcmp(buffer,"q")==0) {
				 dprintf(connection_fd, "Closed\n");
				 close(connection_fd);
				 break;
			 } else if ( sscanf(buffer, "Ethernet%d", &cmd)==1 ) {
				 if (cmd < SAI_MAX_NUM_OF_PORTS) {
					 fpautilsPortDump(SAI_DEFAULT_ETH_SWID_CNS, cmd, connection_fd);
				 }
				 else {
					 dprintf(connection_fd, "Unknown Ethernet\n");
				 }
			 } else {
				 dprintf(connection_fd, "Unknown Command\n");
			 }
		 } while ( nbytes > 0);
	 }
	 close(socket_fd);
	 unlink(address_str);
	 return NULL;
}
