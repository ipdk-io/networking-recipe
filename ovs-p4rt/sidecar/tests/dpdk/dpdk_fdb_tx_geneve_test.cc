// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareFdbTableEntryforV4GeneveTunnel().
// DPDK version.

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class DpdkFdbTxGeneveTest : public BaseTableTest {
 protected:
  DpdkFdbTxGeneveTest() {}

  void SetUp() { SelectTable("l2_fwd_tx_table"); }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitFdbInfo(uint8_t tunnel_type) {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(MAC_ADDR));
    fdb_info.tnl_info.tunnel_type = tunnel_type;
  }

  void InitAction() {
    constexpr char IPV4_DST_ADDR[] = "10.20.30.40";
    constexpr int IPV4_PREFIX_LEN = 8;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    fdb_info.tnl_info.remote_ip.family = AF_INET;
    fdb_info.tnl_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

    // TODO(derek): 8-bit VNI (tunnel_id)
    fdb_info.tnl_info.vni = 86;

    SelectAction("set_tunnel");
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 2);

    const int TUNNEL_ID_PARAM = GetParamId("tunnel_id");
    ASSERT_NE(TUNNEL_ID_PARAM, -1);

    const int DST_ADDR_PARAM = GetParamId("dst_addr");
    ASSERT_NE(DST_ADDR_PARAM, -1);

    for (const auto& param : params) {
      const auto& param_value = param.value();
      int param_id = param.param_id();
      if (param_id == TUNNEL_ID_PARAM) {
        CheckTunnelIdParam(param_value);
      } else if (param_id == DST_ADDR_PARAM) {
        CheckDstAddrParam(param_value);
      } else {
        FAIL() << "Unexpected param_id (" << param_id << ")";
      }
    }
  }

  void CheckDstAddrParam(const std::string& param_value) const {
    constexpr int DST_ADDR_SIZE = 4;
    ASSERT_EQ(param_value.size(), DST_ADDR_SIZE);

    auto word_value = ntohl(DecodeWordValue(param_value));
    ASSERT_EQ(word_value, fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr);
  }

  void CheckTunnelIdParam(const std::string& param_value) const {
    // TODO(derek): 8-bit value for 24-bit action parameter.
    EXPECT_EQ(param_value.size(), 1);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, fdb_info.tnl_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tnl_info.vni is 0x" << fdb_info.tnl_info.vni << '\n'
        << std::dec;
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_L2_FWD_TX_TABLE); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    const int MFID_DST_MAC = GetMatchFieldId("dst_mac");
    EXPECT_NE(MFID_DST_MAC, -1);

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

TEST_F(DpdkFdbTxGeneveTest, remove_entry) {
  // Arrange
  InitFdbInfo(OVS_TUNNEL_GENEVE);

  // Act
  PrepareFdbTableEntryforV4GeneveTunnel(&table_entry, fdb_info, p4info,
                                        REMOVE_ENTRY, detail);

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(DpdkFdbTxGeneveTest, insert_entry) {
  // Arrange
  InitFdbInfo(OVS_TUNNEL_GENEVE);
  InitAction();

  // Act
  PrepareFdbTableEntryforV4GeneveTunnel(&table_entry, fdb_info, p4info,
                                        INSERT_ENTRY, detail);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
