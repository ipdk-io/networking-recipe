/*
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * NEXT_HOP_TABLE for Linux Networking v3.
 */

#ifndef __LNW_NEXTHOP_TABLE_H__
#define __LNW_NEXTHOP_TABLE_H__

// Verified for ES2K
#define LNW_NEXTHOP_TABLE "linux_networking_control.nexthop_table"

#define LNW_NEXTHOP_TABLE_KEY_NEXTHOP_ID "user_meta.cmeta.nexthop_id"

#define LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP_INFO \
  "linux_networking_control.set_nexthop_info_dmac"
#define LNW_ACTION_SET_NEXTHOP_PARAM_RIF "router_interface_id"
#define LNW_ACTION_SET_NEXTHOP_PARAM_EGRESS_PORT "egress_port"
#define LNW_ACTION_SET_NEXTHOP_PARAM_DMAC_HIGH "dmac_high"
#define LNW_ACTION_SET_NEXTHOP_PARAM_DMAC_LOW "dmac_low"

#define LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP_LAG \
  "linux_networking_control.set_nexthop_lag"
#define LNW_ACTION_SET_NEXTHOP_LAG_PARAM_DMAC_HIGH "dmac_high"
#define LNW_ACTION_SET_NEXTHOP_LAG_PARAM_DMAC_LOW "dmac_low"
#define LNW_ACTION_SET_NEXTHOP_LAG_PARAM_LAG_ID "lag_group_id"

#endif /* __LNW_NEXTHOP_TABLE_H__ */
