// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Prototypes for ES2K-specific private functions.
//

#include <cstdbool>
#include <cstdint>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "lib/ovsp4rt_session.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

//----------------------------------------------------------------------
// 'Config' functions
//----------------------------------------------------------------------

extern absl::Status ConfigDecapTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigDstIpMacMapTableEntry(
    ovs_p4rt::OvsP4rtSession* session, struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigFdbSmacTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigL2TunnelTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigRxTunnelSrcPortTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigSrcIpMacMapTableEntry(
    ovs_p4rt::OvsP4rtSession* session, struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigVlanPopTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigVlanPushTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern absl::Status ConfigureVsiSrcPortTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct src_port_info& sp,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

//----------------------------------------------------------------------
// 'Get' functions
//----------------------------------------------------------------------

extern absl::StatusOr<::p4::v1::ReadResponse> GetFdbTunnelTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetFdbVlanTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV4TableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV6TableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetTxAccVsiTableEntry(
    ovs_p4rt::OvsP4rtSession* session, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetVmDstTableEntry(
    ovs_p4rt::OvsP4rtSession* session, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetVmSrcTableEntry(
    ovs_p4rt::OvsP4rtSession* session, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info);

extern absl::StatusOr<::p4::v1::ReadResponse> GetVlanPushTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info);

//----------------------------------------------------------------------
// 'Prepare' functions
//----------------------------------------------------------------------

extern void PrepareDecapModTableEntry(::p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry);

extern void PrepareDecapModAndVlanPushTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareDstIpMacMapTableEntry(::p4::v1::TableEntry* table_entry,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern void PrepareEncapAndVlanPopTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareFdbRxVlanTableEntry(
    ::p4::v1::TableEntry* table_entry,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareFdbSmacTableEntry(::p4::v1::TableEntry* table_entry,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareGeneveDecapModTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareGeneveDecapModAndVlanPushTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareGeneveEncapTableEntry(::p4::v1::TableEntry* table_entry,
                                         const struct tunnel_info& tunnel_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern void PrepareGeneveEncapAndVlanPopTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareL2ToTunnelV4(::p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry);

extern void PrepareL2ToTunnelV6(::p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry);

extern void PrepareRxTunnelTableEntry(::p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry);

extern void PrepareSrcIpMacMapTableEntry(::p4::v1::TableEntry* table_entry,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern void PrepareSrcPortTableEntry(::p4::v1::TableEntry* table_entry,
                                     const struct src_port_info& sp,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareTxAccVsiTableEntry(::p4::v1::TableEntry* table_entry,
                                      uint32_t sp,
                                      const ::p4::config::v1::P4Info& p4info);

extern void PrepareVxlanEncapAndVlanPopTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6EncapTableEntry(::p4::v1::TableEntry* table_entry,
                                     const struct tunnel_info& tunnel_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareV6EncapAndVlanPopTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6GeneveEncapTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6GeneveEncapAndVlanPopTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6RxTunnelTableEntry(::p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

extern void PrepareV6TunnelTermTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6VxlanEncapTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6VxlanEncapAndVlanPopTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVxlanDecapModTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVxlanDecapModAndVlanPushTableEntry(
    ::p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVlanPopTableEntry(::p4::v1::TableEntry* table_entry,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareVlanPushTableEntry(::p4::v1::TableEntry* table_entry,
                                      const uint16_t vlan_id,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry);

}  // namespace ovs_p4rt
