// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

constexpr char SET_VXLAN_UNDERLAY_V4[] = "set_vxlan_underlay_v4";
constexpr char POP_VLAN_SET_VXLAN_UNDERLAY_V4[] =
    "pop_vlan_set_vxlan_underlay_v4";

constexpr char SET_VXLAN_UNDERLAY_V6[] = "set_vxlan_underlay_v6";
constexpr char POP_VLAN_SET_VXLAN_UNDERLAY_V6[] =
    "pop_vlan_set_vxlan_underlay_v6";

class FdbTxVxlanEntryTest : public BaseTableTest {
 protected:
  FdbTxVxlanEntryTest() {}

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitLearnInfo(uint8_t tunnel_type) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.bridge_id = 42;
    fdb_info.tnl_info.tunnel_type = tunnel_type;
  }

  void InitV4NativeTagged(const std::string& action_name) {
    fdb_info.tnl_info.local_ip.family = AF_INET;
    fdb_info.tnl_info.remote_ip.family = AF_INET;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    fdb_info.tnl_info.vni = 0x1984U;
    SelectAction(action_name);
  }

  void InitV4NativeUntagged(const std::string& action_name) {
    fdb_info.tnl_info.local_ip.family = AF_INET;
    fdb_info.tnl_info.remote_ip.family = AF_INET;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    fdb_info.tnl_info.vni = 0x1776U;
    SelectAction(action_name);
  }

  void InitV6NativeTagged(const std::string& action_name) {
    fdb_info.tnl_info.local_ip.family = AF_INET6;
    fdb_info.tnl_info.remote_ip.family = AF_INET6;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    fdb_info.tnl_info.vni = 0xFACEU;
    SelectAction(action_name);
  }

  void InitV6NativeUntagged(const std::string& action_name) {
    fdb_info.tnl_info.local_ip.family = AF_INET6;
    fdb_info.tnl_info.remote_ip.family = AF_INET6;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    fdb_info.tnl_info.vni = 0xCEDEU;
    SelectAction(action_name);
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    const auto& param = params[0];

    ASSERT_EQ(param.param_id(), GetParamId("tunnel_id"));
    CheckTunnelIdParam(param.value());
  }

  void CheckTunnelIdParam(const std::string& param_value) const {
    EXPECT_EQ(param_value.size(), 3);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, fdb_info.tnl_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tnl_info.vni is 0x" << fdb_info.tnl_info.vni << '\n'
        << std::setw(0) << std::dec;
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_L2_FWD_TX_TABLE); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    constexpr char BRIDGE_ID_KEY[] = "user_meta.pmeta.bridge_id";
    constexpr char DST_MAC_KEY[] = "dst_mac";

    const int MFID_BRIDGE_ID = GetMatchFieldId(BRIDGE_ID_KEY);
    EXPECT_NE(MFID_BRIDGE_ID, -1);

    const int MFID_DST_MAC = GetMatchFieldId(DST_MAC_KEY);
    EXPECT_NE(MFID_DST_MAC, -1);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      auto field_id = match.field_id();
      if (field_id == MFID_DST_MAC) {
        CheckMacAddr(match);
      } else if (field_id == MFID_BRIDGE_ID) {
        CheckBridgeId(match);
      } else {
        FAIL() << "Unexpected field_id (" << field_id << ")";
      }
    }
  }

  void CheckMacAddr(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i], fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckBridgeId(const ::p4::v1::FieldMatch& match) const {
    constexpr int BRIDGE_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);

    // widen so values will be treated as ints
    ASSERT_EQ(uint32_t(match_value[0]), uint32_t(fdb_info.bridge_id));
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  //----------------------------
  // Protected member data
  //----------------------------

  struct mac_learning_info fdb_info = {0};
  DiagDetail detail;
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(FdbTxVxlanEntryTest, remove_v4_tagged_entry) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(SET_VXLAN_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, fdb_info, p4info,
                                       REMOVE_ENTRY, detail);

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(FdbTxVxlanEntryTest, insert_v4_tagged_entry) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeTagged(SET_VXLAN_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, fdb_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(FdbTxVxlanEntryTest, insert_v4_untagged_entry) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV4NativeUntagged(POP_VLAN_SET_VXLAN_UNDERLAY_V4);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, fdb_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(FdbTxVxlanEntryTest, insert_v6_tagged_entry) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV6NativeTagged(SET_VXLAN_UNDERLAY_V6);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, fdb_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(FdbTxVxlanEntryTest, insert_v6_untagged_entry) {
  // Arrange
  InitLearnInfo(OVS_TUNNEL_VXLAN);
  InitV6NativeUntagged(POP_VLAN_SET_VXLAN_UNDERLAY_V6);

  // Act
  PrepareFdbTableEntryforV4VxlanTunnel(&table_entry, fdb_info, p4info,
                                       INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
