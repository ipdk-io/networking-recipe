// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareVxlanDecapModAndVlanPushTableEntry().

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

// These values are hard-coded.
constexpr uint16_t PCP = 1;
constexpr uint16_t DEI = 0;

class VxlanDecapModVlanPushTest : public BaseTableTest {
 protected:
  VxlanDecapModVlanPushTest() {}

  void SetUp() { SelectTable("vxlan_decap_and_push_vlan_mod_table"); }

  void InitAction() { SelectAction("vxlan_decap_and_push_vlan"); }

  void InitTunnelInfo() {
    // TODO(derek): vni encoded as bit<16>.
    tunnel_info.vni = 0xFACE;                 // bit<16>
    // TODO(derek): port_vlan encoded as bit<8>.
    tunnel_info.vlan_info.port_vlan = 0xED;   // bit<8>
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
    constexpr uint16_t PCP_VALUE = 1;  // hard-wired

    EXPECT_EQ(value.size(), PCP_PARAM_SIZE);

    auto pcp_param = uint16_t(value[0] & 0xFF);
    EXPECT_EQ(pcp_param, PCP_VALUE);
  }

  void CheckDeiParam(const std::string& value) const {
    constexpr int DEI_PARAM_SIZE = 1;
    constexpr uint16_t DEI_VALUE = 0;  // hard-wired

    EXPECT_EQ(value.size(), DEI_PARAM_SIZE);

    auto dei_param = uint16_t(value[0] & 0xFF);
    EXPECT_EQ(dei_param, DEI_VALUE);
  }

  void CheckVlanIdParam(const std::string& value) const {
    // TODO(derek): vlan_id encoded as bit<8>.
    constexpr int VLAN_PARAM_SIZE = 1;
    EXPECT_EQ(value.size(), VLAN_PARAM_SIZE);

    uint32_t vlan_param = DecodeWordValue(value);
    EXPECT_EQ(vlan_param, tunnel_info.vlan_info.port_vlan);
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
    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    constexpr int MOD_BLOB_PTR_SIZE = 3;
    ASSERT_EQ(match_value.size(), MOD_BLOB_PTR_SIZE);

    uint32_t mod_blob_ptr = DecodeVniValue(match_value);
    EXPECT_EQ(mod_blob_ptr, tunnel_info.vni);
  }

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  //----------------------------
  // Protected member data
  //----------------------------

  struct tunnel_info tunnel_info = {0};
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(VxlanDecapModVlanPushTest, remove_entry) {
  // Arrange
  InitTunnelInfo();

  // Act
  PrepareVxlanDecapModAndVlanPushTableEntry(&table_entry, tunnel_info, p4info,
                                            REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(VxlanDecapModVlanPushTest, insert_entry) {
  // Arrange
  InitTunnelInfo();
  InitAction();

  // Act
  PrepareVxlanDecapModAndVlanPushTableEntry(&table_entry, tunnel_info, p4info,
                                            INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckAction();
}

#ifdef WIDE_VNI_VALUES

TEST_F(VxlanDecapModVlanPushTest, insert_entry_with_24_bit_vni) {
  // Arrange
  InitTunnelInfo();
  tunnel_info.vni = 0xDEFACE;
  InitAction();

  // Act
  PrepareVxlanDecapModAndVlanPushTableEntry(&table_entry, tunnel_info, p4info,
                                            INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckAction();
}

#endif  // WIDE_VNI_VALUES

}  // namespace ovsp4rt
