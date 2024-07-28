// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <iostream>

#include "es2k/p4_name_mapping.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "table_entry_test.h"

namespace ovsp4rt {

constexpr uint32_t TABLE_ID = 46342225U;
constexpr int MAC_ADDR_SIZE = 6;
constexpr int BRIDGE_ID_SIZE = 1;

class PrepareFdbSmacEntryTest : public ::testing::Test { 
 protected:
  // Test case variables
  struct mac_learning_info fdb_info = {0};
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
};

TEST_F(PrepareFdbSmacEntryTest, remove_table_entry_is_correct) {
  constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  constexpr uint8_t BRIDGE_ID = 42;
  constexpr bool insert_entry = false;

  // Arrange

  // Act
  PrepareFdbSmacTableEntry(&table_entry, fdb_info, p4info, insert_entry,
                           detail);

  // Assert
  EXPECT_EQ(detail.table_id, LOG_L2_FWD_SMAC_TABLE);
  EXPECT_EQ(table_entry.table_id(), TABLE_ID);

  // number of match fields
  ASSERT_EQ(table_entry.match_size(), 2);

  {
    // match field 1: sa
    auto match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), 1);
    ASSERT_TRUE(match.has_exact());

    auto match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(uint8_t(match_value[i]), MAC_ADDR[i]);
    }
  }

  {
    // match field 2: bridge_id
    auto match = table_entry.match()[1];
    ASSERT_EQ(match.field_id(), 2);
    ASSERT_TRUE(match.has_exact());

    auto match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);
    ASSERT_EQ(uint8_t(match_value[0]), BRIDGE_ID);
  }
}

} // namespace ovsp4rt
