// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>

#include <string>

#include "absl/flags/flag.h"
#include "client/ovsp4rt_client.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logging.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_internal_api.h"
#include "ovsp4rt_private.h"

#if defined(DPDK_TARGET)
#include "dpdk/p4_name_mapping.h"
#elif defined(ES2K_TARGET)
#include "es2k/p4_name_mapping.h"
#endif

namespace ovsp4rt {

#if defined(ES2K_TARGET)
static const std::string tunnel_v6_param_name[] = {
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_1, ACTION_SET_TUNNEL_V6_PARAM_IPV6_2,
    ACTION_SET_TUNNEL_V6_PARAM_IPV6_3, ACTION_SET_TUNNEL_V6_PARAM_IPV6_4};
#endif

std::string EncodeByteValue(int arg_count...) {
  std::string byte_value;
  va_list args;
  va_start(args, arg_count);

  for (int arg = 0; arg < arg_count; ++arg) {
    uint8_t byte = va_arg(args, int);
    byte_value.push_back(byte);
  }

  va_end(args);
  return byte_value;
}

// Encodes tunnel_info.vni as a "tunnel_id" action parameter,
// which is bit<20> in all cases except set_ipsec_tunnel.
static inline std::string EncodeTunnelId(uint32_t vni) {
  return EncodeByteValue(3, (vni >> 16) & 0x0F, (vni >> 8) & 0xFF, vni & 0xFF);
}

// Encodes tunnel_info.vni as a "vni" or "mod_blob_ptr" match
// field or action parameter, which are bit<24> in all cases.
static inline std::string EncodeVniValue(uint32_t vni) {
  return EncodeByteValue(3, (vni >> 16) & 0xFF, (vni >> 8) & 0xFF, vni & 0xFF);
}

std::string CanonicalizeIp(const uint32_t ipv4addr) {
  // note: low-to-high byte order
  return EncodeByteValue(4, (ipv4addr & 0xff), ((ipv4addr >> 8) & 0xff),
                         ((ipv4addr >> 16) & 0xff), ((ipv4addr >> 24) & 0xff));
}

std::string CanonicalizeIpv6(const struct in6_addr ipv6addr) {
  return EncodeByteValue(
      16, ipv6addr.__in6_u.__u6_addr8[0], ipv6addr.__in6_u.__u6_addr8[1],
      ipv6addr.__in6_u.__u6_addr8[2], ipv6addr.__in6_u.__u6_addr8[3],
      ipv6addr.__in6_u.__u6_addr8[4], ipv6addr.__in6_u.__u6_addr8[5],
      ipv6addr.__in6_u.__u6_addr8[6], ipv6addr.__in6_u.__u6_addr8[7],
      ipv6addr.__in6_u.__u6_addr8[8], ipv6addr.__in6_u.__u6_addr8[9],
      ipv6addr.__in6_u.__u6_addr8[10], ipv6addr.__in6_u.__u6_addr8[11],
      ipv6addr.__in6_u.__u6_addr8[12], ipv6addr.__in6_u.__u6_addr8[13],
      ipv6addr.__in6_u.__u6_addr8[14], ipv6addr.__in6_u.__u6_addr8[15]);
}

std::string CanonicalizeMac(const uint8_t mac[6]) {
  return EncodeByteValue(6, (mac[0] & 0xff), (mac[1] & 0xff), (mac[2] & 0xff),
                         (mac[3] & 0xff), (mac[4] & 0xff), (mac[5] & 0xff));
}

int GetTableId(const ::p4::config::v1::P4Info& p4info,
               const std::string& t_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() == t_name) return pre.id();
  }
  return -1;
}

int GetActionId(const ::p4::config::v1::P4Info& p4info,
                const std::string& a_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() == a_name) return pre.id();
  }
  return -1;
}

int GetParamId(const ::p4::config::v1::P4Info& p4info,
               const std::string& a_name, const std::string& param_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() != a_name) continue;
    for (const auto& param : action.params())
      if (param.name() == param_name) return param.id();
  }
  return -1;
}

