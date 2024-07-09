// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_ENCODE_H
#define OVSP4RT_ENCODE_H

#include <stdbool.h>

#include <nlohmann/json.hpp>

#include "ovsp4rt/ovs-p4rt.h"

namespace ovs_p4rt {

//----------------------------------------------------------------------
// Convert struct contents to JSON.
//----------------------------------------------------------------------

// Convert p4_ipaddr to json.
extern void IpAddrToJson(nlohmann::json& json, const struct p4_ipaddr& info);

// Convert ip_mac_map_info to json.
void IpMacMapInfoToJson(nlohmann::json& json, const ip_mac_map_info& info);

// Convert uint8_t mac_addr[6] to json.
void MacAddrToJson(nlohmann::json& json, const uint8_t mac_addr[6]);

// Convert mac_learning_info to json.
extern void MacLearningInfoToJson(nlohmann::json& json,
                                  const struct mac_learning_info& info);

// Convert port_vlan_info to json.
extern void PortVlanInfoToJson(nlohmann::json& json,
                               const struct port_vlan_info& info);

// Convert src_port_info to json.
extern void SrcPortInfoToJson(nlohmann::json& json,
                              const struct src_port_info& info);

// Convert tunnel_info to json.
extern void TunnelInfoToJson(nlohmann::json& json,
                             const struct tunnel_info& info);

// Convert vlan_info to json.
extern void VlanInfoToJson(nlohmann::json& json, const struct vlan_info& info);

}  // namespace ovs_p4rt

//----------------------------------------------------------------------
// Return JSON representation of API inputs.
//----------------------------------------------------------------------

extern nlohmann::json EncodeMacLearningInfo(
    const char* func_name, const struct mac_learning_info& learn_info,
    bool insert_entry);

extern nlohmann::json EncodeSrcPortInfo(const char* func_name,
                                        const struct src_port_info& sp_info,
                                        bool insert_entry);

#endif  // OVSP4RT_ENCODE_H
