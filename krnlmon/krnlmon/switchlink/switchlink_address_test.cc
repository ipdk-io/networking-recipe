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
#include <linux/if_arp.h>
#include <memory.h>
#include <netlink/msg.h>

#include <vector>

#include "gtest/gtest.h"

extern "C" {
#include "switchlink_handlers.h"
#include "switchlink_int.h"
}

using namespace std;

#define IPV4_ADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

// enum for diff operation types
enum operation_type {
  ADD_ADDRESS = 1,
  DELETE_ADDRESS = 2,
};

/*
 * Result variables.
 */
struct test_results {
  // Parameter values
  switchlink_ip_addr_t addr;
  switchlink_ip_addr_t gateway;
  switchlink_handle_t intf_h;
  // Handler tracking
  enum operation_type opType;
  int num_handler_calls;
};

// Value to return for the switch interface handle.
const switchlink_handle_t TEST_INTF_H = 0x10001;

vector<test_results> results(2);

/*
 * Dummy function for switchlink_create_route(). This function is
 * invoked by switchlink_process_address_msg() when the msgtype is
 * RTM_NEWADDR for both IPv4 and IPv6 type of addresses. The actual
 * method creates route and adds entry to the database. Since this is
 * dummy method, here the objective is to validate the invocation of
 * this method with correct arguments. All the input params are stored
 * in the test results structure and validated against each test case.
 */
