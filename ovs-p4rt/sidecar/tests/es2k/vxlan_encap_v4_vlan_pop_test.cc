// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareVxlanEncapAndVlanPopTableEntry().

#define DUMP_JSON

#include <stdint.h>

#include "absl/types/optional.h"
#include "gtest/gtest.h"
#include "ip_tunnel_test.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class VxlanEncapV4VlanPopTest : public IpTunnelTest {
 protected:
  VxlanEncapV4VlanPopTest() {}

  void SetUp() { SelectTable("vxlan_encap_vlan_pop_mod_table"); }

  void InitAction() { SelectAction("vxlan_encap_vlan_pop"); }

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {
    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    ASSERT_EQ(action.action_id(), ActionId());

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
    constexpr int SRC_ADDR_SIZE = 4;
    ASSERT_EQ(value.size(), SRC_ADDR_SIZE);

    auto word_value = ntohl(DecodeWordValue(value));
    ASSERT_EQ(word_value, tunnel_info.local_ip.ip.v4addr.s_addr);
  }

  void CheckDstAddrParam(const std::string& value) const {
    constexpr int DST_ADDR_SIZE = 4;
    ASSERT_EQ(value.size(), DST_ADDR_SIZE);

    auto word_value = ntohl(DecodeWordValue(value));
    ASSERT_EQ(word_value, tunnel_info.remote_ip.ip.v4addr.s_addr);
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

  void CheckTableEntry() const { ASSERT_EQ(table_entry.table_id(), TableId()); }
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(VxlanEncapV4VlanPopTest, remove_entry) {
  // Arrange
  InitV4TunnelInfo(OVS_TUNNEL_VXLAN);

  // Act
  PrepareVxlanEncapAndVlanPopTableEntry(&table_entry, tunnel_info, p4info,
                                        REMOVE_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(VxlanEncapV4VlanPopTest, insert_entry) {
  // Arrange
  InitV4TunnelInfo(OVS_TUNNEL_VXLAN);
  InitAction();

  // Act
  PrepareVxlanEncapAndVlanPopTableEntry(&table_entry, tunnel_info, p4info,
                                        INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

TEST_F(VxlanEncapV4VlanPopTest, insert_entry_24_bit_vni) {
  // Arrange
  InitV4TunnelInfo(OVS_TUNNEL_VXLAN);
  InitAction();
  tunnel_info.vni = 0x95054;

  // Act
  PrepareVxlanEncapAndVlanPopTableEntry(&table_entry, tunnel_info, p4info,
                                        INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
