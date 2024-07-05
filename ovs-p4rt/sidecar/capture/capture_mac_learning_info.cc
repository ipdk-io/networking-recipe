/*
 * Copyright 2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdint.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "capture/ovsp4rt_capture.h"
#include "ovsp4rt/ovs-p4rt.h"

namespace ovs_p4rt {

void CaptureMacLearningInfo(const char* func_name,
                            const struct mac_learning_info& learn_info,
                            bool insert_entry) {
  nlohmann::json json_info;

  json_info["name"] = func_name;
  json_info["version"] = 1;

  // discrete fields
  auto& fdb_info = json_info["params"]["learn_info"];
  fdb_info["is_tunnel"] = learn_info.is_tunnel;
  fdb_info["is_vlan"] = learn_info.is_vlan;
  fdb_info["mac_addr"] = {learn_info.mac_addr[0], learn_info.mac_addr[1],
                          learn_info.mac_addr[2], learn_info.mac_addr[3],
                          learn_info.mac_addr[4], learn_info.mac_addr[5]};
  fdb_info["bridge_id"] = learn_info.bridge_id;
  fdb_info["src_port"] = learn_info.src_port;
  fdb_info["rx_src_port"] = learn_info.rx_src_port;

  // vlan_info
  auto& vlan_info = fdb_info["vlan_info"];
  vlan_info["port_vlan_mode"] = learn_info.vlan_info.port_vlan_mode;
  vlan_info["port_vlan"] = learn_info.vlan_info.port_vlan;

  // tunnel_info
  auto& tnl_info = fdb_info["tnl_info"];
  tnl_info["ifindex"] = learn_info.tnl_info.ifindex;
  tnl_info["port_id"] = learn_info.tnl_info.port_id;
  tnl_info["src_port"] = learn_info.tnl_info.src_port;

  // local ip address
  const auto& learn_local_ip = learn_info.tnl_info.local_ip;
  auto& local_ip = tnl_info["local_ip"];
  local_ip["family"] = learn_local_ip.family;
  local_ip["prefix_len"] = learn_local_ip.prefix_len;

  if (learn_local_ip.family == AF_INET) {
    local_ip["ipv4_addr"] = {learn_local_ip.ip.v4addr.s_addr};
  } else if (learn_local_ip.family == AF_INET6) {
    const uint32_t* v6addr = &learn_local_ip.ip.v6addr.__in6_u.__u6_addr32[0];
    local_ip["ipv6_addr"] = {v6addr[0], v6addr[1], v6addr[2], v6addr[3]};
  }

  // remote ip address
  const auto& learn_remote_ip = learn_info.tnl_info.remote_ip;
  auto& remote_ip = tnl_info["remote_ip"];
  remote_ip["family"] = learn_remote_ip.family;
  remote_ip["prefix_len"] = learn_remote_ip.prefix_len;

  if (learn_remote_ip.family == AF_INET) {
    remote_ip["ipv4_addr"] = {learn_remote_ip.ip.v4addr.s_addr};
  } else if (learn_remote_ip.family == AF_INET6) {
    const uint32_t* v6addr = &learn_remote_ip.ip.v6addr.__in6_u.__u6_addr32[0];
    remote_ip["ipv6_addr"] = {v6addr[0], v6addr[1], v6addr[2], v6addr[3]};
  }

  json_info["params"]["insert_entry"] = insert_entry;

  std::cout << std::endl << json_info.dump(2) << std::endl;
}

}  // namespace ovs_p4rt
