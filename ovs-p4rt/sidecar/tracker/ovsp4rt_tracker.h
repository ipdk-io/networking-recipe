// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_TRACKER_H_
#define OVSP4RT_TRACKER_H_

#include <stdbool.h>
#include <stdint.h>

#include <nlohmann/json.hpp>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

// Captures the inputs and outputs to an API function.
class Tracker {
 public:
  void captureInput(const char* func_name, const struct mac_learning_info& info,
                    bool insert_entry);

  void captureInput(const char* func_name, const ip_mac_map_info& info,
                    bool insert_entry);

  void captureInput(const char* func_name, const tunnel_info& info,
                    bool insert_entry);

  void captureInput(const char* func_name, const src_port_info info,
                    bool insert_entry);

  void captureInput(const char* func_name, uint16_t vlan_id, bool insert_entry);

  void captureOutput(const char* func, ::p4::v1::WriteRequest& request) {}

  void saveSnapshot() {}

 private:
  nlohmann::json input_;
  nlohmann::json output_;
};

}  // namespace ovs_p4rt

#endif  // OVSP4RT_TRACKER_H_
