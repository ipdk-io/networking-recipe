// Copyright (c) 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
// TODO: ovs-p4rt logging
#include "ovs_p4rt_session.h"
#include <arpa/inet.h>
#include "absl/flags/flag.h"
#include "openvswitch/ovs-p4rt.h"
#include "p4_name_mapping.h"

ABSL_FLAG(std::string, grpc_addr, "127.0.0.1:9339",
          "P4Runtime server address.");
ABSL_FLAG(uint64_t, device_id, 1, "P4Runtime device ID.");

using OvsP4rtStream =
    ::grpc::ClientReaderWriter<p4::v1::StreamMessageRequest,
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
  return EncodeByteValue(4, (ipv4addr & 0xff),
                            ((ipv4addr >> 8) & 0xff),
                            ((ipv4addr >> 16) & 0xff),
                            ((ipv4addr >> 24) & 0xff));
}

std::string CanonicalizeMac(const uint8_t mac[6]) {
  return EncodeByteValue(6, (mac[0] & 0xff),
                            (mac[1] & 0xff),
                            (mac[2] & 0xff),
                            (mac[3] & 0xff),
                            (mac[4] & 0xff),
                            (mac[5] & 0xff));
}

int GetTableId(const ::p4::config::v1::P4Info &p4info,
                 const std::string &t_name) {
  for (const auto &table : p4info.tables()) {
    const auto &pre = table.preamble();
    if (pre.name() == t_name) return pre.id();
  }
  return -1;
}

int GetActionId(const ::p4::config::v1::P4Info &p4info,
                  const std::string &a_name) {
  for (const auto &action : p4info.actions()) {
    const auto &pre = action.preamble();
    if (pre.name() == a_name) return pre.id();
  }
  return -1;
}

int GetParamId(const ::p4::config::v1::P4Info &p4info,
                 const std::string &a_name, const std::string &param_name) {
  for (const auto &action : p4info.actions()) {
    const auto &pre = action.preamble();
    if (pre.name() != a_name) continue;
    for (const auto &param : action.params())
      if (param.name() == param_name) return param.id();
  }
  return -1;
}

int GetMatchFieldId(const ::p4::config::v1::P4Info &p4info,
              const std::string &t_name, const std::string &mf_name) {
  for (const auto &table : p4info.tables()) {
    const auto &pre = table.preamble();
    if (pre.name() != t_name) continue;
    for (const auto &mf : table.match_fields())
      if (mf.name() == mf_name) return mf.id();
  }
  return -1;
}

void PrepareFdbTxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info learn_info,
                                const ::p4::config::v1::P4Info p4info,
                                bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_DST_MAC));

  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

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
  return;
}

void PrepareFdbRxVlanTableEntry(p4::v1::TableEntry* table_entry,
                                const struct mac_learning_info learn_info,
                                const ::p4::config::v1::P4Info p4info,
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

  return;
}

void PrepareFdbTableEntryforTunnel(p4::v1::TableEntry* table_entry,
                                   const struct mac_learning_info learn_info,
                                   const ::p4::config::v1::P4Info p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, L2_FWD_TX_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, L2_FWD_TX_TABLE, L2_FWD_TX_TABLE_KEY_DST_MAC));
  std::string mac_addr = CanonicalizeMac(learn_info.mac_addr);
  match->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
      auto table_action = table_entry->mutable_action();
      auto action = table_action->mutable_action();
      action->set_action_id(GetActionId(p4info, L2_FWD_TX_TABLE_ACTION_SET_TUNNEL));
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
        std::string ip_address = CanonicalizeIp(learn_info.tnl_info.remote_ip.v4addr);
        param->set_value(ip_address);
      }
  }
  return;
}

