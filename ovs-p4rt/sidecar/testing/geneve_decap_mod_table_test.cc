// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareGeneveDecapModTableEntry()

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class GeneveDecapModTableTest : public BaseTableTest {
 protected:
  GeneveDecapModTableTest() {}

  void SetUp() { SelectTable("geneve_decap_mod_table"); }

  void InitAction() { SelectAction("geneve_decap_outer_hdr"); }

  void InitTunnelInfo() {
    tunnel_info.vni = 0x1234;
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), helper.action_id());

    // Action has no parameters.
    EXPECT_EQ(action.params_size(), 0);
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    constexpr char MOD_BLOB_PTR[] = "vmeta.common.mod_blob_ptr";
    const int MF_MOD_BLOB_PTR = helper.GetMatchFieldId(MOD_BLOB_PTR);
    ASSERT_NE(MF_MOD_BLOB_PTR, -1);

    ASSERT_EQ(table_entry.match_size(), 1);

    const auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);

    CheckVniMatch(match);
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
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  //----------------------------
  // Protected member data
  //----------------------------

  struct tunnel_info tunnel_info = {0};
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(GeneveDecapModTableTest, remove_entry) {
  // Arrange
  InitTunnelInfo();

  // Act
  PrepareGeneveDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                  REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(GeneveDecapModTableTest, insert_entry) {
  // Arrange
  InitTunnelInfo();
  InitAction();

  // Act
  PrepareGeneveDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                  INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