int GetMatchFieldId(const ::p4::config::v1::P4Info& p4info,
                    const std::string& t_name, const std::string& mf_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() != t_name) continue;
    for (const auto& mf : table.match_fields())
      if (mf.name() == mf_name) return mf.id();
  }
  return -1;
}

static inline int32_t ValidIpAddr(uint32_t nw_addr) {
  return (nw_addr && nw_addr != INADDR_ANY && nw_addr != INADDR_LOOPBACK &&
          nw_addr != 0xffffffff);
}

#if defined(ES2K_TARGET)
void PrepareFdbSmacTableEntry(p4::v1::TableEntry* table_entry,
                              const struct mac_learning_info& learn_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_SMAC_TABLE;
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
}
#endif  // ES2K_TARGET

void PrepareFdbTxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;
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
        // TODO(derek): port_id truncated to 8 bits. [es2k]
        // See https://github.com/ipdk-io/networking-recipe/issues/619
        param->set_value(EncodeByteValue(1, port_id));
      }
      {
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD,
                       ACTION_REMOVE_VLAN_AND_FWD_PARAM_VLAN_PTR));
        // TODO(derek): port_vlan truncated to 8 bits. [es2k]
        // See https://github.com/ipdk-io/networking-recipe/issues/620
        param->set_value(EncodeByteValue(1, learn_info.vlan_info.port_vlan));
      }
    } else {
      action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
      {
        auto param = action->add_params();
        param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD,
                                       ACTION_L2_FWD_PARAM_PORT));
        auto port_id = learn_info.src_port;
        // TODO(derek): port_id truncated to 8 bits. [es2k]
        // See https://github.com/ipdk-io/networking-recipe/issues/619
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
      // TODO(derek) Questionable value semantics. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/689
      auto port_id = learn_info.vln_info.vlan_id - 1;
      // TODO(derek): vlan_id truncated to 8 bits. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/689
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

#if defined(ES2K_TARGET)

void PrepareFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_RX_TABLE;
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

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_RX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.rx_src_port;
      // TODO(derek): port_id truncated to 8 bits. [es2k]
      // See https://github.com/ipdk-io/networking-recipe/issues/682
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
}

#elif defined(DPDK_TARGET)

void PrepareFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_RX_WITH_TUNNEL_TABLE;
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
      // TODO(derek): questionable value semantics. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/683
      auto port_id = learn_info.vln_info.vlan_id - 1;
      // TODO(derek): vlan_id truncated to 8 bits. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/683
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
}

#else
#error "ASSERT: Unknown TARGET type!"
#endif

void PrepareFdbTableEntryforV4VxlanTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;
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
#endif

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
      // TODO(derek): 8-bit value for 24-bit action parameter. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/677
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
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
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
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

#ifdef ES2K_TARGET

// Never called when DPDK_TARGET is enabled.
void PrepareFdbTableEntryforV4GeneveTunnel(
    p4::v1::TableEntry* table_entry, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry,
    DiagDetail& detail) {
  detail.table_id = LOG_L2_FWD_TX_TABLE;
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
#endif

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
      // note: 8-bit vni (dpdk)
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
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
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
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      } else {
        action->set_action_id(
            GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6));
        {
          auto param = action->add_params();
          param->set_param_id(
              GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6,
                         ACTION_PARAM_TUNNEL_ID));
          param->set_value(EncodeTunnelId(learn_info.tnl_info.vni));
        }
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

