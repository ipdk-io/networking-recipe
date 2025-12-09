/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SWITCH_TABLE_H__
#define __SWITCH_TABLE_H__

#include "switch_base_types.h"
#include "switchutils/switch_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWITCH_TABLE_ID_VALID(_table_id) \
  _table_id > SWITCH_TABLE_NONE&& _table_id < SWITCH_TABLE_MAX

typedef enum switch_table_id_ {

  SWITCH_TABLE_NONE = 0,

  /* Ingress Port */
  SWITCH_TABLE_INGRESS_PORT_MAPPING = 1,
  SWITCH_TABLE_EGRESS_PORT_MAPPING = 2,
  SWITCH_TABLE_INGRESS_PORT_PROPERTIES = 3,

  /* Rmac */
  SWITCH_TABLE_OUTER_RMAC = 21,
  SWITCH_TABLE_INNER_RMAC = 22,

  /* L2 */
  SWITCH_TABLE_SMAC = 41,
  SWITCH_TABLE_DMAC = 42,

  /* FIB */
  SWITCH_TABLE_IPV4_HOST = 61,
  SWITCH_TABLE_IPV6_HOST = 62,
  SWITCH_TABLE_IPV4_LPM = 63,
  SWITCH_TABLE_IPV6_LPM = 64,
  SWITCH_TABLE_SMAC_REWRITE = 65,
  SWITCH_TABLE_MTU = 66,
  SWITCH_TABLE_URPF = 67,
  SWITCH_TABLE_IPV4_LOCAL_HOST = 68,

  /* Nexthop */
  SWITCH_TABLE_NHOP = 81,
  SWITCH_TABLE_NHOP_GROUP = 82,
  SWITCH_TABLE_NHOP_MEMBER_SELECT = 83,

  /* Rewrite */
  SWITCH_TABLE_REWRITE = 101,

  /* Tunnel */
  SWITCH_TABLE_IPV4_SRC_VTEP = 121,
  SWITCH_TABLE_IPV4_DST_VTEP = 122,
  SWITCH_TABLE_IPV6_SRC_VTEP = 123,
  SWITCH_TABLE_IPV6_DST_VTEP = 124,
  SWITCH_TABLE_TUNNEL = 125,
  SWITCH_TABLE_TUNNEL_REWRITE = 126,
  SWITCH_TABLE_TUNNEL_DECAP = 127,
  SWITCH_TABLE_TUNNEL_SMAC_REWRITE = 128,
  SWITCH_TABLE_TUNNEL_DMAC_REWRITE = 129,
  SWITCH_TABLE_TUNNEL_SIP_REWRITE = 130,
  SWITCH_TABLE_IPV4_TUNNEL_DIP_REWRITE = 131,
  SWITCH_TABLE_IPV6_TUNNEL_DIP_REWRITE = 132,
  SWITCH_TABLE_TUNNEL_MPLS = 133,
  SWITCH_TABLE_IPV4_VTEP = 134,
  SWITCH_TABLE_IPV6_VTEP = 135,
  SWITCH_TABLE_TUNNEL_TERM = 136,

  /* BD */
  SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING = 141,
  SWITCH_TABLE_BD = 142,
  SWITCH_TABLE_BD_FLOOD = 143,
  SWITCH_TABLE_INGRESS_BD_STATS = 144,
  SWITCH_TABLE_VLAN_DECAP = 145,
  SWITCH_TABLE_VLAN_XLATE = 146,
  SWITCH_TABLE_EGRESS_BD = 147,
  SWITCH_TABLE_EGRESS_BD_STATS = 148,
  SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING = 149,

  /* acl stats */
  SWITCH_TABLE_ACL_STATS = 151,
  SWITCH_TABLE_RACL_STATS = 152,
  SWITCH_TABLE_EGRESS_ACL_STATS = 153,

  /* ACL */
  SWITCH_TABLE_IPV4_ACL = 161,
  SWITCH_TABLE_IPV6_ACL = 162,
  SWITCH_TABLE_IPV4_RACL = 163,
  SWITCH_TABLE_IPV6_RACL = 164,
  SWITCH_TABLE_SYSTEM_ACL = 165,
  SWITCH_TABLE_MAC_ACL = 166,
  SWITCH_TABLE_EGRESS_SYSTEM_ACL = 167,
  SWITCH_TABLE_EGRESS_IPV4_ACL = 168,
  SWITCH_TABLE_EGRESS_IPV6_ACL = 169,
  SWITCH_TABLE_IPV4_MIRROR_ACL = 170,
  SWITCH_TABLE_IPV6_MIRROR_ACL = 171,
  SWITCH_TABLE_ECN_ACL = 172,
  SWITCH_TABLE_MAC_QOS_ACL = 173,
  SWITCH_TABLE_IPV4_QOS_ACL = 174,
  SWITCH_TABLE_LOW_PRI_SYSTEM_ACL = 175,
  SWITCH_TABLE_SYSTEM_REASON_ACL = 176,
  SWITCH_TABLE_EGRESS_IPV4_MIRROR_ACL = 177,
  SWITCH_TABLE_EGRESS_IPV6_MIRROR_ACL = 178,
  SWITCH_TABLE_IPV4_DTEL_ACL = 179,
  SWITCH_TABLE_IPV6_DTEL_ACL = 180,
  SWITCH_TABLE_PFC_ACL = 181,
  SWITCH_TABLE_EGRESS_PFC_ACL = 182,
  // insert any new acl tables here
  // increment the below MAX_ACL enum value
  SWITCH_TABLE_MAX_ACL = 183,

  /* Multicast */
  SWITCH_TABLE_OUTER_MCAST_STAR_G = 184,
  SWITCH_TABLE_OUTER_MCAST_SG = 185,
  SWITCH_TABLE_OUTER_MCAST_RPF = 186,
  SWITCH_TABLE_MCAST_RPF = 187,
  SWITCH_TABLE_IPV4_MCAST_S_G = 188,
  SWITCH_TABLE_IPV4_MCAST_STAR_G = 189,
  SWITCH_TABLE_IPV6_MCAST_S_G = 190,
  SWITCH_TABLE_IPV6_MCAST_STAR_G = 191,
  SWITCH_TABLE_RID = 192,
  SWITCH_TABLE_REPLICA_TYPE = 193,
  SWITCH_TABLE_IPV4_BRIDGE_MCAST_S_G = 194,
  SWITCH_TABLE_IPV4_BRIDGE_MCAST_STAR_G = 195,
  SWITCH_TABLE_IPV6_BRIDGE_MCAST_S_G = 196,
  SWITCH_TABLE_IPV6_BRIDGE_MCAST_STAR_G = 197,

  /* STP */
  SWITCH_TABLE_STP = 201,

  /* LAG */
  SWITCH_TABLE_LAG_GROUP = 221,
  SWITCH_TABLE_LAG_SELECT = 222,

  /* Mirror */
  SWITCH_TABLE_MIRROR = 241,

  /* Meter */
  SWITCH_TABLE_METER_INDEX = 261,
  SWITCH_TABLE_METER_ACTION = 262,
  SWITCH_TABLE_EGRESS_METER_INDEX = 263,
  SWITCH_TABLE_EGRESS_METER_ACTION = 264,

  /* Stats */
  SWITCH_TABLE_DROP_STATS = 281,

  /* Nat */
  SWITCH_TABLE_NAT_DST = 301,
  SWITCH_TABLE_NAT_SRC = 302,
  SWITCH_TABLE_NAT_TWICE = 303,
  SWITCH_TABLE_NAT_FLOW = 304,

  /* Qos */
  SWITCH_TABLE_INGRESS_QOS_MAP = 320,
  SWITCH_TABLE_INGRESS_QOS_MAP_DSCP = 321,
  SWITCH_TABLE_INGRESS_QOS_MAP_PCP = 322,
  SWITCH_TABLE_QUEUE = 323,
  SWITCH_TABLE_EGRESS_QOS_MAP = 324,

  /* Wred */
  SWITCH_TABLE_WRED = 341,

  /* Meter color action */
  SWITCH_TABLE_METER_COLOR_ACTION = 342,
  SWITCH_TABLE_NEIGHBOR = 343,
  SWITCH_TABLE_MAX = 512

} switch_table_id_t;

typedef struct switch_table_s {
  bool valid;
  switch_size_t table_size;
  switch_size_t num_entries;
  switch_direction_t direction;
  char table_name[SWITCH_MAX_STRING_SIZE];
} switch_table_t;

switch_status_t switch_api_table_size_get(switch_device_t device,
                                          switch_table_id_t table_id,
                                          switch_size_t* table_size);

switch_status_t switch_api_table_all_get(switch_device_t device,
                                         switch_size_t* num_entries,
                                         switch_table_t* api_table_info);

switch_status_t switch_api_table_entry_count_get(switch_device_t device,
                                                 switch_table_id_t table_id,
                                                 switch_uint32_t* num_entries);

switch_status_t switch_table_init(switch_device_t device,
                                  switch_size_t* table_sizes);

switch_status_t switch_table_free(switch_device_t device);

switch_status_t switch_api_table_size_get_internal(switch_device_t device,
                                                   switch_table_id_t table_id,
                                                   switch_size_t* table_size);

switch_status_t switch_table_default_sizes_get(switch_size_t* table_sizes);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_TABLE_H__ */
