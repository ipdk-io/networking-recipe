// Copyright 2024 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_encode.h"

#include <stdbool.h>
#include <stdint.h>

#include <nlohmann/json.hpp>

#include "ovsp4rt/ovs-p4rt.h"

namespace {
constexpr uint32_t LEARN_INFO_SCHEMA = 1;
constexpr uint32_t PORT_INFO_SCHEMA = 1;
}  // namespace

namespace ovs_p4rt {

//----------------------------------------------------------------------
// Convert struct contents to JSON.
//----------------------------------------------------------------------

// Convert p4_ipaddr to json.
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

// Convert ip_mac_map_info to json.
void IpMacMapInfoToJson(nlohmann::json& json, const ip_mac_map_info& info) {
  MacAddrToJson(json["src_mac_addr"], info.src_mac_addr);
  MacAddrToJson(json["dst_mac_addr"], info.dst_mac_addr);
  IpAddrToJson(json["src_ip_addr"], info.src_ip_addr);
  IpAddrToJson(json["dst_ip_addr"], info.dst_ip_addr);
}

// Convert uint8_t mac_addr[6] to json.
void MacAddrToJson(nlohmann::json& json, const uint8_t mac_addr[6]) {
  json = {mac_addr[0], mac_addr[1], mac_addr[2],
          mac_addr[3], mac_addr[4], mac_addr[5]};
}

// Convert mac_learning_info to json.
void MacLearningInfoToJson(nlohmann::json& json,
                           const struct mac_learning_info& info) {
  json["is_tunnel"] = info.is_tunnel;
  json["is_vlan"] = info.is_vlan;

  MacAddrToJson(json["mac_addr"], info.mac_addr);

  json["bridge_id"] = info.bridge_id;
  json["src_port"] = info.src_port;
  json["rx_src_port"] = info.rx_src_port;

  PortVlanInfoToJson(json["vlan_info"], info.vlan_info);

  if (info.is_tunnel) {
    TunnelInfoToJson(json["tnl_info"], info.tnl_info);
  } else if (info.is_vlan) {
    VlanInfoToJson(json["vln_info"], info.vln_info);
  }
}

// Convert port_vlan_info to json.
void PortVlanInfoToJson(nlohmann::json& json,
                        const struct port_vlan_info& info) {
  json["port_vlan_mode"] = info.port_vlan_mode;
  json["port_vlan"] = info.port_vlan;
}

// Convert src_port_info to json.
void SrcPortInfoToJson(nlohmann::json& json, const struct src_port_info& info) {
  json["bridge_id"] = info.bridge_id;
  json["vlan_id"] = info.vlan_id;
  json["src_port"] = info.src_port;
}

// Convert tunnel_info to json.
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

// Convert vlan_info to json.
void VlanInfoToJson(nlohmann::json& json, const struct vlan_info& info) {
  json["vlan_id"] = info.vlan_id;
}

//----------------------------------------------------------------------
// Return JSON representation of API input.
//----------------------------------------------------------------------

// ovsp4rt_config_fdb_entry()
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

// ovsp4rt_config_src_port_entry()
// ovsp4rt_config_tunnel_src_port_entry()
nlohmann::json EncodeSrcPortInfo(const char* func_name,
                                 const struct src_port_info& sp_info,
                                 bool insert_entry) {
  nlohmann::json json;

  json["func_name"] = func_name;
  json["schema"] = PORT_INFO_SCHEMA;
  json["struct_name"] = "src_port_info";

  auto& params = json["params"];
  SrcPortInfoToJson(params["port_info"], sp_info);
  params["insert_entry"] = insert_entry;

  return json;
}

}  // namespace ovs_p4rt
