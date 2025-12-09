/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SWITCHLINK_HANDLERS_H__
#define __SWITCHLINK_HANDLERS_H__

#include <stdbool.h>
#include <stdint.h>

#include "krnlmon_options.h"
#include "switchlink/switchlink.h"
#include "switchlink/switchlink_db.h"

extern void switchlink_init_api(void);

// SWITCHLINK_LINK_TYPE_VXLAN handlers
extern void switchlink_create_tunnel_interface(
    switchlink_db_tunnel_interface_info_t* tnl_intf);
extern void switchlink_delete_tunnel_interface(uint32_t ifindex);

#ifdef LAG_OPTION
// SWITCHLINK_LINK_TYPE_BOND handlers
extern void switchlink_create_lag(switchlink_db_interface_info_t* lag_info);
extern void switchlink_delete_lag(uint32_t ifindex);
extern void switchlink_create_lag_member(
    switchlink_db_lag_member_info_t* lag_member_info);
extern void switchlink_delete_lag_member(uint32_t ifindex);
#endif

// SWITCHLINK_LINK_TYPE_TUN handlers
extern void switchlink_create_interface(switchlink_db_interface_info_t* intf);
extern void switchlink_delete_interface(uint32_t ifindex);

// RTM_NEWNEIGH/ RTM_DELNEIGH handlers
extern void switchlink_create_neigh(switchlink_handle_t vrf_h,
                                    const switchlink_ip_addr_t* ipaddr,
                                    switchlink_mac_addr_t mac_addr,
                                    switchlink_handle_t intf_h);
extern void switchlink_delete_neigh(switchlink_handle_t vrf_h,
                                    const switchlink_ip_addr_t* ipaddr,
                                    switchlink_handle_t intf_h);
extern void switchlink_create_mac(switchlink_mac_addr_t mac_addr,
                                  switchlink_handle_t bridge_h,
                                  switchlink_handle_t intf_h);
extern void switchlink_delete_mac(switchlink_mac_addr_t mac_addr,
                                  switchlink_handle_t bridge_h);

// Nexthop handlers
extern int switchlink_create_nexthop(
    switchlink_db_nexthop_info_t* nexthop_info);
extern int switchlink_delete_nexthop(switchlink_handle_t nhop_h);

// RTM_NEWROUTE/ RTM_DELROUTE handlers
extern void switchlink_create_route(switchlink_handle_t vrf_h,
                                    const switchlink_ip_addr_t* dst,
                                    const switchlink_ip_addr_t* gateway,
                                    switchlink_handle_t ecmp_h,
                                    switchlink_handle_t intf_h);
extern void switchlink_delete_route(switchlink_handle_t vrf_h,
                                    const switchlink_ip_addr_t* dst);

// ECMP handlers
extern int switchlink_create_ecmp(switchlink_db_ecmp_info_t* ecmp_info);
extern void switchlink_delete_ecmp(switchlink_handle_t ecmp_h);

// VRF handler
extern int switchlink_create_vrf(switchlink_handle_t* vrf_h);

// General utility function
extern bool validate_delete_nexthop(uint32_t using_by,
                                    enum switchlink_nhop_using_by type);

#endif /* __SWITCHLINK_HANDLERS_H__ */
