// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
// TODO: ovs-p4rt logging

#include <arpa/inet.h>

#include <string>

#include "absl/flags/flag.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovsp4rt_credentials.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt_session.h"
#include "ovsp4rt_utils.h"

#if defined(DPDK_TARGET)
#include "dpdk/ovsp4rt_dpdk_private.h"
#include "dpdk/p4_name_mapping.h"
#elif defined(ES2K_TARGET)
#include "es2k/ovsp4rt_es2k_private.h"
#include "es2k/p4_name_mapping.h"
#else
#error "ASSERT: Unknown TARGET type!"
#endif

#define DEFAULT_OVS_P4RT_ROLE_NAME "ovs-p4rt"

ABSL_FLAG(uint64_t, device_id, 1, "P4Runtime device ID.");
ABSL_FLAG(std::string, role_name, DEFAULT_OVS_P4RT_ROLE_NAME,
          "P4 config role name.");

namespace ovs_p4rt {

void PrepareFdbTxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

#if defined(ES2K_TARGET)
  // Based on p4 program for ES2K, we need to provide a match key Bridge ID
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_BRIDGE_ID));

  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

#if defined(LNW_V2)
  // Based on p4 program for ES2K, we need to provide a match key SMAC flag
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE,
                                       L2_FWD_TX_TABLE_KEY_SMAC_LEARNED));

  match2->mutable_exact()->set_value(EncodeByteValue(1, 1));
#endif

  if (insert_entry) {
    /* Action param configured by user in TX_ACC_VSI_TABLE is used as port_id
     * We call GET api to fetch this value and pass it to FDB programming.
     */
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      action->set_action_id(
          GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD));
      {
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD,
                       ACTION_REMOVE_VLAN_AND_FWD_PARAM_PORT_ID));
        auto port_id = learn_info.src_port;
        param->set_value(EncodeByteValue(1, port_id));
      }
      {
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD,
                       ACTION_REMOVE_VLAN_AND_FWD_PARAM_VLAN_PTR));
        param->set_value(EncodeByteValue(1, learn_info.vlan_info.port_vlan));
      }
    } else {
      action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
      {
        auto param = action->add_params();
        param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD,
                                       ACTION_L2_FWD_PARAM_PORT));
        auto port_id = learn_info.src_port;
        param->set_value(EncodeByteValue(1, port_id));
      }
    }
  }
#elif defined(DPDK_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.vln_info.vlan_id - 1;
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return;
}

void PrepareFdbTableEntryforV4VxlanTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);
#if defined(ES2K_TARGET)
  // Based on p4 program for ES2K, we need to provide a match key Bridge ID
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_BRIDGE_ID));

  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

#if defined(LNW_V2)
  // Based on p4 program for ES2K, we need to provide a match key SMAC flag
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE,
                                       L2_FWD_TX_TABLE_KEY_SMAC_LEARNED));

  match2->mutable_exact()->set_value(EncodeByteValue(1, 1));
#endif  // LNW_V2
#endif  // ES2K_TARGET

#if defined(DPDK_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_DST_ADDR));
      std::string ip_address =
          CanonicalizeIp(learn_info.tnl_info.remote_ip.ip.v4addr.s_addr);
      param->set_value(ip_address);
    }
  }
#elif defined(ES2K_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();

    if (learn_info.tnl_info.local_ip.family == AF_INET &&
        learn_info.tnl_info.remote_ip.family == AF_INET) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V4,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      }
    } else if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
               learn_info.tnl_info.remote_ip.family == AF_INET6) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V6,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

void PrepareFdbTableEntryforV4GeneveTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);
#if defined(ES2K_TARGET)
  // Based on p4 program for ES2K, we need to provide a match key Bridge ID
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_BRIDGE_ID));

  match1->mutable_exact()->set_value(EncodeByteValue(1, learn_info.bridge_id));

#if defined(LNW_V2)
  // Based on p4 program for ES2K, we need to provide a match key SMAC flag
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE,
                                       L2_FWD_TX_TABLE_KEY_SMAC_LEARNED));

  match2->mutable_exact()->set_value(EncodeByteValue(1, 1));
#endif  // LNW_V2
#endif  // ES2K_TARGET