void PrepareL2ToTunnelV4(p4::v1::TableEntry* table_entry,
                         const struct mac_learning_info& learn_info,
                         const ::p4::config::v1::P4Info& p4info,
                         bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_TO_TUNNEL_V4_TABLE;
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
                         bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_L2_TO_TUNNEL_V6_TABLE;
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

absl::Status ConfigFdbSmacTableEntry(ClientInterface& client,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareFdbSmacTableEntry(table_entry, learn_info, p4info, insert_entry,
                           detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

absl::Status ConfigL2TunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
      learn_info.tnl_info.remote_ip.family == AF_INET6) {
    PrepareL2ToTunnelV6(table_entry, learn_info, p4info, insert_entry, detail);
  } else {
    PrepareL2ToTunnelV4(table_entry, learn_info, p4info, insert_entry, detail);
  }

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

#endif  // ES2K_TARGET

absl::Status ConfigFdbTxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, insert_entry,
                             detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

absl::Status ConfigFdbRxVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareFdbRxVlanTableEntry(table_entry, learn_info, p4info, insert_entry,
                             detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
}

absl::Status ConfigFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

#if defined(DPDK_TARGET)
  PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                       insert_entry, detail);
#elif defined(ES2K_TARGET)
  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                         insert_entry, detail);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                          insert_entry, detail);
  } else {
    if (!insert_entry) {
      // Tunnel type doesn't matter for delete. So calling one of the functions
      // to prepare the entry
      PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info,
                                           insert_entry, detail);
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailureWithMacAddr(insert_entry, detail.getLogTableName(),
                          learn_info.mac_addr);
  }
  return status;
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}

#if defined(ES2K_TARGET)
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
}
#endif  // ES2K_TARGET

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
#endif
}

#if defined(ES2K_TARGET)

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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, ((src_port >> 8) & 0xff), (src_port & 0xff)));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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

      // To work around a bug in the Linux Networking P4 program, we
      // ignore the src_port value specified by the caller and instead
      // set the src_port param to (dst_port * 2).
      uint16_t src_port = htons(tunnel_info.dst_port) * 2;

      param->set_value(
          EncodeByteValue(2, (src_port >> 8) & 0xff, src_port & 0xff));
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
      param->set_value(EncodeVniValue(tunnel_info.vni));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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
}

#endif  // ES2K_TARGET

void PrepareTunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                 const struct tunnel_info& tunnel_info,
                                 const ::p4::config::v1::P4Info& p4info,
                                 bool insert_entry) {
  // match remote ipv4 addr
  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));

#if defined(ES2K_TARGET)
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));

  // TODO(derek): table does not have a bridge_id match field. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/617 for details.
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID));
  match->mutable_exact()->set_value(EncodeByteValue(1, tunnel_info.bridge_id));

  // match vni
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_VNI));
  match2->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));
#elif defined(DPDK_TARGET)
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));

  // match vxlan tunnel type
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE));
  match->mutable_exact()->set_value(EncodeByteValue(1, TUNNEL_TYPE_VXLAN));

  // match local ipv4 addr
  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST));
  match2->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));
#else
#error "ASSERT: Unknown TARGET type!"
#endif

#if defined(DPDK_TARGET)
  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_DECAP_OUTER_IPV4));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_DECAP_OUTER_IPV4,
                                     ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID));
      // TODO(derek): tunnel_id truncated to 8 bits. [dpdk]
      // See https://github.com/ipdk-io/networking-recipe/issues/685
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
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

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
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR, ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    }
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif
}

#if defined(ES2K_TARGET)
void PrepareV6TunnelTermTableEntry(p4::v1::TableEntry* table_entry,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, IPV6_TUNNEL_TERM_TABLE));

  // TODO(derek): table does not have a bridge_id match field. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/617
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
  match2->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(GetActionId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN));
        auto param = action->add_params();
        param->set_param_id(
            GetParamId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN,
                       ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

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
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else if (tunnel_info.tunnel_type == OVS_TUNNEL_GENEVE) {
        action->set_action_id(
            GetActionId(p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR));
        auto param = action->add_params();
        param->set_param_id(GetParamId(
            p4info, ACTION_SET_GENEVE_DECAP_OUTER_HDR, ACTION_PARAM_TUNNEL_ID));
        param->set_value(EncodeTunnelId(tunnel_info.vni));

      } else {
        std::cout << "Unsupported tunnel type" << std::endl;
      }
    }
  }
}
#endif  // ES2K_TARGET

