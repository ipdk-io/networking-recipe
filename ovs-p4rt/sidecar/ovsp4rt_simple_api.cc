// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ovsprt C API functions that do not have a C++ implementation.

#include <string.h>

#include "ovsp4rt/ovs-p4rt.h"

//----------------------------------------------------------------------
// ovsp4rt_str_to_tunnel_type (common)
//----------------------------------------------------------------------
enum ovs_tunnel_type ovsp4rt_str_to_tunnel_type(const char* tnl_type) {
  if (tnl_type) {
    if (strcmp(tnl_type, "vxlan") == 0) {
      return OVS_TUNNEL_VXLAN;
    } else if (strcmp(tnl_type, "geneve") == 0) {
      return OVS_TUNNEL_GENEVE;
    }
  }
  return OVS_TUNNEL_UNKNOWN;
}
