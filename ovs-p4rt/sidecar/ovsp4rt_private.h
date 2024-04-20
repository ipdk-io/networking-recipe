// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_PRIVATE_H_
#define OVSP4RT_PRIVATE_H_

#include <cstdbool>

#include "absl/flags/declare.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovsp4rt_session.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

ABSL_DECLARE_FLAG(uint64_t, device_id);
ABSL_DECLARE_FLAG(std::string, role_name);

namespace ovs_p4rt {

extern ::absl::Status ConfigDstIpMacMapTableEntry(
    OvsP4rtSession* session, struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbRxVlanTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigFdbSmacTableEntry(
    OvsP4rtSession* session, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigFdbTunnelTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigFdbTxVlanTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigL2TunnelTableEntry(
    OvsP4rtSession* session, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigRxTunnelSrcPortTableEntry(
    OvsP4rtSession* session, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigSrcIpMacMapTableEntry(OvsP4rtSession* session,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern ::absl::Status ConfigVlanPopTableEntry(
    OvsP4rtSession* session, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigVlanPushTableEntry(
    OvsP4rtSession* session, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::Status ConfigureVsiSrcPortTableEntry(
    OvsP4rtSession* session, const struct src_port_info& sp,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetFdbTunnelTableEntry(
    OvsP4rtSession* session, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetFdbVlanTableEntry(
    OvsP4rtSession* session, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV4TableEntry(
    OvsP4rtSession* session, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV6TableEntry(
    OvsP4rtSession* session, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetTxAccVsiTableEntry(
    OvsP4rtSession* session, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetVmDstTableEntry(
    OvsP4rtSession* session, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info);

extern ::absl::StatusOr<::p4::v1::ReadResponse> GetVmSrcTableEntry(
    OvsP4rtSession* session, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info);

extern void PrepareEncapTableEntry(::p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry);

extern void PrepareFdbRxVlanTableEntry(
    ::p4::v1::TableEntry* table_entry,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareSrcPortTableEntry(::p4::v1::TableEntry* table_entry,
                                     const struct src_port_info& sp,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareVxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

}  // namespace ovs_p4rt

#endif  // OVSP4RT_PRIVATE_H_
