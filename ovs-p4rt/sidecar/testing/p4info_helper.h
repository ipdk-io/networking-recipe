// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef P4INFO_HELPER_H_
#define P4INFO_HELPER_H_

#include <iostream>
#include <string>

#include "p4/config/v1/p4info.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

class P4InfoHelper {
 public:
  P4InfoHelper(const ::p4::config::v1::P4Info& p4info)
      : p4info_(p4info),
        table_(nullptr),
        action_(nullptr),
        table_id_(-1),
        action_id_(-1) {}

  int GetActionId(const std::string& action_name) const {
    for (const auto& action : p4info_.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        return pre.id();
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
    return -1;
  }

  int GetMatchFieldId(const std::string& mf_name) const {
    for (const auto& mf : table_->match_fields()) {
      if (mf.name() == mf_name) {
        return mf.id();
      }
    }
    std::cerr << "Match Field '" << mf_name << "' not found!\n";
    return -1;
  }

  int GetParamId(const std::string& param_name) const {
    for (const auto& param : action_->params()) {
      if (param.name() == param_name) {
        return param.id();
      }
    }
    std::cerr << "Action Param '" << param_name << "' not found!\n";
    return -1;
  }

  void SelectAction(const std::string& action_name) {
    for (const auto& action : p4info_.actions()) {
      const auto& pre = action.preamble();
      if (pre.name() == action_name || pre.alias() == action_name) {
        action_ = &action;
        action_id_ = pre.id();
        return;
      }
    }
    std::cerr << "Action '" << action_name << "' not found!\n";
  }

  void SelectTable(const std::string& table_name) {
    for (const auto& table : p4info_.tables()) {
      const auto& pre = table.preamble();
      if (pre.name() == table_name || pre.alias() == table_name) {
        table_ = &table;
        table_id_ = pre.id();
        return;
      }
    }
    std::cerr << "Table '" << table_name << "' not found!\n";
  }

  int table_id() const { return table_id_; }
  int action_id() const { return action_id_; }
  bool has_table() const { return table_ != nullptr; }

 private:
  const ::p4::config::v1::P4Info& p4info_;

  const ::p4::config::v1::Table* table_;
  const ::p4::config::v1::Action* action_;

  int table_id_;
  int action_id_;
};

}  // namespace ovsp4rt

#endif  // P4INFO_HELPER_H_
