// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
// TODO: ovs-p4rt logging

#include <arpa/inet.h>

#include "absl/flags/flag.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovs_p4rt_session.h"
#include "ovs_p4rt_tls_credentials.h"

#if defined(DPDK_TARGET)
#include "dpdk/p4_name_mapping.h"
#elif defined(ES2K_TARGET)
#include "es2k/p4_name_mapping.h"
#endif

ABSL_FLAG(std::string, grpc_addr, "localhost:9559",
          "P4Runtime server address.");
ABSL_FLAG(uint64_t, device_id, 1, "P4Runtime device ID.");

namespace ovs_p4rt {

using OvsP4rtStream = ::grpc::ClientReaderWriter<p4::v1::StreamMessageRequest,
                                                 p4::v1::StreamMessageResponse>;

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

std::string CanonicalizeIp(const uint32_t ipv4addr) {
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
  // Based on p4 program for ES2K, we need to provide a match key Tunnel Flag
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_TUN_FLAG));

  match1->mutable_exact()->set_value(EncodeByteValue(1, 0));
#endif

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
#if defined(DPDK_TARGET)
      auto port_id = learn_info.vln_info.vlan_id - 1;
#elif defined(ES2K_TARGET)
      auto port_id = learn_info.mac_addr[1] + ES2K_VPORT_ID_OFFSET;
#else
      auto port_id = 0;
#endif
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
  return;
}

#if defined(ES2K_TARGET)
void PrepareFdbTxV6VlanTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct mac_learning_info& learn_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_IPV6_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_IPV6_TABLE,
                                      L2_FWD_TX_IPV6_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  // Based on p4 program for ES2K, we need to provide a match key Tunnel Flag
  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE,
                                       L2_FWD_TX_IPV6_TABLE_KEY_TUN_FLAG));

  match1->mutable_exact()->set_value(EncodeByteValue(1, 0));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_TX_IPV6_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, L2_FWD_TX_IPV6_TABLE_ACTION_L2_FWD,
                                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.mac_addr[1] + ES2K_VPORT_ID_OFFSET;
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
  return;
}
#endif

#if defined(ES2K_TARGET)
/* Sem Bypass table is specific to ES2K platform. When Data packets exchange
 * between overlay network (VSI to VSI), along with l2_fwd_tx table SEM bypass
 * table also should be programmed.
 * Match Key: DMAC
 * Action: VSI ID to which a matching packet need to forwarded.
 */
void PrepareSemBypassTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info& learn_info,
                                const ::p4::config::v1::P4Info& p4info,
                                bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, SEM_BYPASS_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, SEM_BYPASS_TABLE, SEM_BYPASS_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, SEM_BYPASS_TABLE_ACTION_SET_DEST));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, SEM_BYPASS_TABLE_ACTION_SET_DEST,
                                     ACTION_SET_DEST_PARAM_PORT_ID));
      auto port_id = learn_info.mac_addr[1] + ES2K_VPORT_ID_OFFSET;
      param->set_value(EncodeByteValue(1, port_id));
    }
  }
  return;
}
#endif

void PrepareFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
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
#if defined(DPDK_TARGET)
      auto port_id = learn_info.vln_info.vlan_id - 1;
#elif defined(ES2K_TARGET)
      auto port_id = learn_info.mac_addr[1] + ES2K_VPORT_ID_OFFSET;
#else
      auto port_id = 0;
#endif
      param->set_value(EncodeByteValue(1, port_id));
    }
  }

  return;
}

#if defined(ES2K_TARGET)
void PrepareFdbRxV6VlanTableEntry(p4::v1::TableEntry* table_entry,
                                  const struct mac_learning_info& learn_info,
                                  const ::p4::config::v1::P4Info& p4info,
                                  bool insert_entry) {
  table_entry->set_table_id(
      GetTableId(p4info, L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE,
                      L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE_KEY_DST_MAC));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(
        GetActionId(p4info, L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE_ACTION_L2_FWD));
    {
      auto param = action->add_params();
      param->set_param_id(
          GetParamId(p4info, L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE_ACTION_L2_FWD,
                     ACTION_L2_FWD_PARAM_PORT));
      auto port_id = learn_info.mac_addr[1] + ES2K_VPORT_ID_OFFSET;
      param->set_value(EncodeByteValue(1, port_id));
    }
  }

  return;
}
#endif

