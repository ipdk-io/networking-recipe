// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef BASE_TABLE_TEST_H_
#define BASE_TABLE_TEST_H_

#include <stdint.h>

#include <string>

#include "gtest/gtest.h"
#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "stratum/lib/utils.h"

namespace ovsp4rt {

constexpr bool INSERT_ENTRY = true;
constexpr bool REMOVE_ENTRY = false;

// P4Info object describing the pipeline configuration.
static ::p4::config::v1::P4Info p4info;

class BaseTableTest : public ::testing::Test {
 protected:
  BaseTableTest();

  // Initializes the p4info object.
  void SetUpTestSuite();

  // Selects the table associated with the UUT.
  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        TABLE = &table;
        TABLE_ID = pre.id();
        return;
      }
    }
  }

  // Returns the object identifier of the named action.
  int GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    return -1;
  }

  // Returns the object identifier of the named match field.
  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : TABLE->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    return -1;
  }

  // Decodes a 24-bit parameter and returns its value as an unsigned integer.
  static uint32_t DecodeTunnelId(const std::string& string_value) {
    return DecodeWordValue(string_value) & 0xffffff;
  }

  // Decodes a parameter and returns its value as an unsigned integer.
  static uint32_t DecodeWordValue(const std::string& string_value) {
    uint32_t word_value = 0;
    for (int i = 0; i < string_value.size(); i++) {
      word_value = (word_value << 8) | (string_value[i] & 0xff);
    }
    return word_value;
  }

  // Dumps the table entry in JSON.
  void DumpTableEntry() const;

  // Table entry to be inserted or removed. Output of the UUT.
  ::p4::v1::TableEntry table_entry;

  // Identifier of the selected table.
  uint32_t TABLE_ID;

  // Value of the --dump_json command-line flag.
  bool dump_json_;

 private:
  // Pointer to selected table.
  const ::p4::config::v1::Table* TABLE = nullptr;
};

}  // namespace ovsp4rt

#endif  // BASE_TABLE_TEST_H_
