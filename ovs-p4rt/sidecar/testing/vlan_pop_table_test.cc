// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareVlanPopTableEntry()

#include <stdint.h>

#include <string>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_helper.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class VlanPopTableTest : public ::testing::Test {
 protected:
  VlanPopTableTest() : helper(p4info) {}

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { helper.SelectTable("vlan_pop_mod_table"); }

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

  void InitAction() { helper.SelectAction("vlan_pop"); }

  void InitVlanInfo() {
    // TODO(derek): vlan_id is encoded as one byte (?)
    vlan_id = 0x86;
  }

  //----------------------------
  // Table-specific checks
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), helper.action_id());
  }

  void CheckMatches() const {
    constexpr char MOD_BLOB_PTR [] = "vmeta.common.mod_blob_ptr";
    const int MF_MOD_BLOB_PTR = helper.GetMatchFieldId(MOD_BLOB_PTR);

    ASSERT_NE(MF_MOD_BLOB_PTR, -1);

    ASSERT_EQ(table_entry.match_size(), 1);
    const auto& match = table_entry.match()[0];

    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);
    CheckVlanIdMatch(match);
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  void CheckTableEntry() const {
    ASSERT_TRUE(helper.has_table());
    EXPECT_EQ(table_entry.table_id(), helper.table_id());
  }

  void CheckVlanIdMatch(const ::p4::v1::FieldMatch& match) const {
    // TODO(derek): vlan_id (nominally bit<12>) is encoded as bit<8>.
    // What gives?
    constexpr int VLAN_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    EXPECT_EQ(match_value.size(), VLAN_ID_SIZE);

    auto vlan_value = DecodeWordValue(match_value);
    ASSERT_EQ(vlan_value, vlan_id);
  }

  //----------------------------
  // Protected member data
  //----------------------------

  ::p4::v1::TableEntry table_entry;
  P4InfoHelper helper;
  uint16_t vlan_id;
};

//----------------------------------------------------------------------
// PrepareSampleTableEntry()
//----------------------------------------------------------------------

TEST_F(VlanPopTableTest, remove_entry) {
  // Arrange
  InitVlanInfo();

  // Act
  PrepareVlanPopTableEntry(&table_entry, vlan_id, p4info, REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(VlanPopTableTest, insert_entry) {
  // Arrange
  InitVlanInfo();
  InitAction();

  // Act
  PrepareVlanPopTableEntry(&table_entry, vlan_id, p4info, INSERT_ENTRY);

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