#if defined(DPDK_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL,
                                     ACTION_SET_TUNNEL_PARAM_DST_ADDR));
      std::string ip_address =
          CanonicalizeIp(learn_info.tnl_info.remote_ip.ip.v4addr.s_addr);
      param->set_value(ip_address);
    }
  }
#elif defined(ES2K_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();

    if (learn_info.tnl_info.local_ip.family == AF_INET &&
        learn_info.tnl_info.remote_ip.family == AF_INET) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V4,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      }
    } else if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
               learn_info.tnl_info.remote_ip.family == AF_INET6) {
      if (learn_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
        action->set_action_id(GetActionId(
            p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(GetParamId(
              p4info, L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V6,
              ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
        }
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

absl::Status ConfigFdbTxVlanTableEntry(
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
  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigFdbRxVlanTableEntry(
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
  PrepareFdbRxVlanTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigFdbTunnelTableEntry(
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

#if defined(DPDK_TARGET)
  PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                       insert_entry);
#elif defined(ES2K_TARGET)
  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                         insert_entry);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                          insert_entry);
  } else {
    if (!insert_entry) {
      // Tunnel type doesn't matter for delete. So calling one of the functions
      // to prepare the entry
      PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                           insert_entry);
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
  return ovs_p4rt::SendWriteRequest(session, write_request);
}

/* VXLAN_ENCAP_MOD_TABLE */
void PrepareVxlanEncapTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct tunnel_info& tunnel_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, VXLAN_ENCAP_MOD_TABLE,
                      VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_VXLAN_ENCAP));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_DST_ADDR));
      param->set_value(CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));
    }
#if defined(ES2K_TARGET)
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_SRC_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(EncodeByteValue(2, (((dst_port * 2) >> 8) & 0xff),
                                       ((dst_port * 2) & 0xff)));
    }
#endif
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                     ACTION_VXLAN_ENCAP_PARAM_DST_PORT));
      uint16_t dst_port = htons(tunnel_info.dst_port);

      param->set_value(
          EncodeByteValue(2, ((dst_port >> 8) & 0xff), (dst_port & 0xff)));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_ENCAP, ACTION_VXLAN_ENCAP_PARAM_VNI));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }

  return;
}

void PrepareEncapTableEntry(p4::v1::TableEntry* table_entry,
                            const struct tunnel_info& tunnel_info,
                            const ::p4::config::v1::P4Info& p4info,
                            bool insert_entry) {
#if defined(DPDK_TARGET)
  PrepareVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  if (tunnel_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareVxlanEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareGeneveEncapTableEntry(table_entry, tunnel_info, p4info,
                                 insert_entry);
  } else {
    std::cout << "ERROR: Unsupported tunnel type" << std::endl;
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return;
}

void PrepareTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct tunnel_info& tunnel_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));

#if defined(ES2K_TARGET)
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.bridge_id));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_VNI));
  match2->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.vni));
#elif defined(DPDK_TARGET)
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE));
  match->mutable_exact()->set_value(EncodeByteValue(1, TUNNEL_TYPE_VXLAN));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST));
  match2->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
#endif  // DPDK_TARGET

#if defined(DPDK_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_DECAP_OUTER_IPV4));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_DECAP_OUTER_IPV4,
                                     ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }
#elif defined(ES2K_TARGET)
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
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return;
}

absl::Status ConfigEncapTableEntry(ovs_p4rt::OvsP4rtSession* session,
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

#if defined(DPDK_TARGET)
  PrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      PrepareEncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                       insert_entry);
    } else {
      PrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
    }
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_UNTAGGED) {
      PrepareV6EncapAndVlanPopTableEntry(table_entry, tunnel_info, p4info,
                                         insert_entry);
    } else {
      PrepareV6EncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

absl::Status ConfigTunnelTermTableEntry(ovs_p4rt::OvsP4rtSession* session,
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
#if defined(DPDK_TARGET)
  PrepareTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);
#elif defined(ES2K_TARGET)
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    PrepareTunnelTermTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6TunnelTermTableEntry(table_entry, tunnel_info, p4info,
                                  insert_entry);
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

}  // namespace ovs_p4rt
