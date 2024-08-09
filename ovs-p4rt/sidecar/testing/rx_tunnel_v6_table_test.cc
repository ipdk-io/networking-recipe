// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareV6RxTunnelTableEntry().

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class RxTunnelPortV6TableTest : public BaseTableTest {
 protected:
  RxTunnelPortV6TableTest() {}

  void SetUp() { helper.SelectTable("rx_ipv6_tunnel_source_port"); }

  //----------------------------
  // Utility methods
  //----------------------------

  static void DecodeIpv6AddrValue(const std::string& string_value,
                                  std::vector<uint32_t>& ipv6_addr) {
    constexpr int IPV6_ADDR_SIZE = 16;

    ASSERT_EQ(string_value.size(), IPV6_ADDR_SIZE);

    ipv6_addr.clear();
    for (int base = 0; base < IPV6_ADDR_SIZE; base += 4) {
      uint32_t word_value = 0;
      for (int i = 0; i < 4; i++) {
        word_value = (word_value << 8) | (string_value[base + i] & 0xFF);
      }
      ipv6_addr.push_back(ntohl(word_value));
    }
  }

  static inline uint32_t Ipv6AddrWord(const struct p4_ipaddr& ipaddr, int i) {
    return ipaddr.ip.v6addr.__in6_u.__u6_addr32[i];
  }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitAction() { helper.SelectAction("set_source_port"); }

  void InitTunnelInfo() {
    constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
    constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
    constexpr int IPV6_PREFIX_LEN = 64;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_DST_ADDR;
    tunnel_info.remote_ip.prefix_len = IPV6_PREFIX_LEN;

    tunnel_info.vni = 0x911;
    tunnel_info.src_port = 0xFADE;
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    const int SOURCE_PORT_ID = helper.GetParamId("source_port");
    ASSERT_NE(SOURCE_PORT_ID, -1);

    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    if (helper.action_id()) {
      EXPECT_EQ(action.action_id(), helper.action_id());
    }

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), SOURCE_PORT_ID);

    const auto src_port = DecodeWordValue(param.value()) & 0xffff;
    ASSERT_EQ(src_port, tunnel_info.src_port);
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    const int MFID_IPV6_SRC = helper.GetMatchFieldId("ipv6_src");
    const int MFID_VNI = helper.GetMatchFieldId("vni");

    ASSERT_NE(MFID_IPV6_SRC, -1);
    ASSERT_NE(MFID_VNI, -1);

    ASSERT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      int field_id = match.field_id();
      if (field_id == MFID_IPV6_SRC) {
        CheckIpv6AddrMatch(match);
      } else if (field_id == MFID_VNI) {
        CheckVniMatch(match);
      }
    }
  }

  void CheckIpv6AddrMatch(const ::p4::v1::FieldMatch& match) const {
    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    std::vector<uint32_t> ipv6_addr;
    DecodeIpv6AddrValue(match_value, ipv6_addr);

    const struct p4_ipaddr& remote_ip = tunnel_info.remote_ip;

    for (int i = 0; i < 4; i++) {
      uint32_t word_value = Ipv6AddrWord(remote_ip, i);
      EXPECT_EQ(ipv6_addr[i], word_value)
          << "ipv6_addr[" << i << "] does not match\n"
          << std::hex << std::setw(8) << "  expected value is 0x" << word_value
          << '\n'
          << "  actual value is   0x" << ipv6_addr[i] << '\n'
          << std::dec << std::setw(0);
    }
  }

  void CheckVniMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int VNI_SIZE = 3;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    EXPECT_EQ(match_value.size(), VNI_SIZE);

    auto vni_value = DecodeVniValue(match_value);
    ASSERT_EQ(vni_value, tunnel_info.vni);
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(helper.has_table());
    EXPECT_EQ(table_entry.table_id(), helper.table_id());
  }

  //----------------------------
  // Protected member data
  //----------------------------
  struct tunnel_info tunnel_info = {0};
};

//----------------------------------------------------------------------
// PrepareV6RxTunnelTableEntry()
//----------------------------------------------------------------------

TEST_F(RxTunnelPortV6TableTest, remove_entry) {
  // Arrange
  InitTunnelInfo();

  // Act
  PrepareV6RxTunnelTableEntry(&table_entry, tunnel_info, p4info, REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(RxTunnelPortV6TableTest, insert_entry) {
  // Arrange
  InitTunnelInfo();
  InitAction();

  // Act
  PrepareV6RxTunnelTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
