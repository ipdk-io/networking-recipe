/*
 * Copyright (c) 2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ovsp4rt/ovs-p4rt.h"

constexpr uint8_t kMacAddrV4[6] = {0xd, 0xe, 0xa, 0xd, 0, 0};
constexpr uint32_t kIpAddrV4 = 0x0a010203U;  // 10.1.2.3

void get_v4_fdb_info(mac_learning_info& fdb_info) {
  memcpy(fdb_info.mac_addr, kMacAddrV4, sizeof(fdb_info.mac_addr));
  fdb_info.tnl_info.remote_ip.family = AF_INET;
  fdb_info.tnl_info.remote_ip.ip.v4addr.s_addr = kIpAddrV4;
}
constexpr uint8_t kMacAddrV6[] = {0xb, 0xe, 0xe, 0xb, 0xe, 0xe};
constexpr uint32_t kIpAddrV6[] = {0, 66, 129, 512};
constexpr int kIpAddrV6Len = sizeof(kIpAddrV6) / sizeof(kIpAddrV6[0]);

void get_v6_fdb_info(mac_learning_info& fdb_info) {
  memcpy(fdb_info.mac_addr, kMacAddrV6, sizeof(fdb_info.mac_addr));
  fdb_info.tnl_info.remote_ip.family = AF_INET6;
  for (int i = 0; i < kIpAddrV6Len; i++) {
    fdb_info.tnl_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32[i] = kIpAddrV6[i];
  }
}

void fdb_info_v4_test() {
  struct mac_learning_info fdb_info = {0};
  get_v4_fdb_info(fdb_info);
  ovsp4rt_config_fdb_entry(fdb_info, true, nullptr);
}

void fdb_info_v6_test() {
  struct mac_learning_info fdb_info = {0};
  get_v6_fdb_info(fdb_info);
  ovsp4rt_config_fdb_entry(fdb_info, true, nullptr);
}

int main() {
  fdb_info_v4_test();
  fdb_info_v6_test();
}
