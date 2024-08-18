// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareL2ToTunnelV4().

#define DUMP_JSON

#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class L2ToV4TunnelTest : public BaseTableTest {
 protected:
  L2ToV4TunnelTest() {}

  void SetUp() { SelectTable("l2_to_tunnel_v4"); }

  //----------------------------
  // Initialization
  //----------------------------

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));
  }

  void InitTunnelInfo() {
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    fdb_info.tnl_info.remote_ip.family = AF_INET;
    fdb_info.tnl_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

    SelectAction("set_tunnel_v4");
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    // Table entry specifies an action
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    // Action ID is what we expect
    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    // Only one parameter
    ASSERT_EQ(action.params_size(), 1);

    // Param ID is what we expect
    const auto& param = action.params()[0];
    ASSERT_EQ(param.param_id(), GetParamId("dst_addr"));

    // Value has 4 bytes
    const auto& param_value = param.value();
    ASSERT_EQ(param_value.size(), 4);

    // Value is what we expect
    auto word_value = ntohl(DecodeWordValue(param_value));
    ASSERT_EQ(word_value, fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr);
  }

  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const {
    // LogTableId is correct for this table
    EXPECT_EQ(detail.table_id, LOG_L2_TO_TUNNEL_V4_TABLE);
  }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    constexpr char V4_KEY_DA[] = "hdrs.mac[vmeta.common.depth].da";
    const int MFID_DA = GetMatchFieldId(V4_KEY_DA);
    ASSERT_NE(MFID_DA, -1);

    // Exactly one match field
    ASSERT_EQ(table_entry.match_size(), 1);

    // Match Field ID is what we expect
    const auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MFID_DA);

    // Value is what we expect
    CheckMacAddrMatch(match);
  }

  void CheckMacAddrMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    // This is an exact-match field
    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    // Value is correct for a mac address
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    // Value is what we expect
    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i] & 0xFF, fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { EXPECT_FALSE(table_entry.has_action()); }

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
// Test cases
//----------------------------------------------------------------------
TEST_F(L2ToV4TunnelTest, remove_entry) {
  // Arrange
  InitFdbInfo();
  InitTunnelInfo();

  // Act
  PrepareL2ToTunnelV4(&table_entry, fdb_info, p4info, REMOVE_ENTRY, detail);
  DumpTableEntry();

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(L2ToV4TunnelTest, insert_entry) {
  // Arrange
  InitFdbInfo();
  InitTunnelInfo();

  // Act
  PrepareL2ToTunnelV4(&table_entry, fdb_info, p4info, INSERT_ENTRY, detail);
  DumpTableEntry();

  // Assert
  // We've already checked Detail, TableEntry, and Matches.
  CheckAction();
}

}  // namespace ovsp4rt
