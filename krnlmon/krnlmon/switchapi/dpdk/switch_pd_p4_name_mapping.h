/*
 * Copyright 2022-2024 Intel Corporation.
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

#ifndef __SWITCH_PD_P4_NAME_MAPPING__
#define __SWITCH_PD_P4_NAME_MAPPING__

/* List of tables and corresponding actions */

/* VXLAN_ENCAP_MOD_TABLE */
#define LNW_VXLAN_ENCAP_MOD_TABLE \
  "linux_networking_control.vxlan_encap_mod_table"

#define LNW_VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vendormeta_mod_data_ptr"

#define LNW_VXLAN_ENCAP_MOD_TABLE_ACTION_VXLAN_ENCAP \
  "linux_networking_control.vxlan_encap"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR "src_addr"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_DST_ADDR "dst_addr"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_DST_PORT "dst_port"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_VNI "vni"

/* RIF_MOD_TABLE */
#define LNW_RIF_MOD_TABLE "linux_networking_control.rif_mod_table"

#define LNW_RIF_MOD_TABLE_KEY_RIF_MOD_MAP_ID "local_metadata.rif_mod_map_id"

#define LNW_RIF_MOD_TABLE_ACTION_SET_SRC_MAC \
  "linux_networking_control.set_src_mac"
#define LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR "src_mac_addr"

/* NEIGHBOR_MOD_TABLE */
#define LNW_NEIGHBOR_MOD_TABLE "linux_networking_control.neighbor_mod_table"

#define LNW_NEIGHBOR_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vendormeta_mod_data_ptr"

#define LNW_NEIGHBOR_MOD_TABLE_ACTION_SET_OUTER_MAC \
  "linux_networking_control.set_outer_mac"
#define LNW_ACTION_SET_OUTER_MAC_PARAM_DST_MAC_ADDR "dst_mac_addr"

/* IPV4_TUNNEL_TERM_TABLE */
#define LNW_IPV4_TUNNEL_TERM_TABLE \
  "linux_networking_control.ipv4_tunnel_term_table"

#define LNW_IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE "tunnel_type"
#define LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC "ipv4_src"
#define LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST "ipv4_dst"

#define LNW_IPV4_TUNNEL_TERM_TABLE_ACTION_DECAP_OUTER_IPV4 \
  "linux_networking_control.decap_outer_ipv4"
#define LNW_ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID "tunnel_id"

/* L2_FWD_RX_TABLE */
#define LNW_L2_FWD_RX_TABLE "linux_networking_control.l2_fwd_rx_table"

#define LNW_L2_FWD_RX_TABLE_KEY_DST_MAC "dst_mac"

#define LNW_L2_FWD_RX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"
#define LNW_ACTION_L2_FWD_PARAM_PORT "port"

/* L2_FWD_RX_WITH_TUNNEL_TABLE */
#define LNW_L2_FWD_RX_WITH_TUNNEL_TABLE \
  "linux_networking_control.l2_fwd_rx_with_tunnel_table"

#define LNW_L2_FWD_RX_WITH_TUNNEL_TABLE_KEY_DST_MAC "dst_mac"

#define LNW_L2_FWD_RX_WITH_TUNNEL_TABLE_ACTION_L2_FWD \
  "linux_networking_control.l2_fwd"

/* L2_FWD_TX_TABLE */
#define LNW_L2_FWD_TX_TABLE "linux_networking_control.l2_fwd_tx_table"

#define LNW_L2_FWD_TX_TABLE_KEY_DST_MAC "dst_mac"

#define LNW_L2_FWD_TX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"

#define LNW_L2_FWD_TX_TABLE_ACTION_SET_TUNNEL \
  "linux_networking_control.set_tunnel"
#define LNW_ACTION_SET_TUNNEL_PARAM_TUNNEL_ID "tunnel_id"
#define LNW_ACTION_SET_TUNNEL_PARAM_DST_ADDR "dst_addr"

/* NEXTHOP_TABLE */
#define LNW_NEXTHOP_TABLE "linux_networking_control.nexthop_table"

#define LNW_NEXTHOP_TABLE_KEY_NEXTHOP_ID "local_metadata.nexthop_id"

#define LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP \
  "linux_networking_control.set_nexthop"
#define LNW_ACTION_SET_NEXTHOP_PARAM_RIF "router_interface_id"
#define LNW_ACTION_SET_NEXTHOP_PARAM_NEIGHBOR_ID "neighbor_id"
#define LNW_ACTION_SET_NEXTHOP_PARAM_EGRESS_PORT "egress_port"

/* ECMP_HASH_TABLE */
#define LNW_ECMP_HASH_TABLE "linux_networking_control.ecmp_hash_table"

#define LNW_ECMP_HASH_TABLE_KEY_HOST_INFO_TX_EXTENDED_FLEX_0 \
  "local_metadata.host_info_tx_extended_flex_0"
#define LNW_ECMP_HASH_TABLE_KEY_HASH "local_metadata.hash"

#define LNW_ECMP_HASH_TABLE_ACTION_SET_NEXTHOP_ID \
  "linux_networking_control.set_nexthop_id"

#define LNW_ECMP_HASH_SIZE 65536
#define LNW_MAX_MEMBERS 512

/* IPV4_TABLE */
#define LNW_IPV4_TABLE "linux_networking_control.ipv4_table"

#define LNW_IPV4_TABLE_KEY_IPV4_DST_MATCH "local_metadata.ipv4_dst_match"

#define LNW_IPV4_TABLE_ACTION_SET_NEXTHOP_ID \
  "linux_networking_control.set_nexthop_id"
#define LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID "nexthop_id"

#define LNW_IPV4_TABLE_ACTION_ECMP_HASH_ACTION \
  "linux_networking_control.ecmp_hash_action"
#define LNW_ACTION_ECMP_HASH_ACTION_PARAM_ECMP_GROUP_ID "ecmp_group_id"

/* AS_ECMP_TABLE */
#define LNW_AS_ECMP_TABLE "linux_networking_control.as_ecmp"

#define LNW_AS_ECMP_SELECTOR_TABLE "linux_networking_control.as_ecmp_sel"

#define LNW_SELECTOR_GROUP_ID "$SELECTOR_GROUP_ID"

#define LNW_SELECTOR_MEMBER_ID "$ACTION_MEMBER_ID"

#define LNW_ACTION_MEMBER_STATUS "$ACTION_MEMBER_STATUS"

#define LNW_ACTION_MAX_GROUP_SIZE "$MAX_GROUP_SIZE"
#define LNW_SELECTOR_ACTION_ID 0

#endif /* __SWITCH_PD_P4_NAME_MAPPING__ */
