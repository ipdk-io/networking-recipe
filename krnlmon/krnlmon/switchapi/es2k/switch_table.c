/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2023 Intel Corporation.
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

#include "switchapi/switch_table.h"

#include "switchapi/switch_internal.h"

#define __FILE_ID__ SWITCH_TABLE

static const char* switch_table_id_to_string(switch_table_id_t table_id) {
  switch (table_id) {
    case SWITCH_TABLE_INGRESS_PORT_MAPPING:
      return "ingress port mapping";
    case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
      return "ingress port properties";
    case SWITCH_TABLE_EGRESS_PORT_MAPPING:
      return "egress port mapping";

    /* Rmac */
    case SWITCH_TABLE_OUTER_RMAC:
      return "outer rmac";
    case SWITCH_TABLE_INNER_RMAC:
      return "inner rmac";

    /* L2 */
    case SWITCH_TABLE_SMAC:
      return "smac";
    case SWITCH_TABLE_DMAC:
      return "dmac";

    /* FIB */
    case SWITCH_TABLE_IPV4_HOST:
      return "ipv4 host";
    case SWITCH_TABLE_IPV6_HOST:
      return "ipv6 host";
    case SWITCH_TABLE_IPV4_LPM:
      return "ipv4 lpm";
    case SWITCH_TABLE_IPV6_LPM:
      return "ipv6 lpm";
    case SWITCH_TABLE_SMAC_REWRITE:
      return "smac rewrite";
    case SWITCH_TABLE_MTU:
      return "mtu";
    case SWITCH_TABLE_URPF:
      return "urpf";
    case SWITCH_TABLE_IPV4_LOCAL_HOST:
      return "ipv4 local host";

    /* Nexthop */
    case SWITCH_TABLE_NHOP:
      return "nexthop";
    case SWITCH_TABLE_NHOP_GROUP:
      return "nhop group";
    case SWITCH_TABLE_NHOP_MEMBER_SELECT:
      return "nhop member select";

    /* Rewrite */
    case SWITCH_TABLE_REWRITE:
      return "rewrite";

    /* Tunnel */
    case SWITCH_TABLE_IPV4_SRC_VTEP:
      return "ipv4 src vtep";
    case SWITCH_TABLE_IPV4_DST_VTEP:
      return "ipv4 dst vtep";
    case SWITCH_TABLE_IPV6_SRC_VTEP:
      return "ipv6 src vtep";
    case SWITCH_TABLE_IPV6_DST_VTEP:
      return "ipv6 dst vtep";
    case SWITCH_TABLE_TUNNEL:
      return "tunnel";
    case SWITCH_TABLE_TUNNEL_REWRITE:
      return "tunnel rewrite";
    case SWITCH_TABLE_TUNNEL_DECAP:
      return "tunnel decap";
    case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
      return "tunnel smac rewrite";
    case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
      return "tunnel dmac rewrite";
    case SWITCH_TABLE_IPV4_TUNNEL_DIP_REWRITE:
      return "tunnel dip v4 rewrite";
    case SWITCH_TABLE_IPV6_TUNNEL_DIP_REWRITE:
      return "tunnel dip v6 rewrite";
    case SWITCH_TABLE_TUNNEL_MPLS:
      return "mpls";
    case SWITCH_TABLE_IPV4_VTEP:
      return "ipv4 vtep";
    case SWITCH_TABLE_IPV6_VTEP:
      return "ipv4 vtep";
    case SWITCH_TABLE_TUNNEL_TERM:
      return "tunnel term";
    case SWITCH_TABLE_EGRESS_IPV4_ACL:
      return "egress ipv4 acl";
    case SWITCH_TABLE_EGRESS_IPV6_ACL:
      return "egress ipv6 acl";
    case SWITCH_TABLE_ECN_ACL:
      return "ecn acl";
    case SWITCH_TABLE_PFC_ACL:
      return "pfc acl";
    case SWITCH_TABLE_EGRESS_PFC_ACL:
      return "egress pfc acl";
    case SWITCH_TABLE_MAX_ACL:
      return "max acl";
    case SWITCH_TABLE_INGRESS_QOS_MAP:
      return "ingress qos map";
    case SWITCH_TABLE_WRED:
      return "wred";
    case SWITCH_TABLE_NEIGHBOR:
      return "neighbor";
    case SWITCH_TABLE_TUNNEL_SIP_REWRITE:
      return "tunnel sip rewrite";
    case SWITCH_TABLE_MAX:
      return "none";
    /* BD */
    case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
      return "port vlan bd mapping";
    case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
      return "port vlan ifindex mapping";
    case SWITCH_TABLE_BD:
      return "bd";
    case SWITCH_TABLE_BD_FLOOD:
      return "bd flood";
    case SWITCH_TABLE_INGRESS_BD_STATS:
      return "ingress bd stats";
    case SWITCH_TABLE_EGRESS_BD_STATS:
      return "egress bd stats";
    case SWITCH_TABLE_VLAN_DECAP:
      return "vlan decap";
    case SWITCH_TABLE_VLAN_XLATE:
      return "vlan xlate";
    case SWITCH_TABLE_EGRESS_BD:
      return "egress bd";

    /* ACL */
    case SWITCH_TABLE_IPV4_ACL:
      return "ipv4 acl";
    case SWITCH_TABLE_IPV6_ACL:
      return "ipv6 acl";
    case SWITCH_TABLE_IPV4_RACL:
      return "ipv4 racl";
    case SWITCH_TABLE_IPV6_RACL:
      return "ipv6 racl";
    case SWITCH_TABLE_SYSTEM_ACL:
      return "system acl";
    case SWITCH_TABLE_MAC_ACL:
      return "mac acl";
    case SWITCH_TABLE_MAC_QOS_ACL:
      return "mac qos acl";
    case SWITCH_TABLE_IPV4_QOS_ACL:
      return "ipv4 qos acl";
    case SWITCH_TABLE_IPV4_DTEL_ACL:
      return "ipv4 dtel acl";
    case SWITCH_TABLE_IPV6_DTEL_ACL:
      return "ipv6 dtel acl";
    case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
      return "egress system acl";
    case SWITCH_TABLE_ACL_STATS:
      return "acl stats";
    case SWITCH_TABLE_RACL_STATS:
      return "racl stats";
    case SWITCH_TABLE_EGRESS_ACL_STATS:
      return "egress_acl stats";
    case SWITCH_TABLE_LOW_PRI_SYSTEM_ACL:
      return "low priority system acl";
    case SWITCH_TABLE_SYSTEM_REASON_ACL:
      return "system reason acl";

    /* Multicast */
    case SWITCH_TABLE_OUTER_MCAST_STAR_G:
      return "outer mcast star g";
    case SWITCH_TABLE_OUTER_MCAST_SG:
      return "outer mcast sg";
    case SWITCH_TABLE_IPV4_MCAST_S_G:
      return "ipv4 mcast sg";
    case SWITCH_TABLE_IPV4_MCAST_STAR_G:
      return "ipv4 mcast star g";
    case SWITCH_TABLE_IPV6_MCAST_S_G:
      return "ipv6 mcast sg";
    case SWITCH_TABLE_IPV6_MCAST_STAR_G:
      return "ipv6 mcast star g";
    case SWITCH_TABLE_OUTER_MCAST_RPF:
      return "outer mcast rpf";
    case SWITCH_TABLE_MCAST_RPF:
      return "mcast rpf";
    case SWITCH_TABLE_RID:
      return "rid";
    case SWITCH_TABLE_REPLICA_TYPE:
      return "replica type";
    case SWITCH_TABLE_IPV4_BRIDGE_MCAST_S_G:
      return "ipv4 bridge mcast sg";
    case SWITCH_TABLE_IPV4_BRIDGE_MCAST_STAR_G:
      return "ipv4 bridge mcast star g";
    case SWITCH_TABLE_IPV6_BRIDGE_MCAST_S_G:
      return "ipv6 bridge mcast sg";
    case SWITCH_TABLE_IPV6_BRIDGE_MCAST_STAR_G:
      return "ipv6 bridge mcast star g";

    /* STP */
    case SWITCH_TABLE_STP:
      return "stp";

    /* LAG */
    case SWITCH_TABLE_LAG_GROUP:
      return "lag group";
    case SWITCH_TABLE_LAG_SELECT:
      return "lag select";

    /* Mirror */
    case SWITCH_TABLE_MIRROR:
      return "mirror";
    case SWITCH_TABLE_IPV4_MIRROR_ACL:
      return "ingress ipv4 mirror acl";
    case SWITCH_TABLE_IPV6_MIRROR_ACL:
      return "ingress ipv6 mirror acl";
    case SWITCH_TABLE_EGRESS_IPV4_MIRROR_ACL:
      return "egress ipv4 mirror acl";
    case SWITCH_TABLE_EGRESS_IPV6_MIRROR_ACL:
      return "egress ipv6 mirror acl";

    /* Meter */
    case SWITCH_TABLE_METER_INDEX:
      return "ingress meter index";
    case SWITCH_TABLE_METER_ACTION:
      return "ingress meter action";

    /* Egress Meter */
    case SWITCH_TABLE_EGRESS_METER_INDEX:
      return "egress meter index";
    case SWITCH_TABLE_EGRESS_METER_ACTION:
      return "egress meter action";

    /* Stats */
    case SWITCH_TABLE_DROP_STATS:
      return "drop stats";

    /* Nat */
    case SWITCH_TABLE_NAT_DST:
      return "nat dst";
    case SWITCH_TABLE_NAT_SRC:
      return "nat src";
    case SWITCH_TABLE_NAT_TWICE:
      return "nat twice";
    case SWITCH_TABLE_NAT_FLOW:
      return "nat flow";

    /* Qos */
    case SWITCH_TABLE_INGRESS_QOS_MAP_DSCP:
      return "ingress qos map dscp";
    case SWITCH_TABLE_INGRESS_QOS_MAP_PCP:
      return "ingress qos map pcp";
    case SWITCH_TABLE_QUEUE:
      return "queue";
    case SWITCH_TABLE_EGRESS_QOS_MAP:
      return "egress qos map";

    case SWITCH_TABLE_METER_COLOR_ACTION:
      return "meter color action";

    case SWITCH_TABLE_NONE:
    default:
      return "unknown";
  }
}

