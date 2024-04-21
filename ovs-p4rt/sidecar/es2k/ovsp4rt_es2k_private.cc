// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// ES2K-specific private functions.
//

#include "ovsp4rt_es2k_private.h"

#include <cstdbool>
#include <string>

#include "absl/flags/flag.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovsp4rt_credentials.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt_session.h"
#include "ovsp4rt_utils.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4_name_mapping.h"

namespace ovs_p4rt {

static const std::string tunnel_v6_param_name[] = {
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_1, ACTION_SET_TUNNEL_V6_PARAM_IPV6_2,
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_3, ACTION_SET_TUNNEL_V6_PARAM_IPV6_4};

void PrepareFdbSmacTableEntry(p4::v1::TableEntry* table_entry,
                              const struct mac_learning_info& learn_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
#if defined(LNW_V2)
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_SMAC_TABLE));
  table_entry->set_priority(1);
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_SMAC_TABLE, L2_FWD_SMAC_TABLE_KEY_SA));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_ternary()->set_value(mac_addr);
  match->mutable_ternary()->set_mask(
      EncodeByteValue(6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_SMAC_TABLE_ACTION_SMAC_LEARN));
  }
#elif defined(LNW_V3)
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_SMAC_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_SMAC_TABLE, L2_FWD_SMAC_TABLE_KEY_SA));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, L2_FWD_SMAC_TABLE,
                                       L2_FWD_SMAC_TABLE_KEY_BRIDGE_ID));
  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_SMAC_TABLE_ACTION_NO_ACTION));
  }
#endif  // LNW_V3
}

void PrepareFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_RX_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_RX_TABLE, L2_FWD_RX_TABLE_KEY_DST_MAC));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  // Based on p4 program for ES2K, we need to provide a match key Bridge ID
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_RX_TABLE, L2_FWD_RX_TABLE_KEY_BRIDGE_ID));

  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

#if defined(LNW_V2)
  // Based on p4 program for ES2K, we need to provide a match key Bridge ID
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, L2_FWD_RX_TABLE,
                                       L2_FWD_RX_TABLE_KEY_SMAC_LEARNED));

  match2->mutable_exact()->set_value(EncodeByteValue(1, 1));
#endif

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.rx_src_port;
      param->set_value(EncodeByteValue(1, port_id));
    }
  }

  return;
}

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
  return;
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
  return;
}

absl::Status ConfigFdbSmacTableEntry(ovs_p4rt::OvsP4rtSession* session,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }
  PrepareFdbSmacTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigL2TunnelTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
      learn_info.tnl_info.remote_ip.family == AF_INET6) {
    PrepareL2ToTunnelV6(table_entry, learn_info, p4info, insert_entry);
  } else {
    PrepareL2ToTunnelV4(table_entry, learn_info, p4info, insert_entry);
  }
  return ovs_p4rt::SendWriteRequest(session, write_request);
}

/* GENEVE_ENCAP_MOD_TABLE */
void PrepareGeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct tunnel_info& tunnel_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_ENCAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, GENEVE_ENCAP_MOD_TABLE,
                      GENEVE_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, (((dst_port * 2) >> 8) & 0xff),
                                       ((dst_port * 2) & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP,
                                     ACTION_GENEVE_ENCAP_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

/* VXLAN_ENCAP_V6_MOD_TABLE */
void PrepareV6VxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_V6_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, VXLAN_ENCAP_V6_MOD_TABLE,
                      VXLAN_ENCAP_V6_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP_V6));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, ((dst_port * 2) >> 8) & 0xff,
                                       (dst_port * 2) & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6,
                                     ACTION_VXLAN_ENCAP_V6_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

/* GENEVE_ENCAP_V6_MOD_TABLE */
void PrepareV6GeneveEncapTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct tunnel_info& tunnel_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_ENCAP_V6_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, GENEVE_ENCAP_V6_MOD_TABLE,
                      GENEVE_ENCAP_V6_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP_V6));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, ((dst_port * 2) >> 8) & 0xff,
                                       (dst_port * 2) & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_V6,
                                     ACTION_GENEVE_ENCAP_V6_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

