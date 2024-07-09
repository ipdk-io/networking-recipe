// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "absl/flags/parse.h"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
  // As part of initialization, this function parses the command line,
  // removing any arguments that are specific to googletest.
  testing::InitGoogleTest(&argc, argv);

  // Parses the remaining command-line arguments for Abseil flags
  // defined by the test program.
  absl::ParseCommandLine(argc, argv);

  return RUN_ALL_TESTS();
}
