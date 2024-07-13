// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef IPV4_TUNNEL_TEST_DEFS_H_
#define IPV4_TUNNEL_TEST_DEFS_H_

#include <arpa/inet.h>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovs_p4rt {

constexpr char IPV4_SRC_ADDR[] = "10.0.0.1";
constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
constexpr int IPV4_PREFIX_LEN = 24;

constexpr uint16_t SRC_PORT = 4;
constexpr uint16_t DST_PORT = 1984;
constexpr uint16_t VNI = 0x101;

//----------------------------------------------------------------------

void InitV4TunnelInfo(tunnel_info& info) {
  EXPECT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR, &info.local_ip.ip.v4addr.s_addr),
            1);
  info.local_ip.prefix_len = IPV4_PREFIX_LEN;

  EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR, &info.remote_ip.ip.v4addr.s_addr),
            1);
  info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

  info.src_port = htons(SRC_PORT);
  info.dst_port = htons(DST_PORT);
  info.vni = htons(VNI);
}

}  // namespace ovs_p4rt

#endif  // IPV4_TUNNEL_TEST_DEFS_H_
