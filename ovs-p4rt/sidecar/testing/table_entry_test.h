// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef TABLE_ENTRY_TEST_H_
#define TABLE_ENTRY_TEST_H_

#include <stdbool.h>
#include <stdint.h>

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4info_text.h"
#include "stratum/lib/utils.h"

ABSL_FLAG(bool, dump_json, false, "Dump output object in JSON");

namespace ovsp4rt {

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
using stratum::ParseProtoFromString;

static ::p4::config::v1::P4Info p4info;

class TableEntryTest : public ::testing::Test {
 protected:
  TableEntryTest() {
    dump_json_ = absl::GetFlag(FLAGS_dump_json);
  };

  static void SetUpTestSuite() {
    ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
    if (!status.ok()) {
      std::exit(EXIT_FAILURE);
    }
  }

  static uint16_t DecodePortValue(const std::string& string_value) {
    uint16_t port_value = DecodeWordValue(string_value) & 0xffff;
    // Port values are encoded low byte first.
    return ntohs(port_value);
  }

  static uint16_t DecodeVniValue(const std::string& string_value) {
    return DecodeWordValue(string_value) & 0xffff;
  }

  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  void DumpTableEntry(const ::p4::v1::TableEntry& table_entry) {
    if (dump_json_) {
      JsonPrintOptions options;
      options.add_whitespace = true;
      options.preserve_proto_field_names = true;
      std::string output;
      ASSERT_TRUE(MessageToJsonString(table_entry, &output, options).ok());
      std::cout << output << std::endl;
    }
  }

  bool dump_json_;
};

}  // namespace ovsp4rt

#endif  // TABLE_ENTRY_TEST_H_
