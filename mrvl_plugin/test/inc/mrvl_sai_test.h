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

#if !defined (__MRVL_SAI_TEST_H_)
#define __MRVL_SAI_TEST_H_

#include    "mrvl_sai.h"

int mrvl_sai_vlan_create_test(IN sai_vlan_id_t vlan_id);
int mrvl_sai_vlan_add_ports_test(IN sai_vlan_id_t vlan_id,
                                 IN uint32_t port_count,
                                 IN sai_vlan_port_t *port_list);

int mrvl_sai_fdb_add_test(IN sai_mac_t mac_addr,
                          IN sai_vlan_id_t vlan_id,
                          IN sai_uint32_t type,
                          IN sai_object_id_t port_id,
                          IN sai_uint32_t pkt_action);

int mrvl_sai_rif_add_test(IN sai_router_interface_type_t interface_type,
                          IN sai_uint32_t port_vlan,
                          IN bool         use_default_mac,
                          IN sai_mac_t    mac_addr,
                          IN sai_packet_action_t  nbr_miss_act,
                          IN sai_object_id_t  vr_id,
                          OUT sai_object_id_t *rif_id);

int mrvl_sai_nbr_add_test(IN sai_neighbor_entry_t *neighbor_entry,
                          IN sai_mac_t            mac_addr); 
int mrvl_sai_nh_add_test(IN  uint32_t ipv4,
                         IN  sai_object_id_t *rif_id,
                         OUT sai_object_id_t* next_hop_id);

int mrvl_sai_virtual_router_add_test(_Out_ sai_object_id_t* vr_id);

#endif // __MRVL_SAI_TEST_H_
