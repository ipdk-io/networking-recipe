// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <iostream>

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

TEST_F(PrepareL2ToTunnelTest, insert_fdb_v4_tunnel) {
  // Arrange
  struct mac_learning_info fdb_info = {
      .is_tunnel = true,
      .is_vlan = true,
      .mac_addr = {0xd, 0xe, 0xa, 0xd, 0, 0},
      .bridge_id = 19,
  };

  // Act
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
  PrepareL2ToTunnelV4(&table_entry, fdb_info, p4info, true, detail);

  // Assert
  DumpMessageAsJson(table_entry);
}

TEST_F(PrepareL2ToTunnelTest, delete_fdb_v4_tunnel) {
  // Arrange
  struct mac_learning_info fdb_info = {
      .is_tunnel = true,
      .is_vlan = true,
      .mac_addr = {0xb, 0xe, 0xe, 0x0f, 0, 0},
      .bridge_id = 84,
  };

  // Act
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
  PrepareL2ToTunnelV4(&table_entry, fdb_info, p4info, false, detail);

  // Assert
  DumpMessageAsJson(table_entry);
}

TEST_F(PrepareL2ToTunnelTest, insert_fdb_v6_tunnel) {
  // Arrange
  struct mac_learning_info fdb_info = {
      .is_tunnel = true,
      .is_vlan = true,
      .mac_addr = {0xb, 0xe, 0xe, 0xb, 0xe, 0xe},
      .bridge_id = 17,
  };

  // Act
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, true, detail);

  // Assert
  DumpMessageAsJson(table_entry);
}

TEST_F(PrepareL2ToTunnelTest, delete_fdb_v6_tunnel) {
  // Arrange
  struct mac_learning_info fdb_info = {
      .is_tunnel = true,
      .is_vlan = true,
      .mac_addr = {0xb, 0xa, 0xa, 0xb, 0xa, 0xa},
      .bridge_id = 76,
  };

  // Act
  ::p4::v1::TableEntry table_entry;
  DiagDetail detail;
  PrepareL2ToTunnelV6(&table_entry, fdb_info, p4info, false, detail);

  // Assert
  DumpMessageAsJson(table_entry);
}

}  // namespace ovs_p4rt
