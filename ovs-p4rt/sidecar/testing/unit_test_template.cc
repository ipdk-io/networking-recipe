// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareSampleTableEntry()

#include <stdint.h>

#include <iostream>
#include <string>

#ifdef DUMP_JSON
#include "absl/flags/flag.h"
#include "google/protobuf/util/json_util.h"
#endif
#include "gtest/gtest.h"
#ifdef DIAG_DETAIL
#include "logging/ovsp4rt_diag_detail.h"
#endif
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

#ifdef DUMP_JSON
ABSL_FLAG(bool, dump_json, false, "Dump output table_entry in JSON");
#endif

namespace ovsp4rt {

#ifdef DUMP_JSON
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
#endif
using stratum::ParseProtoFromString;

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

static ::p4::config::v1::P4Info p4info;

class TemplateTest : public ::testing::Test {
 protected:
  TemplateTest() {
#ifdef DUMP_JSON
    dump_json_ = absl::GetFlag(FLAGS_dump_json);
#endif
  }

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  void SetUp() { SelectTable("sample_table"); }

  //----------------------------
  // P4Info lookup methods
  //----------------------------

  int GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
    return -1;
  }

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    std::cerr << "Match Field '" << mf_name << "' not found!\n";
    return -1;
  }

  int GetParamId(const std::string& param_name) const {
    for (const auto& param : ACTION->params()) {
      if (param.name() == param_name) {
        return param.id();
      }
    }
    std::cerr << "Action Param '" << param_name << "' not found!\n";
    return -1;
  }

#ifdef SELECT_ACTION
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
#endif

  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
    std::cerr << "Table '" << table_name << "' not found!\n";
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

#ifdef DUMP_JSON
  void DumpTableEntry() {
    if (dump_json_) {
      JsonPrintOptions options;
      options.add_whitespace = true;
      options.preserve_proto_field_names = true;
      std::string output;
      ASSERT_TRUE(MessageToJsonString(table_entry, &output, options).ok());
      std::cout << output << std::endl;
    }
  }
#endif

  //----------------------------
  // Initialization methods
  //----------------------------

  void InitAction() { SelectAction("sample_action"); }

  void InitInputInfo() {
    // SAMPLE CODE
    constexpr uint8_t MAC_ADDR[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    constexpr uint8_t BRIDGE_ID = 99;
    constexpr uint32_t SRC_PORT = 0x42;

    memcpy(input_info.mac_addr, MAC_ADDR, sizeof(input_info.mac_addr));
    input_info.bridge_id = BRIDGE_ID;
    input_info.rx_src_port = SRC_PORT;
    // SAMPLE CODE
  }

  //----------------------------
  // Test-specific methods
  //----------------------------

  void CheckAction() const {
    // SAMPLE CODE
    const int PORT_PARAM = GetParamId("port");

#ifdef SELECT_ACTION
    ASSERT_NE(ACTION_ID, -1);
#endif

    ASSERT_TRUE(table_entry.has_action());
    const auto& table_action = table_entry.action();

    const auto& action = table_action.action();
    EXPECT_EQ(action.action_id(), ACTION_ID);

    auto params = action.params();
    ASSERT_EQ(action.params_size(), 1);

    auto param = params[0];
    ASSERT_EQ(param.param_id(), PORT_PARAM);
    uint32_t port = DecodeWordValue(param.value());
    EXPECT_EQ(port, input_info.rx_src_port);
    // SAMPLE CODE
  }

  void CheckBridgeId(const ::p4::v1::FieldMatch& match) const {
    constexpr int BRIDGE_ID_SIZE = 1;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    ASSERT_EQ(match_value.size(), BRIDGE_ID_SIZE);

    // widen so values will be treated as ints
    ASSERT_EQ(uint32_t(match_value[0]), uint32_t(input_info.bridge_id));
  }

#ifdef DIAG_DETAIL
  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_TEMPLATE_TABLE); }
#endif

  void CheckMacAddr(const ::p4::v1::FieldMatch& match) const {
    constexpr int MAC_ADDR_SIZE = 6;

    ASSERT_TRUE(match.has_exact());
    const auto& match_value = match.exact().value();

    ASSERT_EQ(match_value.size(), MAC_ADDR_SIZE);

    for (int i = 0; i < MAC_ADDR_SIZE; i++) {
      EXPECT_EQ(match_value[i], input_info.mac_addr[i])
          << "mac_addr[" << i << "] is incorrect";
    }
  }

  void CheckMatches() const {
  }

  void CheckTableEntry() const {
    ASSERT_FALSE(TABLE == nullptr);
    EXPECT_EQ(table_entry.table_id(), TABLE_ID);
  }

  //----------------------------
  // Protected member data
  //----------------------------

  // Working variables
  struct input_info input_info = {0};
  ::p4::v1::TableEntry table_entry;
#ifdef DIAG_DETAIL
  DiagDetail detail;
#endif

  // Comparison variables
  uint32_t TABLE_ID;
#ifdef SELECT_ACTION
  int ACTION_ID = -1;
#endif

 private:
  //----------------------------
  // Private member data
  //----------------------------

#ifdef SELECT_ACTION
  const ::p4::config::v1::Action* ACTION = nullptr;
#endif
  const ::p4::config::v1::Table* TABLE = nullptr;

#ifdef DUMP_JSON
  bool dump_json_ = false;
#endif
};

//----------------------------------------------------------------------
// PrepareSampleTableEntry()
//----------------------------------------------------------------------

TEST_F(TemplateTest, remove_entry) {
  // Arrange
  InitInputInfo();

  // Act
  PrepareSampleTableEntry(&table_entry, input_info, p4info, REMOVE_ENTRY
#ifdef DIAG_DETAIL
      , detail
#endif
  );
#ifdef DUMP_JSON
  DumpTableEntry();
#endif

  // Assert
#ifdef DIAG_DETAIL
  CheckDetail();
#endif
  CheckTableEntry();
  CheckMatches();
}

TEST_F(TemplateTest, insert_entry) {
  // Arrange
  InitInputInfo();
  InitAction();

  // Act
  PrepareSampleTableEntry(&table_entry, input_info, p4info, INSERT_ENTRY
#ifdef DIAG_DETAIL
      , detail
#endif
  );
#ifdef DUMP_JSON
  DumpTableEntry();
#endif

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
