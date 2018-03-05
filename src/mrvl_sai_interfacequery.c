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

service_method_table_t g_services;
bool                   g_initialized = false;

char sai_ver[] = "v1.0";

/*
 * Routine Description:
 *     Adapter module initialization call. This is NOT for SDK initialization.
 *
 * Arguments:
 *     [in] flags - reserved for future use, must be zero
 *     [in] services - methods table with services provided by adapter host
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_api_initialize(_In_ uint64_t flags, _In_ const service_method_table_t* services)
{
    mrvl_sai_trace_init();
	MRVL_SAI_LOG_ENTER();

    MRVL_SAI_LOG_INF("\n my Marvell SAI version is %s\n", sai_ver);

	if ((NULL == services) || (NULL == services->profile_get_next_value) || (NULL == services->profile_get_value)) {
    	MRVL_SAI_LOG_ERR("Invalid services handle passed to SAI API initialize\n");
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    memcpy(&g_services, services, sizeof(g_services));

    if (0 != flags) {
    	MRVL_SAI_LOG_ERR("Invalid flags passed to SAI API initialize\n");
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    g_initialized = true;
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *     Retrieve a pointer to the C-style method table for desired SAI
 *     functionality as specified by the given sai_api_id.
 *
 * Arguments:
 *     [in] sai_api_id - SAI api ID
 *     [out] api_method_table - Caller allocated method table
 *           The table must remain valid until the sai_api_uninitialize() is called
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_api_query(_In_ sai_api_t sai_api_id, _Out_ void** api_method_table)
{
	MRVL_SAI_LOG_ENTER();
	if (NULL == api_method_table) {
    	MRVL_SAI_LOG_ERR("NULL method table passed to SAI API initialize\n");
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (!g_initialized) {
    	MRVL_SAI_LOG_ERR("SAI API not initialized before calling API query\n");
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_UNINITIALIZED);
    }

    switch (sai_api_id) {
    case SAI_API_SWITCH:
        *(const sai_switch_api_t**)api_method_table = &switch_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_PORT:
        *(const sai_port_api_t**)api_method_table = &port_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_FDB:
        *(const sai_fdb_api_t**)api_method_table = &fdb_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_VLAN:
        *(const sai_vlan_api_t**)api_method_table = &vlan_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_HOSTIF:
        *(const sai_hostif_api_t**)api_method_table = &host_interface_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_ROUTER_INTERFACE:
        *(const sai_router_interface_api_t**)api_method_table = &router_interface_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_ACL:
        *(const sai_acl_api_t**)api_method_table = &acl_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_NEIGHBOR:
        *(const sai_neighbor_api_t**)api_method_table = &neighbor_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_NEXT_HOP:
        *(const sai_next_hop_api_t**)api_method_table = &nexthop_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_NEXT_HOP_GROUP:
        *(const sai_next_hop_group_api_t**)api_method_table = &next_hop_group_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_ROUTE:
        *(const sai_route_api_t**)api_method_table = &route_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_VIRTUAL_ROUTER:
        *(const sai_virtual_router_api_t**)api_method_table = &virtual_router_api;
        MRVL_SAI_LOG_EXIT();
        MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_LAG:
    	*(const sai_lag_api_t **)api_method_table= &lag_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
    /*!*********************************!*/
    /*!              STUBS              !*/
    /*!*********************************!*/
    case SAI_API_SCHEDULER_GROUP:
    	*(const sai_scheduler_group_api_t **)api_method_table= &scheduler_group_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_HASH:
    	*(const sai_hash_api_t **)api_method_table= &hash_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_QUEUE:
    	*(const sai_queue_api_t **)api_method_table= &queue_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_STP:
    	*(const sai_stp_api_t **)api_method_table= &stp_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_L2MC:
        *(const sai_l2mc_api_t **)api_method_table= &l2mc_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_L2MC_GROUP:
        *(const sai_l2mc_group_api_t **)api_method_table= &l2mc_group_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_BRIDGE:
        *(const sai_bridge_api_t **)api_method_table= &bridge_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    case SAI_API_UDF:
    	*(const sai_udf_api_t **)api_method_table= &udf_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);

    /*!*********************************!*/
    /*!        NOT IMPLEMENTED          !*/
    /*!*********************************!*/
    case SAI_API_BUFFER:
    	*(const sai_buffer_api_t **)api_method_table= &buffer_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_MIRROR:
    	*(const sai_mirror_api_t **)api_method_table= &mirror_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_POLICER:
    	*(const sai_policer_api_t **)api_method_table= &policer_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_QOS_MAP:
    	*(const sai_qos_map_api_t **)api_method_table= &qos_map_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);
    
    case SAI_API_SAMPLEPACKET:
    	*(const sai_samplepacket_api_t **)api_method_table= &samplepacket_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_SCHEDULER:
    	*(const sai_scheduler_api_t **)api_method_table= &scheduler_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_TUNNEL:
    	*(const sai_tunnel_api_t **)api_method_table= &tunnel_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_WRED:
    	*(const sai_wred_api_t **)api_method_table= &wred_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_IPMC:
        *(const sai_ipmc_api_t **)api_method_table= &ipmc_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_RPF_GROUP:
        *(const sai_rpf_group_api_t **)api_method_table= &rpf_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_IPMC_GROUP:
        *(const sai_ipmc_group_api_t **)api_method_table= &ipmc_group_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    case SAI_API_MCAST_FDB:
        *(const sai_mcast_fdb_api_t **)api_method_table= &mcast_fdb_api;
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_IMPLEMENTED);

    default:
    	*api_method_table = NULL;
    	MRVL_SAI_LOG_ERR("Invalid API type %d\n", sai_api_id);
    	MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_NOT_SUPPORTED);
    }
}

