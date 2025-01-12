// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Defines the interface to the C++ functions that implement
// the core logic of the ovsp4rt C API.

#ifndef OVSP4RT_INTERNAL_API_H_
#define OVSP4RT_INTERNAL_API_H_

#include "absl/status/status.h"
#include "client/ovsp4rt_client_interface.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

extern absl::Status DoConfigFdbEntry(ClientInterface& client,
                                     struct mac_learning_info learn_info,
                                     bool insert_entry, const char* grpc_addr);

extern absl::Status DoConfigTunnelEntry(ClientInterface& client,
                                        const struct tunnel_info& tunnel_info,
                                        bool insert_entry,
                                        const char* grpc_addr);

#if defined(ES2K_TARGET)

extern absl::Status DoConfigIpMacMapEntry(ClientInterface& client,
                                          const struct ip_mac_map_info& ip_info,
                                          bool insert_entry,
                                          const char* grpc_addr);

extern absl::Status DoConfigRxTunnelSrcEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    bool insert_entry, const char* grpc_addr);

extern absl::Status DoConfigSrcPortEntry(ClientInterface& client,
                                         struct src_port_info vsi_sp,
                                         bool insert_entry,
                                         const char* grpc_addr);

extern absl::Status DoConfigTunnelSrcPortEntry(
    ClientInterface& client, const struct src_port_info& tnl_sp,
    bool insert_entry, const char* grpc_addr);

extern absl::Status DoConfigVlanEntry(ClientInterface& client, uint16_t vlan_id,
                                      bool insert_entry, const char* grpc_addr);

#endif  // ES2K_TARGET

}  // namespace ovsp4rt

#endif  // OVSP4RT_INTERNAL_API_H_
