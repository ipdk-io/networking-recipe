// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareGeneveEncapTableEntry().

// TODO(derek):
// - Replace hard-coded IDs with p4info lookups.
// - Make sure all action params are checked.

#include <stdint.h>

#include "absl/types/optional.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "testing/ipv4_tunnel_test.h"

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

constexpr uint32_t TABLE_ID = 41319073U;
constexpr uint32_t ACTION_ID = 25818889U;

enum {
  MF_MOD_BLOB_PTR = 1,
};

enum {
  SRC_PORT_PARAM_ID = 3,
  DST_PORT_PARAM_ID = 4,
  VNI_PARAM_ID = 5,
};

class GeneveEncapV4TableEntryTest : public Ipv4TunnelTest {
 protected:
  struct tunnel_info tunnel_info = {0};
  p4::v1::TableEntry table_entry;

  void CheckAction() const {
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

    ASSERT_TRUE(src_port.has_value());

    // To work around a bug in the Linux Networking P4 program, we
    // ignore the src_port value specified by the caller and instead
    // set the src_port param to (dst_port * 2).
    EXPECT_EQ(src_port.value(), DST_PORT * 2);  // SRC_PORT

    ASSERT_TRUE(dst_port.has_value());
    EXPECT_EQ(dst_port.value(), DST_PORT);

    ASSERT_TRUE(vni.has_value());
    EXPECT_EQ(vni.value(), VNI);
  }

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  void CheckMatches() const {
    ASSERT_EQ(table_entry.match_size(), 1);

    auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);

    CheckVniMatch(match);
  }

  void CheckVniMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int VNI_SIZE = 3;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    ASSERT_EQ(match_value.size(), VNI_SIZE);

    uint32_t vni_value = DecodeVniValue(match_value);
    EXPECT_EQ(vni_value, tunnel_info.vni);
  }

  void CheckTableEntry() const { ASSERT_EQ(table_entry.table_id(), TABLE_ID); }
};

//----------------------------------------------------------------------
// Test PrepareGeneveEncapTableEntry()
//----------------------------------------------------------------------

TEST_F(GeneveEncapV4TableEntryTest, remove_entry) {
  // Arrange
  InitV4TunnelInfo(tunnel_info, OVS_TUNNEL_GENEVE);

  // Act
  PrepareGeneveEncapTableEntry(&table_entry, tunnel_info, p4info, REMOVE_ENTRY);
  DumpTableEntry(table_entry);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(GeneveEncapV4TableEntryTest, insert_entry) {
  // Arrange
  InitV4TunnelInfo(tunnel_info, OVS_TUNNEL_GENEVE);

  // Act
  PrepareGeneveEncapTableEntry(&table_entry, tunnel_info, p4info, INSERT_ENTRY);
  DumpTableEntry(table_entry);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
