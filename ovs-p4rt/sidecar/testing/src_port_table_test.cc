// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareSrcPortTableEntry()

#define DUMP_JSON

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class SrcPortTableTest : public BaseTableTest {
 protected:
  SrcPortTableTest() {}

  void SetUp() { SelectTable("source_port_to_bridge_map"); }

  void InitAction() { SelectAction("set_bridge_id"); }

  void InitMapInfo() {
    port_info.bridge_id = 42;
    port_info.vlan_id = 0x123;
    port_info.src_port = 0x1066;
  }

  //----------------------------
  // CheckAction()
  //----------------------------
  void CheckAction() const {
    const int BRIDGE_ID_PARAM = GetParamId("bridge_id");
    ASSERT_NE(BRIDGE_ID_PARAM, -1);

    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    ASSERT_EQ(action.action_id(), ActionId());

    const auto& params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    const auto& param = params[0];
    ASSERT_EQ(param.param_id(), BRIDGE_ID_PARAM);

    auto& param_value = param.value();
    EXPECT_EQ(param_value.size(), 1);

    auto bridge_id = DecodeWordValue(param_value);
    EXPECT_EQ(bridge_id, port_info.bridge_id);
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------
  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------
  void CheckMatches() const {
    constexpr char SRC_PORT_KEY[] = "user_meta.cmeta.source_port";
    const int MF_SRC_PORT = GetMatchFieldId(SRC_PORT_KEY);
    ASSERT_NE(MF_SRC_PORT, -1);

    constexpr char VID_KEY[] = "hdrs.vlan_ext[vmeta.common.depth].hdr.vid";
    const int MF_VID = GetMatchFieldId(VID_KEY);
    ASSERT_NE(MF_VID, -1);

    EXPECT_EQ(table_entry.match_size(), 2);

    for (const auto& match : table_entry.match()) {
      int field_id = match.field_id();
      if (field_id == MF_SRC_PORT) {
        CheckSrcPortMatch(match);
      } else if (field_id == MF_VID) {
        CheckVidMatch(match);
      } else {
        FAIL() << "Unexpected match field ID (" << field_id << ")";
      }
    }
  }

  void CheckSrcPortMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int PORT_SIZE = 2;
    constexpr uint16_t PORT_MASK = 0xffff;

    ASSERT_TRUE(match.has_ternary());

    const auto& match_value = match.ternary().value();
    EXPECT_EQ(match_value.size(), PORT_SIZE);
    // src_port is encoded high-byte first.
    const auto& port_value = DecodeWordValue(match_value);
    EXPECT_EQ(port_value, port_info.src_port);

    const auto& match_mask = match.ternary().mask();
    EXPECT_EQ(match_mask.size(), PORT_SIZE);
    auto port_mask = DecodeWordValue(match_mask);
    EXPECT_EQ(port_mask, PORT_MASK);
  }

  void CheckVidMatch(const ::p4::v1::FieldMatch& match) const {
    constexpr int VID_SIZE = 2;
    constexpr uint16_t VLAN_MASK = 0x0fff;

    ASSERT_TRUE(match.has_ternary());

    const auto& ternary_value = match.ternary().value();
    EXPECT_EQ(ternary_value.size(), VID_SIZE);
    auto vid_value = DecodeWordValue(ternary_value);
    EXPECT_EQ(vid_value, port_info.vlan_id);

    const auto& ternary_mask = match.ternary().mask();
    EXPECT_EQ(ternary_mask.size(), VID_SIZE);
    auto vid_mask = DecodeWordValue(ternary_mask);
    EXPECT_EQ(vid_mask, VLAN_MASK);
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------
  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  struct src_port_info port_info = {0};
};

//----------------------------------------------------------------------
// PrepareSrcPortTableEntry()
//----------------------------------------------------------------------

TEST_F(SrcPortTableTest, remove_entry) {
  // Arrange
  InitMapInfo();

  // Act
  PrepareSrcPortTableEntry(&table_entry, port_info, p4info, REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(SrcPortTableTest, insert_entry) {
  // Arrange
  InitMapInfo();
  InitAction();

  // Act
  PrepareSrcPortTableEntry(&table_entry, port_info, p4info, INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
