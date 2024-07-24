/*
 * Copyright (c) 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdint.h>

#include "ovsp4rt/ovs-p4rt.h"

#ifdef __cplusplus
extern "C" {
#endif

void ovsp4rt_config_fdb_entry(struct mac_learning_info learn_info,
                              bool insert_entry, const char* grpc_addr) {
  return;
}

void ovsp4rt_config_ip_mac_map_entry(struct ip_mac_map_info learn_info,
                                     bool insert_entry, const char* grpc_addr) {
}

void ovsp4rt_config_rx_tunnel_src_entry(struct tunnel_info tunnel_info,
                                        bool insert_entry,
                                        const char* grpc_addr) {
  return;
}

void ovsp4rt_config_src_port_entry(struct src_port_info vsi_sp,
                                   bool insert_entry, const char* grpc_addr) {
  return;
}

void ovsp4rt_config_tunnel_src_port_entry(struct src_port_info tnl_sp,
                                          bool insert_entry,
                                          const char* grpc_addr) {
  return;
}

void ovsp4rt_config_tunnel_entry(struct tunnel_info tunnel_info,
                                 bool insert_entry, const char* grpc_addr) {
  return;
}

void ovsp4rt_config_vlan_entry(uint16_t vlan_id, bool insert_entry,
                               const char* grpc_addr) {
  return;
}

enum ovs_tunnel_type ovsp4rt_str_to_tunnel_type(const char* tnl_type) {
  return OVS_TUNNEL_UNKNOWN;
}

#ifdef __cplusplus
}  // "C"
#endif