absl::Status ConfigFdbTxVlanTableEntry(ovs_p4rt_cpp::OvsP4rtSession* session,
                                       const struct mac_learning_info learn_info,
                                       const ::p4::config::v1::P4Info p4info,
                                       bool insert_entry) {
  ::p4::v1::Entity entity;
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  if (insert_entry) {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToInsert(session,
                                      &write_request, &entity);
  } else {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToDelete(session,
                                      &write_request, &entity);
  }  
  PrepareFdbTxVlanTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt_cpp::SendWriteRequest(session, write_request);
}

absl::Status ConfigFdbRxVlanTableEntry(ovs_p4rt_cpp::OvsP4rtSession* session,
                                       const struct mac_learning_info learn_info,
                                       const ::p4::config::v1::P4Info p4info,
                                       bool insert_entry) {
  ::p4::v1::Entity entity;
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;  
  if (insert_entry) {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToInsert(session,
                                      &write_request, &entity);
  } else {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToDelete(session,
                                      &write_request, &entity);
  }
  PrepareFdbRxVlanTableEntry(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt_cpp::SendWriteRequest(session, write_request);
}

absl::Status ConfigFdbTunnelTableEntry(ovs_p4rt_cpp::OvsP4rtSession* session,
                                       const struct mac_learning_info learn_info,
                                       const ::p4::config::v1::P4Info p4info,
                                       bool insert_entry) {
  ::p4::v1::Entity entity;
  ::p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;
  if (insert_entry) {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToInsert(session,
                                      &write_request, &entity);
  } else {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToDelete(session,
                                      &write_request, &entity);
  }  
  PrepareFdbTableEntryforTunnel(table_entry, learn_info, p4info, insert_entry);
  return ovs_p4rt_cpp::SendWriteRequest(session, write_request);
}

void ConfigFdbTableEntry(struct mac_learning_info learn_info, bool insert_entry) {
    // Start a new client session.
    auto status_or_session = ovs_p4rt_cpp::OvsP4rtSession::Create(
      absl::GetFlag(FLAGS_grpc_addr), ::grpc::InsecureChannelCredentials(),
      absl::GetFlag(FLAGS_device_id));
    if (!status_or_session.ok()) {
        return;
    }

    // Unwrap the session from the StatusOr object.
    std::unique_ptr<ovs_p4rt_cpp::OvsP4rtSession> session =
      std::move(status_or_session).value();
    ::p4::config::v1::P4Info p4info;
    ::absl::Status status = ovs_p4rt_cpp::GetForwardingPipelineConfig(session.get(),
		            &p4info);
    if (!status.ok())
        return;    

    if (learn_info.is_tunnel) {
        status = ConfigFdbTunnelTableEntry(session.get(), learn_info,
                                           p4info, insert_entry);
    } else if (learn_info.is_vlan) {
        status = ConfigFdbTxVlanTableEntry(session.get(), learn_info,
                                           p4info, insert_entry);
        if(!status.ok())
            return;
        status = ConfigFdbRxVlanTableEntry(session.get(), learn_info,
                                           p4info, insert_entry);
    }
    if (!status.ok())
        return;

  return;
}

void PrepareEncapTableEntry(p4::v1::TableEntry* table_entry,
                            const struct tunnel_info tunnel_info,
                            const ::p4::config::v1::P4Info p4info,
                            bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, VXLAN_ENCAP_MOD_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, VXLAN_ENCAP_MOD_TABLE, 
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
       param->set_value(CanonicalizeIp(tunnel_info.local_ip.v4addr));
     }
     {
       auto param = action->add_params();
       param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                           ACTION_SET_TUNNEL_PARAM_DST_ADDR));
       param->set_value(CanonicalizeIp(tunnel_info.remote_ip.v4addr));
     }
     {
       auto param = action->add_params();
       param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP,
                                        ACTION_VXLAN_ENCAP_PARAM_DST_PORT));
       uint16_t dst_port = htons(tunnel_info.dst_port);

       param->set_value(EncodeByteValue(2, ((dst_port >> 8) & 0xff),
                                        (dst_port & 0xff)));
     }
     {
       auto param = action->add_params();
       param->set_param_id(GetParamId(p4info, ACTION_VXLAN_ENCAP, 
                                        ACTION_VXLAN_ENCAP_PARAM_VNI));
       param->set_value(EncodeByteValue(1, tunnel_info.vni));
     }
  }

  return;
}

