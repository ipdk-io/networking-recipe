// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <stdarg.h>
#include "p4rt_perf_util.h"

std::string EncodeByteValue(int arg_count...) {
  std::string byte_value;
  va_list args;
  va_start(args, arg_count);

  for (int arg = 0; arg < arg_count; ++arg) {
    uint8_t byte = va_arg(args, int);
    byte_value.push_back(byte);
  }

  va_end(args);
  return byte_value;
}

std::string CanonicalizeIp(const uint32_t ipv4addr) {
  return EncodeByteValue(4, (ipv4addr & 0xff), ((ipv4addr >> 8) & 0xff),
                         ((ipv4addr >> 16) & 0xff), ((ipv4addr >> 24) & 0xff));
}

std::string CanonicalizeIpv6(const struct in6_addr ipv6addr) {
  return EncodeByteValue(
      16, ipv6addr.__in6_u.__u6_addr8[0], ipv6addr.__in6_u.__u6_addr8[1],
      ipv6addr.__in6_u.__u6_addr8[2], ipv6addr.__in6_u.__u6_addr8[3],
      ipv6addr.__in6_u.__u6_addr8[4], ipv6addr.__in6_u.__u6_addr8[5],
      ipv6addr.__in6_u.__u6_addr8[6], ipv6addr.__in6_u.__u6_addr8[7],
      ipv6addr.__in6_u.__u6_addr8[8], ipv6addr.__in6_u.__u6_addr8[9],
      ipv6addr.__in6_u.__u6_addr8[10], ipv6addr.__in6_u.__u6_addr8[11],
      ipv6addr.__in6_u.__u6_addr8[12], ipv6addr.__in6_u.__u6_addr8[13],
      ipv6addr.__in6_u.__u6_addr8[14], ipv6addr.__in6_u.__u6_addr8[15]);
}

std::string CanonicalizeMac(const uint8_t mac[6]) {
  return EncodeByteValue(6, (mac[0] & 0xff), (mac[1] & 0xff), (mac[2] & 0xff),
                         (mac[3] & 0xff), (mac[4] & 0xff), (mac[5] & 0xff));
}

int GetTableId(const ::p4::config::v1::P4Info& p4info,
               const std::string& t_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() == t_name) return pre.id();
  }
  return -1;
}

int GetActionId(const ::p4::config::v1::P4Info& p4info,
                const std::string& a_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() == a_name) return pre.id();
  }
  return -1;
}

int GetParamId(const ::p4::config::v1::P4Info& p4info,
               const std::string& a_name, const std::string& param_name) {
  for (const auto& action : p4info.actions()) {
    const auto& pre = action.preamble();
    if (pre.name() != a_name) continue;
    for (const auto& param : action.params())
      if (param.name() == param_name) return param.id();
  }
  return -1;
}

int GetMatchFieldId(const ::p4::config::v1::P4Info& p4info,
                    const std::string& t_name, const std::string& mf_name) {
  for (const auto& table : p4info.tables()) {
    const auto& pre = table.preamble();
    if (pre.name() != t_name) continue;
    for (const auto& mf : table.match_fields())
      if (mf.name() == mf_name) return mf.id();
  }
  return -1;
}
