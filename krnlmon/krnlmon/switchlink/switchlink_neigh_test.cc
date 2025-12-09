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
#include "switchlink_handlers.h"
#include "switchlink_int.h"
}

using namespace std;

#define IPV4_ADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

// enum for diff operation types
enum operation_type {
  CREATE_MAC = 1,
  CREATE_NEIGHBOR = 2,
  DELETE_NEIGHBOR = 3,
};

/*
 * Result variables.
 */
struct test_results {
  // Parameter values
  switchlink_mac_addr_t mac_addr;
  switchlink_ip_addr_t ipaddr;
  switchlink_handle_t bridge_h;
  switchlink_handle_t intf_h;
  switchlink_handle_t vrf_h;
  // Handler tracking
  enum operation_type opType;
};

// Value to return for the switch interface and bridge handle.
const switchlink_handle_t TEST_INTF_H = 0x10001;
const switchlink_handle_t TEST_BRIDGE_H = 0x20002;

vector<test_results> results(2);

/*
 * Dummy function for switchlink_create_mac(). This function is
 * invoked by switchlink_process_neigh_msg() when the msgtype is
 * RTM_NEWNEIGH. The actual method creates a MAC entry.
 *
 * Since this is a dummy method, the objective is to validate
 * the invocation of this method with correct arguments. All the
 * input params are stored in the test results structure and
 * validated by the test case.
 */
void switchlink_create_mac(switchlink_mac_addr_t mac_addr,
                           switchlink_handle_t bridge_h,
                           switchlink_handle_t intf_h) {
  struct test_results temp = {0};
  memcpy(temp.mac_addr, mac_addr, sizeof(switchlink_mac_addr_t));
  temp.bridge_h = bridge_h;
  temp.intf_h = intf_h;
  temp.opType = CREATE_MAC;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_create_neigh(). This function is
 * invoked by switchlink_process_neigh_msg() when the msgtype is
 * RTM_NEWNEIGH. The actual method creates neighbor, nexthop and
 * route entry.
 *
 * Since this is a dummy method, the objective is to validate
 * the invocation of this method with correct arguments. All the
 * input params are stored in the test results structure and
 * validated by the test case.
 */
void switchlink_create_neigh(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* ipaddr,
                             switchlink_mac_addr_t mac_addr,
                             switchlink_handle_t intf_h) {
  struct test_results temp = {0};
  if (ipaddr) {
    temp.ipaddr = *ipaddr;
  }
  memcpy(temp.mac_addr, mac_addr, sizeof(switchlink_mac_addr_t));
  temp.intf_h = intf_h;
  temp.opType = CREATE_NEIGHBOR;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_delete_neigh(). This function is
 * invoked by switchlink_process_neigh_msg() when the msgtype is
 * RTM_DELNEIGH. The actual method deletes neighbor, nexthop and
 * route entry.
 *
 * Since this is a dummy method, the objective is to validate
 * the invocation of this method with correct arguments. All the
 * input params are stored in the test results structure and
 * validated by the test case.
 */
void switchlink_delete_neigh(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* addr,
                             switchlink_handle_t intf_h) {
  struct test_results temp = {0};
  if (addr) {
    temp.ipaddr = *addr;
  }
  temp.intf_h = intf_h;
  temp.opType = DELETE_NEIGHBOR;
  results.push_back(temp);
}

/*
 * Dummy function for switchlink_db_get_interface_info(). This function
 * is invoked by switchlink_process_neigh_msg() to get the intf
 * info from the database.
 *
 * Since this is a dummy method, we are passing an ifindex 1 to get L3
 * interface info and ifindex 2 to get L2 interface info from the db.
 *
 * The actual function can also return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND
 * if it is not able to find the interface in the database.
 * That scenario is handled as the default case.
 */
switchlink_db_status_t switchlink_db_get_interface_info(
    uint32_t ifindex, switchlink_db_interface_info_t* intf_info) {
  switch (ifindex) {
    case 1:
      intf_info->intf_h = TEST_INTF_H;
      intf_info->intf_type = SWITCHLINK_INTF_TYPE_L3;
      break;
    case 2:
      intf_info->intf_h = TEST_INTF_H;
      intf_info->bridge_h = TEST_BRIDGE_H;
      intf_info->intf_type = SWITCHLINK_INTF_TYPE_L2_ACCESS;
      break;
    default:
      return SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND;
  }
  return SWITCHLINK_DB_STATUS_SUCCESS;
}

/*
 * Test fixture.
 */
class SwitchlinkNeighborTest : public ::testing::Test {
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
 * Creates an IPv4 neighbor
 *
 * Validates switchlink_process_neigh_msg(). It parses an
 * RTM_NEWNEIGH message, invokes switchlink_create_neigh()
 * for creating neighbor, nexthop and route entry.
 *
 * In create call...
 * We invoke switchlink_create_mac()...
 * ...and switchlink_create_neigh()....
 * Hence we expect 2 test_results here, and this is the
 * reason why test_results is a vector of size 2.
 */
TEST_F(SwitchlinkNeighborTest, createIPv4Neighbor) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET,
      .ndm_ifindex = 1,
      .ndm_state = NUD_REACHABLE,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);
  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, NDA_DST, htonl(ipv4_addr));
  nla_put(nlmsg_, NDA_LLADDR, sizeof(mac_addr), &mac_addr);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for NEIGHBOR creation
  EXPECT_EQ(results[0].opType, CREATE_NEIGHBOR);
  EXPECT_EQ(results[0].vrf_h, 0);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H);
  EXPECT_EQ(results[0].ipaddr.family, AF_INET);
  EXPECT_EQ(results[0].ipaddr.ip.v4addr.s_addr, ipv4_addr);
  EXPECT_EQ(results[0].ipaddr.prefix_len, 32);
  EXPECT_EQ(memcmp(results[0].mac_addr, mac_addr, sizeof(mac_addr)), 0);
}

