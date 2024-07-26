// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef IPV6_TUNNEL_TEST_H_
#define IPV6_TUNNEL_TEST_H_

#include <arpa/inet.h>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "testing/table_entry_test.h"

namespace ovsp4rt {

constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
constexpr int IPV6_PREFIX_LEN = 64;

constexpr uint16_t SRC_PORT = 0x1984;
constexpr uint16_t DST_PORT = 0x4224;
constexpr uint32_t VNI = 0x1066;

class Ipv6TunnelTest : public TableEntryTest {
 protected:
  Ipv6TunnelTest() {}

  void InitV6TunnelInfo(tunnel_info& info) {
    EXPECT_EQ(inet_pton(AF_INET6, IPV6_SRC_ADDR,
                        &info.local_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_SRC_ADDR;
    info.local_ip.prefix_len = IPV6_PREFIX_LEN;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                        &info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_DST_ADDR;
    info.remote_ip.prefix_len = IPV6_PREFIX_LEN;

    info.src_port = SRC_PORT;
    info.dst_port = DST_PORT;
    info.vni = VNI;
  };
};

}  // namespace ovsp4rt

#endif  // IPV6_TUNNEL_TEST_H_
