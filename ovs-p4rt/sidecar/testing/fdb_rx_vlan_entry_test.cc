// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbRxVlanTableEntry()

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class FdbRxVlanEntryTest : public BaseTableTest {
 protected:
  FdbRxVlanEntryTest() {}

  void SetUp() { SelectTable("l2_fwd_rx_table"); }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitAction() { SelectAction("l2_fwd"); }

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 99;
    constexpr uint32_t SRC_PORT = 0x42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.bridge_id = BRIDGE_ID;
    fdb_info.rx_src_port = SRC_PORT;
  }

  //----------------------------
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    const int PORT_PARAM = GetParamId("port");

    ASSERT_NE(ActionId(), -1);

    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), PORT_PARAM);
    uint32_t port = DecodeWordValue(param.value());
    EXPECT_EQ(port, fdb_info.rx_src_port);
  }

  void CheckBridgeId(const ::p4::v1::FieldMatch& match) const {
    constexpr int BRIDGE_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);

    // widen so values will be treated as ints
    ASSERT_EQ(uint32_t(match_value[0]), uint32_t(fdb_info.bridge_id));
  }

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_L2_FWD_RX_TABLE); }

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
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  //----------------------------
  // Protected member data
  //----------------------------

  // Working variables
  struct mac_learning_info fdb_info = {0};
  DiagDetail detail;
};

//----------------------------------------------------------------------
// PrepareFdbRxVlanTableEntry()
//----------------------------------------------------------------------

TEST_F(FdbRxVlanEntryTest, remove_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareFdbRxVlanTableEntry(&table_entry, fdb_info, p4info, REMOVE_ENTRY,
                             detail);
  DumpTableEntry();

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
}

TEST_F(FdbRxVlanEntryTest, insert_entry) {
  // Arrange
  InitFdbInfo();
  InitAction();

  // Act
  PrepareFdbRxVlanTableEntry(&table_entry, fdb_info, p4info, INSERT_ENTRY,
                             detail);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
