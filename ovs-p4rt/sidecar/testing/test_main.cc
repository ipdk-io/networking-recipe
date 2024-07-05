// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "absl/flags/parse.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  absl::ParseCommandLine(argc, argv);
  return RUN_ALL_TESTS();
}