/*
 * Routine Description:
 *   Uninitialization of the adapter module. SAI functionalities, retrieved via
 *   sai_api_query() cannot be used after this call.
 *
 * Arguments:
 *   None
 *
 * Return Values:
 *   SAI_STATUS_SUCCESS on success
 *   Failure status code on error
 */
sai_status_t sai_api_uninitialize(void)
{
	MRVL_SAI_LOG_ENTER();
	memset(&g_services, 0, sizeof(g_services));
    g_initialized = false;
    MRVL_SAI_LOG_EXIT();
    mrvl_sai_trace_close();
    return (SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *     Set log level for sai api module. The default log level is SAI_LOG_WARN.
 *
 * Arguments:
 *     [in] sai_api_id - SAI api ID
 *     [in] log_level - log level
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_log_set(_In_ sai_api_t sai_api_id, _In_ sai_log_level_t log_level)
{
	MRVL_SAI_LOG_ENTER();
    switch (log_level) {
    case SAI_LOG_LEVEL_DEBUG:
        break;

    case SAI_LOG_LEVEL_INFO:
        break;

    case SAI_LOG_LEVEL_NOTICE:
        break;

    case SAI_LOG_LEVEL_WARN:
        break;

    case SAI_LOG_LEVEL_ERROR:
        break;

    case SAI_LOG_LEVEL_CRITICAL:
        break;
    
    default:
    	MRVL_SAI_LOG_ERR("Invalid log level %d\n", log_level);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }

    switch (sai_api_id) {
    case SAI_API_SWITCH:
        break;

    case SAI_API_PORT:
        break;

    case SAI_API_FDB:
        break;

    case SAI_API_VLAN:
        break;

    case SAI_API_VIRTUAL_ROUTER:
        break;

    case SAI_API_ROUTE:
        break;

    case SAI_API_NEXT_HOP:
        break;

    case SAI_API_NEXT_HOP_GROUP:
        break;

    case SAI_API_ROUTER_INTERFACE:
        break;

    case SAI_API_NEIGHBOR:
        break;

    case SAI_API_ACL:
        break;

    case SAI_API_HOSTIF:
        break;

    case SAI_API_MIRROR:
        break;

    case SAI_API_SAMPLEPACKET:
        break;

    case SAI_API_STP:
        break;

    case SAI_API_LAG:
        break;

    case SAI_API_POLICER:
    	break;

    case SAI_API_WRED:
    	break;

    case SAI_API_QOS_MAP:
    	break;

    case SAI_API_QUEUE:
    	break;

    case SAI_API_SCHEDULER:
    	break;

    case SAI_API_SCHEDULER_GROUP:
    	break;

    case SAI_API_BUFFER:
    	break;

    case SAI_API_HASH:
        break;

    case SAI_API_UDF:
    	break;

    case SAI_API_TUNNEL:
    	break;
    case SAI_API_L2MC:
        break;
    case SAI_API_IPMC:
        break;
    case SAI_API_RPF_GROUP:
        break;
    case SAI_API_L2MC_GROUP:
        break;
    case SAI_API_IPMC_GROUP:
        break;
    case SAI_API_MCAST_FDB:
        break;
    case SAI_API_BRIDGE:
    	break;

    default:
    	MRVL_SAI_LOG_ERR("Invalid API type %d\n", sai_api_id);
        MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}

/*
 * Routine Description:
 *     Query sai object type.
 *
 * Arguments:
 *     [in] sai_object_id_t
 *
 * Return Values:
 *    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
 *    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
 */
sai_object_type_t sai_object_type_query(_In_ sai_object_id_t sai_object_id)
{
    sai_object_type_t type;

    MRVL_SAI_LOG_ENTER();
    if (SAI_NULL_OBJECT_ID == sai_object_id)
    {
        MRVL_SAI_LOG_ERR("NULL object id\n");
    	MRVL_SAI_LOG_EXIT();
        return SAI_OBJECT_TYPE_NULL;
    }
    type = ((mrvl_object_id_t *)&sai_object_id)->object_type; 
    if SAI_TYPE_CHECK_RANGE(type) {
    	MRVL_SAI_LOG_EXIT();
        return type;
    } else {
    	MRVL_SAI_LOG_ERR("Unknown type %d\n", type);
    	MRVL_SAI_LOG_EXIT();
        return SAI_OBJECT_TYPE_NULL;
    }
}
/*
 * Routine Description:
 *     Query sai switch id.
 *
 * Arguments:
 *     [in] sai_object_id_t
 *
 * Return Values:
 *    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
 *    Otherwise, return a valid SAI_OBJECT_TYPE_SWITCH object on which
 *    provided object id belongs. If valid switch id object is provided
 *    as input parameter it should return itself.
 */
sai_object_id_t sai_switch_id_query(
        _In_ sai_object_id_t sai_object_id)
{
    if (SAI_NULL_OBJECT_ID == sai_object_id)
    {
        MRVL_SAI_LOG_ERR("NULL object id\n");
    	MRVL_SAI_LOG_EXIT();
        return SAI_OBJECT_TYPE_NULL;
    }
    sai_object_type_t type = ((mrvl_object_id_t*)&sai_object_id)->object_type;
    MRVL_SAI_LOG_ENTER();
    if (SAI_TYPE_CHECK_RANGE(type)) 
    {
        if (SAI_STATUS_SUCCESS != mrvl_sai_utl_is_object_type(sai_object_id, SAI_OBJECT_TYPE_SWITCH)) {
            MRVL_SAI_LOG_ERR("Object is not of type switch\n");
            MRVL_SAI_LOG_EXIT();
            return SAI_NULL_OBJECT_ID;
        }
        MRVL_SAI_LOG_EXIT()
        return sai_object_id;
    } else {
    	MRVL_SAI_LOG_ERR("Invalid object type %d", type);
    	MRVL_SAI_LOG_EXIT();
        return SAI_NULL_OBJECT_ID;
    }
}
/**
 * @brief Generate dump file. The dump file may include SAI state information and vendor SDK information.
 *
 * @param[in] dump_file_name Full path for dump file
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_dbg_generate_dump(
        _In_ const char *dump_file_name)
{
    MRVL_SAI_LOG_ENTER();
    if (NULL == dump_file_name)
    {
        MRVL_SAI_LOG_ERR("NULL dump file\n");
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_INVALID_PARAMETER);
    }
    if (!g_initialized)
    {
    	MRVL_SAI_LOG_ERR("Can't generate debug dump before creating switch\n");
        MRVL_SAI_LOG_EXIT();
    	MRVL_SAI_API_RETURN(SAI_STATUS_UNINITIALIZED);
    }
    MRVL_SAI_LOG_EXIT();
    MRVL_SAI_API_RETURN(SAI_STATUS_SUCCESS);
}
