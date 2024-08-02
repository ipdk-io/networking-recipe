// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "base_table_test.h"

#include <iostream>

#include "absl/flags/flag.h"
#include "google/protobuf/util/json_util.h"
#include "gtest/gtest.h"
#include "p4info_text.h"

ABSL_FLAG(bool, dump_json, false, "Dump table entry in JSON");

namespace ovsp4rt {

using google::protobuf::util::JsonPrintOptions;
using google::protobuf::util::MessageToJsonString;
using stratum::ParseProtoFromString;

BaseTableTest::BaseTableTest() {
  dump_json_ = absl::GetFlag(FLAGS_dump_json);
}

void BaseTableTest::SetUpTestSuite() {
  ::util::Status status = ParseProtoFromString(P4INFO_TEXT, &p4info);
  if (!status.ok()) {
    std::exit(EXIT_FAILURE);
  }
}

void BaseTableTest::DumpTableEntry() const {
  if (dump_json_) {
    JsonPrintOptions options;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;
    std::string output;
    ASSERT_TRUE(MessageToJsonString(table_entry, &output, options).ok());
    std::cout << output << std::endl;
  }
}

} // namespace ovsp4rt