absl::Status ConfigEncapTableEntry(ClientInterface& client,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

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

  return client.sendWriteRequest(write_request);
}

#if defined(ES2K_TARGET)

void PrepareVxlanDecapModTableEntry(p4::v1::TableEntry* table_entry,
                                    const struct tunnel_info& tunnel_info,
                                    const ::p4::config::v1::P4Info& p4info,
                                    bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_DECAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VXLAN_DECAP_MOD_TABLE,
                                      VXLAN_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    {
      action->set_action_id(GetActionId(p4info, ACTION_VXLAN_DECAP_OUTER_HDR));
    }
  }
}

void PrepareGeneveDecapModTableEntry(p4::v1::TableEntry* table_entry,
                                     const struct tunnel_info& tunnel_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, GENEVE_DECAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, GENEVE_DECAP_MOD_TABLE,
                                      GENEVE_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR));
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    {
      action->set_action_id(GetActionId(p4info, ACTION_GENEVE_DECAP_OUTER_HDR));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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
      // note: magic number
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_DEI));
      // note: magic number
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_VXLAN_DECAP_AND_PUSH_VLAN,
                     ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID));
      // TODO(derek): port_vlan truncated to 8 bits. [es2k]
      // See https://github.com/ipdk-io/networking-recipe/issues/678
      param->set_value(EncodeByteValue(1, tunnel_info.vlan_info.port_vlan));
    }
  }
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
  match->mutable_exact()->set_value(EncodeVniValue(tunnel_info.vni));

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
      // note: magic number
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_DEI));
      // note: magic number
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, ACTION_GENEVE_DECAP_AND_PUSH_VLAN,
                     ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID));
      // TODO(derek): port_vlan truncated to 8 bits. [es2k]
      // See https://github.com/ipdk-io/networking-recipe/issues/679
      param->set_value(EncodeByteValue(1, tunnel_info.vlan_info.port_vlan));
    }
  }
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
}

absl::Status ConfigDecapTableEntry(ClientInterface& client,
                                   const struct tunnel_info& tunnel_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  if (tunnel_info.vlan_info.port_vlan_mode == P4_PORT_VLAN_NATIVE_TAGGED) {
    PrepareDecapModTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else {
    PrepareDecapModAndVlanPushTableEntry(table_entry, tunnel_info, p4info,
                                         insert_entry);
  }

  return client.sendWriteRequest(write_request);
}

void PrepareVlanPushTableEntry(p4::v1::TableEntry* table_entry,
                               const uint16_t vlan_id,
                               const ::p4::config::v1::P4Info& p4info,
                               bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VLAN_PUSH_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VLAN_PUSH_MOD_TABLE,
                                      VLAN_PUSH_MOD_KEY_MOD_BLOB_PTR));
  // note: mod_blob_ptr is bit<24>, vlan_id is bit<12>, encoded value is bit<8>.
  match->mutable_exact()->set_value(EncodeByteValue(1, vlan_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_PCP));
      // note: magic number
      // note: pcp is bit<3>
      param->set_value(EncodeByteValue(1, 1));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_DEI));
      // note: magic number
      // note: dei is bit<1>
      param->set_value(EncodeByteValue(1, 0));
    }
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, VLAN_PUSH_MOD_ACTION_VLAN_PUSH,
                                     ACTION_VLAN_PUSH_PARAM_VLAN_ID));
      // note: vlan_id is bit<12>, encoded value is bit<8>
      param->set_value(EncodeByteValue(1, vlan_id));
    }
  }
}

void PrepareVlanPopTableEntry(p4::v1::TableEntry* table_entry,
                              const uint16_t vlan_id,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VLAN_POP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VLAN_POP_MOD_TABLE,
                                      VLAN_POP_MOD_KEY_MOD_BLOB_PTR));
  // TODO(derek): vlan_id truncated to 8 bits. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/684
  match->mutable_exact()->set_value(EncodeByteValue(1, vlan_id));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, VLAN_POP_MOD_ACTION_VLAN_POP));
  }
}

