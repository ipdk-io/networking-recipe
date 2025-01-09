// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Standard implementation of the ovsp4rt C API.

#include "client/ovsp4rt_client.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_internal_api.h"

//----------------------------------------------------------------------
// ovsp4rt_config_fdb_entry (DPDK, ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_fdb_entry(struct mac_learning_info learn_info,
                              bool insert_entry, const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigFdbEntry(client, learn_info, insert_entry, grpc_addr);
}

//----------------------------------------------------------------------
// ovsp4rt_config_tunnel_entry (DPDK, ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_tunnel_entry(struct tunnel_info tunnel_info,
                                 bool insert_entry, const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigTunnelEntry(client, tunnel_info, insert_entry, grpc_addr);
}

#if defined(DPDK_TARGET)

//----------------------------------------------------------------------
// Unimplemented functions (DPDK)
//----------------------------------------------------------------------
void ovsp4rt_config_rx_tunnel_src_entry(struct tunnel_info tunnel_info,
                                        bool insert_entry,
                                        const char* grpc_addr) {}

void ovsp4rt_config_vlan_entry(uint16_t vlan_id, bool insert_entry,
                               const char* grpc_addr) {}

void ovsp4rt_config_tunnel_src_port_entry(struct src_port_info tnl_sp,
                                          bool insert_entry,
                                          const char* grpc_addr) {}

void ovsp4rt_config_src_port_entry(struct src_port_info vsi_sp,
                                   bool insert_entry, const char* grpc_addr) {}

void ovsp4rt_config_ip_mac_map_entry(struct ip_mac_map_info ip_info,
                                     bool insert_entry, const char* grpc_addr) {
}

#elif defined(ES2K_TARGET)

//----------------------------------------------------------------------
// ovsp4rt_config_ip_mac_map_entry (ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_ip_mac_map_entry(struct ip_mac_map_info ip_info,
                                     bool insert_entry, const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigIpMacMapEntry(client, ip_info, insert_entry, grpc_addr);
}

//----------------------------------------------------------------------
// ovsp4rt_config_rx_tunnel_src_entry (ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_rx_tunnel_src_entry(struct tunnel_info tunnel_info,
                                        bool insert_entry,
                                        const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigRxTunnelSrcEntry(client, tunnel_info, insert_entry, grpc_addr);
}

//----------------------------------------------------------------------
// ovsp4rt_config_src_port_entry (ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_src_port_entry(struct src_port_info vsi_sp,
                                   bool insert_entry, const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigSrcPortEntry(client, vsi_sp, insert_entry, grpc_addr);
}

//----------------------------------------------------------------------
// ovsp4rt_config_tunnel_src_port_entry (ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_tunnel_src_port_entry(struct src_port_info tnl_sp,
                                          bool insert_entry,
                                          const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigTunnelSrcPortEntry(client, tnl_sp, insert_entry, grpc_addr);
}

//----------------------------------------------------------------------
// ovsp4rt_config_vlan_entry (ES2K)
//----------------------------------------------------------------------
void ovsp4rt_config_vlan_entry(uint16_t vlan_id, bool insert_entry,
                               const char* grpc_addr) {
  using namespace ovsp4rt;

  Client client;

  ConfigVlanEntry(client, vlan_id, insert_entry, grpc_addr);
}

#endif  // ES2K_TARGET
