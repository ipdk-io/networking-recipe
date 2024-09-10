// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareL2ToTunnelV6().

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class L2ToV6TunnelTest : public BaseTableTest {
 protected:
  L2ToV6TunnelTest() {}

  void SetUp() { SelectTable("l2_to_tunnel_v6"); }

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    constexpr char IPV6_ADDR[] = "face:deaf:feed:aced:cede:dead:beef:cafe";

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));

    p4_ipaddr& remote_ip = fdb_info.tnl_info.remote_ip;
    EXPECT_EQ(inet_pton(AF_INET6, IPV6_ADDR,
                        &remote_ip.ip.v6addr.__in6_u.__u6_addr16),
              1);
    fdb_info.tnl_info.remote_ip.family = AF_INET6;
    fdb_info.tnl_info.remote_ip.prefix_len = 32;

    SelectAction("set_tunnel_v6");
  }

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    ASSERT_EQ(action.params_size(), 4);

    const struct p4_ipaddr& remote_ip = fdb_info.tnl_info.remote_ip;

    // TODO(derek): look up param IDs by name.
    // TODO(derek): Do not assume that params are in order.
    for (int i = 0; i < action.params_size(); i++) {
      const auto& param = action.params()[i];
      ASSERT_EQ(param.param_id(), i + 1);

      const auto& param_value = param.value();
      ASSERT_EQ(param_value.size(), 4);

      auto word_value = ntohl(DecodeWordValue(param_value));
      ASSERT_EQ(word_value, Ipv6AddrWord(remote_ip, i));
    }
  }

  static inline uint32_t Ipv6AddrWord(const struct p4_ipaddr& ipaddr, int i) {
    return ipaddr.ip.v6addr.__in6_u.__u6_addr32[i];
  }

  void CheckDetail() const {
    EXPECT_EQ(detail.table_id, LOG_L2_TO_TUNNEL_V6_TABLE);
  }

  void CheckMatches() const {
    constexpr char V6_KEY_DA[] = "hdrs.mac[vmeta.common.depth].da";
    const int MFID_DA = GetMatchFieldId(V6_KEY_DA);
    ASSERT_NE(MFID_DA, -1);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 1);

    auto match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MFID_DA);

    CheckMacAddrMatch(match);
  }

  void CheckMacAddrMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i] & 0xFF, fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckNoAction() const { EXPECT_FALSE(table_entry.has_action()); }

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  struct mac_learning_info fdb_info = {0};
  DiagDetail detail;
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------
TEST_F(L2ToV6TunnelTest, remove_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, REMOVE_ENTRY, detail);
  DumpTableEntry();

  // Assert
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(L2ToV6TunnelTest, insert_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, INSERT_ENTRY, detail);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
