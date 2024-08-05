// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbTxVlanTableEntry()
//
// TODO(derek): port and vlan_ptr parameter values are truncated to
// 8 bits. Need to fix or document why this is correct.

#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class FdbTxVlanEntryTest : public ::testing::Test {
 protected:
  FdbTxVlanEntryTest() {}

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // P4Info lookup methods
  //----------------------------

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    std::cerr << "Match Field '" << mf_name << "' not found!\n";
    return -1;
  }

  int GetParamId(const std::string& param_name) const {
    for (const auto& param : ACTION->params()) {
      if (param.name() == param_name) {
        return param.id();
      }
    }
    std::cerr << "Action Param '" << param_name << "' not found!\n";
    return -1;
  }

  void SelectAction(const std::string& action_name) {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        ACTION = &action;
        ACTION_ID = pre.id();
        return;
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
  }

  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
    std::cerr << "Table '" << table_name << "' not found!\n";
  }

  //----------------------------
  // Utility methods
  //----------------------------

  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 86;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
  }

  void InitTagged() {
#if 0
    constexpr uint32_t SRC_PORT = 0x1776;  // bit<32> -> bit<8>
#else
    constexpr uint32_t SRC_PORT = 0x76;  // bit<32> -> bit<8>
#endif

    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    fdb_info.src_port = SRC_PORT;

    SelectAction("l2_fwd");
  }

  void InitUntagged() {
#if 0
    constexpr uint32_t SRC_PORT = 0x1984;  // bit<32> -> bit<8>
    constexpr uint32_t VLAN_PTR = 0x1066;  // bit<24> -> bit<8>
#else
    constexpr uint32_t SRC_PORT = 0x84;  // bit<8>
    constexpr uint32_t VLAN_PTR = 0x66;  // bit<8>
#endif

    fdb_info.vlan_info.port_vlan = VLAN_PTR;
    fdb_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    fdb_info.src_port = SRC_PORT;

    SelectAction("remove_vlan_and_fwd");
  }

  //----------------------------
  // Test-specific methods
  //----------------------------

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

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_L2_FWD_TX_TABLE); }

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
        CheckMacAddr(match);
      } else if (field_id == MFID_BRIDGE_ID) {
        CheckBridgeId(match);
      } else {
        FAIL() << "Unexpected field_id (" << field_id << ")";
      }
    }
  }

  void CheckTableEntry() const {
    ASSERT_FALSE(TABLE == nullptr);
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
  }

  void CheckTaggedAction() const {
    const int PORT_PARAM = GetParamId("port");

    ASSERT_NE(ACTION_ID, -1);

    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ACTION_ID);

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), PORT_PARAM);
    uint32_t port = DecodeWordValue(param.value());
    EXPECT_EQ(port, fdb_info.src_port);
  }

  void CheckUntaggedAction() const {
    const int PORT_PARAM = GetParamId("port_id");
    const int VLAN_PARAM = GetParamId("vlan_ptr");

    ASSERT_NE(ACTION_ID, -1);

    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ACTION_ID);

    auto& params = action.params();
    ASSERT_EQ(action.params_size(), 2);

    {
      auto& param = params[0];
      ASSERT_EQ(param.param_id(), PORT_PARAM);
      uint32_t port_id = DecodeWordValue(param.value());
      EXPECT_EQ(port_id, fdb_info.src_port);
    }

    {
      auto& param = params[1];
      ASSERT_EQ(param.param_id(), VLAN_PARAM);
      uint32_t port_vlan = DecodeWordValue(param.value());
      EXPECT_EQ(port_vlan, fdb_info.vlan_info.port_vlan);
    }
  }

  //----------------------------
  // Protected member data
  //----------------------------

  // Working variables
  struct mac_learning_info fdb_info = {0};
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  // Comparison variables
  uint32_t TABLE_ID;
  int ACTION_ID = -1;

 private:
  //----------------------------
  // Private member data
  //----------------------------

  const ::p4::config::v1::Table* TABLE = nullptr;
  const ::p4::config::v1::Action* ACTION = nullptr;
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
}

TEST_F(FdbTxVlanEntryTest, insert_untagged_entry) {
  // Arrange
  InitFdbInfo();
  InitUntagged();

  // Act
  PrepareFdbTxVlanTableEntry(&table_entry, fdb_info, p4info, INSERT_ENTRY,
                             detail);

  // Assert
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
  CheckTaggedAction();
}

}  // namespace ovsp4rt
