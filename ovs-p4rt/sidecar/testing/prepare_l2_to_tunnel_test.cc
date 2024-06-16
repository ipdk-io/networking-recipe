// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_file_path.h"
#include "stratum/lib/utils.h"

namespace ovs_p4rt {

class EncodeTest : public ::testing::Test {
#if 0
 protected:
  // Sets up the test fixture.
  void SetUp() override {}

  // Tears down the test fixture.
  void TearDown() override {}
#endif
};

TEST_F(EncodeTest, can_initialize_p4info_from_text_file) {
  ::p4::config::v1::P4Info p4info;
  ::util::Status status = stratum::ReadProtoFromTextFile(P4INFO_FILE_PATH,
                                                         &p4info);
  ASSERT_TRUE(status.ok());
}

}  // namespace ovs_p4rt
