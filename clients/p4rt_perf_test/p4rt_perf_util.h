// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef P4RT_PERF_UTIL_H
#define P4RT_PERF_UTIL_H

#include <arpa/inet.h>

#include "p4/v1/p4runtime.grpc.pb.h"
#include "p4/v1/p4runtime.pb.h"

std::string EncodeByteValue(int arg_count...);
std::string CanonicalizeIp(const uint32_t ipv4addr);
std::string CanonicalizeIpv6(const struct in6_addr ipv6addr);
std::string CanonicalizeMac(const uint8_t mac[6]);

int GetTableId(const ::p4::config::v1::P4Info& p4info,
               const std::string& t_name);

int GetActionId(const ::p4::config::v1::P4Info& p4info,
                const std::string& a_name);

int GetParamId(const ::p4::config::v1::P4Info& p4info,
               const std::string& a_name, const std::string& param_name);

int GetMatchFieldId(const ::p4::config::v1::P4Info& p4info,
                    const std::string& t_name, const std::string& mf_name);

#endif  // P4RT_PERF_UTIL_H
