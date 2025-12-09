/*
 * Copyright 2023-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arpa/inet.h>
#include <memory.h>
#include <netlink/msg.h>

#include <vector>

#include "gtest/gtest.h"

extern "C" {
#include "switchlink_globals.h"
#include "switchlink_handlers.h"
#include "switchlink_int.h"
}

using namespace std;

switchlink_handle_t g_default_vrf_h = 2;
switchlink_handle_t g_cpu_rx_nhop_h = 3;

#define IPV4_ADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

// enum for diff operation types
enum operation_type {
  PROCESS_ECMP = 1,
  CREATE_ROUTE = 2,
  DELETE_ROUTE = 3,
};

/*
 * Result variables.
 */
struct test_results {
  // Parameter values
  switchlink_ip_addr_t dst;
  switchlink_ip_addr_t gateway;
  switchlink_handle_t intf_h;
  switchlink_handle_t ecmp_h;
  switchlink_db_nexthop_info_t nexthop_info;
  switchlink_db_ecmp_info_t ecmp_info;
  // Handler tracking
  enum operation_type opType;
};

// Value to return for the switch interface handle.
const switchlink_handle_t TEST_INTF_H1 = 0x10001;
const switchlink_handle_t TEST_INTF_H2 = 0x20002;
const switchlink_handle_t TEST_ECMP_H = 0x30003;

const uint8_t v4_SRC_PREFIX_LEN = 24;
const uint8_t v4_DST_PREFIX_LEN = 24;
const uint8_t v6_SRC_PREFIX_LEN = 64;
const uint8_t v6_DST_PREFIX_LEN = 64;

const unsigned char RTM_TOS = 1;
const unsigned char RTM_TABLE_ID = 1;
const unsigned char RTM_PROTOCOL = 1;
const unsigned char RTM_SCOPE = 1;
const unsigned char RTM_TYPE = 1;
const unsigned char RTM_FLAGS = 0x1;

const unsigned short RTNH_LEN = 8;
const unsigned char RTNH_HOPS = 2;
const unsigned char RTNH_FLAGS = 0x2;

vector<test_results> results(2);

/*
 * Dummy function for switchlink_create_route(). This function is
 * invoked by switchlink_process_route_msg() when the msgtype is
 * RTM_NEWROUTE. The actual method creates route and adds entry
 * to the database.
 *
 * Since this is a dummy method, the objective is to validate
 * the invocation of this method with correct arguments. All the
 * input params are stored in the test results structure and
 * validated by the test case.
 */
