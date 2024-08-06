// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include "absl/types/optional.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "testing/ipv4_tunnel_test.h"

namespace ovsp4rt {

class VxlanEncapV4TableTest : public Ipv4TunnelTest {};

//----------------------------------------------------------------------
// PrepareVxlanEncapTableEntry()
//----------------------------------------------------------------------

TEST_F(VxlanEncapV4TableTest, minimal) {
  struct tunnel_info tunnel_info = {0};
  p4::v1::TableEntry table_entry;
  constexpr bool insert_entry = true;

  constexpr uint32_t TABLE_ID = 40763773U;
  constexpr uint32_t ACTION_ID = 20733968U;

  enum {
    SRC_PORT_PARAM_ID = 3,
    DST_PORT_PARAM_ID = 4,
    VNI_PARAM_ID = 5,
  };

  // Arrange
  InitV4TunnelInfo(tunnel_info, OVS_TUNNEL_VXLAN);

  // Act
  PrepareVxlanEncapTableEntry(&table_entry, tunnel_info, p4info, insert_entry);
  DumpTableEntry(table_entry);

  // Assert
  ASSERT_EQ(table_entry.table_id(), TABLE_ID);
  ASSERT_TRUE(table_entry.has_action());

  auto table_action = table_entry.action();
  auto action = table_action.action();
  ASSERT_EQ(action.action_id(), ACTION_ID);

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
      src_port = DecodePortValue(param_value);
    } else if (param_id == DST_PORT_PARAM_ID) {
      dst_port = DecodePortValue(param_value);
    } else if (param_id == VNI_PARAM_ID) {
      vni = DecodeVniValue(param_value);
    }
  }

#if defined(ES2K_TARGET)
  ASSERT_TRUE(src_port.has_value());

  if (check_src_port_) {
    // To work around a bug in the Linux Networking P4 program, we
    // ignore the src_port value specified by the caller and instead
    // set the src_port param to (dst_port * 2).
    EXPECT_EQ(src_port.value(), DST_PORT * 2);  // SRC_PORT
  }
#endif

  ASSERT_TRUE(dst_port.has_value());
  EXPECT_EQ(dst_port.value(), DST_PORT);

  ASSERT_TRUE(vni.has_value());
  EXPECT_EQ(vni.value(), VNI);
}

}  // namespace ovsp4rt
