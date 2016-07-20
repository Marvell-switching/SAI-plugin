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
    inet_ntop(AF_INET6, &value, value_str, max_length);

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
             (NULL == attr_list[ii].value.vlanlist.list)) ||
            ((SAI_ATTR_VAL_TYPE_VLANPORTLIST == functionality_attr[index].type) &&
             (NULL == attr_list[ii].value.vlanportlist.list))) {
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
    MRVL_SAI_LOG_ERR("item not found %d\n",attrib_id);
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
    case SAI_ATTR_VAL_TYPE_VLANPORTLIST:
    case SAI_ATTR_VAL_TYPE_PORTBREAKOUT:
        if (SAI_ATTR_VAL_TYPE_PORTBREAKOUT == type) {
            pos += snprintf(value_str, max_length, "breakout mode %d.", value.portbreakout.breakout_mode);
        }
        if (pos > max_length) {
            return SAI_STATUS_SUCCESS;
        }

        count = (SAI_ATTR_VAL_TYPE_OBJLIST == type) ? value.objlist.count :
                (SAI_ATTR_VAL_TYPE_U32LIST == type) ? value.u32list.count :
                (SAI_ATTR_VAL_TYPE_S32LIST == type) ? value.s32list.count :
                (SAI_ATTR_VAL_TYPE_VLANLIST == type) ? value.vlanlist.count :
                (SAI_ATTR_VAL_TYPE_VLANPORTLIST == type) ? value.vlanportlist.count :
                value.portbreakout.port_list.count;
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
            } else if (SAI_ATTR_VAL_TYPE_VLANPORTLIST == type) {
                pos += snprintf(value_str + pos, max_length - pos, " %" PRIx64, value.vlanportlist.list[ii].port_id);
            } else {
                pos += snprintf(value_str + pos, max_length - pos, " %" PRIx64, value.portbreakout.port_list.list[ii]);
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
                        "    #%u %s val:%s \n",
                        ii,
                        functionality_attr[index].attrib_name,
                        value_str);
        if (pos > max_length) {
            break;
        }
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
#if 0
/*#ifndef _WIN32*/
void sai_log_cb(sai_log_level_t severity, const char *module_name, char *msg)
{
    int   level;
    char *level_str;

    /* translate SDK log level to syslog level */
    switch (severity) {
    case SAI_LOG_NOTICE:
        level     = LOG_NOTICE;
        level_str = "NOTICE";
        break;

    case SAI_LOG_INFO:
        level     = LOG_INFO;
        level_str = "INFO";
        break;

    case SAI_LOG_ERROR:
        level     = LOG_ERR;
        level_str = "ERR";
        break;

    case SAI_LOG_WARN:
        level     = LOG_WARNING;
        level_str = "WARNING";
        break;

    case SAI_LOG_DEBUG:
        level     = LOG_DEBUG;
        level_str = "DEBUG";
        break;

    default:
        level     = LOG_DEBUG;
        level_str = "DEBUG";
        break;
    }

    syslog(level, "[%s.%s] %s", module_name, level_str, msg);
}
/*#else*/
void sai_log_cb(sai_log_level_t severity, const char *module_name, char *msg)
{
    UNREFERENCED_PARAMETER(severity);
    UNREFERENCED_PARAMETER(module_name);
    UNREFERENCED_PARAMETER(msg);
}
/*#endif*/

void utils_log_vprint(const sai_log_level_t severity, const char *module_name, const char *p_str, va_list args)
{
    char buffer[LOG_ENTRY_SIZE_MAX];

    vsnprintf(buffer, LOG_ENTRY_SIZE_MAX, p_str, args);

    sai_log_cb(severity, module_name, buffer);
}

void utils_log(const sai_log_level_t severity, const char *module_name, const char *p_str, ...)
{
    va_list args;

    if (severity < SAI_LOG_INFO) {
        return;
    }

    va_start(args, p_str);
    utils_log_vprint(severity, module_name, p_str, args);
    va_end(args);
}
#endif


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
    group_entry.groupTypeSemantics = OFPGT_INDIRECT;
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

