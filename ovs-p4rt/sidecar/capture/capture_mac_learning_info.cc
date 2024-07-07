// Copyright 2024 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#include <stdbool.h>
#include <stdint.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "capture/ovsp4rt_capture.h"
#include "capture/ovsp4rt_encode.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace {
constexpr uint32_t LEARN_INFO_SCHEMA = 1;
}

namespace ovs_p4rt {

void PortVlanInfoToJson(nlohmann::json& json,
                        const struct port_vlan_info& info) {
  json["port_vlan_mode"] = info.port_vlan_mode;
  json["port_vlan"] = info.port_vlan;
}

void IpAddrToJson(nlohmann::json& json, const struct p4_ipaddr& info) {
  json["family"] = info.family;
  json["prefix_len"] = info.prefix_len;

  if (info.family == AF_INET) {
    json["ipv4_addr"] = {info.ip.v4addr.s_addr};
  } else if (info.family == AF_INET6) {
    const uint32_t* v6addr = &info.ip.v6addr.__in6_u.__u6_addr32[0];
    json["ipv6_addr"] = {v6addr[0], v6addr[1], v6addr[2], v6addr[3]};
  }
}

void TunnelInfoToJson(nlohmann::json& json, const struct tunnel_info& info) {
  json["ifindex"] = info.ifindex;
  json["port_id"] = info.port_id;
  json["src_port"] = info.src_port;

  IpAddrToJson(json["local_ip"], info.local_ip);
  IpAddrToJson(json["remote_ip"], info.remote_ip);

  json["dst_port"] = info.dst_port;
  json["vni"] = info.vni;

  PortVlanInfoToJson(json["vlan_info"], info.vlan_info);

  json["bridge_id"] = info.bridge_id;
  json["tunnel_type"] = info.tunnel_type;
}

void VlanInfoToJson(nlohmann::json& json, const struct vlan_info& info) {
  json["vlan_id"] = info.vlan_id;
}

void MacLearningInfoToJson(nlohmann::json& json,
                           const struct mac_learning_info& info) {
  json["is_tunnel"] = info.is_tunnel;
  json["is_vlan"] = info.is_vlan;
  json["mac_addr"] = {info.mac_addr[0], info.mac_addr[1], info.mac_addr[2],
                      info.mac_addr[3], info.mac_addr[4], info.mac_addr[5]};
  json["bridge_id"] = info.bridge_id;
  json["src_port"] = info.src_port;
  json["rx_src_port"] = info.rx_src_port;

  PortVlanInfoToJson(json["vlan_info"], info.vlan_info);

  if (info.is_tunnel) {
    TunnelInfoToJson(json["tnl_info"], info.tnl_info);
  } else if (info.is_vlan) {
    VlanInfoToJson(json["vlan_info"], info.vln_info);
  }
}

nlohmann::json EncodeMacLearningInfo(const char* func_name,
                                     const struct mac_learning_info& learn_info,
                                     bool insert_entry) {
  nlohmann::json json;

  json["func_name"] = func_name;
  json["schema"] = LEARN_INFO_SCHEMA;
  json["struct_name"] = "mac_learning_info";

  auto& params = json["params"];
  MacLearningInfoToJson(params["learn_info"], learn_info);
  params["insert_entry"] = insert_entry;

  return json;
}

void CaptureMacLearningInfo(const char* func_name,
                            const struct mac_learning_info& learn_info,
                            bool insert_entry) {
  auto json = EncodeMacLearningInfo(func_name, learn_info, insert_entry);
  std::cout << std::endl << json.dump(2) << std::endl;
}

}  // namespace ovs_p4rt
