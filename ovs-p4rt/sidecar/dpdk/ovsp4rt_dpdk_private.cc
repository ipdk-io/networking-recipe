// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// DPDK-specific private functions.
//

#include "ovsp4rt_dpdk_private.h"

#include <cstdbool>
#include <string>

#include "common/ovsp4rt_utils.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovsp4rt_dpdk_defs.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

void PrepareFdbRxVlanTableEntry(::p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_RX_WITH_TUNNEL_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, L2_FWD_RX_WITH_TUNNEL_TABLE,
                                      L2_FWD_TX_TABLE_KEY_DST_MAC));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.vln_info.vlan_id - 1;
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
}

}  // namespace ovs_p4rt