switch_status_t switch_table_init(switch_device_t device,
                                  switch_size_t* table_sizes) {
  switch_table_t* table_info = NULL;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  const switch_char_t* table_str = NULL;

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("table init: Failed to get table info on device %d:%s",
                      device, switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    table_info[index].table_size = table_sizes[index];
    table_info[index].direction = switch_table_id_to_direction(index);
    table_info[index].num_entries = 0;
    if (table_sizes[index]) {
      table_info[index].valid = TRUE;
      table_str = switch_table_id_to_string(index);
      SWITCH_MEMCPY(&table_info[index].table_name, table_str,
                    strlen(table_str));
    }
  }

  return status;
}

switch_status_t switch_table_free(switch_device_t device) {
  switch_table_t* table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("table free: Failed to get table info on device %d:%s",
                      device, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(table_info, 0, SWITCH_TABLE_MAX * sizeof(switch_table_t));
  return status;
}

switch_status_t switch_api_table_size_get_internal(switch_device_t device,
                                                   switch_table_id_t table_id,
                                                   switch_size_t* table_size) {
  switch_table_t* table_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TABLE_ID_VALID(table_id));
  SWITCH_ASSERT(table_size != NULL);

  status = switch_device_table_get(device, &table_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "get table size: Failed to get table info on device %d, error: %s",
        device, switch_error_to_string(status));
    return status;
  }

  *table_size = table_info[table_id].table_size;

  return status;
}

