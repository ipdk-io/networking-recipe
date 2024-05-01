// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Prototypes for private common functions.
//

#include <cstdbool>
#include <cstdint>
#include <string>

#include "absl/flags/declare.h"
#include "absl/status/status.h"
#include "lib/ovsp4rt_session.h"
#include "openvswitch/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

ABSL_DECLARE_FLAG(uint64_t, device_id);
ABSL_DECLARE_FLAG(std::string, role_name);

namespace ovs_p4rt {

//----------------------------------------------------------------------
// 'Config' functions
//----------------------------------------------------------------------

extern absl::Status ConfigEncapTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbRxVlanTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbTxVlanTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbTunnelTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigTunnelTermTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

//----------------------------------------------------------------------
// 'Prepare' functions
//----------------------------------------------------------------------

extern void PrepareFdbTableEntryforV4GeneveTunnel(
    ::p4::v1::TableEntry* table_entry,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareFdbTableEntryforV4VxlanTunnel(
    ::p4::v1::TableEntry* table_entry,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareFdbTxVlanTableEntry(
    ::p4::v1::TableEntry* table_entry,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVxlanEncapTableEntry(::p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

extern void PrepareEncapTableEntry(::p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry);

extern void PrepareTunnelTermTableEntry(::p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

}  // namespace ovs_p4rt
