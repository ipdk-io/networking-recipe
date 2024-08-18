// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbSmacTableEntry()

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class FdbSmacEntryTest : public BaseTableTest {
 protected:
  FdbSmacEntryTest() {}

  void SetUp() { SelectTable("l2_fwd_smac_table"); }

  //----------------------------
  // Initialization
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

  void CheckBridgeIdMatch(const ::p4::v1::FieldMatch& match) const {
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

  void CheckMacAddrMatch(const ::p4::v1::FieldMatch& match) const {
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
    constexpr char SMAC_KEY[] = "hdrs.mac[vmeta.common.depth].sa";
    const int MFID_SMAC = GetMatchFieldId(SMAC_KEY);
    ASSERT_NE(MFID_SMAC, -1);

    constexpr char BRIDGE_ID_KEY[] = "user_meta.pmeta.bridge_id";
    const int MFID_BRIDGE_ID = GetMatchFieldId(BRIDGE_ID_KEY);
    ASSERT_NE(MFID_BRIDGE_ID, -1);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      auto field_id = match.field_id();
      if (field_id == MFID_SMAC) {
        CheckMacAddrMatch(match);
      } else if (field_id == MFID_BRIDGE_ID) {
        CheckBridgeIdMatch(match);
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