absl::Status ConfigVlanPushTableEntry(ClientInterface& client,
                                      const uint16_t vlan_id,
                                      const ::p4::config::v1::P4Info& p4info,
                                      bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareVlanPushTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::Status ConfigVlanPopTableEntry(ClientInterface& client,
                                     const uint16_t vlan_id,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareVlanPopTableEntry(table_entry, vlan_id, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
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
}

void PrepareSrcIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct ip_mac_map_info& ip_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_SRC_IP_MAC_MAP_TABLE;
  table_entry->set_table_id(GetTableId(p4info, SRC_IP_MAC_MAP_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, SRC_IP_MAC_MAP_TABLE,
                                      SRC_IP_MAC_MAP_TABLE_KEY_SRC_IP));
  match->mutable_exact()->set_value(
      CanonicalizeIp(ip_info.src_ip_addr.ip.v4addr.s_addr));

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
}

void PrepareDstIpMacMapTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct ip_mac_map_info& ip_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry, DiagDetail& detail) {
  detail.table_id = LOG_DST_IP_MAC_MAP_TABLE;
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
}

void PrepareTxAccVsiTableEntry(p4::v1::TableEntry* table_entry, uint32_t sp,
                               const ::p4::config::v1::P4Info& p4info) {
  table_entry->set_table_id(GetTableId(p4info, TX_ACC_VSI_TABLE));

  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, TX_ACC_VSI_TABLE, TX_ACC_VSI_TABLE_KEY_VSI));
  // TODO(derek): sp value truncated to 8 bits. [es2k]
  // See https://github.com/ipdk-io/networking-recipe/issues/680 for details.
  match->mutable_exact()->set_value(
      EncodeByteValue(1, (sp - ES2K_VPORT_ID_OFFSET)));

#if 0
  /* unused match key of 0, code is added for reference */
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, TX_ACC_VSI_TABLE,
                      TX_ACC_VSI_TABLE_KEY_ZERO_PADDING));
  match->mutable_exact()->set_value(EncodeByteValue(1, 0));
#endif
}

absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV4TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareL2ToTunnelV4(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetL2ToTunnelV6TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareL2ToTunnelV6(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding = false) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

#if defined(DPDK_TARGET)
  PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info, false,
                                       detail);
#elif defined(ES2K_TARGET)
  if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_VXLAN) {
    PrepareFdbTableEntryforV4VxlanTunnel(table_entry, learn_info, p4info, false,
                                         detail);
  } else if (learn_info.tnl_info.tunnel_type == OVS_TUNNEL_GENEVE) {
    PrepareFdbTableEntryforV4GeneveTunnel(table_entry, learn_info, p4info,
                                          false, detail);
  } else {
    // TODO(derek): display tunnel type in message.
    return absl::UnknownError("Unsupported tunnel type");
  }
#else
#error "ASSERT: Unknown TARGET type!"
#endif

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetFdbVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding = false) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetVmSrcTableEntry(
    ClientInterface& client, struct ip_mac_map_info ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareSrcIpMacMapTableEntry(table_entry, ip_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetVmDstTableEntry(
    ClientInterface& client, const struct ip_mac_map_info& ip_info,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initReadRequest(&read_request);

  PrepareDstIpMacMapTableEntry(table_entry, ip_info, p4info, false, detail);

  return client.sendReadRequest(read_request);
}

absl::StatusOr<::p4::v1::ReadResponse> GetTxAccVsiTableEntry(
    ClientInterface& client, uint32_t sp,
    const ::p4::config::v1::P4Info& p4info) {
  ::p4::v1::ReadRequest read_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initReadRequest(&read_request);

  PrepareTxAccVsiTableEntry(table_entry, sp, p4info);

  return client.sendReadRequest(read_request);
}

absl::Status ConfigureVsiSrcPortTableEntry(
    ClientInterface& client, const struct src_port_info& sp,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareSrcPortTableEntry(table_entry, sp, p4info, insert_entry);

  return client.sendWriteRequest(write_request);
}

absl::Status ConfigRxTunnelSrcPortTableEntry(
    ClientInterface& client, const struct tunnel_info& tunnel_info,
    const ::p4::config::v1::P4Info& p4info, bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    PrepareRxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6RxTunnelTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  }

  return client.sendWriteRequest(write_request);
}

#endif  // ES2K_TARGET

absl::Status ConfigTunnelTermTableEntry(ClientInterface& client,
                                        const struct tunnel_info& tunnel_info,
                                        const ::p4::config::v1::P4Info& p4info,
                                        bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);
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

  return client.sendWriteRequest(write_request);
}

