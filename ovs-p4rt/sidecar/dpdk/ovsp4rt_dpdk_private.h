// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Prototypes for DPDK-specific private functions.
//

#include <cstdbool>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

extern void PrepareFdbRxVlanTableEntry(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry);

}  // namespace ovs_p4rt
