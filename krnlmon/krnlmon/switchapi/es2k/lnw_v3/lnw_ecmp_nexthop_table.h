/*
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * ECMP_NEXTHOP_TABLE for Linux Networking v3.
 */

#ifndef __LNW_ECMP_NEXTHOP_TABLE_H__
#define __LNW_ECMP_NEXTHOP_TABLE_H__

// Verified for ES2K
#define LNW_ECMP_NEXTHOP_TABLE "linux_networking_control.ecmp_nexthop_table"

#define LNW_ECMP_NEXTHOP_TABLE_KEY_ECMP_NEXTHOP_ID "user_meta.cmeta.nexthop_id"

#define LNW_ECMP_NEXTHOP_TABLE_ACTION_SET_ECMP_NEXTHOP_INFO_DMAC \
  "linux_networking_control.ecmp_set_nexthop_info_dmac"
#define LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_RIF "router_interface_id"
#define LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_DMAC_HIGH "dmac_high"
#define LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_DMAC_LOW "dmac_low"
#define LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_EGRESS_PORT "egress_port"

#endif /* __LNW_ECMP_NEXTHOP_TABLE_H__ */
