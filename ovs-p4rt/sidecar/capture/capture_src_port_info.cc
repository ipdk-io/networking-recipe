// Copyright 2024 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdint.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "capture/ovsp4rt_capture.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace {
constexpr uint32_t PORT_INFO_SCHEMA = 1;
}

namespace ovs_p4rt {

void SrcPortInfoToJson(nlohmann::json& json,
                       const struct src_port_info& info) {
  json["bridge_id"] = info.bridge_id;
  json["vlan_id"] = info.vlan_id;
  json["src_port"] = info.src_port;
}

void CaptureSrcPortInfo(const char* func_name,
                        const struct src_port_info& sp_info,
                        bool insert_entry) {
  nlohmann::json json;

  json["func_name"] = func_name;
  json["schema"] = PORT_INFO_SCHEMA;
  json["struct_name"] = "src_port_info";

  auto& params = json["params"];
  SrcPortInfoToJson(params["port_info"], sp_info);
  params["insert_entry"] = insert_entry;

  // Return json object?
  std::cout << std::endl << json.dump(2) << std::endl;
}

}  // namespace ovs_p4rt
