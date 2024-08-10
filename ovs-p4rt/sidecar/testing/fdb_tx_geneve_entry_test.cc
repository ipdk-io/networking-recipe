// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbTableEntryforV4GeneveTunnel().

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

constexpr char SET_GENEVE_UNDERLAY_V4[] = "set_geneve_underlay_v4";
constexpr char POP_VLAN_SET_GENEVE_UNDERLAY_V4[] =
    "pop_vlan_set_geneve_underlay_v4";

constexpr char SET_GENEVE_UNDERLAY_V6[] = "set_geneve_underlay_v6";
constexpr char POP_VLAN_SET_GENEVE_UNDERLAY_V6[] =
    "pop_vlan_set_geneve_underlay_v6";

class FdbTxGeneveEntryTest : public BaseTableTest {
 protected:
  FdbTxGeneveEntryTest() {}

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // Initialization methods
  //----------------------------

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
    SelectAction(action_name);
  }

  void InitV4NativeUntagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET;
    learn_info.tnl_info.remote_ip.family = AF_INET;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    learn_info.tnl_info.vni = 0x1776U;
    SelectAction(action_name);
  }

  void InitV6NativeTagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET6;
    learn_info.tnl_info.remote_ip.family = AF_INET6;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    learn_info.tnl_info.vni = 0xFACEU;
    SelectAction(action_name);
  }

  void InitV6NativeUntagged(const std::string& action_name) {
    learn_info.tnl_info.local_ip.family = AF_INET6;
    learn_info.tnl_info.remote_ip.family = AF_INET6;
    learn_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    learn_info.tnl_info.vni = 0xCEDEU;
    SelectAction(action_name);
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    constexpr uint32_t PARAM_ID = 1;

    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    ASSERT_EQ(action.action_id(), ActionId());

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    const auto& param = params[0];
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

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    // TODO(derek): check match fields
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  // Working variables
  struct mac_learning_info learn_info = {0};
  DiagDetail detail;
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(FdbTxGeneveEntryTest, insert_v4_tagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_GENEVE);
  InitV4NativeTagged(SET_GENEVE_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4GeneveTunnel(&table_entry, learn_info, p4info,
                                        INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(FdbTxGeneveEntryTest, insert_v4_untagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_GENEVE);
  InitV4NativeUntagged(POP_VLAN_SET_GENEVE_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4GeneveTunnel(&table_entry, learn_info, p4info,
                                        INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(FdbTxGeneveEntryTest, insert_v6_tagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_GENEVE);
  InitV6NativeTagged(SET_GENEVE_UNDERLAY_V6);

  // Act
  PrepareFdbTableEntryforV4GeneveTunnel(&table_entry, learn_info, p4info,
                                        INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(FdbTxGeneveEntryTest, insert_v6_untagged_entry_minimal) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_GENEVE);
  InitV6NativeUntagged(POP_VLAN_SET_GENEVE_UNDERLAY_V6);

  // Act
  PrepareFdbTableEntryforV4GeneveTunnel(&table_entry, learn_info, p4info,
                                        INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
