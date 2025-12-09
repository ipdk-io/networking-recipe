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

#ifndef __SWITCH_DEVICE_H__
#define __SWITCH_DEVICE_H__

#include "switch_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct switch_api_device_info_s {
  /** default vrf id */
  //  switch_vrf_t default_vrf;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** default vlan id */
  //  switch_vlan_t default_vlan;

  /** vlan handle */
  switch_handle_t vlan_handle;

  /** group handle */
  switch_handle_t group_handle;

  /** switch mac address */
  switch_mac_addr_t mac;

  /** router mac handle */
  switch_handle_t rmac_handle;

  /** maximum lag groups supported */
  switch_uint16_t max_lag_groups;

  /** maximum lag members supported */
  switch_uint16_t max_lag_members;

  /** lag hashing flags */
  switch_uint32_t lag_hash_flags;

  /** install learnt dmacs */
  bool install_dmac;

  /** maximum vrf supported */
  switch_uint16_t max_vrf;

  /** maximum ports */
  switch_uint32_t max_ports;

  /** list of front port handles */
  switch_handle_list_t port_list;

  /** ethernet cpu port */
  //  switch_port_t eth_cpu_port;

  /** pcie cpu port */
  //  switch_port_t pcie_cpu_port;

  /** counter refresh interval */
  switch_uint32_t refresh_interval;

  /** mac aging interval */
  switch_int32_t aging_interval;

  /** tunnel destination mac address */
  switch_mac_addr_t tunnel_dmac;

  /** number of active ports */
  switch_uint16_t num_active_ports;

  /** maximum port mtu */
  switch_uint32_t max_port_mtu;

  /** mac learning flag */
  bool mac_learning;

  /** vxlan default port */
  switch_uint16_t vxlan_default_port;

} switch_api_device_info_t;

switch_status_t switch_api_init(switch_device_t device);

switch_status_t switch_api_device_add(switch_device_t device);

switch_status_t switch_api_device_remove(switch_device_t device);

switch_status_t switch_api_device_default_rmac_handle_get(
    switch_device_t device, switch_handle_t* rmac_handle);

switch_status_t switch_api_get_default_nhop_group(
    switch_device_t device, switch_handle_t* nhop_group_handle);

switch_status_t switch_api_device_tunnel_dmac_get(switch_device_t device,
                                                  switch_mac_addr_t* mac_addr);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __SWITCH_DEVICE_H__ */
