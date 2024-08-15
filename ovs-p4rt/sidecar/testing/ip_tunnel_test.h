// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef IP_TUNNEL_TEST_H_
#define IP_TUNNEL_TEST_H_

#include <arpa/inet.h>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovsp4rt {

class IpTunnelTest : public BaseTableTest {
 protected:
  IpTunnelTest() {}

  void InitV4TunnelInfo(uint8_t tunnel_type) {
    constexpr char IPV4_SRC_ADDR[] = "10.20.30.40";
    constexpr char IPV4_DST_ADDR[] = "192.168.17.5";
    constexpr int IPV4_PREFIX_LEN = 24;

    constexpr uint16_t SRC_PORT = 0x1066;
    constexpr uint16_t DST_PORT = 0x4224;
    constexpr uint16_t VNI = 0x1776;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_SRC_ADDR,
                        &tunnel_info.local_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_SRC_ADDR;
    tunnel_info.local_ip.prefix_len = IPV4_PREFIX_LEN;
    tunnel_info.local_ip.family = AF_INET;

    EXPECT_EQ(inet_pton(AF_INET, IPV4_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v4addr.s_addr),
              1)
        << "Error converting " << IPV4_DST_ADDR;
    tunnel_info.remote_ip.prefix_len = IPV4_PREFIX_LEN;
    tunnel_info.remote_ip.family = AF_INET;

    tunnel_info.src_port = SRC_PORT;
    tunnel_info.dst_port = DST_PORT;
    tunnel_info.vni = VNI;
    tunnel_info.tunnel_type = tunnel_type;
  };

  void InitV6TunnelInfo(uint8_t tunnel_type) {
    constexpr char IPV6_SRC_ADDR[] = "fe80::215:5dff:fefa";
    constexpr char IPV6_DST_ADDR[] = "fe80::215:192.168.17.5";
    constexpr int IPV6_PREFIX_LEN = 64;

    constexpr uint16_t SRC_PORT = 0x1984;
    constexpr uint16_t DST_PORT = 0x4224;
    constexpr uint16_t VNI = 0x1066;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_SRC_ADDR,
                        &tunnel_info.local_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_SRC_ADDR;
    tunnel_info.local_ip.prefix_len = IPV6_PREFIX_LEN;
    tunnel_info.local_ip.family = AF_INET6;

    EXPECT_EQ(inet_pton(AF_INET6, IPV6_DST_ADDR,
                        &tunnel_info.remote_ip.ip.v6addr.__in6_u.__u6_addr32),
              1)
        << "Error converting " << IPV6_DST_ADDR;
    tunnel_info.remote_ip.prefix_len = IPV6_PREFIX_LEN;
    tunnel_info.remote_ip.family = AF_INET6;

    tunnel_info.src_port = SRC_PORT;
    tunnel_info.dst_port = DST_PORT;
    tunnel_info.vni = VNI;
    tunnel_info.tunnel_type = tunnel_type;
  };

  struct tunnel_info tunnel_info = {0};
};

}  // namespace ovsp4rt

#endif  // IP_TUNNEL_TEST_H_
