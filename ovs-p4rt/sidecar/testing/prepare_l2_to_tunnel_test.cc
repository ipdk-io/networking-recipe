// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>

#include "gtest/gtest.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_file_path.h"
#include "stratum/lib/utils.h"

namespace ovs_p4rt {

static ::p4::config::v1::P4Info p4info;

class PrepareL2ToTunnelTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
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
};

TEST_F(PrepareL2ToTunnelTest, test1) {}

TEST_F(PrepareL2ToTunnelTest, test2) {}

}  // namespace ovs_p4rt
