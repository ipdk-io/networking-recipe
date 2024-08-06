// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareL2ToTunnelV4().

#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "logging/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

ABSL_FLAG(bool, dump_json, false, "Dump output table_entry in JSON");

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
using stratum::ParseProtoFromString;

static ::p4::config::v1::P4Info p4info;

class L2ToTunnelV4TableTest : public ::testing::Test {
 protected:
  L2ToTunnelV4TableTest() { dump_json_ = absl::GetFlag(FLAGS_dump_json); }

  static void SetUpTestSuite() {
    ::util::Status status = stratum::ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable("l2_to_tunnel_v4"); }

  //----------------------------
  // P4Info lookup methods
  //----------------------------

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    std::cerr << "Match Field '" << mf_name << "' not found!\n";
    return -1;
  }

  int GetParamId(const std::string& param_name) const {
    for (const auto& param : ACTION->params()) {
      if (param.name() == param_name) {
        return param.id();
      }
    }
    std::cerr << "Action Param '" << param_name << "' not found!\n";
    return -1;
  }

  void SelectAction(const std::string& action_name) {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        ACTION = &action;
        ACTION_ID = pre.id();
        return;
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
  }

  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
    std::cerr << "Table '" << table_name << "' not found!\n";
  }

  //----------------------------
  // Utility methods
  //----------------------------

  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  void DumpTableEntry() {
    if (dump_json_) {
      JsonPrintOptions options;
      options.add_whitespace = true;
      options.preserve_proto_field_names = true;
      std::string output;
      ASSERT_TRUE(MessageToJsonString(table_entry, &output, options).ok());
      std::cout << output << std::endl;
    }
  }

  //----------------------------
  // Initialization methods
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
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    // ACTION_ID is defined (sanity check)
    ASSERT_NE(ACTION_ID, -1);

    // Table entry specifies an action
    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    // Action ID is what we expect
    auto action = table_action.action();
    EXPECT_EQ(action.action_id(), ACTION_ID);

    // Only one parameter
    ASSERT_EQ(action.params_size(), 1);

    // Param ID is what we expect
    auto param = action.params()[0];
    ASSERT_EQ(param.param_id(), GetParamId("dst_addr"));

    // Value has 4 bytes
    auto param_value = param.value();
    ASSERT_EQ(param_value.size(), 4);

    // Value is what we expect
    auto word_value = ntohl(DecodeWordValue(param_value));
    ASSERT_EQ(word_value, fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr);
  }

  void CheckDetail() const {
    // LogTableId is correct for this table
    EXPECT_EQ(detail.table_id, LOG_L2_TO_TUNNEL_V4_TABLE);
  }

  void CheckMatches() const {
    constexpr char V4_KEY_DA[] = "hdrs.mac[vmeta.common.depth].da";
    const int MFID_DA = GetMatchFieldId(V4_KEY_DA);

    // Exactly one match field
    ASSERT_EQ(table_entry.match_size(), 1);

    // Match Field ID is what we expect
    auto match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MFID_DA);

    // Value is what we expect
    CheckMacAddr(match);
  }

  void CheckMacAddr(const ::p4::v1::FieldMatch& match) const {
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

  void CheckNoAction() const {
    // Table entry does not specify an action
    EXPECT_FALSE(table_entry.has_action());
  }

  void CheckTableEntry() const {
    // Table is defined (sanity check)
    ASSERT_FALSE(TABLE == nullptr);

    // Table ID is what we expect
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
  }

  //----------------------------
  // Protected member data
  //----------------------------

  // Working variables
  struct mac_learning_info fdb_info = {0};
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;

  // Comparison variables
  uint32_t TABLE_ID;
  int ACTION_ID = -1;

 private:
  //----------------------------
  // Private member data
  //----------------------------
  const ::p4::config::v1::Table* TABLE = nullptr;
  const ::p4::config::v1::Action* ACTION = nullptr;

  bool dump_json_;
};

//----------------------------------------------------------------------
// L2ToTunnelV4TableTest
//----------------------------------------------------------------------
TEST_F(L2ToTunnelV4TableTest, remove_entry) {
  // Arrange
  InitFdbInfo();
  InitTunnelInfo();

  // Act
  PrepareL2ToTunnelV4(&table_entry, fdb_info, p4info, REMOVE_ENTRY, detail);
  DumpTableEntry();

  // Assert
  ASSERT_EQ(table_entry.table_id(), TABLE_ID);
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(L2ToTunnelV4TableTest, insert_entry) {
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
