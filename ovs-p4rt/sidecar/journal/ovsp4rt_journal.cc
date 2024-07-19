// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_journal.h"

#include "ovsp4rt_encode.h"

namespace {
constexpr uint32_t LEARN_INFO_SCHEMA = 1;
constexpr uint32_t PORT_INFO_SCHEMA = 1;
}  // namespace

namespace ovsp4rt {

// mac_learning_info
void Journal::recordInput(const char* func_name,
                          const struct mac_learning_info& info,
                          bool insert_entry) {
  input_ = EncodeMacLearningInfo(func_name, info, insert_entry);
}

// ip_mac_map_info
void Journal::recordInput(const char* func_name, const ip_mac_map_info& info,
                          bool insert_entry) {}

// tunnel_info
void Journal::recordInput(const char* func_name, const tunnel_info& info,
                          bool insert_entry) {}

// src_port_info
void Journal::recordInput(const char* func_name, const src_port_info info,
                          bool insert_entry) {
  input_ = EncodeSrcPortInfo(func_name, info, insert_entry);
}

// vlan_id
void Journal::recordInput(const char* func_name, uint16_t vlan_id,
                          bool insert_entry) {}

// ::p4::v1::WriteRequest
void recordOutput(const char* func, ::p4::v1::WriteRequest& request) {}

}  // namespace ovsp4rt