void switchlink_create_route(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* dst,
                             const switchlink_ip_addr_t* gateway,
                             switchlink_handle_t ecmp_h,
                             switchlink_handle_t intf_h) {
  struct test_results temp = {0};
  if (dst) {
    temp.dst = *dst;
  }
  if (gateway) {
    temp.gateway = *gateway;
  }
  temp.intf_h = intf_h;
  temp.ecmp_h = ecmp_h;
  temp.opType = CREATE_ROUTE;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_delete_route(). This function is
 * invoked by switchlink_process_route_msg() when the msgtype is
 * RTM_DELROUTE. The actual method deletes route and removes
 * entry from the database.
 *
 * Since this is a dummy method, the objective is to validate
 * the invocation of this method with correct arguments. All the
 * input params are stored in the test results structure and
 * validated by the test case.
 */
void switchlink_delete_route(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* dst) {
  struct test_results temp = {0};
  if (dst) {
    temp.dst = *dst;
  }
  temp.opType = DELETE_ROUTE;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_db_get_interface_info(). This function
 * is invoked by switchlink_process_route_msg() to get the intf
 * info from the database.
 *
 * Since this is a dummy method, we are passing an ifindex 1 to get valid
 * interface info.
 *
 * The actual function can also return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND
 * if it is not able to find the interface in the database.
 * That scenario is mocked by passing an ifindex of 2.
 */
switchlink_db_status_t switchlink_db_get_interface_info(
    uint32_t ifindex, switchlink_db_interface_info_t* intf_info) {
  switch (ifindex) {
    case 1:
      intf_info->intf_h = TEST_INTF_H1;
      break;
    case 2:
      intf_info->intf_h = TEST_INTF_H2;
      break;
    default:
      return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND;
  }
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Dummy function for switchlink_create_nexthop(). Returns 0 on success
 * and -1 in case of error.
 */
int switchlink_create_nexthop(switchlink_db_nexthop_info_t* nexthop_info) {
  return (nexthop_info->intf_h == TEST_INTF_H2) ? 0 : -1;
}

/*
 * Dummy function for switchlink_create_ecmp().
 */
int switchlink_create_ecmp(switchlink_db_ecmp_info_t* ecmp_info) { return 0; }

/*
 * Dummy function for switchlink_db_add_ecmp().
 */
switchlink_db_status_t switchlink_db_add_ecmp(
    switchlink_db_ecmp_info_t* ecmp_info) {
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Dummy function for switchlink_db_update_nexthop_using_by().
 */
switchlink_db_status_t switchlink_db_update_nexthop_using_by(
    switchlink_db_nexthop_info_t* nexthop_info) {
  struct test_results temp = {0};
  if (nexthop_info) {
    temp.nexthop_info = *nexthop_info;
  }
  results.push_back(temp);
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Dummy function for switchlink_db_add_nexthop().
 */
switchlink_db_status_t switchlink_db_add_nexthop(
    switchlink_db_nexthop_info_t* nexthop_info) {
  struct test_results temp = {0};
  if (nexthop_info) {
    temp.nexthop_info = *nexthop_info;
  }
  results.push_back(temp);
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Dummy function for switchlink_db_get_ecmp_info().
 */
switchlink_db_status_t switchlink_db_get_ecmp_info(
    switchlink_db_ecmp_info_t* ecmp_info) {
  ecmp_info->ecmp_h = TEST_ECMP_H;
  if (ecmp_info) {
    results[0].ecmp_info = *ecmp_info;
  }
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Dummy function for switchlink_db_get_nexthop_info().
 * Returns success for intf handle: TEST_INTF_H1
 * Returns failure for intf handle: TEST_INTF_H2
 */
switchlink_db_status_t switchlink_db_get_nexthop_info(
    switchlink_db_nexthop_info_t* nexthop_info) {
  if (nexthop_info->intf_h == TEST_INTF_H1) {
    nexthop_info->nhop_h = 1;
    return SWITCHLINK_DB_STATUS_SUCCESS;
  } else if (nexthop_info->intf_h == TEST_INTF_H2) {
    nexthop_info->nhop_h = 2;
    return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND;
  }

  return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND;
}

/*
 * Test fixture.
 */
class SwitchlinkRouteTest : public ::testing::Test {
 protected:
  struct nl_msg* nlmsg_ = nullptr;

  // Sets up the test fixture.
  void SetUp() override { ResetVariables(); }

  // Tears down the test fixture.
  void TearDown() override {
    if (nlmsg_) {
      nlmsg_free(nlmsg_);
      nlmsg_ = nullptr;
    }
  }

  void ResetVariables() {
    // result variables
    results.clear();
  }
};

/*
 * Creates an IPv4 route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_NEWROUTE message, invokes switchlink_create_route()
 * for creating route entry.
 *
 */
TEST_F(SwitchlinkRouteTest, createIPv4Route) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET,
      .rtm_dst_len = v4_DST_PREFIX_LEN,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  const uint32_t src_addr = IPV4_ADDR(10, 10, 10, 10);
  const uint32_t dst_addr = IPV4_ADDR(20, 20, 20, 20);
  const uint32_t gateway_addr = IPV4_ADDR(20, 20, 20, 1);
  const uint32_t oifindex = 1;
  const uint32_t iifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, RTA_SRC, htonl(src_addr));
  nla_put_u32(nlmsg_, RTA_DST, htonl(dst_addr));
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr));
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);
  nla_put_u32(nlmsg_, RTA_IIF, iifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for ROUTE creation
  EXPECT_EQ(results[0].opType, CREATE_ROUTE);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H1);
  EXPECT_EQ(results[0].dst.family, AF_INET);
  EXPECT_EQ(results[0].dst.ip.v4addr.s_addr, dst_addr);
  EXPECT_EQ(results[0].dst.prefix_len, v4_DST_PREFIX_LEN);
  EXPECT_EQ(results[0].gateway.family, AF_INET);
  EXPECT_EQ(results[0].gateway.ip.v4addr.s_addr, gateway_addr);
  EXPECT_EQ(results[0].gateway.prefix_len, 32);
}

/*
 * Validates switchlink_process_route_msg() for an invalid
 * interface. The function simply returns without invoking
 * switchlink_route_create() method and the test results is
 * expected to be empty.
 */
TEST_F(SwitchlinkRouteTest, invalidInterface) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET,
      .rtm_dst_len = v4_DST_PREFIX_LEN,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  const uint32_t src_addr = IPV4_ADDR(10, 10, 10, 10);
  const uint32_t dst_addr = IPV4_ADDR(20, 20, 20, 20);
  const uint32_t gateway_addr = IPV4_ADDR(20, 20, 20, 1);
  const uint32_t oifindex = 3;
  const uint32_t iifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, RTA_SRC, htonl(src_addr));
  nla_put_u32(nlmsg_, RTA_DST, htonl(dst_addr));
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr));
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);
  nla_put_u32(nlmsg_, RTA_IIF, iifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 0);
}

