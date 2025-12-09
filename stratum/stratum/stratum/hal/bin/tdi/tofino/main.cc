// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/bin/tdi/main.h"

int main(int argc, char* argv[]) {
  return stratum::hal::tdi::Main(argc, argv).error_code();
}
