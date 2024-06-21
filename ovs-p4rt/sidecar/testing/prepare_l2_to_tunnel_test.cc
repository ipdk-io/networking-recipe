// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "lib/ovsp4rt_diag_detail.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_file_path.h"
#include "stratum/lib/utils.h"

namespace ovs_p4rt {

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
using stratum::PrintProtoToString;

static ::p4::config::v1::P4Info p4info;

//----------------------------------------------------------------------
// PrepareL2ToTunnelTest
//----------------------------------------------------------------------
class PrepareL2ToTunnelTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // Initialize p4info message from file.
    ::util::Status status =
        stratum::ReadProtoFromTextFile(P4INFO_FILE_PATH, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

#if 0
  // Sets up the test fixture.
  void SetUp() override {}

  // Tears down the test fixture.
  void TearDown() override {}
#endif

  void DumpMessageAsJson(const google::protobuf::Message& message) {
    JsonPrintOptions options;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;
    std::string output;
    ASSERT_TRUE(MessageToJsonString(message, &output, options).ok());
    std::cout << output << std::endl;
  }
};

uint32_t DecodeWordValue(const std::string string_value) {
  uint32_t word_value;
  for (int i = 0; i < string_value.size(); i++) {
    word_value = (word_value << 8) | string_value[i];
  }
  return word_value;
}

std::string HexByte(uint8_t byte_val) {
  std::ostringstream hex_val;
  hex_val << std::hex << std::setw(2) << std::setfill('0') << byte_val;
  return hex_val.str();
}

//----------------------------------------------------------------------
// GetL2ToTunnelV4TableEntry
//----------------------------------------------------------------------
constexpr uint8_t kMacAddrV4[6] = {0xd, 0xe, 0xa, 0xd, 0, 0};
// TODO(derek): IPv4 address encoding?
constexpr uint32_t kIpAddrV4 = 0x0a010203U; // 10.1.2.3
// TODO(derek): How stable are the table and action IDs?
constexpr uint32_t kTableIdV4 = 43337754U;
constexpr uint32_t kActionIdV4 = 23805991U;

void init_v4_fdb_info(mac_learning_info& fdb_info) {
  memcpy(fdb_info.mac_addr, kMacAddrV4, sizeof(fdb_info.mac_addr));
  fdb_info.tnl_info.remote_ip.family = AF_INET;
  // TODO(derek): byte order?
  fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr = kIpAddrV4;
}

TEST_F(PrepareL2ToTunnelTest, GetL2ToTunnelV4TableEntry) {
  // Arrange
  struct mac_learning_info fdb_info = {0};
  init_v4_fdb_info(fdb_info);

  // Act
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
  PrepareL2ToTunnelV4(&table_entry, fdb_info, p4info, true, detail);

  // Assert
  DumpMessageAsJson(table_entry);

  ASSERT_EQ(table_entry.table_id(), kTableIdV4);

  // match: mac_addr
  ASSERT_EQ(table_entry.match_size(), 1);

  auto match = table_entry.match()[0];
  ASSERT_EQ(match.field_id(), 1);

  ASSERT_TRUE(match.has_exact());

  auto match_value = match.exact().value();
  ASSERT_EQ(match_value.size(), sizeof(kMacAddrV4));

  for (int i = 0; i < sizeof(kMacAddrV4); i++) {
    ASSERT_EQ(uint8_t(match_value[i]), kMacAddrV4[i]);
  }

  // action: set_tunnel_v4(dst_addr)
  ASSERT_TRUE(table_entry.has_action());

  auto table_action = table_entry.action();
  auto action = table_action.action();
  ASSERT_EQ(action.action_id(), kActionIdV4);

  ASSERT_EQ(action.params_size(), 1);

  auto param = action.params()[0];
  ASSERT_EQ(param.param_id(), 1);

  auto param_value = param.value();
  ASSERT_EQ(param_value.size(), 4);
  // TODO(derek): IPv4 address byte order?
}

#if 0
//----------------------------------------------------------------------
// GetL2ToTunnelV6TableEntry
//----------------------------------------------------------------------
constexpr uint8_t kMacAddrV6[] = {0xb, 0xe, 0xe, 0xb, 0xe, 0xe};
constexpr uint32_t kIpAddrV6[] = {1, 2, 3, 4};
constexpr int kIpAddrV6Len = sizeof(kIpAddrV6) / sizeof(kIpAddrV6[0]);

void init_v6_fdb_info(mac_learning_info& fdb_info) {
  memcpy(fdb_info.mac_addr, kMacAddrV6, sizeof(fdb_info.mac_addr));
  fdb_info.tnl_info.remote_ip.family = AF_INET6;
  fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr = kIpAddrV4;
  //
  for (int i = 0; i < kIpAddrV6Len; i++) {
    fdb_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[i] = kIpAddrV6[i];
  }
}

TEST_F(PrepareL2ToTunnelTest, GetL2ToTunnelV6TableEntry) {
  // Arrange
  struct mac_learning_info fdb_info = {0};
  init_v6_fdb_info(fdb_info);

  // Act
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, true, detail);

  // Assert
  DumpMessageAsJson(table_entry);
}
#endif

}  // namespace ovs_p4rt
