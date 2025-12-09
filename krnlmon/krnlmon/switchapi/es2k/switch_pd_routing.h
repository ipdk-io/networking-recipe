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

#ifndef __SWITCH_PD_ROUTING_H__
#define __SWITCH_PD_ROUTING_H__

#include "switch_pd_p4_name_routing.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_handle.h"
#include "switchapi/switch_l3.h"
#include "switchapi/switch_nhop.h"
#include "switchapi/switch_rmac_int.h"

#ifdef __cplusplus
extern "C" {
#endif

/** enum to decide the proper key and action based on ecmp or nhop
 * */
typedef enum switch_ip_table_action_s {
  SWITCH_ACTION_NHOP = 0,
  SWITCH_ACTION_NHOP_GROUP = 1,
  SWITCH_ACTION_NONE = 2
} switch_ip_table_action_t;

/**
 * create pd_routing structure to hold
 * the data to be sent to
 * the backend to update the table */
typedef struct switch_pd_routing_info_s {
  switch_handle_t neighbor_handle;

  switch_handle_t nexthop_handle;

  switch_handle_t rif_handle;

  switch_mac_addr_t dst_mac_addr;

  uint8_t port_id;

} switch_pd_routing_info_t;

switch_status_t switch_pd_nexthop_table_entry(
    switch_device_t device, const switch_pd_routing_info_t* api_nexthop_pd_info,
    bool entry_add);

switch_status_t switch_pd_ecmp_nexthop_table_entry(
    switch_device_t device, const switch_pd_routing_info_t* api_nexthop_pd_info,
    bool entry_add);

switch_status_t switch_pd_rmac_table_entry(switch_device_t device,
                                           switch_rmac_entry_t* rmac_entry,
                                           switch_handle_t rif_handle,
                                           bool entry_type);

switch_status_t switch_pd_rif_mod_start_entry(switch_device_t device,
                                              switch_rmac_entry_t* rmac_entry,
                                              switch_handle_t rif_handle,
                                              bool entry_add);

switch_status_t switch_pd_rif_mod_mid_entry(switch_device_t device,
                                            switch_rmac_entry_t* rmac_entry,
                                            switch_handle_t rif_handle,
                                            bool entry_add);

switch_status_t switch_pd_rif_mod_last_entry(switch_device_t device,
                                             switch_rmac_entry_t* rmac_entry,
                                             switch_handle_t rif_handle,
                                             bool entry_add);

switch_status_t switch_pd_ipv4_table_entry(
    switch_device_t device, const switch_api_route_entry_t* api_route_entry,
    bool entry_add, switch_ip_table_action_t action);

switch_status_t switch_pd_ipv6_table_entry(
    switch_device_t device, const switch_api_route_entry_t* api_route_entry,
    bool entry_add, switch_ip_table_action_t action);

switch_status_t switch_pd_ecmp_hash_table_entry(
    switch_device_t device, const switch_nhop_group_info_t* ecmp_info,
    bool entry_add);

switch_status_t switch_routing_table_entry(
    switch_device_t device, const switch_pd_routing_info_t* api_routing_info,
    bool entry_type);

#ifdef __cplusplus
}
#endif

#endif
