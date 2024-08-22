// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_PRIVATE_H_
#define OVSP4RT_PRIVATE_H_

#include <stdarg.h>
#include <stdbool.h>

#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

//----------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------

extern std::string EncodeByteValue(int arg_count...);

//----------------------------------------------------------------------
// Common functions
//----------------------------------------------------------------------

extern void PrepareFdbRxVlanTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void PrepareFdbTableEntryforV4GeneveTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void PrepareFdbTableEntryforV4VxlanTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void PrepareFdbTxVlanTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail);

extern void PrepareVxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

extern void PrepareTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

//----------------------------------------------------------------------
// ES2K-specific functions
//----------------------------------------------------------------------

#if defined(ES2K_TARGET)

extern void PrepareDstIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry, DiagDetail& detail);

extern void PrepareFdbSmacTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry, DiagDetail& detail);

extern void PrepareSrcIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry, DiagDetail& detail);

extern void PrepareL2ToTunnelV4(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry, DiagDetail& detail);

extern void PrepareL2ToTunnelV6(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry, DiagDetail& detail);

extern void PrepareGeneveDecapModTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareGeneveDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareGeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                         const struct tunnel_info& tunnel_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry);

extern void PrepareGeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6GeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6GeneveEncapTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareRxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry);

extern void PrepareV6RxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry);

extern void PrepareSrcPortTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct src_port_info& sp,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareTxAccVsiTableEntry(p4::v1::TableEntry* table_entry,
                                      uint32_t sp,
                                      const ::p4::config::v1::P4Info& p4info);

extern void PrepareV6TunnelTermTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry);

extern void PrepareVlanPushTableEntry(p4::v1::TableEntry* table_entry,
                                      const uint16_t vlan_id,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry);

extern void PrepareVxlanDecapModTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVxlanDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareVxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6VxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

extern void PrepareV6VxlanEncapTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

#endif  // ES2K_TARGET

}  // namespace ovsp4rt

#endif  // OVSP4RT_PRIVATE_H_