#if defined(ES2K_TARGET)

absl::Status ConfigDstIpMacMapTableEntry(ClientInterface& client,
                                         const struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareDstIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry,
                               detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailure(insert_entry, detail.getLogTableName());
  }
  return status;
}

absl::Status ConfigSrcIpMacMapTableEntry(ClientInterface& client,
                                         const struct ip_mac_map_info& ip_info,
                                         const ::p4::config::v1::P4Info& p4info,
                                         bool insert_entry) {
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  DiagDetail detail;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareSrcIpMacMapTableEntry(table_entry, ip_info, p4info, insert_entry,
                               detail);

  auto status = client.sendWriteRequest(write_request);
  if (!status.ok()) {
    LogFailure(insert_entry, detail.getLogTableName());
  }
  return status;
}

//----------------------------------------------------------------------
// Predicate functions (ES2K)
//----------------------------------------------------------------------

static inline bool HaveL2ToTunnelV4TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  return GetL2ToTunnelV4TableEntry(client, learn_info, p4info).ok();
}

static inline bool HaveL2ToTunnelV6TableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info) {
  return GetL2ToTunnelV6TableEntry(client, learn_info, p4info).ok();
}

static inline bool HaveFdbTunnelTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding = false) {
  return GetFdbTunnelTableEntry(client, learn_info, p4info, adding).ok();
}

static inline bool HaveFdbVlanTableEntry(
    ClientInterface& client, const struct mac_learning_info& learn_info,
    const ::p4::config::v1::P4Info& p4info, bool adding = false) {
  return GetFdbVlanTableEntry(client, learn_info, p4info, adding).ok();
}

static inline bool HaveVmSrcTableEntry(ClientInterface& client,
                                       struct ip_mac_map_info ip_info,
                                       const ::p4::config::v1::P4Info& p4info) {
  return GetVmSrcTableEntry(client, ip_info, p4info).ok();
}

static inline bool HaveVmDstTableEntry(ClientInterface& client,
                                       const struct ip_mac_map_info& ip_info,
                                       const ::p4::config::v1::P4Info& p4info) {
  return GetVmDstTableEntry(client, ip_info, p4info).ok();
}

