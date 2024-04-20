// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cstdbool>
#include <string>

#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovsp4rt_credentials.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt_session.h"

//----------------------------------------------------------------------
// Functions with C interfaces
//----------------------------------------------------------------------

void ConfigFdbTableEntry(struct mac_learning_info learn_info, bool insert_entry,
                         const char* p4rt_addr) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = OvsP4rtSession::Create(
      p4rt_addr, GenerateClientCredentials(), ::absl::GetFlag(FLAGS_device_id),
      ::absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
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

void ConfigIpMacMapTableEntry(struct ip_mac_map_info ip_info, bool insert_entry,
                              const char* p4rt_addr) {
  /* Unimplemented for DPDK target */
}

void ConfigRxTunnelSrcTableEntry(struct tunnel_info tunnel_info,
                                 bool insert_entry, const char* p4rt_addr) {
  /* Unimplemented for DPDK target */
}

void ConfigSrcPortTableEntry(struct src_port_info vsi_sp, bool insert_entry,
                             const char* p4rt_addr) {
  /* Unimplemented for DPDK target */
}

void ConfigTunnelSrcPortTableEntry(struct src_port_info tnl_sp,
                                   bool insert_entry, const char* p4rt_addr) {
  /* Unimplemented for DPDK target */
}

void ConfigVlanTableEntry(uint16_t vlan_id, bool insert_entry,
                          const char* p4rt_addr) {
  /* Unimplemented for DPDK target */
}