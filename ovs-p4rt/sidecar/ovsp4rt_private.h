// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_PRIVATE_H_
#define OVSP4RT_PRIVATE_H_

#include <cstdbool>

#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

void PrepareVxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct tunnel_info& tunnel_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry);

//----------------------------------------------------------------------

#if defined(ES2K_TARGET)

void PrepareL2ToTunnelV4(p4::v1::TableEntry* table_entry,
                         const struct mac_learning_info& learn_info,
                         const ::p4::config::v1::P4Info& p4info,
                         bool insert_entry, DiagDetail& detail);

void PrepareL2ToTunnelV6(p4::v1::TableEntry* table_entry,
                         const struct mac_learning_info& learn_info,
                         const ::p4::config::v1::P4Info& p4info,
                         bool insert_entry, DiagDetail& detail);

void PrepareGeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

void PrepareGeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry);

void PrepareV6GeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct tunnel_info& tunnel_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry);

void PrepareVxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

void PrepareV6VxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

void PrepareV6VxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry);

#endif  // ES2K_TARGET

}  // namespace ovs_p4rt

#endif  // OVSP4RT_PRIVATE_H_