/*
 * Creates an IPv4 route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_NEWROUTE message, invokes switchlink_create_route()
 * for creating route entry.
 *
 * The destination prefix len and addr is 0.
 */
TEST_F(SwitchlinkRouteTest, zeroDestPrefixLen) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET,
      .rtm_dst_len = 0,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  const uint32_t src_addr = IPV4_ADDR(10, 10, 10, 10);
  const uint32_t dst_addr = IPV4_ADDR(20, 20, 20, 20);
  const uint32_t gateway_addr = IPV4_ADDR(20, 20, 20, 1);
  const uint32_t oifindex = 1;
  const uint32_t iifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, RTA_SRC, htonl(src_addr));
  nla_put_u32(nlmsg_, RTA_DST, htonl(dst_addr));
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr));
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);
  nla_put_u32(nlmsg_, RTA_IIF, iifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for ROUTE creation
  EXPECT_EQ(results[0].opType, CREATE_ROUTE);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H1);
  EXPECT_EQ(results[0].dst.family, AF_INET);
  EXPECT_EQ(results[0].dst.ip.v4addr.s_addr, 0);
  EXPECT_EQ(results[0].dst.prefix_len, 0);
  EXPECT_EQ(results[0].gateway.family, AF_INET);
  EXPECT_EQ(results[0].gateway.ip.v4addr.s_addr, gateway_addr);
  EXPECT_EQ(results[0].gateway.prefix_len, 32);
}

/*
 * Validates switchlink_process_route_msg() for an unspecified
 * address family. The function simply returns without invoking
 * switchlink_route_create() method and the test results is
 * expected to be empty.
 */
TEST_F(SwitchlinkRouteTest, verifyUnspecifiedAddressFamily) {
  struct rtmsg hdr = {
      .rtm_family = AF_UNSPEC,
      .rtm_dst_len = v4_DST_PREFIX_LEN,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  const uint32_t src_addr = IPV4_ADDR(10, 10, 10, 10);
  const uint32_t dst_addr = IPV4_ADDR(20, 20, 20, 20);
  const uint32_t gateway_addr = IPV4_ADDR(20, 20, 20, 1);
  const uint32_t oifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, RTA_SRC, htonl(src_addr));
  nla_put_u32(nlmsg_, RTA_DST, htonl(dst_addr));
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr));
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 0);
}

/*
 * Creates an IPv6 route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_NEWROUTE message, invokes switchlink_create_route()
 * for creating route entry.
 *
 */