void PrepareV6EncapTableEntry(p4::v1::TableEntry* table_entry,
                              const struct tunnel_info& tunnel_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareV6VxlanEncapTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareV6GeneveEncapTableEntry(table_entry, tunnel_info, p4info,
                                   insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }

  return;
}

/* VXLAN_ENCAP_VLAN_POP_MOD_TABLE */
void PrepareVxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, VXLAN_ENCAP_VLAN_POP_MOD_TABLE,
      VXLAN_ENCAP_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, (((dst_port * 2) >> 8) & 0xff),
                                       ((dst_port * 2) & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_VLAN_POP,
                                     ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

/* GENEVE_ENCAP_VLAN_POP_MOD_TABLE */
void PrepareGeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, GENEVE_ENCAP_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, GENEVE_ENCAP_VLAN_POP_MOD_TABLE,
      GENEVE_ENCAP_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, (((dst_port * 2) >> 8) & 0xff),
                                       ((dst_port * 2) & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_GENEVE_ENCAP_VLAN_POP,
                                     ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

void PrepareEncapAndVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                                      const struct tunnel_info& tunnel_info,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                          insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                           insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }

  return;
}

/* VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE */
void PrepareV6VxlanEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE,
      VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, ((dst_port * 2) >> 8) & 0xff,
                                       (dst_port * 2) & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP_V6_VLAN_POP,
                                     ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

/* GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE */
void PrepareV6GeneveEncapAndVlanPopTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(
      p4info, GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE,
      GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, ((dst_port * 2) >> 8) & 0xff,
                                       (dst_port * 2) & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, (dst_port >> 8) & 0xff, dst_port & 0xff));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_ENCAP_V6_VLAN_POP,
                     ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

void PrepareV6EncapAndVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareV6VxlanEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                            insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareV6GeneveEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                             insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }

  return;
}

void PrepareRxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                               const struct tunnel_info& tunnel_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_KEY_VNI));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT,
          ACTION_SET_SRC_PORT));
      param->set_value(EncodeByteValue(2, ((tunnel_info.src_port >> 8) & 0xff),
                                       (tunnel_info.src_port & 0xff)));
    }
  }

  return;
}

void PrepareV6RxTunnelTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct tunnel_info& tunnel_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_KEY_VNI));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE,
                      RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_KEY_IPV6_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT,
          ACTION_SET_SRC_PORT));
      param->set_value(EncodeByteValue(2, ((tunnel_info.src_port >> 8) & 0xff),
                                       (tunnel_info.src_port & 0xff)));
    }
  }

  return;
}

void PrepareV6TunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, IPV6_TUNNEL_TERM_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                      IPV6_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.bridge_id));

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                       IPV6_TUNNEL_TERM_TABLE_KEY_IPV6_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                       IPV6_TUNNEL_TERM_TABLE_KEY_VNI));
  match2->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeByteValue(1, tunnel_info.vni));
      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeByteValue(1, tunnel_info.vni));
      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    } else {
      if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(p4info, ACTION_SET_VXLAN_DECAP_OUTER_HDR,
                                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeByteValue(1, tunnel_info.vni));
      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR, ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeByteValue(1, tunnel_info.vni));
      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    }
  }
  return;
}

void PrepareVxlanDecapModTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct tunnel_info& tunnel_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_DECAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VXLAN_DECAP_MOD_TABLE,
                                      VXLAN_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    {
      action->set_action_id(GetActionId(p4info, ACTION_VXLAN_DECAP_OUTER_HDR));
    }
  }
  return;
}

void PrepareGeneveDecapModTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct tunnel_info& tunnel_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_DECAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, GENEVE_DECAP_MOD_TABLE,
                                      GENEVE_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    {
      action->set_action_id(GetActionId(p4info, ACTION_GENEVE_DECAP_OUTER_HDR));
    }
  }
  return;
}

void PrepareDecapModTableEntry(p4::v1::TableEntry* table_entry,
                               const struct tunnel_info& tunnel_info,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanDecapModTableEntry(table_entry, tunnel_info, p4info,
                                   insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveDecapModTableEntry(table_entry, tunnel_info, p4info,
                                    insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }

  return;
}

void PrepareVxlanDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE,
                      VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_PCP));
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_DEI));
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID));
      param->set_value(EncodeByteValue(1, tunnel_info.vlan_info.port_vlan));
    }
  }
  return;
}

void PrepareGeneveDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE,
                      GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_PCP));
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_DEI));
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID));
      param->set_value(EncodeByteValue(1, tunnel_info.vlan_info.port_vlan));
    }
  }
  return;
}

void PrepareDecapModAndVlanPushTableEntry(
    p4::v1::TableEntry* table_entry, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                              insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                               insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }

  return;
}

absl::Status ConfigDecapTableEntry(ovs_p4rt::OvsP4rtSession* session,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED) {
    PrepareDecapModTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else {
    PrepareDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                         insert_entry);
  }

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

void PrepareVlanPushTableEntry(p4::v1::TableEntry* table_entry,
                               const uint16_t vlan_id,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VLAN_PUSH_MOD_TABLE,
                                      VLAN_PUSH_MOD_KEY_MOD_BLOB_PTR));

  match->mutable_exact()->set_value(EncodeByteValue(1, vlan_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_PCP));

      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_DEI));

      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_VLAN_ID));

      param->set_value(EncodeByteValue(1, vlan_id));
    }
  }
  return;
}

void PrepareVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                              const uint16_t vlan_id,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VLAN_POP_MOD_TABLE,
                                      VLAN_POP_MOD_KEY_MOD_BLOB_PTR));

  match->mutable_exact()->set_value(EncodeByteValue(1, vlan_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, VLAN_POP_MOD_ACTION_VLAN_POP));
  }
  return;
}

absl::Status ConfigVlanPushTableEntry(ovs_p4rt::OvsP4rtSession* session,
                                      const uint16_t vlan_id,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  PrepareVlanPushTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetVlanPushTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const uint16_t vlan_id,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareVlanPushTableEntry(table_entry, vlan_id, p4info, false);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::Status ConfigVlanPopTableEntry(ovs_p4rt::OvsP4rtSession* session,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  PrepareVlanPopTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

void PrepareSrcPortTableEntry(p4::v1::TableEntry* table_entry,
                              const struct src_port_info& sp,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE));
  auto match = table_entry->add_match();
  table_entry->set_priority(1);
  match->set_field_id(
      GetMatchFieldId(p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE,
                      SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_SRC_PORT));
  match->mutable_ternary()->set_value(
      EncodeByteValue(2, ((sp.src_port >> 8) & 0xff), (sp.src_port & 0xff)));
  match->mutable_ternary()->set_mask(EncodeByteValue(2, 0xff, 0xff));

  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE,
                      SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_VID));
  match1->mutable_ternary()->set_value(
      EncodeByteValue(2, ((sp.vlan_id >> 8) & 0x0f), (sp.vlan_id & 0xff)));
  match1->mutable_ternary()->set_mask(EncodeByteValue(2, 0x0f, 0xff));
  // match1->mutable_ternary()->set_mask(EncodeByteValue(1, 0xff));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_SET_BRIDGE_ID));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_SET_BRIDGE_ID,
          ACTION_SET_BRIDGE_ID_PARAM_BRIDGE_ID));
      param->set_value(EncodeByteValue(1, sp.bridge_id));
    }
  }

  return;
}

void PrepareSrcIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                  struct ip_mac_map_info& ip_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, SRC_IP_MAC_MAP_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, SRC_IP_MAC_MAP_TABLE,
                                      SRC_IP_MAC_MAP_TABLE_KEY_SRC_IP));
  match->mutable_exact()->set_value(
      CanonicalizeIp((ip_info.src_ip_addr.ip.v4addr.s_addr)));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP,
                                     ACTION_SET_SRC_MAC_HIGH));
      std::string mac_high =
          EncodeByteValue(2, (ip_info.src_mac_addr[0] & 0xff),
                          (ip_info.src_mac_addr[1] & 0xff));
      param->set_value(mac_high);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP,
                                     ACTION_SET_SRC_MAC_MID));
      std::string mac_mid = EncodeByteValue(2, (ip_info.src_mac_addr[2] & 0xff),
                                            (ip_info.src_mac_addr[3] & 0xff));
      param->set_value(mac_mid);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     SRC_IP_MAC_MAP_TABLE_ACTION_SMAC_MAP,
                                     ACTION_SET_SRC_MAC_LOW));
      std::string mac_low = EncodeByteValue(2, (ip_info.src_mac_addr[4] & 0xff),
                                            (ip_info.src_mac_addr[5] & 0xff));
      param->set_value(mac_low);
    }
  }

  return;
}

void PrepareDstIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                  struct ip_mac_map_info& ip_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, DST_IP_MAC_MAP_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, DST_IP_MAC_MAP_TABLE,
                                      DST_IP_MAC_MAP_TABLE_KEY_DST_IP));
  match->mutable_exact()->set_value(
      CanonicalizeIp((ip_info.dst_ip_addr.ip.v4addr.s_addr)));
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP,
                                     ACTION_SET_DST_MAC_HIGH));
      std::string mac_high =
          EncodeByteValue(2, (ip_info.dst_mac_addr[0] & 0xff),
                          (ip_info.dst_mac_addr[1] & 0xff));
      param->set_value(mac_high);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP,
                                     ACTION_SET_DST_MAC_MID));
      std::string mac_mid = EncodeByteValue(2, (ip_info.dst_mac_addr[2] & 0xff),
                                            (ip_info.dst_mac_addr[3] & 0xff));
      param->set_value(mac_mid);
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info,
                                     DST_IP_MAC_MAP_TABLE_ACTION_DMAC_MAP,
                                     ACTION_SET_DST_MAC_LOW));
      std::string mac_low = EncodeByteValue(2, (ip_info.dst_mac_addr[4] & 0xff),
                                            (ip_info.dst_mac_addr[5] & 0xff));
      param->set_value(mac_low);
    }
  }

  return;
}

void PrepareTxAccVsiTableEntry(p4::v1::TableEntry* table_entry, uint32_t sp,
                               const ::p4::config::v1::P4Info& p4info) {
  table_entry->set_table_id(GetTableId(p4info, TX_ACC_VSI_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, TX_ACC_VSI_TABLE, TX_ACC_VSI_TABLE_KEY_VSI));

  match->mutable_exact()->set_value(
      EncodeByteValue(1, (sp - ES2K_VPORT_ID_OFFSET)));
#if 0
  /* unused match key of 0, code is added for reference */
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, TX_ACC_VSI_TABLE, TX_ACC_VSI_TABLE_KEY_ZERO_PADDING));

  match->mutable_exact()->set_value(EncodeByteValue(1, 0));
#endif
  return;
}

absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV4TableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareL2ToTunnelV4(table_entry, learn_info, p4info, false);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV6TableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareL2ToTunnelV6(table_entry, learn_info, p4info, false);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetFdbTunnelTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                         false);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                          false);
  } else {
    return absl::UnknownError("Unsupported tunnel type");
  }

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetFdbVlanTableEntry(
    ovs_p4rt::OvsP4rtSession* session,
    const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, false);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetVmSrcTableEntry(
    ovs_p4rt::OvsP4rtSession* session, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareSrcIpMacMapTableEntry(table_entry, ip_info, p4info, false);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetVmDstTableEntry(
    ovs_p4rt::OvsP4rtSession* session, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareDstIpMacMapTableEntry(table_entry, ip_info, p4info, false);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetTxAccVsiTableEntry(
    ovs_p4rt::OvsP4rtSession* session, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = ovs_p4rt::SetupTableEntryToRead(session, &read_request);

  PrepareTxAccVsiTableEntry(table_entry, sp, p4info);

  return ovs_p4rt::SendReadRequest(session, read_request);
}

absl::Status ConfigureVsiSrcPortTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct src_port_info& sp,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  PrepareSrcPortTableEntry(table_entry, sp, p4info, insert_entry);

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigRxTunnelSrcPortTableEntry(
    ovs_p4rt::OvsP4rtSession* session, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    PrepareRxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6RxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  }

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigDstIpMacMapTableEntry(ovs_p4rt::OvsP4rtSession* session,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  PrepareDstIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry);

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigSrcIpMacMapTableEntry(ovs_p4rt::OvsP4rtSession* session,
                                         struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
    table_entry = ovs_p4rt::SetupTableEntryToInsert(session, &write_request);
  } else {
    table_entry = ovs_p4rt::SetupTableEntryToDelete(session, &write_request);
  }

  PrepareSrcIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry);

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

}  // namespace ovs_p4rt
