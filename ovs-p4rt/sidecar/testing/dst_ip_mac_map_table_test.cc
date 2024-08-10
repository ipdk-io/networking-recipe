// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareDstIpMacMapTableEntry()

//#define DUMP_JSON

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

class DstIpMacMapTableTest : public BaseTableTest {
 protected:
  DstIpMacMapTableTest() {}

  void SetUp() { SelectTable("vm_dst_ip4_mac_map_table"); }

  void InitAction() { SelectAction("vm_dst_ip4_mac_map_action"); }

  //----------------------------
  // InitMapInfo()
  //----------------------------

  void InitMapInfo() {
    constexpr uint8_t DST_MAC[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr char IPV4_DST_ADDR[] = "10.20.30.40";
    constexpr int IPV4_PREFIX_LEN = 24;

    memcpy(map_info.dst_mac_addr, DST_MAC, sizeof(map_info.dst_mac_addr));

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &map_info.dst_ip_addr.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    map_info.dst_ip_addr.family = AF_INET;
    map_info.dst_ip_addr.prefix_len = IPV4_PREFIX_LEN;
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const int DMAC_HIGH = GetParamId("dmac_high");
    ASSERT_NE(DMAC_HIGH, -1);

    const int DMAC_MID = GetParamId("dmac_mid");
    ASSERT_NE(DMAC_MID, -1);

    const int DMAC_LOW = GetParamId("dmac_low");
    ASSERT_NE(DMAC_LOW, -1);

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 3);

    for (const auto& param : params) {
      auto param_id = param.param_id();
      const auto& param_value = param.value();
      if (param_id == DMAC_HIGH) {
        ASSERT_EQ(param_value.size(), 2);
        CheckMacByte(param_value[0] & 0xFF, 0);
        CheckMacByte(param_value[1] & 0xFF, 1);
      } else if (param_id == DMAC_MID) {
        ASSERT_EQ(param_value.size(), 2);
        CheckMacByte(param_value[0], 2);
        CheckMacByte(param_value[1], 3);
      } else if (param_id == DMAC_LOW) {
        ASSERT_EQ(param_value.size(), 2);
        CheckMacByte(param_value[0], 4);
        CheckMacByte(param_value[1], 5);
      } else {
        FAIL() << "Unexpected param_id (" << param_id << ")";
      }
    }
  }

  void CheckMacByte(uint16_t actual, int index) const {
    EXPECT_EQ(actual, uint16_t(map_info.dst_mac_addr[index]))
        << "dst_mac_addr[" << index << "] does not match";
  }

  void CheckMacByte(const std::string& param_value, int actual_index,
                    int expected_index) {
    ASSERT_EQ(param_value.size(), 2);
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const {
    EXPECT_EQ(detail.table_id, LOG_DST_IP_MAC_MAP_TABLE);
  }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    const int MFID_IPV4_DST = helper.GetMatchFieldId("ipv4_dst");
    ASSERT_NE(MFID_IPV4_DST, -1);

    ASSERT_EQ(table_entry.match_size(), 1);

    const auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MFID_IPV4_DST);
    CheckIpAddrMatch(match);
  }

  void CheckIpAddrMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int IPV4_ADDR_SIZE = 4;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    EXPECT_EQ(match_value.size(), IPV4_ADDR_SIZE);

    auto addr_value = ntohl(DecodeWordValue(match_value));
    ASSERT_EQ(addr_value, map_info.dst_ip_addr.ip.v4addr.s_addr);
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

  struct ip_mac_map_info map_info = {0};
  DiagDetail detail;
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(DstIpMacMapTableTest, remove_entry) {
  // Arrange
  InitMapInfo();

  // Act
  PrepareDstIpMacMapTableEntry(&table_entry, map_info, p4info, REMOVE_ENTRY,
                               detail);
  DumpTableEntry();

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(DstIpMacMapTableTest, insert_entry) {
  // Arrange
  InitMapInfo();
  InitAction();

  // Act
  PrepareDstIpMacMapTableEntry(&table_entry, map_info, p4info, INSERT_ENTRY,
                               detail);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