TEST_F(SwitchlinkRouteTest, createIPv6Route) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET6,
      .rtm_dst_len = v6_DST_PREFIX_LEN,
      .rtm_src_len = v6_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  struct in6_addr gateway_addr6;
  struct in6_addr dst_addr6;
  struct in6_addr src_addr6;

  inet_pton(AF_INET6, "1001::1", &src_addr6);

  inet_pton(AF_INET6, "2001::1", &gateway_addr6);
  // Word 0 of IPv6 address.
  const uint16_t GATEWAY_V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t GATEWAY_V6ADDR_7 = htons(0x0001);

  inet_pton(AF_INET6, "2001::2", &dst_addr6);
  // Word 0 of IPv6 address.
  const uint16_t DST_V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t DST_V6ADDR_7 = htons(0x0002);

  const uint32_t oifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put(nlmsg_, RTA_SRC, sizeof(src_addr6), &src_addr6);
  nla_put(nlmsg_, RTA_DST, sizeof(dst_addr6), &dst_addr6);
  nla_put(nlmsg_, RTA_GATEWAY, sizeof(gateway_addr6), &gateway_addr6);
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for ROUTE creation
  EXPECT_EQ(results[0].opType, CREATE_ROUTE);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H1);
  EXPECT_EQ(results[0].dst.family, AF_INET6);
  EXPECT_EQ(results[0].dst.ip.v6addr.__in6_u.__u6_addr16[0], DST_V6ADDR_0);
  EXPECT_EQ(results[0].dst.ip.v6addr.__in6_u.__u6_addr16[7], DST_V6ADDR_7);
  EXPECT_EQ(results[0].dst.prefix_len, v6_DST_PREFIX_LEN);
  EXPECT_EQ(results[0].gateway.family, AF_INET6);
  EXPECT_EQ(results[0].gateway.ip.v6addr.__in6_u.__u6_addr16[0],
            GATEWAY_V6ADDR_0);
  EXPECT_EQ(results[0].gateway.ip.v6addr.__in6_u.__u6_addr16[7],
            GATEWAY_V6ADDR_7);
  EXPECT_EQ(results[0].gateway.prefix_len, 128);
}

/*
 * Deletes an IPv4 route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_DELROUTE message, invokes switchlink_delete_route()
 * for deleting route entry.
 *
 */
TEST_F(SwitchlinkRouteTest, deleteIPv4Route) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET,
      .rtm_dst_len = v4_DST_PREFIX_LEN,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  const uint32_t src_addr = IPV4_ADDR(10, 10, 10, 10);
  const uint32_t dst_addr = IPV4_ADDR(20, 20, 20, 20);
  const uint32_t gateway_addr = IPV4_ADDR(20, 20, 20, 1);
  const uint32_t oifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, RTA_SRC, htonl(src_addr));
  nla_put_u32(nlmsg_, RTA_DST, htonl(dst_addr));
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr));
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for ROUTE creation
  EXPECT_EQ(results[0].opType, DELETE_ROUTE);
  EXPECT_EQ(results[0].dst.family, AF_INET);
  EXPECT_EQ(results[0].dst.ip.v4addr.s_addr, dst_addr);
  EXPECT_EQ(results[0].dst.prefix_len, v4_DST_PREFIX_LEN);
}

/*
 * Deletes an IPv6 route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_DELROUTE message, invokes switchlink_delete_route()
 * for deleting route entry.
 *
 */
TEST_F(SwitchlinkRouteTest, deleteIPv6Route) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET6,
      .rtm_dst_len = v6_DST_PREFIX_LEN,
      .rtm_src_len = v6_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  struct in6_addr gateway_addr6;
  struct in6_addr dst_addr6;
  struct in6_addr src_addr6;

  inet_pton(AF_INET6, "1001::1", &src_addr6);

  inet_pton(AF_INET6, "2001::1", &gateway_addr6);
  // Word 0 of IPv6 address.
  // const uint16_t GATEWAY_V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  // const uint16_t GATEWAY_V6ADDR_7 = htons(0x0001);

  inet_pton(AF_INET6, "2001::2", &dst_addr6);
  // Word 0 of IPv6 address.
  const uint16_t DST_V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t DST_V6ADDR_7 = htons(0x0002);

  const uint32_t oifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put(nlmsg_, RTA_SRC, sizeof(src_addr6), &src_addr6);
  nla_put(nlmsg_, RTA_DST, sizeof(dst_addr6), &dst_addr6);
  nla_put(nlmsg_, RTA_GATEWAY, sizeof(gateway_addr6), &gateway_addr6);
  nla_put_u32(nlmsg_, RTA_OIF, oifindex);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for ROUTE creation
  EXPECT_EQ(results[0].opType, DELETE_ROUTE);
  EXPECT_EQ(results[0].dst.family, AF_INET6);
  EXPECT_EQ(results[0].dst.ip.v6addr.__in6_u.__u6_addr16[0], DST_V6ADDR_0);
  EXPECT_EQ(results[0].dst.ip.v6addr.__in6_u.__u6_addr16[7], DST_V6ADDR_7);
  EXPECT_EQ(results[0].dst.prefix_len, v6_DST_PREFIX_LEN);
}

