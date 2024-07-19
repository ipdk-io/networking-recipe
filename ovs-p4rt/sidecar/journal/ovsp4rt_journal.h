// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_JOURNAL_H_
#define OVSP4RT_JOURNAL_H_

#include <stdbool.h>
#include <stdint.h>

#include <nlohmann/json.hpp>
#include <vector>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

// Captures the inputs and outputs to an API function.
class Journal {
 public:
  void recordInput(const char* func_name, const struct mac_learning_info& info,
                   bool insert_entry);

  void recordInput(const char* func_name, const ip_mac_map_info& info,
                   bool insert_entry);

  void recordInput(const char* func_name, const tunnel_info& info,
                   bool insert_entry);

  void recordInput(const char* func_name, const src_port_info info,
                   bool insert_entry);

  void recordInput(const char* func_name, uint16_t vlan_id, bool insert_entry);

  void recordOutput(const char* func, ::p4::v1::WriteRequest& request) {}

  void saveEntry() {}

 private:
  nlohmann::json input_;
  std::vector<nlohmann::json> output_;
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_JOURNAL_H_