void PrepareFdbTableEntryforV4Tunnel(p4::v1::TableEntry* table_entry,
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
  // Based on p4 program for ES2K, we need to provide a match key Tunnel Flag
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_TUN_FLAG));

  match1->mutable_exact()->set_value(EncodeByteValue(1, 0));
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
    action->set_action_id(GetActionId(
        p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V4_OVERLAY_V4));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V4_OVERLAY_V4,
          ACTION_SET_TUNNEL_UNDERLAY_V4_OVERLAY_V4_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V4_OVERLAY_V4,
          ACTION_SET_TUNNEL_UNDERLAY_V4_OVERLAY_V4_PARAM_DST_ADDR));
      std::string ip_address =
          CanonicalizeIp(learn_info.tnl_info.remote_ip.ip.v4addr.s_addr);
      param->set_value(ip_address);
    }
  }
#endif
  return;
}

#if defined(ES2K_TARGET)
void PrepareFdbTableEntryforV6Tunnel(p4::v1::TableEntry* table_entry,
                                     const struct mac_learning_info& learn_info,
                                     const ::p4::config::v1::P4Info& p4info,
                                     bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_IPV6_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_IPV6_TABLE,
                                      L2_FWD_TX_IPV6_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  // Based on p4 program for ES2K, we need to provide a match key Tunnel Flag
  auto match1 = table_entry->add_match();
  match1->set_field_id(
      GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_TUN_FLAG));

  match1->mutable_exact()->set_value(EncodeByteValue(1, 0));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(
        p4info, L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6,
          ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, learn_info.tnl_info.vni));
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6,
          ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_1));
      std::string ip_address = CanonicalizeIp(
          learn_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[0]);
      param->set_value(ip_address);
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6,
          ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_2));
      std::string ip_address = CanonicalizeIp(
          learn_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[1]);
      param->set_value(ip_address);
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6,
          ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_3));
      std::string ip_address = CanonicalizeIp(
          learn_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[0]);
      param->set_value(ip_address);
    }

    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(
          p4info, L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6,
          ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_4));
      std::string ip_address = CanonicalizeIp(
          learn_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[0]);
      param->set_value(ip_address);
    }
  }
  return;
}
#endif

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

#if defined(ES2K_TARGET)
absl::Status ConfigFdbTxV6VlanTableEntry(
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
  PrepareFdbTxV6VlanTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt::SendWriteRequest(session, write_request);
}
#endif

#if defined(ES2K_TARGET)
absl::Status ConfigSemBypassTableEntry(
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
  PrepareSemBypassTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt::SendWriteRequest(session, write_request);
}
#endif

#if defined(ES2K_TARGET)
absl::Status ConfigFdbRxV6VlanTableEntry(
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
  PrepareFdbRxV6VlanTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt::SendWriteRequest(session, write_request);
}
#endif

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
  PrepareFdbTableEntryforV4Tunnel(table_entry, learn_info, p4info,
                                  insert_entry);
#elif defined(ES2K_TARGET)
  if (learn_info.tnl_info.local_ip.family == AF_INET6 &&
      learn_info.tnl_info.remote_ip.family == AF_INET6) {
    PrepareFdbTableEntryforV6Tunnel(table_entry, learn_info, p4info,
                                    insert_entry);
  } else {
    PrepareFdbTableEntryforV4Tunnel(table_entry, learn_info, p4info,
                                    insert_entry);
  }
#else
  return absl::UnknownError("Unsupported platform")
#endif
  return ovs_p4rt::SendWriteRequest(session, write_request);
}

void PrepareEncapTableEntry(p4::v1::TableEntry* table_entry,
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

#if defined(ES2K_TARGET)
void PrepareV6EncapTableEntry(p4::v1::TableEntry* table_entry,
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
#endif

void PrepareDecapTableEntry(p4::v1::TableEntry* table_entry,
                            const struct tunnel_info& tunnel_info,
                            const ::p4::config::v1::P4Info& p4info,
                            bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                      IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE));
  match->mutable_exact()->set_value(EncodeByteValue(1, TUNNEL_TYPE_VXLAN));

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.remote_ip.ip.v4addr.s_addr));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST));
  match2->mutable_exact()->set_value(
      CanonicalizeIp(tunnel_info.local_ip.ip.v4addr.s_addr));

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
    action->set_action_id(GetActionId(p4info, ACTION_DECAP_OUTER_HDR));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_DECAP_OUTER_HDR,
                                     ACTION_DECAP_OUTER_HDR_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }
