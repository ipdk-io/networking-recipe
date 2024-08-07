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
#include "p4info_helper.h"
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
  TemplateTest() : helper(p4info) {
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

  void SetUp() { helper.SelectTable("sample_table"); }

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

  void InitAction() { helper.SelectAction("sample_action"); }

  void InitInputInfo() {}

  //----------------------------
  // CheckAction()
  //----------------------------

  void CheckAction() const {}

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

#ifdef DIAG_DETAIL
  //----------------------------
  // CheckDetail()
  //----------------------------

  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_TEMPLATE_TABLE); }
#endif

  //----------------------------
  // CheckMatches()
  //----------------------------

  void CheckMatches() const {}

  //----------------------------
  // CheckTableEntry()
  //----------------------------

  void CheckTableEntry() const {
    ASSERT_TRUE(helper.has_table());
    EXPECT_EQ(table_entry.table_id(), helper.table_id());
  }

  //----------------------------
  // Protected member data
  //----------------------------

  P4InfoHelper helper;

  struct input_info input_info = {0};
  ::p4::v1::TableEntry table_entry;
#ifdef DIAG_DETAIL
  DiagDetail detail;
#endif

#ifdef DUMP_JSON
 private:
  //----------------------------
  // Private member data
  //----------------------------

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
                          ,
                          detail
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
  CheckNoAction();
}

TEST_F(TemplateTest, insert_entry) {
  // Arrange
  InitInputInfo();
  InitAction();

  // Act
  PrepareSampleTableEntry(&table_entry, input_info, p4info, INSERT_ENTRY
#ifdef DIAG_DETAIL
                          ,
                          detail
#endif
  );
#ifdef DUMP_JSON
  DumpTableEntry();
#endif

  // Assert
  CheckAction();
}

}  // namespace ovsp4rt
