// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

using stratum::ParseProtoFromString;

constexpr char TABLE_NAME[] = "vxlan_decap_mod_table";

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class VxlanDecapModEntryTest : public ::testing::Test {
 protected:
  VxlanDecapModEntryTest() {}

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable(TABLE_NAME); }

  //----------------------------
  // P4Info lookup methods
  //----------------------------

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    return -1;
  }

  void SelectAction(const std::string& action_name) {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        ACTION = &action;
        ACTION_ID = pre.id();
        return;
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
  }

  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
  }

  //----------------------------
  // Utility methods
  //----------------------------

  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  static uint16_t DecodeVniValue(const std::string& string_value) {
    return DecodeWordValue(string_value) & 0xffff;
  }

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitAction() {
    SelectAction("vxlan_decap_outer_hdr");
  }

  void InitTunnelInfo() {
    tunnel_info.vni = 0x1776;
  }

  //----------------------------
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    ASSERT_NE(ACTION_ID, -1);

    ASSERT_TRUE(table_entry.has_action());
    auto table_action = table_entry.action();

    auto action = table_action.action();
    EXPECT_EQ(action.action_id(), ACTION_ID);

    EXPECT_EQ(action.params_size(), 0);
  }

  void CheckMatches() const {
    constexpr char MOD_BLOB_PTR[] = "vmeta.common.mod_blob_ptr";
    const int MF_MOD_BLOB_PTR = GetMatchFieldId(MOD_BLOB_PTR);

    ASSERT_EQ(table_entry.match_size(), 1);

    auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);

    CheckVniValue(match);
  }

  void CheckNoAction() const {
    EXPECT_FALSE(table_entry.has_action());
  }

  void CheckTableEntry() const {
    ASSERT_FALSE(TABLE == nullptr) << "Table '" << TABLE_NAME << "' not found";
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
  }

  void CheckVniValue(const ::p4::v1::FieldMatch& match) const {
    constexpr int VNI_SIZE = 3;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    ASSERT_EQ(match_value.size(), VNI_SIZE);

    uint32_t vni_value = DecodeVniValue(match_value);
    EXPECT_EQ(vni_value, tunnel_info.vni);
  }

  //----------------------------
  // Protected member data
  //----------------------------

  // Working variables
  struct tunnel_info tunnel_info = {0};
  ::p4::v1::TableEntry table_entry;

  // Comparison variables
  uint32_t TABLE_ID;
  int ACTION_ID = -1;

 private:
  //----------------------------
  // Private member data
  //----------------------------
  const ::p4::config::v1::Action* ACTION = nullptr;
  const ::p4::config::v1::Table* TABLE = nullptr;
};

//----------------------------------------------------------------------
// PrepareVxlanDecapModTableEntry
//----------------------------------------------------------------------

TEST_F(VxlanDecapModEntryTest, remove_entry) {
  // Arrange
  InitTunnelInfo();

  // Act
  PrepareVxlanDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                 REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(VxlanDecapModEntryTest, insert_entry) {
  // Arrange
  InitTunnelInfo();
  InitAction();

  // Act
  PrepareVxlanDecapModTableEntry(&table_entry, tunnel_info, p4info,
                                 INSERT_ENTRY);

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
