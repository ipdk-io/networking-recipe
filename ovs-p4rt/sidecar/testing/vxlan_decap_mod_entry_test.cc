// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareVxlanDecapModTableEntry().

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class VxlanDecapModEntryTest : public BaseTableTest {
 protected:
  VxlanDecapModEntryTest() {}

  void SetUp() { helper.SelectTable("vxlan_decap_mod_table"); }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitAction() { helper.SelectAction("vxlan_decap_outer_hdr"); }

  void InitTunnelInfo() { tunnel_info.vni = 0x1776; }

  //----------------------------
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    EXPECT_EQ(action.action_id(), helper.action_id());

    EXPECT_EQ(action.params_size(), 0);
  }

  void CheckMatches() const {
    constexpr char MOD_BLOB_PTR[] = "vmeta.common.mod_blob_ptr";
    const int MF_MOD_BLOB_PTR = helper.GetMatchFieldId(MOD_BLOB_PTR);

    ASSERT_EQ(table_entry.match_size(), 1);

    auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);

    CheckVniMatch(match);
  }

  void CheckNoAction() const { EXPECT_FALSE(table_entry.has_action()); }

  void CheckTableEntry() const {
    ASSERT_TRUE(helper.has_table());
    EXPECT_EQ(table_entry.table_id(), helper.table_id());
  }

  void CheckVniMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int VNI_SIZE = 3;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    ASSERT_EQ(match_value.size(), VNI_SIZE);

    uint32_t vni_value = DecodeVniValue(match_value);
    EXPECT_EQ(vni_value, tunnel_info.vni);
  }

  //----------------------------
  // Protected member data
  //----------------------------

  struct tunnel_info tunnel_info = {0};
};

//----------------------------------------------------------------------
// PrepareVxlanDecapModTableEntry()
//----------------------------------------------------------------------

TEST_F(VxlanDecapModEntryTest, remove_entry) {
  // Arrange
  InitTunnelInfo();

  // Act
  PrepareVxlanDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                 REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(VxlanDecapModEntryTest, insert_entry) {
  // Arrange
  InitTunnelInfo();
  InitAction();

  // Act
  PrepareVxlanDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                 INSERT_ENTRY);

  // Assert
  CheckAction();
}

#ifdef WIDE_VNI_VALUES

TEST_F(VxlanDecapModEntryTest, insert_entry_with_20_bit_vni) {
  // Arrange
  InitTunnelInfo();
  InitAction();
  tunnel_info.vni = 0x87124;  // 20-bit value

  // Act
  PrepareVxlanDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                 INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckAction();
}

TEST_F(VxlanDecapModEntryTest, insert_entry_with_24_bit_vni) {
  // Arrange
  InitTunnelInfo();
  InitAction();
  tunnel_info.vni = 0x871244;  // 24-bit value

  // Act
  PrepareVxlanDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                 INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckAction();
}

#endif  // WIDE_VNI_VALUES

}  // namespace ovsp4rt