#endif

  return;
}

#if defined(ES2K_TARGET)
void PrepareV6DecapTableEntry(p4::v1::TableEntry* table_entry,
                              const struct tunnel_info& tunnel_info,
                              const ::p4::config::v1::P4Info& p4info,
                              bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, IPV6_TUNNEL_TERM_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                      IPV6_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE));
  match->mutable_exact()->set_value(EncodeByteValue(1, TUNNEL_TYPE_VXLAN));

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                       IPV6_TUNNEL_TERM_TABLE_KEY_IPV6_SRC));
  match1->mutable_exact()->set_value(
      CanonicalizeIpv6(tunnel_info.remote_ip.ip.v6addr));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV6_TUNNEL_TERM_TABLE,
                                       IPV6_TUNNEL_TERM_TABLE_KEY_IPV6_DST));
  match2->mutable_exact()->set_value(
      CanonicalizeIpv6(tunnel_info.local_ip.ip.v6addr));

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, ACTION_DECAP_OUTER_HDR));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, ACTION_DECAP_OUTER_HDR,
                                     ACTION_DECAP_OUTER_HDR_PARAM_TUNNEL_ID));
      param->set_value(EncodeByteValue(1, tunnel_info.vni));
    }
  }
  return;
}
#endif

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
    PrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6EncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  }
#else
  return absl::UnknownError("Unsupported platform")
#endif

  return ovs_p4rt::SendWriteRequest(session, write_request);
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
#if defined(DPDK_TARGET)
  PrepareDecapTableEntry(table_entry, tunnel_info, p4info, insert_entry);

#elif defined(ES2K_TARGET)
  if (tunnel_info.local_ip.family == AF_INET &&
      tunnel_info.remote_ip.family == AF_INET) {
    PrepareDecapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  } else if (tunnel_info.local_ip.family == AF_INET6 &&
             tunnel_info.remote_ip.family == AF_INET6) {
    PrepareV6DecapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  }
#else
  return absl::UnknownError("Unsupported platform")
#endif

  return ovs_p4rt::SendWriteRequest(session, write_request);
}

}  // namespace ovs_p4rt

//----------------------------------------------------------------------
// Functions with C interfaces
//----------------------------------------------------------------------

void ConfigFdbTableEntry(struct mac_learning_info learn_info,
                         bool insert_entry) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = ovs_p4rt::OvsP4rtSession::Create(
      absl::GetFlag(FLAGS_grpc_addr), GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id));
  if (!status_or_session.ok()) {
    return;
  }

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<ovs_p4rt::OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status =
      ovs_p4rt::GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  if (learn_info.is_tunnel) {
    status = ConfigFdbTunnelTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
  } else if (learn_info.is_vlan) {
    status = ConfigFdbTxVlanTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) return;

    status = ConfigFdbRxVlanTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) return;

#if defined(ES2K_TARGET)
    status = ConfigFdbTxV6VlanTableEntry(session.get(), learn_info, p4info,
                                         insert_entry);
    if (!status.ok()) return;
    status = ConfigSemBypassTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) return;

    status = ConfigFdbRxV6VlanTableEntry(session.get(), learn_info, p4info,
                                         insert_entry);
#endif
  }
  if (!status.ok()) return;
  return;
}

void ConfigTunnelTableEntry(struct tunnel_info tunnel_info, bool insert_entry) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = OvsP4rtSession::Create(
      absl::GetFlag(FLAGS_grpc_addr), GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id));
  if (!status_or_session.ok()) {
    return;
  }

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;
  status =
      ConfigEncapTableEntry(session.get(), tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;
  status =
      ConfigDecapTableEntry(session.get(), tunnel_info, p4info, insert_entry);
  if (!status.ok()) return;
  return;
}
