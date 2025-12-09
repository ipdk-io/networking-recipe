// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <linux/if_arp.h>
#include <memory.h>
#include <netlink/msg.h>

#include <vector>

#include "gtest/gtest.h"

extern "C" {
#include "krnlmon_options.h"
#include "switchlink_globals.h"
#include "switchlink_handlers.h"
#include "switchlink_int.h"
#include "switchlink_link_types.h"
}

#define IPV4_ADDR(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

/******************************************************************************
    We can't use GMock with a C program, so we supply dummy functions to
    replace the ones called by the function we're testing. This allows us
    to test the unit in isolation.

    Each dummy function stores the parameter with which it was called
    in a results structure (below), sets the handler type to an enum
    indicating which function was called, and appends the structure to
    the list of results.

    The SetUp method in the Test Fixture resets the result variables
    at the beginning of each test case, and the test case inspects the
    result variables on return from the function under test.

    NOTE: This technique only works if (1) the test cases are run
    sequentially, not in parallel, or (2) each test case is run in a
    separate process. If that's not the case, we will need to come up
    with a more creative solution (such as thread-specific storage).

    TODO: Look into using CMock instead of the clumsy methodology used
    in this test.
******************************************************************************/

enum handler_type {
  CREATE_INTERFACE = 1,         // switchlink_create_interface
  DELETE_INTERFACE = 2,         // switchlink_delete_interface
  CREATE_TUNNEL_INTERFACE = 3,  // switchlink_create_tunnel_interface
  DELETE_TUNNEL_INTERFACE = 4,  // switchlink_delete_tunnel_interface
};

/**
 * Test results structure. Records the dummy function (handler) called and
 * its parameter values.
 */
struct test_results {
  union {
    // Parameter values
    switchlink_db_tunnel_interface_info_t tunnel_info;
    switchlink_db_interface_info_t interface_info;
    uint32_t ifindex;
  };
  // Handler tracking
  enum handler_type handler;
};

/**
 * Test results vector. Contains one entry for each dummy function called
 * by the UUT.
 */
std::vector<test_results> results(2);

//----------------------------------------------------------------------
// Test doubles (dummy functions)
//----------------------------------------------------------------------

#ifdef LAG_OPTION
void switchlink_create_lag(switchlink_db_interface_info_t* lag_info) {}
void switchlink_delete_lag(uint32_t ifindex) {}
void switchlink_create_lag_member(
    switchlink_db_lag_member_info_t* lag_member_info) {}
void switchlink_delete_lag_member(uint32_t ifindex) {}
#endif

#if defined(ES2K_TARGET)
bool switchlink_validate_driver(const char* ifname) { return true; }
#endif

void switchlink_create_interface(switchlink_db_interface_info_t* intf) {
  struct test_results temp = {0};
  temp.handler = CREATE_INTERFACE;
  if (intf) {
    temp.interface_info = *intf;
  }
  results.push_back(temp);
}

void switchlink_delete_interface(uint32_t ifindex) {
  struct test_results temp = {0};
  temp.handler = DELETE_INTERFACE;
  temp.ifindex = ifindex;
  results.push_back(temp);
}

void switchlink_create_tunnel_interface(
    switchlink_db_tunnel_interface_info_t* tnl_intf) {
  struct test_results temp = {0};
  temp.handler = CREATE_TUNNEL_INTERFACE;
  if (tnl_intf) {
    temp.tunnel_info = *tnl_intf;
  }
  results.push_back(temp);
}

void switchlink_delete_tunnel_interface(uint32_t ifindex) {
  struct test_results temp = {0};
  temp.handler = DELETE_TUNNEL_INTERFACE;
  temp.ifindex = ifindex;
  results.push_back(temp);
}

// This function is not called by the UUT, but it is referenced by a
// function in the same source file. We provide a definition to avoid
// a link error.
int switchlink_create_vrf(switchlink_handle_t* vrf_h) { return 0; }

//----------------------------------------------------------------------
// Test fixture
//----------------------------------------------------------------------

class SwitchlinkTest : public ::testing::Test {
 protected:
  // Netlink message buffer.
  struct nl_msg* nlmsg_ = nullptr;

  // Sets up the test fixture.
  void SetUp() override { InitializeGlobalState(); }

  // Tears down the test fixture.
  void TearDown() override {
    if (nlmsg_) {
      nlmsg_free(nlmsg_);
      nlmsg_ = nullptr;
    }
  }

