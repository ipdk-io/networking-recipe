// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef CAPTURE_API_INPUT_H_
#define CAPTURE_API_INPUT_H_

#include <stdbool.h>
#include <stdint.h>

#include <nlohmann/json.hpp>

#include "ovsp4rt/ovs-p4rt.h"

#define CAPTURE_API_INPUT(_info, _insert_entry, _envoy) \
  do {                                                  \
    CaptureApiInput _input;                             \
    _input.encode(__func__, _info, _insert_entry);      \
    _envoy.captureInput(_input);                        \
  } while (0)

namespace ovs_p4rt {

class CaptureApiInput {
 public:
  CaptureApiInput();

  void encode(const char* func_name, const mac_learning_info& info,
              bool insert_entry);

  void encode(const char* func_name, const ip_mac_map_info& info,
              bool insert_entry) {}

  void encode(const char* func_name, const tunnel_info& info,
              bool insert_entry) {}

  void encode(const char* func_name, const src_port_info info,
              bool insert_entry) {}

  void encode(const char* func_name, uint16_t vlan_id, bool insert_entry) {}

  nlohmann::json json() const { return json_; }

 private:
  nlohmann::json json_;
};

}  // namespace ovs_p4rt

#endif  // CAPTURE_API_INPUT_H_
