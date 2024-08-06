// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;
  
constexpr char SET_VXLAN_UNDERLAY_V4[] = "set_vxlan_underlay_v4";
constexpr char POP_VLAN_SET_VXLAN_UNDERLAY_V4[] =
    "pop_vlan_set_vxlan_underlay_v4";

constexpr char SET_VXLAN_UNDERLAY_V6[] = "set_vxlan_underlay_v6";
constexpr char POP_VLAN_SET_VXLAN_UNDERLAY_V6[] =
    "pop_vlan_set_vxlan_underlay_v6";

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class FdbTxVxlanEntryTest : public ::testing::Test {
 protected:
  FdbTxVxlanEntryTest() {}

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  void InitLearnInfo(uint8_t tunnel_type) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memcpy(learn_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    learn_info.bridge_id = 42;
    learn_info.tnl_info.tunnel_type = tunnel_type;
  }

  void InitV4NativeTagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET;
    learn_info.tnl_info.remote_ip.family = AF_INET;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    learn_info.tnl_info.vni = 0x1984U;
    ACTION_ID = GetActionId(action_name);
  }

  void InitV4NativeUntagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET;
    learn_info.tnl_info.remote_ip.family = AF_INET;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    learn_info.tnl_info.vni = 0x1776U;
    ACTION_ID = GetActionId(action_name);
  }

  void InitV6NativeTagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET6;
    learn_info.tnl_info.remote_ip.family = AF_INET6;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    learn_info.tnl_info.vni = 0xFACEU;
    ACTION_ID = GetActionId(action_name);
  }

  void InitV6NativeUntagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET6;
    learn_info.tnl_info.remote_ip.family = AF_INET6;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    learn_info.tnl_info.vni = 0xCEDEU;
    ACTION_ID = GetActionId(action_name);
  }

  void CheckResults() const {
    ASSERT_FALSE(TABLE == nullptr);

    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    if (ACTION_ID) {
      EXPECT_EQ(action.action_id(), ACTION_ID);
    }

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), PARAM_ID);
    CheckTunnelIdParam(param.value());
  }

  void CheckTunnelIdParam(const std::string& param_value) const {
    EXPECT_EQ(param_value.size(), 3);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, learn_info.tnl_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tnl_info.vni is 0x" << learn_info.tnl_info.vni << '\n'
        << std::setw(0) << std::dec;
  }

  // Working variables
  ::p4::v1::TableEntry table_entry;
  struct mac_learning_info learn_info = {0};
  DiagDetail detail;

  // Values to check against
  uint32_t TABLE_ID;
  uint32_t ACTION_ID = -1;
  uint32_t PARAM_ID = 1;

 private:
  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
    std::cerr << "Table '" << table_name << "' not found\n";
  }

  uint32_t GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    return -1;
  }

  const ::p4::config::v1::Table* TABLE = nullptr;
};

//----------------------------------------------------------------------
// PrepareFdbTableEntryforV4VxlanTunnel()
//----------------------------------------------------------------------

TEST_F(FdbTxVxlanEntryTest, insert_v4_tagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(SET_VXLAN_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

TEST_F(FdbTxVxlanEntryTest, insert_v4_untagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeUntagged(POP_VLAN_SET_VXLAN_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

TEST_F(FdbTxVxlanEntryTest, insert_v6_tagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV6NativeTagged(SET_VXLAN_UNDERLAY_V6);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

TEST_F(FdbTxVxlanEntryTest, insert_v6_untagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV6NativeUntagged(POP_VLAN_SET_VXLAN_UNDERLAY_V6);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

}  // namespace ovsp4rt
