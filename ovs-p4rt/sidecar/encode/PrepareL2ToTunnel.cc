// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "PrepareL2ToTunnel.h"

#include "es2k/p4_name_mapping.h"
#include "ovsp4rt_utils.h"

namespace ovs_p4rt {

static const std::string tunnel_v6_param_name[] = {
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_1, ACTION_SET_TUNNEL_V6_PARAM_IPV6_2,
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_3, ACTION_SET_TUNNEL_V6_PARAM_IPV6_4
};

void PrepareL2ToTunnelV4(p4::v1::TableEntry* table_entry,
                         const struct mac_learning_info& learn_info,
                         const ::p4::config::v1::P4Info& p4info,
                         bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_TO_TUNNEL_V4_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_TO_TUNNEL_V4_TABLE, L2_TO_TUNNEL_V4_KEY_DA));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();

    action->set_action_id(
        GetActionId(p4info, L2_TO_TUNNEL_V4_ACTION_SET_TUNNEL_V4));

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     L2_TO_TUNNEL_V4_ACTION_SET_TUNNEL_V4,
                                     ACTION_SET_TUNNEL_V4_PARAM_DST_ADDR));
      std::string ip_address =
          CanonicalizeIp(learn_info.tnl_info.remote_ip.ip.v4addr.s_addr);
      param->set_value(ip_address);
    }
  }
}

void PrepareL2ToTunnelV6(p4::v1::TableEntry* table_entry,
                         const struct mac_learning_info& learn_info,
                         const ::p4::config::v1::P4Info& p4info,
                         bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_TO_TUNNEL_V6_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_TO_TUNNEL_V6_TABLE, L2_TO_TUNNEL_V6_KEY_DA));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();

    action->set_action_id(
        GetActionId(p4info, L2_TO_TUNNEL_V6_ACTION_SET_TUNNEL_V6));

    for (unsigned int i = 0; i < 4; i++) {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     L2_TO_TUNNEL_V6_ACTION_SET_TUNNEL_V6,
                                     tunnel_v6_param_name[i]));
      std::string ip_address = CanonicalizeIp(
          learn_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[i]);
      param->set_value(ip_address);
    }
  }
}

}  // namespace ovs_p4rt