void PrepareDecapTableEntry(p4::v1::TableEntry* table_entry,
                            const struct tunnel_info tunnel_info,
                            const ::p4::config::v1::P4Info p4info,
                            bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, IPV4_TUNNEL_TERM_TABLE));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                      IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE));
  match->mutable_exact()->set_value(EncodeByteValue(1, TUNNEL_TYPE_VXLAN));

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                      IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC));
  match1->mutable_exact()->set_value(CanonicalizeIp(tunnel_info.remote_ip.v4addr));

  auto match2 = table_entry->add_match();
  match2->set_field_id(GetMatchFieldId(p4info, IPV4_TUNNEL_TERM_TABLE,
                       IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST));
  match2->mutable_exact()->set_value(CanonicalizeIp(tunnel_info.local_ip.v4addr));

  if (insert_entry) {
     auto table_action = table_entry->mutable_action();
     auto action = table_action->mutable_action();
     action->set_action_id(GetActionId(p4info, ACTION_DECAP_OUTER_IPV4));
     {
       auto param = action->add_params();
       param->set_param_id(GetParamId(p4info, ACTION_DECAP_OUTER_IPV4,
                           ACTION_SET_TUNNEL_PARAM_TUNNEL_ID));
       param->set_value(EncodeByteValue(1, tunnel_info.vni));

     }
  }
  return;
}

absl::Status ConfigEncapTableEntry(ovs_p4rt_cpp::OvsP4rtSession* session,
                                   const struct tunnel_info tunnel_info,
                                   const ::p4::config::v1::P4Info p4info,
                                   bool insert_entry) {
  p4::v1::Entity entity;
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToInsert(session,
                                      &write_request, &entity);
  } else {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToDelete(session,
                                      &write_request, &entity);
  }
  PrepareEncapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  return ovs_p4rt_cpp::SendWriteRequest(session, write_request);
}

absl::Status ConfigDecapTableEntry(ovs_p4rt_cpp::OvsP4rtSession* session,
                                   const struct tunnel_info tunnel_info,
                                   const ::p4::config::v1::P4Info p4info,
                                   bool insert_entry) {
  p4::v1::Entity entity;
  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  if (insert_entry) {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToInsert(session,
                                      &write_request, &entity);
  } else {
      table_entry = ovs_p4rt_cpp::SetupTableEntryToDelete(session,
                                      &write_request, &entity);
  }

  PrepareDecapTableEntry(table_entry, tunnel_info, p4info, insert_entry);
  return ovs_p4rt_cpp::SendWriteRequest(session, write_request);
}

void ConfigTunnelTableEntry(struct tunnel_info tunnel_info,
                            bool insert_entry) {
    // Start a new client session.
    auto status_or_session = ovs_p4rt_cpp::OvsP4rtSession::Create(
      absl::GetFlag(FLAGS_grpc_addr), ::grpc::InsecureChannelCredentials(),
      absl::GetFlag(FLAGS_device_id));
    if (!status_or_session.ok()) {
       return;
    }

    // Unwrap the session from the StatusOr object.
    std::unique_ptr<ovs_p4rt_cpp::OvsP4rtSession> session =
      std::move(status_or_session).value();
    ::p4::config::v1::P4Info p4info;
    ::absl::Status status = ovs_p4rt_cpp::GetForwardingPipelineConfig(session.get(),
		            &p4info);
    if(!status.ok())
       return;
    status = ConfigEncapTableEntry(session.get(), tunnel_info,
                                   p4info, insert_entry);
    if(!status.ok())
       return;
    status = ConfigDecapTableEntry(session.get(), tunnel_info,
                                   p4info, insert_entry);
    if(!status.ok())
       return;

  return;
}