/*
 * Verifies the behavior of switchlink_process_neigh_msg() when
 * invalid neighbor state (NUD_INCOMPLETE) is passed in the header.
 */
TEST_F(SwitchlinkNeighborTest, verifyInvalidNeighborState) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET,
      .ndm_ifindex = 1,
      .ndm_state = NUD_INCOMPLETE,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);
  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, NDA_DST, htonl(ipv4_addr));
  nla_put(nlmsg_, NDA_LLADDR, sizeof(mac_addr), &mac_addr);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 0);
}

/*
 * Verifies the behavior of switchlink_process_neigh_msg() when
 * switchlink_db_get_interface_info() isn't able to fetch valid
 * interface info from the database.
 */
TEST_F(SwitchlinkNeighborTest, verifyInvalidInterfaceInfo) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET,
      .ndm_ifindex = 3,
      .ndm_state = NUD_INCOMPLETE,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);
  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, NDA_DST, htonl(ipv4_addr));
  nla_put(nlmsg_, NDA_LLADDR, sizeof(mac_addr), &mac_addr);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 0);
}

/*
 * Tries to create an IPv4 neighbor without any MAC address info
 *
 * Verifies switchlink_process_neigh_msg(). It parses an
 * RTM_NEWNEIGH message with valid ipv4 address but an invalid/null
 * MAC address
 *
 * In this case, switchlink_delete_neigh() will be invoked to
 * remove the neighbor entry even though the message type is
 * RTM_NEWNEIGH.
 */
TEST_F(SwitchlinkNeighborTest, createIPv4NeighborWithInvalidMac) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET,
      .ndm_ifindex = 1,
      .ndm_state = NUD_REACHABLE,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, NDA_DST, htonl(ipv4_addr));

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for NEIGHBOR deletion
  EXPECT_EQ(results[0].opType, DELETE_NEIGHBOR);
  EXPECT_EQ(results[0].vrf_h, 0);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H);
  EXPECT_EQ(results[0].ipaddr.family, AF_INET);
  EXPECT_EQ(results[0].ipaddr.ip.v4addr.s_addr, ipv4_addr);
  EXPECT_EQ(results[0].ipaddr.prefix_len, 32);
}

/*
 * Creates an IPv6 neighbor
 *
 * Validates switchlink_process_neigh_msg(). It parses an
 * RTM_NEWNEIGH message, invokes switchlink_create_neigh()
 * for creating neighbor, nexthop and route entry.
 *
 * In create call...
 * We invoke switchlink_create_mac()...
 * ...and switchlink_create_neigh()....
 * Hence we expect 2 test_results here, and this is the
 * reason why test_results is a vector of size 2.
 */
