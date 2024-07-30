// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// NOTE:
// This is a minimal unit test, used solely to check the table_id
// field. It needs to be expanded to validate all output fields
// for all (tunnel_type, vlan_mode) combinations.

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;

constexpr char TABLE_NAME[] = "ipv6_tunnel_term_table";

constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
constexpr int IPV6_PREFIX_LEN = 64;

constexpr char SET_VXLAN_DECAP_OUTER_HDR[] = "set_vxlan_decap_outer_hdr";
constexpr char SET_VXLAN_DECAP_OUTER_AND_PUSH_VLAN[] =
    "set_vxlan_decap_outer_and_push_vlan";

constexpr char SET_GENEVE_DECAP_OUTER_HDR[] = "set_geneve_decap_outer_hdr";
constexpr char SET_GENEVE_DECAP_OUTER_AND_PUSH_VLAN[] =
    "set_geneve_decap_outer_and_push_vlan";

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class TunnelTermV6TableTest : public ::testing::Test {
 protected:
  TunnelTermV6TableTest() { memset(&tunnel_info, 0, sizeof(tunnel_info)); }

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable(TABLE_NAME); }

  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  void InitV6TunnelInfo() {
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
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1776B;
    ACTION_ID = GetActionId(SET_VXLAN_DECAP_OUTER_HDR);
  }

  void InitVxlanUntagged() {
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    tunnel_info.vni = 0xA1984;
    ACTION_ID = GetActionId(SET_VXLAN_DECAP_OUTER_AND_PUSH_VLAN);
  }

  void InitGeneveTagged() {
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1776B;
    ACTION_ID = GetActionId(SET_GENEVE_DECAP_OUTER_HDR);
  }

  void InitGeneveUntagged() {
    tunnel_info.tunnel_type = OVS_TUNNEL_GENEVE;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_UNTAGGED;
    tunnel_info.vni = 0xA1984;
    ACTION_ID = GetActionId(SET_GENEVE_DECAP_OUTER_AND_PUSH_VLAN);
  }

  void CheckResults() const {
    ASSERT_FALSE(TABLE == nullptr) << "Table '" << TABLE_NAME << "' not found";

    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    if (ACTION_ID) {
      EXPECT_EQ(action.action_id(), ACTION_ID);
    }

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), PARAM_ID);
    CheckTunnelIdParam(param.value());
  }

  void CheckTunnelIdParam(const std::string& param_value) const {
    EXPECT_EQ(param_value.size(), 3);

    uint32_t tunnel_id = DecodeWordValue(param_value);
    EXPECT_EQ(tunnel_id, tunnel_info.vni)
        << "In hexadecimal:\n"
        << "  tunnel_id is 0x" << std::hex << tunnel_id << '\n'
        << "  tunnel_info.vni is 0x" << tunnel_info.vni << '\n'
        << std::setw(0) << std::dec;
  }

  // Working variables
  ::p4::v1::TableEntry table_entry;
  struct tunnel_info tunnel_info;

  // Values to check against
  uint32_t TABLE_ID;
  uint32_t ACTION_ID = -1;
  uint32_t PARAM_ID = 1;  // tunnel_id

 private:
  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
  }

  uint32_t GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    return -1;
  }

  const ::p4::config::v1::Table* TABLE = nullptr;
};

//----------------------------------------------------------------------
// PrepareV6TunnelTermTableEntry() - vxlan
//----------------------------------------------------------------------

TEST_F(TunnelTermV6TableTest, PrepareV6TunnelTermTableEntry_vxlan_untagged) {
  // Arrange
  InitV6TunnelInfo();
  InitVxlanUntagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckResults();
}

TEST_F(TunnelTermV6TableTest, PrepareV6TunnelTermTableEntry_vxlan_tagged) {
  // Arrange
  InitV6TunnelInfo();
  InitVxlanTagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckResults();
}

//----------------------------------------------------------------------
// PrepareV6TunnelTermTableEntry() - geneve
//----------------------------------------------------------------------

TEST_F(TunnelTermV6TableTest, PrepareV6TunnelTermTableEntry_geneve_untagged) {
  // Arrange
  InitV6TunnelInfo();
  InitGeneveUntagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckResults();
}

TEST_F(TunnelTermV6TableTest, PrepareV6TunnelTermTableEntry_geneve_tagged) {
  // Arrange
  InitV6TunnelInfo();
  InitGeneveTagged();

  // Act
  PrepareV6TunnelTermTableEntry(&table_entry, tunnel_info, p4info,
                                INSERT_ENTRY);

  // Assert
  CheckResults();
}

}  // namespace ovsp4rt
