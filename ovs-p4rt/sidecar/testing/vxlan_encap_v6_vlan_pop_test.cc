// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include <string>

#include "absl/flags/flag.h"
#include "absl/types/optional.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "testing/ipv6_tunnel_test_defs.h"
#include "testing/table_entry_test.h"

namespace ovs_p4rt {

//----------------------------------------------------------------------
// PrepareV6VxlanEncapAndVlanPopTableEntry()
//----------------------------------------------------------------------

// constexpr uint32_t VXLAN_ENCAP_V6_MOD_TABLE_ID = 46225003U;
constexpr uint32_t VXLAN_ENCAP_V6_ACTION_ID = 30345128U;

enum {
  SRC_PORT_PARAM_ID = 7,
  DST_PORT_PARAM_ID = 8,
  VNI_PARAM_ID = 9,
};

//----------------------------------------------------------------------

TEST_F(TableEntryTest, vxlan_encap_v6_vlan_pop_params_are_correct) {
  struct tunnel_info tunnel_info = {0};
  p4::v1::TableEntry table_entry;
  constexpr bool insert_entry = true;

  // Arrange
  InitV6TunnelInfo(tunnel_info);

  // Act
  PrepareV6VxlanEncapTableEntry(&table_entry, tunnel_info, p4info,
                                insert_entry);
  DumpTableEntry(table_entry);

  // Assert
  ASSERT_TRUE(table_entry.has_action());

  auto table_action = table_entry.action();
  auto action = table_action.action();
  ASSERT_EQ(action.action_id(), VXLAN_ENCAP_V6_ACTION_ID);

  auto params = action.params();
  int num_params = action.params_size();

  absl::optional<uint16_t> src_port;
  absl::optional<uint16_t> dst_port;
  absl::optional<uint16_t> vni;

  for (int i = 0; i < num_params; ++i) {
    auto param = params[i];
    int param_id = param.param_id();
    auto param_value = param.value();
    if (param_id == SRC_PORT_PARAM_ID) {
      src_port = DecodeWordValue(param_value) & 0xffff;
    } else if (param_id == DST_PORT_PARAM_ID) {
      dst_port = DecodeWordValue(param_value) & 0xffff;
    } else if (param_id == VNI_PARAM_ID) {
      vni = DecodeWordValue(param_value) & 0xffff;
    }
  }

  if (absl::GetFlag(FLAGS_check_src_port)) {
    // The src_port field contains an arbitrary value that has nothing
    // to do with what was specified in the tunnel_info structure.
    // It is a workaround for a long-standing bug in the Linux
    // Networking P4 program.
    ASSERT_TRUE(src_port.has_value());
    EXPECT_EQ(src_port.value(), SRC_PORT);
  }

  ASSERT_TRUE(dst_port.has_value());
  EXPECT_EQ(dst_port.value(), DST_PORT);

  ASSERT_TRUE(vni.has_value());
  EXPECT_EQ(vni.value(), VNI);
}

}  // namespace ovs_p4rt
