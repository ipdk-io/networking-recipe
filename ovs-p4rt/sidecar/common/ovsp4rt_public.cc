// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Target-independent functions declared in ovs-p4rt.h
//

#include <cstdbool>

#include "absl/flags/flag.h"
#include "common/ovsp4rt_private.h"
#include "lib/ovsp4rt_credentials.h"
#include "lib/ovsp4rt_session.h"
#include "openvswitch/ovs-p4rt.h"

#if defined(ES2K_TARGET)
#include "es2k/ovsp4rt_es2k_private.h"
#endif

//----------------------------------------------------------------------
// Functions with C interfaces
//----------------------------------------------------------------------

enum ovs_tunnel_type TunnelTypeStrtoEnum(const char* tnl_type) {
  if (tnl_type) {
    if (strcmp(tnl_type, "vxlan") == 0) {
      return OVS_TUNNEL_VXLAN;
    } else if (strcmp(tnl_type, "geneve") == 0) {
      return OVS_TUNNEL_GENEVE;
    }
  }
  return OVS_TUNNEL_UNKNOWN;
}

void ConfigTunnelTableEntry(struct tunnel_info tunnel_info, bool insert_entry,
                            const char* grpc_addr) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = OvsP4rtSession::Create(
      grpc_addr, GenerateClientCredentials(), absl::GetFlag(FLAGS_device_id),
      absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;
  status =
      ConfigEncapTableEntry(session.get(), tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;

#if defined(ES2K_TARGET)
  status =
      ConfigDecapTableEntry(session.get(), tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;
#endif

  status = ConfigTunnelTermTableEntry(session.get(), tunnel_info, p4info,
                                      insert_entry);
  if (!status.ok()) return;
}
