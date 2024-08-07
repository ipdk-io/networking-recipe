// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_TABLE_TEST_H_
#define BASE_TABLE_TEST_H_

#include <arpa/inet.h>
#include <stdint.h>

#include <iostream>
#include <string>

#ifdef DUMP_JSON
#include "absl/flags/flag.h"
#include "google/protobuf/util/json_util.h"
#endif
#include "gtest/gtest.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "p4info_helper.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

#ifdef DUMP_JSON
ABSL_FLAG(bool, dump_json, false, "Dump table_entry in JSON");
#endif

namespace ovsp4rt {

#ifdef DUMP_JSON
using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
#endif
using stratum::ParseProtoFromString;

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

// P4Info object describing the pipeline configuration.
static ::p4::config::v1::P4Info p4info;

class BaseTableTest : public ::testing::Test {
 protected:
  BaseTableTest() : helper(p4info) {
#ifdef DUMP_JSON
    dump_json_ = absl::GetFlag(FLAGS_dump_json);
#endif
  }

  // Initializes the p4info object.
  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  //----------------------------
  // Utility methods
  //----------------------------

  static uint16_t DecodePortValue(const std::string& string_value) {
    uint16_t port_value = DecodeWordValue(string_value) & 0xffff;
    // Port values are encoded low byte first.
    return ntohs(port_value);
  }

  // Decodes a 24-bit parameter and returns its value as an unsigned integer.
  static uint32_t DecodeTunnelId(const std::string& string_value) {
    return DecodeWordValue(string_value) & 0xffffff;
  }

  static uint16_t DecodeVniValue(const std::string& string_value) {
    return DecodeWordValue(string_value) & 0xffff;
  }

  // Decodes a parameter and returns its value as an unsigned integer.
  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  // Dumps table_entry in JSON.
  void DumpTableEntry() const {
#ifdef DUMP_JSON
    if (dump_json_) {
      JsonPrintOptions options;
      options.add_whitespace = true;
      options.preserve_proto_field_names = true;
      std::string output;
      ASSERT_TRUE(MessageToJsonString(table_entry, &output, options).ok());
      std::cout << output << std::endl;
    }
#endif
  }

  //----------------------------
  // Protected member data
  //----------------------------

  // Table entry to be inserted or removed. Output of the UUT.
  ::p4::v1::TableEntry table_entry;

  // Facilitates use of the p4info object.
  P4InfoHelper helper;

 private:
  //----------------------------
  // Private member data
  //----------------------------

#ifdef DUMP_JSON
  // Value of the --dump_json command-line flag.
  bool dump_json_;
#endif
};

}  // namespace ovsp4rt

#endif  // BASE_TABLE_TEST_H_
