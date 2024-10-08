// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "encode_base_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_encode.h"

namespace ovsp4rt {

class EncodeAddrTest : public EncodeBaseTest {
 protected:
  EncodeAddrTest() {}
};

TEST_F(EncodeAddrTest, can_encode_mac_address) {
  const uint8_t mac_addr[6] = {255, 127, 63, 31, 15, 7};

  nlohmann::json json;
  MacAddrToJson(json, mac_addr);

  if (dump_json_) {
    std::cout << json.dump() << std::endl;
  }
}

TEST_F(EncodeAddrTest, can_encode_ipv4_address) {
  constexpr char IPV4_ADDR[] = "192.168.17.5";
  constexpr int PREFIX_LEN = 32;

  struct p4_ipaddr addr = {0};
  addr.family = AF_INET;
  addr.prefix_len = PREFIX_LEN;
  ASSERT_EQ(inet_pton(AF_INET, IPV4_ADDR, &addr.ip.v4addr), 1);

  nlohmann::json json;
  IpAddrToJson(json, addr);

  if (dump_json_) {
    std::cout << json.dump() << std::endl;
  }

  // Widen the uint8_t fields to int to make it clear they're not chars.
  ASSERT_TRUE(json["family"].is_number());
  auto family = json["family"].template get<int>();
  ASSERT_EQ(family, AF_INET);

  ASSERT_TRUE(json["prefix_len"].is_number());
  auto prefix_len = json["prefix_len"].template get<int>();
  EXPECT_EQ(prefix_len, PREFIX_LEN);

  ASSERT_TRUE(json["ipv4_addr"][0].is_number());
  auto ipv4_addr = json["ipv4_addr"][0].template get<uint32_t>();
  ASSERT_EQ(ipv4_addr, addr.ip.v4addr.s_addr);
}

TEST_F(EncodeAddrTest, can_encode_ipv6_address) {
  constexpr char IPV6_ADDR[] = "fe80::215:5dff:fe59:a7e1";
  constexpr int PREFIX_LEN = 64;

  struct p4_ipaddr addr = {0};
  addr.family = AF_INET6;
  addr.prefix_len = 64;
  ASSERT_EQ(inet_pton(AF_INET6, IPV6_ADDR, &addr.ip.v6addr), 1);

  nlohmann::json json;
  IpAddrToJson(json, addr);

  if (dump_json_) {
    std::cout << json.dump() << std::endl;
  }

  // Widen the uint8_t fields to int to make it clear they're not chars.
  ASSERT_TRUE(json["family"].is_number());
  auto family = json["family"].template get<int>();
  ASSERT_EQ(family, AF_INET6);

  ASSERT_TRUE(json["prefix_len"].is_number());
  auto prefix_len = json["prefix_len"].template get<int>();
  EXPECT_EQ(prefix_len, PREFIX_LEN);

  for (int i = 0; i < 8; i++) {
    auto json_word = json["ipv6_addr"][i].template get<uint16_t>();
    auto addr_word = addr.ip.v6addr.__in6_u.__u6_addr16[i];
    EXPECT_EQ(addr_word, json_word)
        << "ipv6_addr[" << i << "] does not match" << std::hex << '\n'
        << "  addr_word (expected): 0x" << addr_word << '\n'
        << "  json_word (actual)  : 0x" << json_word << std::dec << '\n';
  }
}

}  // namespace ovsp4rt
