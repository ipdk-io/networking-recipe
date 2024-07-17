// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef IPV4_TUNNEL_TEST_H_
#define IPV4_TUNNEL_TEST_H_

#include <arpa/inet.h>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "testing/table_entry_test.h"

namespace ovs_p4rt {

constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
constexpr int IPV4_PREFIX_LEN = 24;

constexpr uint16_t SRC_PORT = 0x1066;
constexpr uint16_t DST_PORT = 0x1984;
constexpr uint16_t VNI = 0x1776;

class Ipv4TunnelTest : public TableEntryTest {
 protected:
  Ipv4TunnelTest() {}

  void InitV4TunnelInfo(tunnel_info& info) {
    EXPECT_EQ(
        inet_pton(AF_INET, IPV4_SRC_ADDR, &info.local_ip.ip.v4addr.s_addr), 1)
        << "Error converting " << IPV4_SRC_ADDR;
    info.local_ip.prefix_len = IPV4_PREFIX_LEN;

    EXPECT_EQ(
        inet_pton(AF_INET, IPV4_DST_ADDR, &info.remote_ip.ip.v4addr.s_addr), 1)
        << "Error converting " << IPV4_DST_ADDR;
    info.remote_ip.prefix_len = IPV4_PREFIX_LEN;

    info.src_port = htons(SRC_PORT);
    info.dst_port = htons(DST_PORT);
    info.vni = htons(VNI);
  };
};

}  // namespace ovs_p4rt

#endif  // IPV4_TUNNEL_TEST_H_
