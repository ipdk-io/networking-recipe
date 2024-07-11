// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/types/optional.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

ABSL_FLAG(bool, dump_json, false, "Dump output object in JSON");
ABSL_FLAG(bool, check_src_port, false, "Verify src_port field");

namespace ovs_p4rt {

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
using stratum::ParseProtoFromString;

static ::p4::config::v1::P4Info p4info;

class GeneveEncapTableEntryTest : public ::testing::Test {
 protected:
  GeneveEncapTableEntryTest() { dump_json_ = absl::GetFlag(FLAGS_dump_json); };
  static void SetUpTestSuite() {
    // Initialize p4info message from file.
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void DumpTableEntry(const ::p4::v1::TableEntry& table_entry) {
    if (dump_json_) {
      JsonPrintOptions options;
      options.add_whitespace = true;
      options.preserve_proto_field_names = true;
      std::string output;
      ASSERT_TRUE(MessageToJsonString(table_entry, &output, options).ok());
      std::cout << output << std::endl;
    }
  }
  bool dump_json_;
};

uint32_t DecodeWordValue(const std::string& string_value) {
  uint32_t word_value = 0;
  for (int i = 0; i < string_value.size(); i++) {
    word_value = (word_value << 8) | (string_value[i] & 0xff);
  }
  return word_value;
}

//----------------------------------------------------------------------
// PrepareGeneveEncapTableEntry
//----------------------------------------------------------------------

constexpr char IPV4_SRC_ADDR[] = "10.0.0.1";
constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
constexpr int IPV4_PREFIX_LEN = 24;

constexpr uint16_t IPV4_SRC_PORT = 4;
constexpr uint16_t IPV4_DST_PORT = 1984;
constexpr uint16_t IPV4_VNI = 0x101;

constexpr uint32_t GENEVE_ENCAP_MOD_TABLE_ID = 41319073U;
constexpr uint32_t GENEVE_ENCAP_ACTION_ID = 25818889U;

enum {
  IPV4_SRC_PORT_PARAM_ID = 3,
  IPV4_DST_PORT_PARAM_ID = 4,
};

//----------------------------------------------------------------------

void InitV4TunnelInfo(tunnel_info& info) {
  ASSERT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR, &info.local_ip.ip.v4addr.s_addr),
            1);
  info.local_ip.prefix_len = IPV4_PREFIX_LEN;

  ASSERT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR, &info.remote_ip.ip.v4addr.s_addr),
            1);
  info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

  info.src_port = htons(IPV4_SRC_PORT);
  info.dst_port = htons(IPV4_DST_PORT);
  info.vni = htons(IPV4_VNI);
}

//----------------------------------------------------------------------

TEST_F(GeneveEncapTableEntryTest, geneve_v4_encap_params_are_correct) {
  struct tunnel_info tunnel_info = {0};
  p4::v1::TableEntry table_entry;
  constexpr bool insert_entry = true;

  // Arrange
  InitV4TunnelInfo(tunnel_info);

  // Act
  PrepareGeneveEncapTableEntry(&table_entry, tunnel_info, p4info, insert_entry);
  DumpTableEntry(table_entry);

  // Assert
  ASSERT_TRUE(table_entry.has_action());

  auto table_action = table_entry.action();
  auto action = table_action.action();
  ASSERT_EQ(action.action_id(), GENEVE_ENCAP_ACTION_ID);

  auto params = action.params();
  int num_params = action.params_size();
  ASSERT_EQ(num_params, 5);

  absl::optional<uint16_t> src_port;
  absl::optional<uint16_t> dst_port;

  for (int i = 0; i < num_params; ++i) {
    auto param = params[i];
    int param_id = param.param_id();
    if (param_id == IPV4_SRC_PORT_PARAM_ID) {
      auto param_value = param.value();
      src_port = DecodeWordValue(param_value) & 0xffff;
    } else if (param_id == IPV4_DST_PORT_PARAM_ID) {
      auto param_value = param.value();
      dst_port = DecodeWordValue(param_value) & 0xffff;
    }
  }

  if (absl::GetFlag(FLAGS_check_src_port)) {
    // The src_port field contains an arbitrary value that has nothing
    // to do with what was specified in the tunnel_info structure.
    // It is a workaround for a long-standing bug in the Linux
    // Networking P4 program.
    ASSERT_TRUE(src_port.has_value());
    ASSERT_EQ(src_port.value(), IPV4_SRC_PORT);
  }

  ASSERT_TRUE(dst_port.has_value());
  ASSERT_EQ(dst_port.value(), IPV4_DST_PORT);
}

