// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#define DUMP_JSON

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class TunnelTermV6TableTest : public BaseTableTest {
 protected:
  TunnelTermV6TableTest() {}

  void SetUp() { helper.SelectTable("ipv6_tunnel_term_table"); }

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

  void InitGeneveTagged() {
    constexpr char SET_GENEVE_DECAP_OUTER_HDR[] = "set_geneve_decap_outer_hdr";
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1776;
    helper.SelectAction(SET_GENEVE_DECAP_OUTER_HDR);
  }

  void InitGeneveUntagged() {
    constexpr char SET_GENEVE_DECAP_OUTER_AND_PUSH_VLAN[] =
        "set_geneve_decap_outer_and_push_vlan";
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    tunnel_info.vni = 0x1984;
    helper.SelectAction(SET_GENEVE_DECAP_OUTER_AND_PUSH_VLAN);
  }

  void InitV6TunnelInfo() {
    constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
    constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
    constexpr int IPV6_PREFIX_LEN = 64;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_SRC_ADDR,
                        &tunnel_info.local_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_SRC_ADDR;
    tunnel_info.local_ip.family = AF_INET6;
    tunnel_info.local_ip.prefix_len = IPV6_PREFIX_LEN;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_DST_ADDR;
    tunnel_info.remote_ip.family = AF_INET6;
    tunnel_info.remote_ip.prefix_len = IPV6_PREFIX_LEN;

    tunnel_info.bridge_id = 99;
  }

  void InitVxlanTagged() {
    constexpr char SET_VXLAN_DECAP_OUTER_HDR[] = "set_vxlan_decap_outer_hdr";
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1066;
    helper.SelectAction(SET_VXLAN_DECAP_OUTER_HDR);
  }

  void InitVxlanUntagged() {
    constexpr char SET_VXLAN_DECAP_OUTER_AND_PUSH_VLAN[] =
        "set_vxlan_decap_outer_and_push_vlan";
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    tunnel_info.vni = 0x1492;
    helper.SelectAction(SET_VXLAN_DECAP_OUTER_AND_PUSH_VLAN);
  }

  //----------------------------
  // CheckAction()
  //----------------------------
  void CheckAction() const {
    const int TUNNEL_ID = helper.GetParamId("tunnel_id");
    ASSERT_NE(TUNNEL_ID, -1);

    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    if (helper.action_id()) {
      EXPECT_EQ(action.action_id(), helper.action_id());
    }

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), TUNNEL_ID);
    CheckTunnelIdParam(param.value());
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  void CheckTunnelIdParam(const std::string& param_value) const {
    EXPECT_EQ(param_value.size(), 3);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, tunnel_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tunnel_info.vni is 0x" << tunnel_info.vni << '\n'
        << std::setw(0) << std::dec;
  }

  //----------------------------
  // CheckMatches()
  //----------------------------
  void CheckMatches() const {
    const int MFID_IPV6_SRC = helper.GetMatchFieldId("ipv6_src");
    const int MFID_VNI = helper.GetMatchFieldId("vni");

    ASSERT_NE(MFID_IPV6_SRC, -1);
    ASSERT_NE(MFID_VNI, -1);

    ASSERT_GE(table_entry.match_size(), 3);

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

 private:
  //----------------------------
  // Private member data
  //----------------------------
#ifdef DUMP_JSON
  bool dump_json_ = false;
#endif
};

//----------------------------------------------------------------------
// PrepareV6TunnelTermTableEntry() - vxlan
//----------------------------------------------------------------------

TEST_F(TunnelTermV6TableTest, remove_vxlan_untagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitVxlanUntagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(TunnelTermV6TableTest, insert_vxlan_untagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitVxlanUntagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckAction();
}

TEST_F(TunnelTermV6TableTest, remove_vxlan_tagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitVxlanTagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(TunnelTermV6TableTest, insert_vxlan_tagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitVxlanTagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckAction();
}

//----------------------------------------------------------------------
// PrepareV6TunnelTermTableEntry() - geneve
//----------------------------------------------------------------------

TEST_F(TunnelTermV6TableTest, remove_geneve_untagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitGeneveUntagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(TunnelTermV6TableTest, insert_geneve_untagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitGeneveUntagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckAction();
}

TEST_F(TunnelTermV6TableTest, remove_geneve_tagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitGeneveTagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(TunnelTermV6TableTest, insert_geneve_tagged_entry) {
  // Arrange
  InitV6TunnelInfo();
  InitGeneveTagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