//----------------------------------------------------------------------
// C++ functions that implement the public API.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// ConfigFdbEntry (ES2K)
//
// learn_info is passed by value because this function may make local
// modifications to it.
//----------------------------------------------------------------------
void ConfigFdbEntry(ClientInterface& client,
                    struct mac_learning_info learn_info, bool insert_entry,
                    const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  /* In the delete case, we do not know whether this is a Tunnel learn
   * entry or a regular VSI learn entry. Check for a match in one of
   * the L2 Tunnel tables and set the appropriate properties in the
   * learn_info structure.
   */
  if (!insert_entry) {
    if (HaveL2ToTunnelV4TableEntry(client, learn_info, p4info)) {
      learn_info.is_tunnel = true;
    }

    /* If learn_info.is_tunnel is not true, then we need to check for v6 table
     * entry as the entry can be either in V4 or V6 tunnel table.
     */
    if (!learn_info.is_tunnel) {
      if (HaveL2ToTunnelV6TableEntry(client, learn_info, p4info)) {
        learn_info.is_tunnel = true;
        learn_info.tnl_info.local_ip.family = AF_INET6;
        learn_info.tnl_info.remote_ip.family = AF_INET6;
      }
    }
  }

  if (learn_info.is_tunnel) {
    if (insert_entry) {
      if (HaveFdbTunnelTableEntry(client, learn_info, p4info, insert_entry)) {
        // Return if entry already exists.
        return;
      }
    }

    status =
        ConfigFdbTunnelTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }

    status = ConfigL2TunnelTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }

    status = ConfigFdbSmacTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }
  } else {
    if (insert_entry) {
      if (HaveFdbVlanTableEntry(client, learn_info, p4info, insert_entry)) {
        // Return if entry already exists.
        return;
      }

      status =
          ConfigFdbRxVlanTableEntry(client, learn_info, p4info, insert_entry);
      if (!status.ok()) {
        // Ignore errors (why?)
      }

      // TODO(derek): refactor (extract method)
      //
      // GetVsiSrcPort(ClientInterface& client, const P4Info& p4info,
      //               uint32_t src_port, uint32_t& vsi_port);
      auto response_or_status =
          GetTxAccVsiTableEntry(client, learn_info.src_port, p4info);
      if (!response_or_status.ok()) {
        return;
      }

      ::p4::v1::ReadResponse read_response =
          std::move(response_or_status).value();
      std::vector<::p4::v1::TableEntry> table_entries;

      table_entries.reserve(read_response.entities().size());

      int param_id =
          GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                     ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

      uint32_t host_sp = 0;
      for (const auto& entity : read_response.entities()) {
        p4::v1::TableEntry table_entry_1 = entity.table_entry();
        auto* table_action = table_entry_1.mutable_action();
        auto* action = table_action->mutable_action();
        for (const auto& param : action->params()) {
          if (param_id == param.param_id()) {
            const std::string& s1 = param.value();
            std::string s2 = s1;
            for (int param_bytes = 0; param_bytes < 4; param_bytes++) {
              host_sp = host_sp << 8 | int(s2[param_bytes]);
            }
            break;
          }
        }
      }

      learn_info.src_port = host_sp;
      // end of refactoring
    }

    status =
        ConfigFdbTxVlanTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }

    status = ConfigFdbSmacTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }
  }
}

//----------------------------------------------------------------------
// ConfigRxTunnelSrcEntry (ES2K)
//----------------------------------------------------------------------
void ConfigRxTunnelSrcEntry(ClientInterface& client,
                            const struct tunnel_info& tunnel_info,
                            bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  status = ConfigRxTunnelSrcPortTableEntry(client, tunnel_info, p4info,
                                           insert_entry);
  if (!status.ok()) return;
}

//----------------------------------------------------------------------
// ConfigTunnelSrcPortEntry (ES2K)
//----------------------------------------------------------------------
void ConfigTunnelSrcPortEntry(ClientInterface& client,
                              const struct src_port_info& tnl_sp,
                              bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  table_entry = client.initWriteRequest(&write_request, insert_entry);

  PrepareSrcPortTableEntry(table_entry, tnl_sp, p4info, insert_entry);

  status = client.sendWriteRequest(write_request);
  if (!status.ok()) return;
}