//----------------------------------------------------------------------
// PrepareV6GeneveEncapTableEntry
//----------------------------------------------------------------------

constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
constexpr int IPV6_PREFIX_LEN = 64;

constexpr uint16_t IPV6_SRC_PORT = 42;
constexpr uint16_t IPV6_DST_PORT = 1066;
constexpr uint16_t IPV6_VNI = 0x202;

constexpr uint32_t GENEVE_ENCAP_V6_MOD_TABLE_ID = 42283616U;
constexpr uint32_t GENEVE_ENCAP_V6_ACTION_ID = 29610186U;

enum {
  IPV6_SRC_PORT_PARAM_ID = 7,
  IPV6_DST_PORT_PARAM_ID = 8,
};

//----------------------------------------------------------------------

void InitV6TunnelInfo(tunnel_info& info) {
  EXPECT_EQ(inet_pton(AF_INET6, IPV6_SRC_ADDR,
                      &info.local_ip.ip.v6addr.__in6_u.__u6_addr32),
            1)
      << "Error converting " << IPV6_SRC_ADDR;
  info.local_ip.prefix_len = IPV6_PREFIX_LEN;

  EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                      &info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
            1)
      << "Error converting " << IPV6_DST_ADDR;
  info.remote_ip.prefix_len = IPV6_PREFIX_LEN;

  info.src_port = htons(IPV6_SRC_PORT);
  info.dst_port = htons(IPV6_DST_PORT);
  info.vni = htons(IPV6_VNI);
}

//----------------------------------------------------------------------

TEST_F(GeneveEncapTableEntryTest, geneve_v6_encap_params_are_correct) {
  struct tunnel_info tunnel_info = {0};
  p4::v1::TableEntry table_entry;
  constexpr bool insert_entry = true;

  // Arrange
  InitV6TunnelInfo(tunnel_info);

  // Act
  PrepareV6GeneveEncapTableEntry(&table_entry, tunnel_info, p4info,
                                 insert_entry);
  DumpTableEntry(table_entry);

  // Assert
  ASSERT_TRUE(table_entry.has_action());

  auto table_action = table_entry.action();
  auto action = table_action.action();
  ASSERT_EQ(action.action_id(), GENEVE_ENCAP_V6_ACTION_ID);

  auto params = action.params();
  int num_params = action.params_size();

  absl::optional<uint16_t> src_port;
  absl::optional<uint16_t> dst_port;

  for (int i = 0; i < num_params; ++i) {
    auto param = params[i];
    int param_id = param.param_id();
    auto param_value = param.value();
    if (param_id == IPV6_SRC_PORT_PARAM_ID) {
      src_port = DecodeWordValue(param_value) & 0xffff;
    } else if (param_id == IPV6_DST_PORT_PARAM_ID) {
      dst_port = DecodeWordValue(param_value) & 0xffff;
    }
  }

  if (absl::GetFlag(FLAGS_check_src_port)) {
    // The src_port field contains an arbitrary value that has nothing
    // to do with what was specified in the tunnel_info structure.
    // It is a workaround for a long-standing bug in the Linux
    // Networking P4 program.
    ASSERT_TRUE(src_port.has_value());
    ASSERT_EQ(src_port.value(), IPV6_SRC_PORT);
  }

  ASSERT_TRUE(dst_port.has_value());
  ASSERT_EQ(dst_port.value(), IPV6_DST_PORT);
}

}  // namespace ovs_p4rt