sai_status_t mrvl_sai_utl_create_l3_unicast_group(_In_ uint32_t     index, 
                                                  _In_ sai_mac_t    dst_mac, 
                                                  _In_ uint32_t     rif_idx, 
                                                  _Inout_ uint32_t  *group)
{
    FPA_GROUP_TABLE_ENTRY_STC   group_entry;
    FPA_GROUP_BUCKET_ENTRY_STC  bucket;
    FPA_STATUS                  fpa_status;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    sai_status_t            status;
    mrvl_sai_rif_table_t    rif_entry;
    uint32_t                ref_group;
    
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
    group_entry.groupIdentifier = *group;
    group_entry.groupTypeSemantics = OFPGT_INDIRECT;
    fpa_status = fpaLibGroupTableEntryAdd(SAI_DEFAULT_ETH_SWID_CNS, &group_entry);
    if (fpa_status == FPA_OK) {
        /* new group crated - create with default values */
        bucket.groupIdentifier = *group;
        bucket.index = 0;
        bucket.type = FPA_GROUP_BUCKET_L3_UNICAST_E;
        memcpy(&bucket.data.l3Unicast.dstMac, dst_mac, 6);;
        status = mrvl_sai_rif_get_entry(rif_idx, &rif_entry);
        if (status != SAI_STATUS_SUCCESS) {
            MRVL_SAI_LOG_ERR("Can't get rif %d\n", rif_idx);
            return status;
        }
        mrvl_sai_rif_get_mtu_size(rif_idx, &bucket.data.l3Unicast.mtu);
        memset(&parsed_group_identifier, 0, sizeof(parsed_group_identifier));
        parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
        if (rif_entry.intf_type == SAI_ROUTER_INTERFACE_TYPE_PORT) {
            parsed_group_identifier.portNum = rif_entry.port_vlan_id; 
        } else {
            parsed_group_identifier.vlanId = rif_entry.port_vlan_id; 
            bucket.data.l3Unicast.vlanId = rif_entry.port_vlan_id;
        }
        fpa_status = fpaLibGroupIdentifierBuild(&parsed_group_identifier, &ref_group);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed to create group identifier vlan %d port %d \n", 
                             parsed_group_identifier.vlanId, 
                             parsed_group_identifier.portNum);
            return SAI_STATUS_FAILURE;
        }
        memset(&group_entry, 0, sizeof(group_entry));
        fpa_status = fpaLibGroupTableGetEntry(SAI_DEFAULT_ETH_SWID_CNS, ref_group, &group_entry);
        if (fpa_status != FPA_OK){
            MRVL_SAI_LOG_ERR("Failed - referance group 0x%x does not exist\n", ref_group);
            return SAI_STATUS_FAILURE;
        }        
        bucket.data.l3Unicast.refGroupId = ref_group;        
        memcpy(&bucket.data.l3Unicast.srcMac, rif_entry.src_mac, 6);
        fpa_status = fpaLibGroupEntryBucketAdd(SAI_DEFAULT_ETH_SWID_CNS, &bucket);
    }
    if (fpa_status != FPA_OK){
        if (fpa_status != FPA_ALREADY_EXIST) {
            MRVL_SAI_LOG_ERR("Failed to add group %d entry status = %d\n", *group, fpa_status); 
            return SAI_STATUS_FAILURE;
        } else {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    } else {
        return SAI_STATUS_SUCCESS; 
    }
}
sai_status_t mrvl_sai_utl_delete_l3_unicast_group(_In_ uint32_t index)
{
    uint32_t                    groupIdentifier;
    FPA_GROUP_ENTRY_IDENTIFIER_STC parsed_group_identifier;
    FPA_STATUS                  fpa_status;
    
    parsed_group_identifier.groupType = FPA_GROUP_L2_INTERFACE_E;
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

/******** Trace *********/

typedef struct MSDK_LOG_TRACE_PARAMS_STCT {
   MRVL_SAI_APP_ID         appId;
   char               appName[MRVL_SAI_TRACE_APP_NAME_LEN];
   bool	              enabled;
   MRVL_SAI_LOG_LEVEL      level;
} MRVL_SAI_TRACE_PARAMS_STC;

static MRVL_SAI_TRACE_PARAMS_STC mrvl_sai_trace_params[] =
{
	{ APPID_GENERAL,   	 	 "GENERAL   ",      true,   SEVERITY_LEVEL_INFO   },
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

	printf("%ld | %s | %s",
      sec - mrvl_sai_trace_start_time,
      mrvl_sai_trace_params[appId].appName,
      tempBuffer);

    printf("%s", COLOR_NRM);

	return;
}

void mrvl_sai_trace_init(void)
{
   mrvl_sai_trace_start_time = time(0);
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