TEST_F(SwitchlinkNeighborTest, createIPv6Neighbor) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET6,
      .ndm_ifindex = 2,
      .ndm_state = NUD_PERMANENT,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};

  struct in6_addr addr6;
  inet_pton(AF_INET6, "2001::1", &addr6);
  // Word 0 of IPv6 address.
  const uint16_t V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t V6ADDR_7 = htons(0x0001);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put(nlmsg_, NDA_DST, sizeof(addr6), &addr6);
  nla_put(nlmsg_, NDA_LLADDR, sizeof(mac_addr), &mac_addr);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for NEIGHBOR creation
  EXPECT_EQ(results[0].opType, CREATE_NEIGHBOR);
  EXPECT_EQ(results[0].vrf_h, 0);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H);
  EXPECT_EQ(results[0].ipaddr.family, AF_INET6);
  EXPECT_EQ(results[0].ipaddr.ip.v6addr.__in6_u.__u6_addr16[0], V6ADDR_0);
  EXPECT_EQ(results[0].ipaddr.ip.v6addr.__in6_u.__u6_addr16[7], V6ADDR_7);
  EXPECT_EQ(results[0].ipaddr.prefix_len, 128);
  EXPECT_EQ(memcmp(results[0].mac_addr, mac_addr, sizeof(mac_addr)), 0);
}

/*
 * Deletes an IPv4 neighbor
 *
 * Validates switchlink_process_neigh_msg(). It parses an
 * RTM_DELNEIGH message, invokes switchlink_delete_neigh()
 * for deleting neighbor, nexthop and route entry.
 *
 * In delete call...
 * We invoke switchlink_delete_neigh().
 */
TEST_F(SwitchlinkNeighborTest, deleteIPv4Neighbor) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET,
      .ndm_ifindex = 1,
      .ndm_state = NUD_STALE,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const uint32_t ipv4_addr = IPV4_ADDR(10, 10, 10, 1);
  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_u32(nlmsg_, NDA_DST, htonl(ipv4_addr));
  nla_put(nlmsg_, NDA_LLADDR, sizeof(mac_addr), &mac_addr);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for NEIGHBOR deletion
  EXPECT_EQ(results[0].opType, DELETE_NEIGHBOR);
  EXPECT_EQ(results[0].vrf_h, 0);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H);
  EXPECT_EQ(results[0].ipaddr.family, AF_INET);
  EXPECT_EQ(results[0].ipaddr.ip.v4addr.s_addr, ipv4_addr);
  EXPECT_EQ(results[0].ipaddr.prefix_len, 32);
}

/*
 * Deletes an IPv6 neighbor
 *
 * Validates switchlink_process_neigh_msg(). It parses an
 * RTM_DELNEIGH message, invokes switchlink_delete_neigh()
 * for creating neighbor, nexthop and route entry.
 *
 * In delete call...
 * We invoke switchlink_delete_neigh().
 */
TEST_F(SwitchlinkNeighborTest, deleteIPv6Neighbor) {
  struct ndmsg hdr = {
      .ndm_family = AF_INET6,
      .ndm_ifindex = 1,
      .ndm_state = NUD_REACHABLE,
      .ndm_flags = 0x1,
      .ndm_type = 1,
  };

  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};

  struct in6_addr addr6;
  inet_pton(AF_INET6, "2001::1", &addr6);
  // Word 0 of IPv6 address.
  const uint16_t V6ADDR_0 = htons(0x2001);
  // Word 7 of IPv6 address.
  const uint16_t V6ADDR_7 = htons(0x0001);

  // Arrange
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELNEIGH, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put(nlmsg_, NDA_DST, sizeof(addr6), &addr6);
  nla_put(nlmsg_, NDA_LLADDR, sizeof(mac_addr), &mac_addr);

  // Act
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

  // Assert
  ASSERT_EQ(results.size(), 1);

  // Verify test results for NEIGHBOR deletion
  EXPECT_EQ(results[0].opType, DELETE_NEIGHBOR);
  EXPECT_EQ(results[0].vrf_h, 0);
  EXPECT_EQ(results[0].intf_h, TEST_INTF_H);
  EXPECT_EQ(results[0].ipaddr.family, AF_INET6);
  EXPECT_EQ(results[0].ipaddr.ip.v6addr.__in6_u.__u6_addr16[0], V6ADDR_0);
  EXPECT_EQ(results[0].ipaddr.ip.v6addr.__in6_u.__u6_addr16[7], V6ADDR_7);
  EXPECT_EQ(results[0].ipaddr.prefix_len, 128);
}
