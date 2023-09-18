// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef P4RT_PERF_H
#define P4RT_PERF_H

#include <stdint.h>

enum OPER { ADD = 1, DEL = 2 };

enum TEST_PROFILE { SIMPLE_L2_DEMO = 1 };

enum STATUS { SUCCESS = 0, INVALID_ARG = 1, INTERNAL_ERR = 2 };

struct ThreadInfo {
  uint32_t tid;
  uint32_t core_id;
  uint64_t start;
  uint64_t num_entries;
  uint32_t oper;
  double time_taken;
  int status;
};

struct TestParams {
  uint32_t num_threads = 1;
  uint32_t oper = 0;
  uint64_t tot_num_entries = 1000000;
  uint32_t profile = SIMPLE_L2_DEMO;
};

struct SimpleL2DemoMacInfo {
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
};

#endif  // P4RT_PERF_H
