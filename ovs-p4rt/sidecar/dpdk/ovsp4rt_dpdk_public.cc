// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// DPDK-specific functions declared in ovs-p4rt.h.
//

#include "absl/flags/flag.h"
#include "common/ovsp4rt_credentials.h"
#include "common/ovsp4rt_private.h"
#include "common/ovsp4rt_session.h"
#include "common/ovsp4rt_utils.h"
#include "openvswitch/ovs-p4rt.h"

//----------------------------------------------------------------------
// Functions with C interfaces
//----------------------------------------------------------------------

void ovsp4rt_config_fdb_entry(struct mac_learning_info learn_info,
                              bool insert_entry, const char* grpc_addr) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = ovs_p4rt::OvsP4rtSession::Create(
      grpc_addr, GenerateClientCredentials(), absl::GetFlag(FLAGS_device_id),
      absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<ovs_p4rt::OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status =
      ovs_p4rt::GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  if (learn_info.is_tunnel) {
    status = ConfigFdbTunnelTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
  } else if (learn_info.is_vlan) {
    status = ConfigFdbTxVlanTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) return;

    status = ConfigFdbRxVlanTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) return;
  }
}

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
