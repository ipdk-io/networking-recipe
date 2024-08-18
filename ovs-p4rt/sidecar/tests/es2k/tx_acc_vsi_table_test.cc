// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareTxAccVsiTableEntry()

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

// For ES2K, when we program VSI ID as part of action, we need to add
// an OFFSET of 16 for the VSI ID.
constexpr uint32_t ES2K_VPORT_ID_OFFSET = 16;

class TxAccVsiTableTest : public BaseTableTest {
 protected:
  TxAccVsiTableTest() {}

  void SetUp() { SelectTable("tx_acc_vsi"); }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatch()
  //----------------------------

  void CheckMatch() const {
    EXPECT_EQ(table_entry.match_size(), 1);
    const auto& match = table_entry.match()[0];

    const int MF_VSI = GetMatchFieldId("vmeta.common.vsi");
    EXPECT_NE(MF_VSI, -1);

    EXPECT_EQ(match.field_id(), MF_VSI);
    CheckVsiMatch(match);
  }

  void CheckVsiMatch(const ::p4::v1::FieldMatch& match) const {
    // Note: bit<11> vsi is encoded as bit<8>.
    constexpr int VSI_SIZE = 1;

    ASSERT_TRUE(match.has_exact());

    const auto& match_value = match.exact().value();
    EXPECT_EQ(match_value.size(), VSI_SIZE);

    auto vsi_value = DecodeWordValue(match_value);
    EXPECT_EQ(vsi_value, info_sp - ES2K_VPORT_ID_OFFSET);
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

  uint32_t info_sp = 0;  // bit<11>
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(TxAccVsiTableTest, sp_8_bits) {
  // Arrange
  info_sp = 42;

  // Act
  PrepareTxAccVsiTableEntry(&table_entry, info_sp, p4info);

  // Assert
  CheckTableEntry();
  CheckMatch();
  CheckNoAction();
}

#if 0

TEST_F(TxAccVsiTableTest, sp_11_bits) {
  // Arrange
  info_sp = 0x765;

  // Act
  PrepareTxAccVsiTableEntry(&table_entry, info_sp, p4info);

  // Assert
  CheckTableEntry();
  CheckMatch();
  CheckNoAction();
}

#endif

}  // namespace ovsp4rt