/*
 * Creates an IPv4 ECMP route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_NEWROUTE message, invokes switchlink_create_route()
 * for creating route entry.
 *
 * It invokes a internal call to process_ecmp() method,
 * where nexthop_info is updated.
 *
 */
TEST_F(SwitchlinkRouteTest, processEcmpUpdatev4Nexthop) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET,
      .rtm_dst_len = v4_DST_PREFIX_LEN,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  struct rtnexthop rt_nh = {
      .rtnh_len = RTNH_LEN,
      .rtnh_flags = RTNH_FLAGS,
      .rtnh_hops = RTNH_HOPS,
      .rtnh_ifindex = 1,
  };

  const uint32_t gateway_addr1 = IPV4_ADDR(20, 20, 20, 1);
  // const uint32_t oifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);

  nla_put(nlmsg_, RTA_MULTIPATH, sizeof(rt_nh), &rt_nh);
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr1));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 2);
  EXPECT_EQ(results[0].nexthop_info.intf_h, TEST_INTF_H1);
  EXPECT_EQ(results[0].nexthop_info.vrf_h, g_default_vrf_h);
  EXPECT_EQ(results[0].nexthop_info.ip_addr.family, AF_INET);
  EXPECT_EQ(results[0].nexthop_info.ip_addr.ip.v4addr.s_addr, gateway_addr1);
  EXPECT_EQ(results[0].nexthop_info.ip_addr.prefix_len, 32);
  EXPECT_EQ(results[0].nexthop_info.using_by, SWITCHLINK_NHOP_FROM_ROUTE);
  EXPECT_EQ(results[1].ecmp_h, TEST_ECMP_H);
}

/*
 * Creates an IPv4 ECMP route
 *
 * Validates switchlink_process_route_msg(). It parses an
 * RTM_NEWROUTE message, invokes switchlink_create_route()
 * for creating route entry.
 *
 * It invokes a internal call to process_ecmp() method,
 * where nexthop_info is created.
 *
 */
TEST_F(SwitchlinkRouteTest, processEcmpCreatev4Nexthop) {
  struct rtmsg hdr = {
      .rtm_family = AF_INET,
      .rtm_dst_len = v4_DST_PREFIX_LEN,
      .rtm_src_len = v4_SRC_PREFIX_LEN,
      .rtm_tos = RTM_TOS,
      .rtm_table = RTM_TABLE_ID,
      .rtm_protocol = RTM_PROTOCOL,
      .rtm_scope = RTM_SCOPE,
      .rtm_type = RTM_TYPE,
      .rtm_flags = RTM_FLAGS,
  };

  struct rtnexthop rt_nh = {
      .rtnh_len = RTNH_LEN,
      .rtnh_flags = RTNH_FLAGS,
      .rtnh_hops = RTNH_HOPS,
      .rtnh_ifindex = 2,
  };

  const uint32_t gateway_addr1 = IPV4_ADDR(20, 20, 20, 1);
  // const uint32_t oifindex = 1;

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWROUTE, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);

  nla_put(nlmsg_, RTA_MULTIPATH, sizeof(rt_nh), &rt_nh);
  nla_put_u32(nlmsg_, RTA_GATEWAY, htonl(gateway_addr1));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 2);
  EXPECT_EQ(results[0].nexthop_info.intf_h, TEST_INTF_H2);
  EXPECT_EQ(results[0].nexthop_info.vrf_h, g_default_vrf_h);
  EXPECT_EQ(results[0].nexthop_info.ip_addr.family, AF_INET);
  EXPECT_EQ(results[0].nexthop_info.ip_addr.ip.v4addr.s_addr, gateway_addr1);
  EXPECT_EQ(results[0].nexthop_info.ip_addr.prefix_len, 32);
  EXPECT_EQ(results[0].nexthop_info.using_by, SWITCHLINK_NHOP_FROM_ROUTE);
  EXPECT_EQ(results[0].ecmp_info.nhops[0], 2);
  EXPECT_EQ(results[1].ecmp_h, TEST_ECMP_H);
}
