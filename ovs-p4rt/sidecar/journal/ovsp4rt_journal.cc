// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_journal.h"

#include "ovsp4rt_encode.h"

namespace ovsp4rt {

// mac_learning_info
void Journal::recordInput(const char* func_name,
                          const struct mac_learning_info& info,
                          bool insert_entry) {
  input_ = EncodeMacLearningInfo(func_name, info, insert_entry);
}

// ip_mac_map_info
void Journal::recordInput(const char* func_name, const ip_mac_map_info& info,
                          bool insert_entry) {
  input_ = EncodeIpMacMapInfo(func_name, info, insert_entry);
}

// tunnel_info
void Journal::recordInput(const char* func_name, const tunnel_info& info,
                          bool insert_entry) {
  input_ = EncodeTunnelInfo(func_name, info, insert_entry);
}

// src_port_info
void Journal::recordInput(const char* func_name, const src_port_info info,
                          bool insert_entry) {
  input_ = EncodeSrcPortInfo(func_name, info, insert_entry);
}

// vlan_id
void Journal::recordInput(const char* func_name, uint16_t vlan_id,
                          bool insert_entry) {
  input_ = EncodeVlanId(func_name, vlan_id, insert_entry);
}

void Journal::recordReadRequest(const ::p4::v1::ReadRequest& request) {
  // TODO(derek): to be implemented. Add func_name parameter?
}

void Journal::recordReadResponse(
    const absl::StatusOr<::p4::v1::ReadResponse>& response) {
  // TODO(derek): to be implemented. Add func_name parameter?
}

void Journal::recordWriteRequest(const ::p4::v1::WriteRequest& request) {
  // TODO(derek): to be implemented. Add func_name parameter?
}

void Journal::recordWriteStatus(const absl::Status& status) {
  // TODO(derek): to be implemented. Add func_name parameter?
}

}  // namespace ovsp4rt