//----------------------------------------------------------------------
// ConfigSrcPortEntry (ES2K)
//
// vsi_sp is passed by value because this function makes local
// modifications to it.
//----------------------------------------------------------------------
void ConfigSrcPortEntry(ClientInterface& client, struct src_port_info vsi_sp,
                        bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  // TODO(derek): refactor (extract method)
  //
  // GetVsiSrcPort(ClientInterface& client, const P4Info& p4info,
  //               uint32_t src_port, uint32_t& vsi_port);
  auto response_or_status =
      GetTxAccVsiTableEntry(client, vsi_sp.src_port, p4info);
  if (!response_or_status.ok()) return;

  ::p4::v1::ReadResponse read_response = std::move(response_or_status).value();
  std::vector<::p4::v1::TableEntry> table_entries;

  table_entries.reserve(read_response.entities().size());

  int param_id =
      GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                 ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

  uint32_t host_sp = 0;
  for (const auto& entity : read_response.entities()) {
    p4::v1::TableEntry table_entry_1 = entity.table_entry();
    auto* table_action = table_entry_1.mutable_action();
    auto* action = table_action->mutable_action();
    for (const auto& param : action->params()) {
      if (param_id == param.param_id()) {
        const std::string& s1 = param.value();
        std::string s2 = s1;
        for (int param_bytes = 0; param_bytes < 4; param_bytes++) {
          host_sp = host_sp << 8 | int(s2[param_bytes]);
        }
        break;
      }
    }
  }

  vsi_sp.src_port = host_sp;
  // end of refactoring

  status = ConfigureVsiSrcPortTableEntry(client, vsi_sp, p4info, insert_entry);
  if (!status.ok()) return;
}

//----------------------------------------------------------------------
// ConfigVlanEntry (ES2K)
//----------------------------------------------------------------------
void ConfigVlanEntry(ClientInterface& client, uint16_t vlan_id,
                     bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  status = ConfigVlanPushTableEntry(client, vlan_id, p4info, insert_entry);
  if (!status.ok()) return;

  status = ConfigVlanPopTableEntry(client, vlan_id, p4info, insert_entry);
  if (!status.ok()) return;
}

#elif defined(DPDK_TARGET)

//----------------------------------------------------------------------
// ConfigFdbEntry (DPDK)
//----------------------------------------------------------------------
void ConfigFdbEntry(ClientInterface& client,
                    const struct mac_learning_info& learn_info,
                    bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  if (learn_info.is_tunnel) {
    status =
        ConfigFdbTunnelTableEntry(client, learn_info, p4info, insert_entry);
  } else if (learn_info.is_vlan) {
    status =
        ConfigFdbTxVlanTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) return;

    status =
        ConfigFdbRxVlanTableEntry(client, learn_info, p4info, insert_entry);
    if (!status.ok()) return;
  }
}

#endif  // DPDK_TARGET

//----------------------------------------------------------------------
// ConfigTunnelEntry (common)
//----------------------------------------------------------------------
void ConfigTunnelEntry(ClientInterface& client,
                       const struct tunnel_info& tunnel_info, bool insert_entry,
                       const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  status = ConfigEncapTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;

#if defined(ES2K_TARGET)
  status = ConfigDecapTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;
#endif

  status =
      ConfigTunnelTermTableEntry(client, tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;
}

#if defined(ES2K_TARGET)

//----------------------------------------------------------------------
// ConfigIpMacMapEntry (ES2K)
//----------------------------------------------------------------------
void ConfigIpMacMapEntry(ClientInterface& client,
                         const struct ip_mac_map_info& ip_info,
                         bool insert_entry, const char* grpc_addr) {
  absl::Status status;

  // Start a new client session.
  status = client.connect(grpc_addr);
  if (!status.ok()) return;

  // Fetch P4Info object from server.
  ::p4::config::v1::P4Info p4info;
  status = client.getPipelineConfig(&p4info);
  if (!status.ok()) return;

  if (insert_entry) {
    if (HaveVmSrcTableEntry(client, ip_info, p4info)) {
      goto try_dstip;
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    status = ConfigSrcIpMacMapTableEntry(client, ip_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }
  }

try_dstip:
  if (insert_entry) {
    if (HaveVmDstTableEntry(client, ip_info, p4info)) {
      return;
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    status = ConfigDstIpMacMapTableEntry(client, ip_info, p4info, insert_entry);
    if (!status.ok()) {
      // Ignore errors (why?)
    }
  }
}

#endif  // ES2K_TARGET

}  // namespace ovsp4rt
