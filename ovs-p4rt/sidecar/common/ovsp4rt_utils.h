// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_UTILS_H_
#define OVSP4RT_UTILS_H_

#include <netinet/in.h>

#include <cstdint>
#include <string>

#include "p4/config/v1/p4info.pb.h"

namespace ovs_p4rt {

extern std::string CanonicalizeIp(const uint32_t ipv4addr);

extern std::string CanonicalizeIpv6(const struct in6_addr ipv6addr);

extern std::string CanonicalizeMac(const uint8_t mac[6]);

extern std::string EncodeByteValue(int arg_count...);

extern int GetActionId(const ::p4::config::v1::P4Info& p4info,
                       const std::string& a_name);

extern int GetMatchFieldId(const ::p4::config::v1::P4Info& p4info,
                           const std::string& t_name,
                           const std::string& mf_name);

extern int GetParamId(const ::p4::config::v1::P4Info& p4info,
                      const std::string& a_name, const std::string& param_name);

extern int GetTableId(const ::p4::config::v1::P4Info& p4info,
                      const std::string& t_name);

static inline int32_t ValidIpAddr(uint32_t nw_addr) {
  return (nw_addr && nw_addr != INADDR_ANY && nw_addr != INADDR_LOOPBACK &&
          nw_addr != 0xffffffff);
}

}  // namespace ovs_p4rt

#endif  // OVSP4RT_UTILS_H_