  void InitializeGlobalState() {
    // switchlink variables
    g_default_vrf_h = 0;
    g_default_bridge_h = 0;
    g_cpu_rx_nhop_h = 0;

    // results vector
    results.clear();
  }
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

/**
 * Creates a generic link.
 *
 * Verifies that switchlink_process_link_msg() parses an RTM_NEWLINK message
 * with (kind = None) and calls switchlink_create_interface() with the correct
 * attributes.
 *
 * A generic link is one that does not specify an IFLA_INFO_KIND attribute.
 * It is used for vanilla ethernet interfaces.
 *
 * When INFO_KIND is omitted, switchlink_process_link_msg() sets the
 * link_type to SWITCHLINK_LINK_TYPE_NONE.
 */
TEST_F(SwitchlinkTest, can_create_generic_link) {
  struct ifinfomsg hdr = {
      .ifi_family = 0,
      .ifi_type = ARPHRD_ETHER,
      .ifi_index = 6,
      .ifi_flags = 0x11043,
      .ifi_change = 0,
  };
  const char ifname[] = "eth0";
  const unsigned char mac_addr[6] = {0x00, 0xdd, 0xee, 0xaa, 0xdd, 0x00};
  const switchlink_handle_t vrf_h = 0xdeadbeefdeadbeefUL;

  //-------------------------------------------------------
  // Arrange: construct an RTM_NEWLINK message that does
  // not specify a Kind attribute.
  //
  // 1) Allocate netlink message buffer
  // 2) Add RTM_NEWLINK header
  // 3) Add payload header
  // 4) Add attributes
  //-------------------------------------------------------
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWLINK, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_string(nlmsg_, IFLA_IFNAME, ifname);
  nla_put(nlmsg_, IFLA_ADDRESS, sizeof(mac_addr), &mac_addr);

  g_default_vrf_h = vrf_h;

  //-------------------------------------------------------
  // Act: call UUT with message
  //-------------------------------------------------------
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);

  //-------------------------------------------------------
  // Assert: verify test case results
  //-------------------------------------------------------
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].handler, CREATE_INTERFACE);
  EXPECT_STREQ(results[0].interface_info.ifname, ifname);
  EXPECT_EQ(results[0].interface_info.ifindex, hdr.ifi_index);
  EXPECT_EQ(results[0].interface_info.vrf_h, vrf_h);
  EXPECT_EQ(
      memcmp(results[0].interface_info.mac_addr, mac_addr, sizeof(mac_addr)),
      0);
}

/**
 * Creates a vxlan link.
 *
 * Verifies that switchlink_process_link_msg() parses an RTM_NEWLINK message
 * with (kind = "vxlan") and calls switchlink_create_tunnel_interface() with
 * the correct attributes.
 */
TEST_F(SwitchlinkTest, can_create_vxlan_link) {
  struct ifinfomsg hdr = {
      .ifi_family = 0,
      .ifi_type = ARPHRD_ETHER,
      .ifi_index = 406,
      .ifi_flags = 0x11043,
      .ifi_change = 0,
  };
  const char ifname[] = "vxlan0";
  const char kind[] = "vxlan";
  const unsigned char mac_addr[6] = {0x66, 0x05, 0x90, 0xe8, 0xc7, 0x14};
  const uint32_t vxlan_id = 1;
  const uint32_t vxlan_group = IPV4_ADDR(40, 1, 1, 1);
  const uint32_t vxlan_local = IPV4_ADDR(40, 1, 1, 2);
  const uint8_t vxlan_ttl = 3;
  const uint16_t vxlan_port = 4789;

  //-------------------------------------------------------
  // Arrange: construct an RTM_NEWLINK message that defines
  // a VxLAN interface
  //-------------------------------------------------------
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWLINK, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);

  nla_put_string(nlmsg_, IFLA_IFNAME, ifname);
  nla_put_u32(nlmsg_, IFLA_MTU, 1450);
  nla_put(nlmsg_, IFLA_ADDRESS, sizeof(mac_addr), &mac_addr);

  // LINKINFO container
  struct nlattr* linkinfo = nla_nest_start(nlmsg_, IFLA_LINKINFO);
  ASSERT_NE(linkinfo, nullptr);

  nla_put_string(nlmsg_, IFLA_INFO_KIND, kind);

  // INFO_DATA container
  struct nlattr* infodata = nla_nest_start(nlmsg_, IFLA_INFO_DATA);
  ASSERT_NE(infodata, nullptr);

  nla_put_u32(nlmsg_, IFLA_VXLAN_ID, vxlan_id);
  nla_put_u32(nlmsg_, IFLA_VXLAN_GROUP, htonl(vxlan_group));
  nla_put_u32(nlmsg_, IFLA_VXLAN_LOCAL, htonl(vxlan_local));
  nla_put_u8(nlmsg_, IFLA_VXLAN_TTL, vxlan_ttl);
  nla_put_u16(nlmsg_, IFLA_VXLAN_PORT, htons(vxlan_port));

  // end INFO_DATA
  nla_nest_end(nlmsg_, infodata);
  // end LINKINFO
  nla_nest_end(nlmsg_, linkinfo);

  //-------------------------------------------------------
  // Act: call UUT with message
  //-------------------------------------------------------
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);

  //-------------------------------------------------------
  // Assert: verify test case results
  //-------------------------------------------------------
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].handler, CREATE_TUNNEL_INTERFACE);
  EXPECT_STREQ(results[0].tunnel_info.ifname, ifname);
  EXPECT_EQ(results[0].tunnel_info.src_ip.ip.v4addr.s_addr, vxlan_local);
  EXPECT_EQ(results[0].tunnel_info.dst_ip.ip.v4addr.s_addr, vxlan_group);
  EXPECT_EQ(results[0].tunnel_info.link_type, SWITCHLINK_LINK_TYPE_VXLAN);
  EXPECT_EQ(results[0].tunnel_info.ifindex, hdr.ifi_index);
  EXPECT_EQ(results[0].tunnel_info.vni_id, vxlan_id);
  EXPECT_EQ(results[0].tunnel_info.dst_port, vxlan_port);
  EXPECT_EQ(results[0].tunnel_info.ttl, vxlan_ttl);
}

