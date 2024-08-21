// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbTxVlanTableEntry().
// DPDK edition.

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class DpdkFdbTxVlanTest : public BaseTableTest {
 protected:
  DpdkFdbTxVlanTest() {}

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // Initialization
  //----------------------------

  void InitAction() { SelectAction("l2_fwd"); }

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    // Note: 8-bit vlan_id.
    constexpr uint32_t VLAN_ID = 42;

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
    fdb_info.vln_info.vlan_id = VLAN_ID;
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const int PORT_PARAM = GetParamId("port");
    ASSERT_NE(PORT_PARAM, -1);

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    const auto& param = params[0];

    ASSERT_EQ(param.param_id(), PORT_PARAM);
    CheckPortParam(param.value());
  }

  void CheckPortParam(const std::string& param_value) const {
    // TODO(derek): unusual value semantics. 
    // TODO(derek) port param is bit<32>. vlan_id is bit<12>.
    EXPECT_EQ(param_value.size(), 1);
    uint32_t port = DecodeWordValue(param_value);
    EXPECT_EQ(port, fdb_info.vln_info.vlan_id - 1);
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    const int MFID_DST_MAC = GetMatchFieldId("dst_mac");
    ASSERT_NE(MFID_DST_MAC, -1);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 1);
    const auto& match = table_entry.match()[0];

    ASSERT_EQ(match.field_id(), MFID_DST_MAC);
    CheckMacAddrMatch(match);
  }

  void CheckMacAddrMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i], fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_L2_FWD_TX_TABLE); }

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

  struct mac_learning_info fdb_info = {0};
  DiagDetail detail;
};

//----------------------------------------------------------------------
// PrepareFdbTxVlanTableEntry()
//----------------------------------------------------------------------

TEST_F(DpdkFdbTxVlanTest, remove_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareFdbTxVlanTableEntry(&table_entry, fdb_info, p4info, REMOVE_ENTRY,
                             detail);

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(DpdkFdbTxVlanTest, insert_entry) {
  // Arrange
  InitFdbInfo();
  InitAction();

  // Act
  PrepareFdbTxVlanTableEntry(&table_entry, fdb_info, p4info, INSERT_ENTRY,
                             detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