void switchlink_create_route(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* addr,
                             const switchlink_ip_addr_t* gateway,
                             switchlink_handle_t ecmp_h,
                             switchlink_handle_t intf_h) {
  struct test_results temp = {};
  if (addr) {
    temp.addr = *addr;
  }
  if (gateway) {
    temp.gateway = *gateway;
  }
  if (intf_h) {
    temp.intf_h = intf_h;
  }
  temp.opType = ADD_ADDRESS;
  temp.num_handler_calls++;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_delete_route(). This function is
 * invoked by switchlink_process_address_msg() when the msgtype is
 * RTM_DELADDR for both IPv4 and IPv6 type of addresses. The actual
 * method deletes route and removes entry from the database. Since this
 * is dummy method, here the objective is to validate the invocation of
 * this method with correct arguments. All the input params are stored
 * in the test results structure and validated against each test case.
 */
void switchlink_delete_route(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* addr) {
  struct test_results temp = {};
  if (addr) {
    temp.addr = *addr;
  }
  temp.opType = DELETE_ADDRESS;
  temp.num_handler_calls++;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_db_get_interface_info(). This function
 * is invoked by switchlink_process_address_msg() for getting the intf
 * info from the database. Since this is a dummy method, we are passing
 * an ifindex 1 to this method and expects it to return an intf_info
 * successfully with ifhandle being 0x10001. The actual function can also
 * return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND in case it is not able to
 * find the interface in the database. That scenario is mocked by passing
 * an ifindex of 2.
 */
switchlink_db_status_t switchlink_db_get_interface_info(
    uint32_t ifindex, switchlink_db_interface_info_t* intf_info) {
  if (ifindex == 1) {
    intf_info->intf_h = TEST_INTF_H;
  } else if (ifindex == 2) {
    intf_info = nullptr;
    return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND;
  }
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Test fixture.
 */
class SwitchlinkAddressTest : public ::testing::Test {
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
 * Validates switchlink_process_address_msg(). It parses an
 * RTM_NEWADDR message, which contains an IPv4 address, and
 * invokes switchlink_create_route() with the correct attributes.
 *
 * We invoke switchlink_create_route() twice...
 * ...once with the subnet mask...
 * ...and once with a /32 prefix mask.
 * Hence we expect 2 test_results here, and this is the
 * reason why test_results has been taken as a vector of size 2.
 */
TEST_F(SwitchlinkAddressTest, addIpv4Address) {
  struct ifaddrmsg hdr = {
      .ifa_family = AF_INET,
      .ifa_prefixlen = 24,
      .ifa_index = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWADDR, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, IFA_ADDRESS, htonl(ipv4_addr));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 2);
  for (size_t i = 0; i < results.size(); i++) {
    EXPECT_EQ(results[i].num_handler_calls, 1);
    EXPECT_EQ(results[i].opType, ADD_ADDRESS);
    EXPECT_EQ(results[i].addr.family, AF_INET);
    EXPECT_EQ(results[i].addr.ip.v4addr.s_addr, ipv4_addr);
    EXPECT_EQ(results[i].gateway.family, AF_INET);
    EXPECT_EQ(results[i].intf_h, TEST_INTF_H);
    const int prefix_len = (i == 0) ? hdr.ifa_prefixlen : 32;
    EXPECT_EQ(results[i].addr.prefix_len, prefix_len);
  }
}

/*
 * Deletes an IPv4 route
 *
 * Validates switchlink_process_address_msg(). It parses an
 * RTM_DELADDR message, which contains an IPv4 address, and
 * invokes switchlink_delete_route() with the correct attributes.
 *
 * We invoke switchlink_delete_route() twice...
 * ...once with the subnet mask...
 * ...and once with a /32 prefix mask.
 * Hence we expect 2 test_results here, and this is the
 * reason why test_results has been taken as a vector of size 2.
 */
TEST_F(SwitchlinkAddressTest, deleteIpv4Address) {
  struct ifaddrmsg hdr = {
      .ifa_family = AF_INET,
      .ifa_prefixlen = 24,
      .ifa_index = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELADDR, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, IFA_ADDRESS, htonl(ipv4_addr));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 2);
  for (size_t i = 0; i < results.size(); i++) {
    EXPECT_EQ(results[i].num_handler_calls, 1);
    EXPECT_EQ(results[i].opType, DELETE_ADDRESS);
    EXPECT_EQ(results[i].addr.family, AF_INET);
    EXPECT_EQ(results[i].addr.ip.v4addr.s_addr, ipv4_addr);
    const int prefix_len = (i == 0) ? hdr.ifa_prefixlen : 32;
    EXPECT_EQ(results[i].addr.prefix_len, prefix_len);
  }
}

/*
 * Creates an IPv6 route
 *
 * Validates switchlink_process_address_msg(). It parses an
 * RTM_NEWADDR message, which contains an IPv6 address, and
 *  invokes switchlink_create_route() with the correct attributes.
 *
 * We invoke switchlink_create_route() twice...
 * ...once with the subnet mask...
 * ...and once with a /128 prefix mask.
 * Hence we expect 2 test_results here, and this is the
 * reason why test_results has been taken as a vector of size 2.
 */
TEST_F(SwitchlinkAddressTest, addIpv6Address) {
  struct ifaddrmsg hdr = {
      .ifa_family = AF_INET6,
      .ifa_prefixlen = 64,
      .ifa_index = 1,
  };

  struct in6_addr addr6;
  inet_pton(AF_INET6, "2001::1", &addr6);
  // Word 0 of IPv6 address.
  const uint16_t V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t V6ADDR_7 = htons(0x0001);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWADDR, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put(nlmsg_, IFA_ADDRESS, sizeof(addr6), &addr6);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 2);
  for (size_t i = 0; i < results.size(); i++) {
    EXPECT_EQ(results[i].num_handler_calls, 1);
    EXPECT_EQ(results[i].opType, ADD_ADDRESS);
    EXPECT_EQ(results[i].addr.family, AF_INET6);
    EXPECT_EQ(results[i].gateway.family, AF_INET6);
    EXPECT_EQ(results[i].intf_h, TEST_INTF_H);
    EXPECT_EQ(results[i].addr.ip.v6addr.__in6_u.__u6_addr16[0], V6ADDR_0);
    EXPECT_EQ(results[i].addr.ip.v6addr.__in6_u.__u6_addr16[7], V6ADDR_7);
    const int prefix_len = (i == 0) ? hdr.ifa_prefixlen : 128;
    EXPECT_EQ(results[i].addr.prefix_len, prefix_len);
  }
}

/*
 * Deletes an IPv6 route
 *
 * Validates switchlink_process_address_msg(). It parses an
 * RTM_DELADDR message, which contains an IPv6 address, and
 * invokes switchlink_delete_route() with the correct attributes.
 *
 * We invoke switchlink_delete_route() twice...
 * ...once with the subnet mask...
 * ...and once with a /128 prefix mask.
 * Hence we expect 2 test_results here, and this is the
 * reason why test_results has been taken as a vector of size 2.
 */
TEST_F(SwitchlinkAddressTest, deleteIpv6Address) {
  struct ifaddrmsg hdr = {
      .ifa_family = AF_INET6,
      .ifa_prefixlen = 64,
      .ifa_index = 1,
  };

  struct in6_addr addr6;
  inet_pton(AF_INET6, "2001::1", &addr6);
  // Word 0 of IPv6 address.
  const uint16_t V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t V6ADDR_7 = htons(0x0001);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELADDR, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put(nlmsg_, IFA_ADDRESS, sizeof(addr6), &addr6);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 2);
  for (size_t i = 0; i < results.size(); i++) {
    EXPECT_EQ(results[i].num_handler_calls, 1);
    EXPECT_EQ(results[i].opType, DELETE_ADDRESS);
    EXPECT_EQ(results[i].addr.family, AF_INET6);
    EXPECT_EQ(results[i].addr.ip.v6addr.__in6_u.__u6_addr16[0], V6ADDR_0);
    EXPECT_EQ(results[i].addr.ip.v6addr.__in6_u.__u6_addr16[7], V6ADDR_7);
    const int prefix_len = (i == 0) ? hdr.ifa_prefixlen : 128;
    EXPECT_EQ(results[i].addr.prefix_len, prefix_len);
  }
}

/*
 * Validates switchlink_process_address_msg() in case wrong
 * address family is passed in the address message header.
 * The function will simply return if the address family
 * doesn't belong to either AF_INET or AF_INET6. In that case,
 * the test results size will be 0.
 */
TEST_F(SwitchlinkAddressTest, wrongAddressFamilyTest) {
  struct ifaddrmsg hdr = {
      .ifa_family = AF_UNIX,
      .ifa_prefixlen = 24,
      .ifa_index = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWADDR, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, IFA_ADDRESS, htonl(ipv4_addr));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 0);
}

/*
 * Validates switchlink_process_address_msg() in case
 * switchlink_db_get_interface_info() is not able to
 * successfully fetch the interface info from the database
 * and returns SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND error.
 * The test results vector size will be 0.
 */
TEST_F(SwitchlinkAddressTest, interfaceNotFoundTest) {
  struct ifaddrmsg hdr = {
      .ifa_family = AF_INET,
      .ifa_prefixlen = 24,
      .ifa_index = 2,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWADDR, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, IFA_ADDRESS, htonl(ipv4_addr));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  EXPECT_EQ(results.size(), 0);
}