/**
 * Attempts to create a bridge link.
 *
 * Verifies that switchlink_process_link_msg() parses an RTM_NEWLINK message
 * with (kind = "bridge") but does not call a handler.
 *
 * switchlink_process_link_msg() does not add bridge links.
 */
TEST_F(SwitchlinkTest, does_not_create_bridge_link) {
  struct ifinfomsg hdr = {
      .ifi_family = 0,
      .ifi_type = ARPHRD_ETHER,
      .ifi_index = 6,
      .ifi_flags = 0x11043,
      .ifi_change = 0x0,
  };
  const char ifname[] = "virbr0";
  const char kind[] = "bridge";
  const unsigned char mac_addr[6] = {0x52, 0x54, 0x00, 0xd9, 0x56, 0x95};
  const switchlink_handle_t vrf_h = 0xdeafbeadfadebadeUL;

  //-------------------------------------------------------
  // Arrange: construct an RTM_NEWLINK message that defines
  // a Bridge interface
  //-------------------------------------------------------
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_NEWLINK, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_string(nlmsg_, IFLA_IFNAME, ifname);
  nla_put(nlmsg_, IFLA_ADDRESS, sizeof(mac_addr), &mac_addr);

  // LINKINFO container
  struct nlattr* linkinfo = nla_nest_start(nlmsg_, IFLA_LINKINFO);
  ASSERT_NE(linkinfo, nullptr);
  nla_put_string(nlmsg_, IFLA_INFO_KIND, kind);

  // INFO_DATA container
  struct nlattr* infodata = nla_nest_start(nlmsg_, IFLA_INFO_DATA);
  ASSERT_NE(infodata, nullptr);
  nla_put_u32(nlmsg_, IFLA_BR_FORWARD_DELAY, 0x200);
  nla_put_u32(nlmsg_, IFLA_BR_HELLO_TIME, 100);
  nla_put_u32(nlmsg_, IFLA_BR_AGEING_TIME, 600);
  nla_put_u32(nlmsg_, IFLA_BR_STP_STATE, 4);

  nla_nest_end(nlmsg_, infodata);
  nla_nest_end(nlmsg_, linkinfo);

  g_default_vrf_h = vrf_h;

  //-------------------------------------------------------
  // Act: call UUT with message
  //-------------------------------------------------------
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);

  //-------------------------------------------------------
  // Assert: verify results of text case
  //-------------------------------------------------------
  ASSERT_EQ(results.size(), 0);
}

/**
 * Deletes a vxlan link.
 *
 * Verifies that switchlink_process_link_msg() parses an RTM_DELLINK message
 * with (kind = "vxlan") and calls switchlink_delete_tunnel_interface() with
 * the correct attributes.
 *
 * The only attributes that matter for an RTM_DELLINK message are the
 * interface index (ifindex), which specifies the interface to be deleted,
 * and IFLA_INFO_KIND, which is used to determine the link type.
 */