switch_status_t switch_table_default_sizes_get(switch_size_t* table_sizes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint16_t index = 0;

  if (!table_sizes) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("Failed to get table default size, error: %s",
                      switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_TABLE_MAX; index++) {
    switch (index) {
      case SWITCH_TABLE_NONE:
        table_sizes[index] = 0;
        break;
#if 0
      case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
        table_sizes[index] = PORTMAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_PORT_MAPPING:
        table_sizes[index] = PORTMAP_TABLE_SIZE;
        break;

      /* Rmac */
      case SWITCH_TABLE_OUTER_RMAC:
        table_sizes[index] = OUTER_ROUTER_MAC_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INNER_RMAC:
        table_sizes[index] = ROUTER_MAC_TABLE_SIZE;
        break;

      /* L2 */
      case SWITCH_TABLE_SMAC:
        table_sizes[index] = MAC_TABLE_SIZE;
        break;

      case SWITCH_TABLE_DMAC:
        table_sizes[index] = MAC_TABLE_SIZE;
        break;

      /* FIB */
      case SWITCH_TABLE_IPV4_HOST:
        table_sizes[index] = IPV4_HOST_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_HOST:
        table_sizes[index] = IPV6_HOST_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_LPM:
        table_sizes[index] = IPV4_LPM_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_LPM:
        table_sizes[index] = IPV6_LPM_TABLE_SIZE;
        break;

      case SWITCH_TABLE_SMAC_REWRITE:
        table_sizes[index] = MAC_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_MTU:
        table_sizes[index] = L3_MTU_TABLE_SIZE;
        break;

      case SWITCH_TABLE_URPF:
        table_sizes[index] = URPF_GROUP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_LOCAL_HOST:
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        table_sizes[index] = IPV4_LOCAL_HOST_TABLE_SIZE;
#else
        table_sizes[index] = 0;
#endif
        break;

      /* Nexthop */
      case SWITCH_TABLE_NHOP:
        table_sizes[index] = NEXTHOP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_ECMP_GROUP:
        table_sizes[index] = ECMP_GROUP_TABLE_SIZE / (MAX_NEIGHBOR_NODES + 1);
        break;

      case SWITCH_TABLE_ECMP_SELECT:
        table_sizes[index] = ECMP_SELECT_TABLE_SIZE;
        break;

      /* Rewrite */
      case SWITCH_TABLE_REWRITE:
        table_sizes[index] = ECMP_SELECT_TABLE_SIZE;
        break;

      /* Tunnel */
      case SWITCH_TABLE_IPV4_SRC_VTEP:
        table_sizes[index] = IPV4_SRC_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_DST_VTEP:
        table_sizes[index] = DEST_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_SRC_VTEP:
        table_sizes[index] = IPV6_SRC_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_DST_VTEP:
        table_sizes[index] = DEST_TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_VTEP:
        table_sizes[index] = DEST_TUNNEL_TABLE_SIZE;
        break;
      case SWITCH_TABLE_IPV6_VTEP:
        table_sizes[index] = DEST_TUNNEL_TABLE_SIZE;
        break;
#endif
      case SWITCH_TABLE_TUNNEL:
        table_sizes[index] = TUNNEL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_TERM:
        table_sizes[index] = TUNNEL_TERM_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NHOP:
        table_sizes[index] = NEXTHOP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NEIGHBOR:
        table_sizes[index] = NEIGHBOR_MOD_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NHOP_MEMBER_SELECT:
        table_sizes[index] = NHOP_MEMBER_HASH_TABLE_SIZE;
        break;
#if 0
      case SWITCH_TABLE_TUNNEL_REWRITE:
        table_sizes[index] = VNID_MAPPING_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_DECAP:
        table_sizes[index] = TUNNEL_DECAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
        table_sizes[index] = TUNNEL_SMAC_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
        table_sizes[index] = TUNNEL_DMAC_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_TUNNEL_DIP_REWRITE:
        table_sizes[index] = IPV4_TUNNEL_DST_REWRITE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_TUNNEL_DIP_REWRITE:
        table_sizes[index] = IPV6_TUNNEL_DST_REWRITE_TABLE_SIZE;
        break;

      /* BD */
      case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
        table_sizes[index] = PORT_VLAN_TABLE_SIZE;
        break;

      case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
        table_sizes[index] = PORT_VLAN_TABLE_SIZE;
        break;

      case SWITCH_TABLE_BD:
        table_sizes[index] = BD_TABLE_SIZE;
        break;

      case SWITCH_TABLE_BD_FLOOD:
        table_sizes[index] = BD_FLOOD_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_BD_STATS:
        table_sizes[index] = BD_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_BD_STATS:
        table_sizes[index] = EGRESS_BD_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_VLAN_DECAP:
        table_sizes[index] = VLAN_DECAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_VLAN_XLATE:
        table_sizes[index] = EGRESS_VLAN_XLATE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_BD:
        table_sizes[index] = EGRESS_BD_MAPPING_TABLE_SIZE;
        break;

      /* ACL */
      case SWITCH_TABLE_IPV4_ACL:
        table_sizes[index] = INGRESS_IP_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_IPV4_ACL:
        table_sizes[index] = EGRESS_IP_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_ACL:
        table_sizes[index] = INGRESS_IPV6_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_IPV6_ACL:
        table_sizes[index] = EGRESS_IPV6_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_RACL:
        table_sizes[index] = INGRESS_IP_RACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_RACL:
        table_sizes[index] = INGRESS_IPV6_RACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_SYSTEM_ACL:
        table_sizes[index] = SYSTEM_ACL_SIZE;
        break;

      case SWITCH_TABLE_LOW_PRI_SYSTEM_ACL:
        table_sizes[index] = SYSTEM_ACL_SIZE;
        break;

      case SWITCH_TABLE_SYSTEM_REASON_ACL:
        table_sizes[index] = SYSTEM_ACL_SIZE;
        break;

      case SWITCH_TABLE_MAC_ACL:
        table_sizes[index] = INGRESS_MAC_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
        table_sizes[index] = EGRESS_SYSTEM_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_ACL_STATS:
        table_sizes[index] = ACL_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_RACL_STATS:
        table_sizes[index] = RACL_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_ACL_STATS:
        table_sizes[index] = EGRESS_ACL_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_MIRROR_ACL:
        table_sizes[index] = MIRROR_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_MIRROR_ACL:
        table_sizes[index] = MIRROR_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_IPV4_MIRROR_ACL:
        table_sizes[index] = MIRROR_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_IPV6_MIRROR_ACL:
        table_sizes[index] = MIRROR_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_DTEL_ACL:
        table_sizes[index] = DTEL_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_DTEL_ACL:
        table_sizes[index] = DTEL_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_ECN_ACL:
        table_sizes[index] = INGRESS_ECN_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_MAC_QOS_ACL:
        table_sizes[index] = INGRESS_MAC_QOS_ACL_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_QOS_ACL:
        table_sizes[index] = INGRESS_IPV4_QOS_ACL_TABLE_SIZE;
        break;

      /* Multicast */
      case SWITCH_TABLE_OUTER_MCAST_STAR_G:
        table_sizes[index] = OUTER_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_OUTER_MCAST_SG:
        table_sizes[index] = OUTER_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_OUTER_MCAST_RPF:
        table_sizes[index] = OUTER_MCAST_RPF_TABLE_SIZE;
        break;

      case SWITCH_TABLE_MCAST_RPF:
        table_sizes[index] = MCAST_RPF_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_MCAST_S_G:
        table_sizes[index] = IPV4_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_MCAST_STAR_G:
        table_sizes[index] = IPV4_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_MCAST_S_G:
        table_sizes[index] = IPV6_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_MCAST_STAR_G:
        table_sizes[index] = IPV6_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_BRIDGE_MCAST_S_G:
        table_sizes[index] = IPV4_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV4_BRIDGE_MCAST_STAR_G:
        table_sizes[index] = IPV4_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_BRIDGE_MCAST_S_G:
        table_sizes[index] = IPV6_MULTICAST_S_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_IPV6_BRIDGE_MCAST_STAR_G:
        table_sizes[index] = IPV6_MULTICAST_STAR_G_TABLE_SIZE;
        break;

      case SWITCH_TABLE_RID:
        table_sizes[index] = RID_TABLE_SIZE;
        break;

      case SWITCH_TABLE_REPLICA_TYPE:
        table_sizes[index] = REPLICA_TYPE_TABLE_SIZE;
        break;

      /* STP */
      case SWITCH_TABLE_STP:
        table_sizes[index] = SPANNING_TREE_TABLE_SIZE;
        break;

      /* LAG */
      case SWITCH_TABLE_LAG_GROUP:
        table_sizes[index] = LAG_GROUP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_LAG_SELECT:
        table_sizes[index] = LAG_SELECT_TABLE_SIZE;
        break;

      /* Mirror */
      case SWITCH_TABLE_MIRROR:
        table_sizes[index] = MIRROR_SESSIONS_TABLE_SIZE;
        break;

      /* Meter */
      case SWITCH_TABLE_METER_INDEX:
        table_sizes[index] = METER_INDEX_TABLE_SIZE;
        break;

      case SWITCH_TABLE_METER_ACTION:
        table_sizes[index] = METER_ACTION_TABLE_SIZE;
        break;

      /* Egress Meter */
      case SWITCH_TABLE_EGRESS_METER_INDEX:
        table_sizes[index] = METER_INDEX_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_METER_ACTION:
        table_sizes[index] = METER_ACTION_TABLE_SIZE;
        break;

      /* Stats */
      case SWITCH_TABLE_DROP_STATS:
        table_sizes[index] = DROP_STATS_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NAT_DST:
        table_sizes[index] = IP_NAT_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NAT_SRC:
        table_sizes[index] = IP_NAT_TABLE_SIZE;
        break;

      case SWITCH_TABLE_NAT_TWICE:
        table_sizes[index] = IP_NAT_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP_DSCP:
        table_sizes[index] = DSCP_TO_TC_AND_COLOR_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP_PCP:
        table_sizes[index] = PCP_TO_TC_AND_COLOR_TABLE_SIZE;
        break;

      case SWITCH_TABLE_QUEUE:
        table_sizes[index] = QUEUE_TABLE_SIZE;
        break;

      case SWITCH_TABLE_INGRESS_QOS_MAP:
        table_sizes[index] = INGRESS_QOS_MAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_EGRESS_QOS_MAP:
        table_sizes[index] = EGRESS_QOS_MAP_TABLE_SIZE;
        break;

      case SWITCH_TABLE_WRED:
        table_sizes[index] = WRED_TABLE_SIZE;
        break;

      case SWITCH_TABLE_METER_COLOR_ACTION:
        table_sizes[index] = COLOR_ACTION_TABLE_SIZE;
        break;
#endif

      default:
        table_sizes[index] = 0;
        break;
    }
  }

  return status;
}

switch_status_t switch_api_table_size_get(switch_device_t device,
                                          switch_table_id_t table_id,
                                          switch_size_t* table_size) {
  return switch_api_table_size_get_internal(device, table_id, table_size);
}
