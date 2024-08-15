// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareV6GeneveEncapAndVlanPopTableEntry().

#include <stdint.h>

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "ip_tunnel_test.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class GeneveEncapV6VlanPopTest : public IpTunnelTest {
 protected:
  GeneveEncapV6VlanPopTest() {}

  void SetUp() { SelectTable("geneve_encap_v6_vlan_pop_mod_table"); }

  void InitAction() { SelectAction("geneve_encap_v6_vlan_pop"); }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ActionId());

    // Get param IDs.
    const int SRC_ADDR_PARAM_ID = GetParamId("src_addr");
    EXPECT_NE(SRC_ADDR_PARAM_ID, -1);

    const int DST_ADDR_PARAM_ID = GetParamId("dst_addr");
    EXPECT_NE(DST_ADDR_PARAM_ID, -1);

    const int SRC_PORT_PARAM_ID = GetParamId("src_port");
    EXPECT_NE(SRC_PORT_PARAM_ID, -1);

    const int DST_PORT_PARAM_ID = GetParamId("dst_port");
    EXPECT_NE(DST_PORT_PARAM_ID, -1);

    const int VNI_PARAM_ID = GetParamId("vni");
    EXPECT_NE(VNI_PARAM_ID, -1);

    // Process action parameters.
    const auto& params = action.params();

    for (const auto& param : params) {
      const auto& param_value = param.value();
      int param_id = param.param_id();

      if (param_id == SRC_ADDR_PARAM_ID) {
        CheckSrcAddrParam(param_value);
      } else if (param_id == DST_ADDR_PARAM_ID) {
        CheckDstAddrParam(param_value);
      } else if (param_id == SRC_PORT_PARAM_ID) {
        CheckSrcPortParam(param_value);
      } else if (param_id == DST_PORT_PARAM_ID) {
        CheckDstPortParam(param_value);
      } else if (param_id == VNI_PARAM_ID) {
        CheckVniParam(param_value);
      } else {
        FAIL() << "Unexpected param_id (" << param_id << ")";
      }
    }
  }
  void CheckSrcAddrParam(const std::string& value) const {
    // TODO(derek): implement CheckSrcAddrParam().
  }

  void CheckDstAddrParam(const std::string& value) const {
    // TODO(derek): implement CheckDstAddrParam().
  }

  void CheckSrcPortParam(const std::string& value) const {
    constexpr int PORT_PARAM_SIZE = 2;
    EXPECT_EQ(value.size(), PORT_PARAM_SIZE);

    auto src_port_param = DecodePortValue(value);
    // To work around a bug in the Linux Networking P4 program, we
    // ignore the src_port value specified by the caller and instead
    // set the src_port param to (dst_port * 2).
    auto expected = tunnel_info.dst_port * 2;
    EXPECT_EQ(src_port_param, expected);
  }

  void CheckDstPortParam(const std::string& value) const {
    constexpr int PORT_PARAM_SIZE = 2;
    EXPECT_EQ(value.size(), PORT_PARAM_SIZE);

    auto dst_port_param = DecodePortValue(value);
    EXPECT_EQ(dst_port_param, tunnel_info.dst_port);
  }

  void CheckVniParam(const std::string& value) const {
    constexpr int VNI_PARAM_SIZE = 3;
    EXPECT_EQ(value.size(), VNI_PARAM_SIZE);

    auto vni_param = DecodeVniValue(value);
    EXPECT_EQ(vni_param, tunnel_info.vni);
  }

  //----------------------------
  // CheckNoAction()
  //----------------------------

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {
    ASSERT_EQ(table_entry.match_size(), 1);
    const auto& match = table_entry.match()[0];

    constexpr char MOD_BLOB_PTR[] = "vmeta.common.mod_blob_ptr";
    const int MF_MOD_BLOB_PTR = GetMatchFieldId(MOD_BLOB_PTR);
    ASSERT_NE(MF_MOD_BLOB_PTR, -1);

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

TEST_F(GeneveEncapV6VlanPopTest, remove_entry) {
  // Arrange
  InitV6TunnelInfo(OVS_TUNNEL_GENEVE);

  // Act
  PrepareV6GeneveEncapAndVlanPopTableEntry(&table_entry, tunnel_info, p4info,
                                           REMOVE_ENTRY);

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(GeneveEncapV6VlanPopTest, insert_entry) {
  // Arrange
  InitV6TunnelInfo(OVS_TUNNEL_GENEVE);
  InitAction();

  // Act
  PrepareV6GeneveEncapAndVlanPopTableEntry(&table_entry, tunnel_info, p4info,
                                           INSERT_ENTRY);

  // Assert
  CheckTableEntry();
  CheckAction();
}

#ifdef WIDE_VNI_VALUE

TEST_F(GeneveEncapV6VlanPopTest, insert_entry_with_24_bit_vni) {
  // Arrange
  InitV4TunnelInfo(OVS_TUNNEL_GENEVE);
  tunnel_info.vni = 0x95054;
  InitAction();

  // Act
  PrepareV6GeneveEncapAndVlanPopTableEntry(&table_entry, tunnel_info, p4info,
                                           INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckAction();
}

#endif  // WIDE_VNI_VALUE

}  // namespace ovsp4rt
