// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#define DUMP_JSON

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

constexpr int TUNNEL_TYPE_VXLAN = 2;

class DpdkTunnelTermTest : public BaseTableTest {
 protected:
  DpdkTunnelTermTest() {}

  void SetUp() { SelectTable("ipv4_tunnel_term_table"); }

  //----------------------------
  // InitTunnelInfo()
  //----------------------------

  void InitTunnelInfo() {
    constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_SRC_PREFIX_LEN = 8;
    constexpr int IPV4_DST_PREFIX_LEN = 24;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR,
                        &tunnel_info.local_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_SRC_ADDR;
    tunnel_info.local_ip.family = AF_INET;
    tunnel_info.local_ip.prefix_len = IPV4_SRC_PREFIX_LEN;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    tunnel_info.remote_ip.family = AF_INET;
    tunnel_info.remote_ip.prefix_len = IPV4_DST_PREFIX_LEN;
  }

  //----------------------------
  // InitAction()
  //----------------------------

  void InitAction() {
    // TODO(derek): tunnel_id truncated to 8 bits
    tunnel_info.vni = 66;

    SelectAction("decap_outer_ipv4");
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const int TUNNEL_ID = GetParamId("tunnel_id");
    ASSERT_NE(TUNNEL_ID, -1);

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    const auto& param = params[0];

    ASSERT_EQ(param.param_id(), TUNNEL_ID);
    CheckTunnelIdParam(param.value());
  }

  void CheckTunnelIdParam(const std::string& param_value) const {
    // TODO(derek): tunnel_id truncated to 8 bits.
    constexpr int TUNNEL_ID_SIZE = 1;
    EXPECT_EQ(param_value.size(), TUNNEL_ID_SIZE);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, tunnel_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tunnel_info.vni is 0x" << tunnel_info.vni << '\n'
        << std::setw(0) << std::dec;
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    const int MFID_IPV4_SRC = helper.GetMatchFieldId("ipv4_src");
    ASSERT_NE(MFID_IPV4_SRC, -1);

    const int MFID_IPV4_DST = helper.GetMatchFieldId("ipv4_dst");
    ASSERT_NE(MFID_IPV4_DST, -1);

    const int MFID_TUNNEL_TYPE = helper.GetMatchFieldId("tunnel_type");
    ASSERT_NE(MFID_TUNNEL_TYPE, -1);

    ASSERT_GE(table_entry.match_size(), 3);

    for (const auto& match : table_entry.match()) {
      int field_id = match.field_id();
      if (field_id == MFID_IPV4_SRC) {
        CheckIpv4AddrMatch(match, tunnel_info.remote_ip);
      } else if (field_id == MFID_IPV4_DST) {
        CheckIpv4AddrMatch(match, tunnel_info.local_ip);
      } else if (field_id == MFID_TUNNEL_TYPE) {
        CheckTunnelTypeMatch(match);
      }
    }
  }

  void CheckIpv4AddrMatch(const ::p4::v1::FieldMatch& match,
                          const struct p4_ipaddr& ipaddr) const {
    constexpr int IPV4_ADDR_SIZE = 4;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    EXPECT_EQ(match_value.size(), IPV4_ADDR_SIZE);

    auto addr_value = ntohl(DecodeWordValue(match_value));
    EXPECT_EQ(addr_value, ipaddr.ip.v4addr.s_addr);
  }

  void CheckTunnelTypeMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int TUNNEL_TYPE_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    EXPECT_EQ(match_value.size(), TUNNEL_TYPE_SIZE);

    const uint16_t tunnel_type = DecodeWordValue(match_value);
    EXPECT_EQ(tunnel_type, TUNNEL_TYPE_VXLAN);
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(helper.has_table());
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

TEST_F(DpdkTunnelTermTest, vxlan_remove_entry) {
  // Arrange
  InitTunnelInfo();

  // Act
  PrepareTunnelTermTableEntry(&table_entry, tunnel_info, p4info, REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(DpdkTunnelTermTest, vxlan_insert_entry) {
  // Arrange
  InitTunnelInfo();
  InitAction();

  // Act
  PrepareTunnelTermTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
