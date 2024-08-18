// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareVlanPushTableEntry().

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class VlanPushModTableTest : public BaseTableTest {
 protected:
  VlanPushModTableTest() {}

  void SetUp() { SelectTable("vlan_push_mod_table"); }

  void InitAction() { SelectAction("vlan_push"); }

  void InitPushInfo() {
    // These values are hard-coded in PrepareVlanPushTableEntry();
    constexpr uint16_t PCP = 1;
    constexpr uint16_t DEI = 0;

    push_info.pcp = PCP;
    push_info.dei = DEI;

    // PrepareVlanPushTableEntry() encodes the value as a single byte.
    push_info.vlan_id = 0xAC;
  }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    const int PCP_PARAM = GetParamId("pcp");
    ASSERT_NE(PCP_PARAM, -1);

    const int DEI_PARAM = GetParamId("dei");
    ASSERT_NE(DEI_PARAM, -1);

    const int VLAN_PARAM = GetParamId("vlan_id");
    ASSERT_NE(VLAN_PARAM, -1);

    EXPECT_EQ(action.params_size(), 3);
    const auto& params = action.params();

    for (const auto& param : params) {
      int param_id = param.param_id();
      const auto& param_value = param.value();
      if (param_id == PCP_PARAM) {
        CheckPcpParam(param_value);
      } else if (param_id == DEI_PARAM) {
        CheckDeiParam(param_value);
      } else if (param_id == VLAN_PARAM) {
        CheckVlanIdParam(param_value);
      } else {
        FAIL() << "Unexpected param_id (" << param_id << ")";
      }
    }
  }

  void CheckPcpParam(const std::string& value) const {
    constexpr int PCP_PARAM_SIZE = 1;
    EXPECT_EQ(value.size(), PCP_PARAM_SIZE);

    auto pcp_param = uint16_t(value[0] & 0xFF);
    EXPECT_EQ(pcp_param, push_info.pcp);
  }

  void CheckDeiParam(const std::string& value) const {
    constexpr int DEI_PARAM_SIZE = 1;
    EXPECT_EQ(value.size(), DEI_PARAM_SIZE);

    auto dei_param = uint16_t(value[0] & 0xFF);
    EXPECT_EQ(dei_param, push_info.dei);
  }

  void CheckVlanIdParam(const std::string& value) const {
    constexpr int VLAN_PARAM_SIZE = 1;
    EXPECT_EQ(value.size(), VLAN_PARAM_SIZE);

    uint32_t vlan_param = DecodeWordValue(value);
    EXPECT_EQ(vlan_param, push_info.vlan_id);
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    const int MF_MOD_BLOB_PTR = GetMatchFieldId("vmeta.common.mod_blob_ptr");
    ASSERT_NE(MF_MOD_BLOB_PTR, -1);

    ASSERT_EQ(table_entry.match_size(), 1);

    const auto& match = table_entry.match()[0];
    ASSERT_EQ(match.field_id(), MF_MOD_BLOB_PTR);

    CheckModBlobPtrMatch(match);
  }

  void CheckModBlobPtrMatch(const ::p4::v1::FieldMatch& match) const {
    // TODO(derek): PrepareVlanPushTableEntry() encodes the mod_blob_ptr
    // value as a single byte, which doesn't make sense. The input is
    // a vlan_id, which is bit<12>. The mod_blob ptr value is bit<24>.
    constexpr int MOD_BLOB_PTR_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    ASSERT_EQ(match_value.size(), MOD_BLOB_PTR_SIZE);

    uint32_t mod_blob_ptr = DecodeVniValue(match_value);
    EXPECT_EQ(mod_blob_ptr, push_info.vlan_id);
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  struct vlan_push_info {
    // We use uint16_t instead of uint8_t so googletest interprets
    // the fields as unsigned ints instead of unsigned chars.
    uint16_t pcp;      // bit<3>
    uint16_t dei;      // bit<1>
    uint16_t vlan_id;  // bit<12>
  };

  struct vlan_push_info push_info = {0};
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(VlanPushModTableTest, remove_entry) {
  // Arrange
  InitPushInfo();

  // Act
  PrepareVlanPushTableEntry(&table_entry, push_info.vlan_id, p4info,
                            REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(VlanPushModTableTest, insert_entry) {
  // Arrange
  InitPushInfo();
  InitAction();

  // Act
  PrepareVlanPushTableEntry(&table_entry, push_info.vlan_id, p4info,
                            INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
