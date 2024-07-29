// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class EncodeTableIdTest : public ::testing::Test {
 protected:
  EncodeTableIdTest() { memset(&learn_info, 0, sizeof(learn_info)); }

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  static uint32_t DecodeTableId(const std::string& string_value) {
    return DecodeWordValue(string_value) & 0xffffff;
  }

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

  void InitV4NativeTagged(uint32_t action_id) {
    learn_info.tnl_info.local_ip.family = AF_INET;
    learn_info.tnl_info.remote_ip.family = AF_INET;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    learn_info.tnl_info.vni = 0x1984E;
    ACTION_ID = action_id;
  }

  void InitV4NativeUntagged(uint32_t action_id) {
    learn_info.tnl_info.local_ip.family = AF_INET;
    learn_info.tnl_info.remote_ip.family = AF_INET;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    learn_info.tnl_info.vni = 0xA1776;
    ACTION_ID = action_id;
  }

  void InitV6NativeTagged(uint32_t action_id) {
    learn_info.tnl_info.local_ip.family = AF_INET6;
    learn_info.tnl_info.remote_ip.family = AF_INET6;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    learn_info.tnl_info.vni = 0xFACED;
    ACTION_ID = action_id;
  }

  void InitV6NativeUntagged(uint32_t action_id) {
    learn_info.tnl_info.local_ip.family = AF_INET6;
    learn_info.tnl_info.remote_ip.family = AF_INET6;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    learn_info.tnl_info.vni = 0xCEDED;
    ACTION_ID = action_id;
  }

  void CheckResults() {
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);

    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    // EXPECT_EQ(action.action_id(), ACTION_ID);

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), PARAM_ID);

    auto param_value = param.value();
    EXPECT_EQ(param_value.size(), 3);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, learn_info.tnl_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tnl_info.vni is 0x" << learn_info.tnl_info.vni << '\n'
        << std::setw(0) << std::dec;
  }

  // Working variables
  p4::v1::TableEntry table_entry;
  struct mac_learning_info learn_info;
  DiagDetail detail;

  // Values to check against
  uint32_t TABLE_ID = 40240205;  // l2_fwd_rx_table
  uint32_t ACTION_ID = -1;
  uint32_t PARAM_ID = 1;
};

TEST_F(EncodeTableIdTest, PrepareFdbTableEntryforV4VxlanTunnel_v4_tagged) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(23849990);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

TEST_F(EncodeTableIdTest, PrepareFdbTableEntryforV4VxlanTunnel_v4_untagged) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeUntagged(31983357);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

TEST_F(EncodeTableIdTest, PrepareFdbTableEntryforV4VxlanTunnel_v6_tagged) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV6NativeTagged(19193142);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

TEST_F(EncodeTableIdTest, PrepareFdbTableEntryforV4VxlanTunnel_v6_untagged) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV6NativeUntagged(23849990);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, learn_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckResults();
}

}  // namespace ovsp4rt
