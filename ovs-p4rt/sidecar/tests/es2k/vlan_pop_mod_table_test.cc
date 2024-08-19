// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareVlanPopTableEntry()

#include <stdint.h>

#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"

namespace ovsp4rt {

class VlanPopTableTest : public BaseTableTest {
 protected:
  VlanPopTableTest() {}

  void SetUp() { SelectTable("vlan_pop_mod_table"); }

  //----------------------------
  // Initialization
  //----------------------------

  void InitAction() { SelectAction("vlan_pop"); }

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
    constexpr char MOD_BLOB_PTR[] = "vmeta.common.mod_blob_ptr";
    const int MF_MOD_BLOB_PTR = GetMatchFieldId(MOD_BLOB_PTR);

    ASSERT_NE(MF_MOD_BLOB_PTR, -1);

    ASSERT_EQ(table_entry.match_size(), 1);
    const auto& match = table_entry.match()[0];

    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);
    CheckVlanIdMatch(match);
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  void CheckVlanIdMatch(const ::p4::v1::FieldMatch& match) const {
    // TODO(derek): vlan_id (nominally bit<12>) is encoded as bit<8>.
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

  uint16_t vlan_id;
};

//----------------------------------------------------------------------
// Test cases
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
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
