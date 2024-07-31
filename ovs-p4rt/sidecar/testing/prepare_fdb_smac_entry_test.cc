// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;

constexpr char TABLE_NAME[] = "l2_fwd_smac_table";

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class MacLearnInfoTest : public ::testing::Test {
 protected:
  MacLearnInfoTest() { memset(&fdb_info, 0, sizeof(fdb_info)); }

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

  int GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    return -1;
  }

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    return -1;
  }

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 42;
  }

#if 0
  void InitV4TunnelInfo() {
    constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR,
                        &tunnel_info.local_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_SRC_ADDR;
    tunnel_info.local_ip.family = AF_INET;
    tunnel_info.local_ip.prefix_len = IPV4_PREFIX_LEN;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    tunnel_info.remote_ip.family = AF_INET;
    tunnel_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

    tunnel_info.bridge_id = 86;
  }

  void InitVxlanTagged() {
    tunnel_info.tunnel_type = OVS_TUNNEL_VXLAN;
    tunnel_info.vlan_info.port_vlan_mode = P4_PORT_VLAN_NATIVE_TAGGED;
    tunnel_info.vni = 0x1776B;
    ACTION_ID = GetActionId(SET_VXLAN_DECAP_OUTER_HDR);
  }
#endif

  void CheckActions() const {}

  void CheckMatches() const {
    constexpr int MAC_ADDR_SIZE = 6;
    constexpr int BRIDGE_ID_SIZE = 1;

    constexpr int MFID_MAC_ADDR = 1;
    constexpr int MFID_BRIDGE_ID = 2;

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 2);

    {
      // match field 1: sa
      auto match = table_entry.match()[0];
      ASSERT_EQ(match.field_id(), MFID_MAC_ADDR);
      ASSERT_TRUE(match.has_exact());

      auto match_value = match.exact().value();
      ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

      for (int i = 0; i < MAC_ADDR_SIZE; i++) {
        EXPECT_EQ(uint8_t(match_value[i]), fdb_info.mac_addr[i]);
      }
    }

    {
      // match field 2: bridge_id
      auto match = table_entry.match()[1];
      ASSERT_EQ(match.field_id(), MFID_BRIDGE_ID);
      ASSERT_TRUE(match.has_exact());

      auto match_value = match.exact().value();
      ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);
      ASSERT_EQ(uint8_t(match_value[0]), fdb_info.tnl_info.bridge_id);
    }
  }

  void CheckDetail() const {
    EXPECT_EQ(detail.table_id, LOG_L2_FWD_SMAC_TABLE);
  }

  void CheckTableEntry() const {
    ASSERT_FALSE(TABLE == nullptr) << "Table '" << TABLE_NAME << "' not found";
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
    CheckMatches();
    CheckActions();
  }

  // Test case variables
  struct mac_learning_info fdb_info = {0};
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  // Values to check against
  uint32_t TABLE_ID;

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

  const ::p4::config::v1::Table* TABLE = nullptr;
};

//----------------------------------------------------------------------
// PrepareFdbSmacTableEntry
//----------------------------------------------------------------------

TEST_F(MacLearnInfoTest, remove_table_entry_is_correct) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareFdbSmacTableEntry(&table_entry, fdb_info, p4info, REMOVE_ENTRY,
                           detail);

  // Assert
  CheckDetail();
  CheckTableEntry();
}

}  // namespace ovsp4rt
