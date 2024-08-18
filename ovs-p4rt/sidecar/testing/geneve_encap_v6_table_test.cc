// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareV6GeneveEncapTableEntry().

#define DUMP_JSON

#include <stdint.h>

#include "absl/types/optional.h"
#include "gtest/gtest.h"
#include "ip_tunnel_test.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class GeneveEncapV6TableTest : public IpTunnelTest {
 protected:
  GeneveEncapV6TableTest() {}

  void SetUp() { SelectTable("geneve_encap_v6_mod_table"); }

  void InitAction() { SelectAction("geneve_encap_v6"); }

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
        CheckAddrParam("src_addr", param_value, tunnel_info.local_ip);
      } else if (param_id == DST_ADDR_PARAM_ID) {
        CheckAddrParam("dst_addr", param_value, tunnel_info.remote_ip);
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

  void CheckAddrParam(const std::string& param_name, const std::string& value,
                      const struct p4_ipaddr& ipaddr) const {
    constexpr int IPV6_ADDR_SIZE = 16;
    constexpr int IPV6_ADDR_WORDS = IPV6_ADDR_SIZE / 2;

    ASSERT_EQ(value.size(), IPV6_ADDR_SIZE);

    for (int word_num = 0; word_num < IPV6_ADDR_WORDS; ++word_num) {
      int byte_num = word_num * 2;
      uint16_t actual =
          (value[byte_num] & 0xFF) | ((value[byte_num + 1] & 0xFF) << 8);
      uint16_t expected = ipaddr.ip.v6addr.__in6_u.__u6_addr16[word_num];
      EXPECT_EQ(actual, expected)
          << param_name << "[" << word_num << "] does not match\n"
          << "  actual:   0x" << std::setw(4) << std::setfill('0') << std::hex
          << actual << '\n'
          << "  expected: 0x" << std::setw(4) << std::setfill('0') << expected
          << '\n'
          << std::dec;
    }
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

TEST_F(GeneveEncapV6TableTest, remove_entry) {
  // Arrange
  InitV6TunnelInfo(OVS_TUNNEL_GENEVE);

  // Act
  PrepareV6GeneveEncapTableEntry(&table_entry, tunnel_info, p4info,
                                 REMOVE_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(GeneveEncapV6TableTest, insert_entry) {
  // Arrange
  InitV6TunnelInfo(OVS_TUNNEL_GENEVE);
  InitAction();

  // Act
  PrepareV6GeneveEncapTableEntry(&table_entry, tunnel_info, p4info,
                                 INSERT_ENTRY);
  DumpTableEntry();

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
