// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareL2ToTunnelV6().

#include <cstdlib>
#include <iostream>
#include <sstream>
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

ABSL_FLAG(bool, dump_json, false, "Dump test output in JSON");

namespace ovsp4rt {

constexpr char TABLE_NAME[] = "l2_to_tunnel_v6";

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
using stratum::ParseProtoFromString;

static ::p4::config::v1::P4Info p4info;

class PrepareL2ToV6TunnelTest : public ::testing::Test {
 protected:
  PrepareL2ToV6TunnelTest() { dump_json_ = absl::GetFlag(FLAGS_dump_json); }

  static void SetUpTestSuite() {
    ::util::Status status = stratum::ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable(TABLE_NAME); }

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

  static inline uint32_t Ipv6AddrWord(const struct p4_ipaddr& ipaddr, int i) {
    return ipaddr.ip.v6addr.__in6_u.__u6_addr32[i];
  }

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
  }

  //----------------------------
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    ASSERT_NE(ACTION_ID, -1);

    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    EXPECT_EQ(action.action_id(), ACTION_ID);

    ASSERT_EQ(action.params_size(), 4);

    const struct p4_ipaddr& remote_ip = fdb_info.tnl_info.remote_ip;

    // TODO(derek): look up param IDs by name.
    // Allow for the possibility that params are not in order. (?)
    for (int i = 0; i < action.params_size(); i++) {
      auto param = action.params()[i];
      ASSERT_EQ(param.param_id(), i + 1);

      auto param_value = param.value();
      ASSERT_EQ(param_value.size(), 4);

      auto word_value = ntohl(DecodeWordValue(param_value));
      ASSERT_EQ(word_value, Ipv6AddrWord(remote_ip, i));
    }
  }

  void CheckDetail() const {
    EXPECT_EQ(detail.table_id, LOG_L2_TO_TUNNEL_V6_TABLE);
  }

  void CheckMatches() const {
    constexpr char V6_KEY_DA[] = "hdrs.mac[vmeta.common.depth].da";
    const int MFID_DA = GetMatchFieldId(V6_KEY_DA);

    // number of match fields
    ASSERT_EQ(table_entry.match_size(), 1);

    auto match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MFID_DA);

    CheckMacAddr(match);
  }

  void CheckMacAddr(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();
    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i] & 0xFF, fdb_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckNoAction() const {
    ASSERT_NE(ACTION_ID, -1);
    EXPECT_FALSE(table_entry.has_action());
  }

  void CheckTableEntry() const {
    ASSERT_FALSE(TABLE == nullptr) << "Table '" << TABLE_NAME << "' not found";
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
  }

  void InitFdbInfo() {
    constexpr uint8_t MAC_ADDR[] = {0xde, 0xad, 0xbe, 0xef, 0x00, 0xe};
    constexpr uint32_t kIpAddrV6[] = {0, 66, 129, 512};
    constexpr int kIpAddrV6Len = sizeof(kIpAddrV6) / sizeof(kIpAddrV6[0]);

    memcpy(fdb_info.mac_addr, MAC_ADDR, sizeof(fdb_info.mac_addr));

    fdb_info.tnl_info.remote_ip.family = AF_INET6;
    // TODO(derek): replace this with pton()
    for (int i = 0; i < kIpAddrV6Len; i++) {
      fdb_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[i] =
          kIpAddrV6[i];
    }

    SelectAction("set_tunnel_v6");
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
// PrepareL2ToV6TunnelTest()
//----------------------------------------------------------------------
TEST_F(PrepareL2ToV6TunnelTest, remove_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, REMOVE_ENTRY, detail);
  DumpTableEntry();

  // Assert
  ASSERT_EQ(table_entry.table_id(), TABLE_ID);
  CheckDetail();
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(PrepareL2ToV6TunnelTest, insert_entry) {
  // Arrange
  InitFdbInfo();

  // Act
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, INSERT_ENTRY, detail);
  DumpTableEntry();

  // Assert
  // We've already checked Detail, TableEntry, and Matches.
  CheckAction();
}

}  // namespace ovsp4rt
