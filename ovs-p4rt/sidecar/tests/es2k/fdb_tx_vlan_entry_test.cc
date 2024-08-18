// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbTxVlanTableEntry()
//
// TODO(derek): port and vlan_ptr parameter values are truncated to
// 8 bits. Need to fix or document why this is correct.

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class FdbTxVlanEntryTest : public BaseTableTest {
 protected:
  FdbTxVlanEntryTest() {}

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // Initialization
  //----------------------------

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 86;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
  }

  void InitTagged() {
#if 0
    constexpr uint32_t SRC_PORT = 0x1776;  // bit<16>
#else
    // TODO(derek): 8-bit src_port
    constexpr uint32_t SRC_PORT = 0x76;  // bit<8>
#endif

    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    fdb_info.src_port = SRC_PORT;

    SelectAction("l2_fwd");
  }

  void InitUntagged() {
#if 0
    constexpr uint32_t SRC_PORT = 0x1984;  // bit<16>
    constexpr uint32_t VLAN_PTR = 0x1066;  // bit<8>
#else
    // TODO(derek): 8-bit src_port and vlan_ptr
    constexpr uint32_t SRC_PORT = 0x84;  // bit<8>
    constexpr uint32_t VLAN_PTR = 0x66;  // bit<8>
#endif

    fdb_info.vlan_info.port_vlan = VLAN_PTR;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    fdb_info.src_port = SRC_PORT;

    SelectAction("remove_vlan_and_fwd");
  }

  //----------------------------
  // CheckTaggedAction()
  //----------------------------

  void CheckTaggedAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const int PORT_PARAM = GetParamId("port");
    ASSERT_NE(PORT_PARAM, -1);

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    const auto& param = params[0];
    ASSERT_EQ(param.param_id(), PORT_PARAM);
    uint32_t port = DecodeWordValue(param.value());
    EXPECT_EQ(port, fdb_info.src_port);
  }

  //----------------------------
  // CheckUntaggedAction()
  //----------------------------

  void CheckUntaggedAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 2);

    const int PORT_PARAM = GetParamId("port_id");
    ASSERT_NE(PORT_PARAM, -1);

    const int VLAN_PARAM = GetParamId("vlan_ptr");
    ASSERT_NE(VLAN_PARAM, -1);

    for (const auto& param : params) {
      int param_id = param.param_id();
      if (param_id == PORT_PARAM) {
        uint32_t port_id = DecodeWordValue(param.value());
        EXPECT_EQ(port_id, fdb_info.src_port);
      } else if (param_id == VLAN_PARAM) {
        uint32_t port_vlan = DecodeWordValue(param.value());
        EXPECT_EQ(port_vlan, fdb_info.vlan_info.port_vlan);
      } else {
        FAIL() << "Unexpected param_id (" << param_id << ")";
      }
    }
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    constexpr char BRIDGE_ID_KEY[] = "user_meta.pmeta.bridge_id";
    constexpr char DST_MAC_KEY[] = "dst_mac";

    const int MFID_BRIDGE_ID = GetMatchFieldId(BRIDGE_ID_KEY);
    const int MFID_DST_MAC = GetMatchFieldId(DST_MAC_KEY);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      auto field_id = match.field_id();
      if (field_id == MFID_DST_MAC) {
        CheckMacAddrMatch(match);
      } else if (field_id == MFID_BRIDGE_ID) {
        CheckBridgeIdMatch(match);
      } else {
        FAIL() << "Unexpected field_id (" << field_id << ")";
      }
    }
  }

  void CheckMacAddrMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i], fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckBridgeIdMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int BRIDGE_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);

    // widen so values will be treated as ints
    ASSERT_EQ(uint32_t(match_value[0]), uint32_t(fdb_info.bridge_id));
  }

  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_L2_FWD_TX_TABLE); }

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
// PrepareFdbTxVlanTableEntry()
//----------------------------------------------------------------------

TEST_F(FdbTxVlanEntryTest, remove_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareFdbTxVlanTableEntry(&table_entry, fdb_info, p4info, REMOVE_ENTRY,
                             detail);

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(FdbTxVlanEntryTest, insert_untagged_entry) {
  // Arrange
  InitFdbInfo();
  InitUntagged();

  // Act
  PrepareFdbTxVlanTableEntry(&table_entry, fdb_info, p4info, INSERT_ENTRY,
                             detail);

  // Assert
  CheckTableEntry();
  CheckUntaggedAction();
}

TEST_F(FdbTxVlanEntryTest, insert_tagged_entry) {
  // Arrange
  InitFdbInfo();
  InitTagged();

  // Act
  PrepareFdbTxVlanTableEntry(&table_entry, fdb_info, p4info, INSERT_ENTRY,
                             detail);

  // Assert
  CheckTableEntry();
  CheckTaggedAction();
}

}  // namespace ovsp4rt
