// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef P4RT_PERF_H
#define P4RT_PERF_H

#include <stdint.h>

enum OPER { ADD = 1, DEL = 2, ADD_DEL = 3 };

enum TEST_PROFILE { SIMPLE_L2_DEMO = 1 };

struct ThreadInfo {
  uint32_t tid;
  uint32_t core_id;
  uint64_t start;
  uint64_t num_entries;
  uint32_t oper;
  double time_taken;
};

struct TestParams {
  int num_threads = 1;
  int oper = 0;
  int tot_num_entries = 1000000;
  int profile = SIMPLE_L2_DEMO;
};

struct SimpleL2DemoMacInfo {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
};

#endif  // P4RT_PERF_H