TEST_F(SwitchlinkTest, can_delete_vxlan_link) {
  struct ifinfomsg hdr = {
      .ifi_family = 0,
      .ifi_type = ARPHRD_ETHER,
      .ifi_index = 1776,
      .ifi_flags = 0,
      .ifi_change = 0,
  };
  const char kind[] = "vxlan";

  //-------------------------------------------------------
  // Arrange: create an RTM_DELLINK message for a VxLAN
  // interface
  //-------------------------------------------------------
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELLINK, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);

  struct nlattr* linkinfo = nla_nest_start(nlmsg_, IFLA_LINKINFO);
  ASSERT_NE(linkinfo, nullptr);
  nla_put_string(nlmsg_, IFLA_INFO_KIND, kind);
  nla_nest_end(nlmsg_, linkinfo);

  //-------------------------------------------------------
  // Act: call UUT with message
  //-------------------------------------------------------
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);

  //-------------------------------------------------------
  // Assert: verify results of test case
  //-------------------------------------------------------
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].handler, DELETE_TUNNEL_INTERFACE);
  EXPECT_EQ(results[0].ifindex, hdr.ifi_index);
}

/**
 * Deletes a tunnel ("tun") link.
 *
 * Verifies that switchlink_process_link_msg() parses an RTM_DELLINK message
 * with (kind = "tun") and calls switchlink_delete_interface() with the
 * correct attributes.
 *
 * Note that switchlink_process_link_msg() classifies a "tun" link as a
 * regular interface and a "vxlan" link as a tunnel interface.
 */
TEST_F(SwitchlinkTest, can_delete_tunnel_link) {
  struct ifinfomsg hdr = {
      .ifi_family = 0,
      .ifi_type = ARPHRD_ETHER,
      .ifi_index = 1984,
      .ifi_flags = 0x1002,
      .ifi_change = 0,
  };
  const char ifname[] = "virbr0-nic";
  const char kind[] = "tun";

  //-------------------------------------------------------
  // Arrange: create an RTM_DELLINK message for a tunnel
  // interface
  //-------------------------------------------------------
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELLINK, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);
  nla_put_string(nlmsg_, IFLA_IFNAME, ifname);

  struct nlattr* linkinfo = nla_nest_start(nlmsg_, IFLA_LINKINFO);
  ASSERT_NE(linkinfo, nullptr);
  nla_put_string(nlmsg_, IFLA_INFO_KIND, kind);

  struct nlattr* infodata = nla_nest_start(nlmsg_, IFLA_INFO_DATA);
  ASSERT_NE(infodata, nullptr);
  nla_put_u8(nlmsg_, IFLA_VXLAN_LINK, 1);
  nla_put_u32(nlmsg_, IFLA_VXLAN_LOCAL, 0);
  nla_put_u8(nlmsg_, IFLA_VXLAN_TTL, 1);
  nla_nest_end(nlmsg_, infodata);

  nla_nest_end(nlmsg_, linkinfo);

  //-------------------------------------------------------
  // Act: call UUT with message
  //-------------------------------------------------------
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);

  //-------------------------------------------------------
  // Assert: verify results of test case
  //-------------------------------------------------------
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].handler, DELETE_INTERFACE);
  EXPECT_EQ(results[0].ifindex, hdr.ifi_index);
}

/**
 * Attempts to delete a generic link.
 *
 * Verifies that switchlink_process_link_msg() parses an RTM_DELLINK message
 * with (kind = None) and returns without creating an interface.
 *
 * switchlink_process_link_msg() determines the link_type by inspecting the
 * IFLA_INFO_KIND attribute. We don't supply this attribute for a generic
 * link because the netlink traces we examined don't specify the info_kind
 * when adding a generic interface.
 *
 * As a result, the link_type defaults to SWITCHLINK_LINK_TYPE_NONE and
 * switchlink_process_link_msg() returns without deleting the link.
 *
 * TODO: Find out whether this is the correct behavior.
 */
TEST_F(SwitchlinkTest, does_not_delete_generic_link) {
  struct ifinfomsg hdr = {
      .ifi_family = 0,
      .ifi_type = ARPHRD_ETHER,
      .ifi_index = 667,
      .ifi_flags = 0,
      .ifi_change = 0,
  };

  //-------------------------------------------------------
  // Arrange: create an RTM_DELLINK message that does not
  // specify a Kind attribute
  //-------------------------------------------------------
  nlmsg_ = nlmsg_alloc_size(1024);
  ASSERT_NE(nlmsg_, nullptr);
  nlmsg_put(nlmsg_, 0, 0, RTM_DELLINK, 0, 0);
  nlmsg_append(nlmsg_, &hdr, sizeof(hdr), NLMSG_ALIGNTO);

  //-------------------------------------------------------
  // Act: call UUT with message
  //-------------------------------------------------------
  const struct nlmsghdr* nlmsg = nlmsg_hdr(nlmsg_);
  switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);

  //-------------------------------------------------------
  // Assert: verify test case results
  //-------------------------------------------------------
  ASSERT_EQ(results.size(), 0);
}
