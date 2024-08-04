// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbSmacTableEntry()

#include <arpa/inet.h>
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

class FdbSmacEntryTest : public ::testing::Test {
 protected:
  FdbSmacEntryTest() {}

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable("l2_fwd_smac_table"); }

  //----------------------------
  // P4Info lookup methods
  //----------------------------

  int GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
    return -1;
  }

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    std::cerr << "Match Field '" << mf_name << "' not found!\n";
    return -1;
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
    constexpr uint8_t BRIDGE_ID = 42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
  }

  //----------------------------
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();
    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), GetActionId("NoAction"));
  }

  void CheckBridgeId(const ::p4::v1::FieldMatch& match) const {
    constexpr int BRIDGE_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);

    // widen so values will be treated as ints
    ASSERT_EQ(uint32_t(match_value[0]), uint32_t(fdb_info.bridge_id));
  }

  void CheckDetail() const {
    EXPECT_EQ(detail.table_id, LOG_L2_FWD_SMAC_TABLE);
  }

  void CheckMacAddr(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i] & 0xFF, fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckMatches() const {
    constexpr int MFID_MAC_ADDR = 1;
    constexpr int MFID_BRIDGE_ID = 2;

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      auto field_id = match.field_id();
      if (field_id == MFID_MAC_ADDR) {
        CheckMacAddr(match);
      } else if (field_id == MFID_BRIDGE_ID) {
        CheckBridgeId(match);
      } else {
        FAIL() << "Unexpected field_id (" << field_id << ")";
      }
    }
  }

  void CheckNoAction() const {
    // Table entry does not specify an action
    EXPECT_FALSE(table_entry.has_action());
  }

  void CheckTableEntry() const {
    // Table is defined (sanity check)
    ASSERT_FALSE(TABLE == nullptr);

    // Table ID is what we expect
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
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

 private:
  //----------------------------
  // Private member data
  //----------------------------

  const ::p4::config::v1::Table* TABLE = nullptr;
};

//----------------------------------------------------------------------
// PrepareFdbSmacTableEntry
//----------------------------------------------------------------------

TEST_F(FdbSmacEntryTest, remove_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareFdbSmacTableEntry(&table_entry, fdb_info, p4info, REMOVE_ENTRY,
                           detail);

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(FdbSmacEntryTest, insert_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareFdbSmacTableEntry(&table_entry, fdb_info, p4info, INSERT_ENTRY,
                           detail);

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
