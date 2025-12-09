/*
 * Copyright (c) 2013-2021 Barefoot Networks, Inc.
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

#ifndef __SWITCH_PD_P4_NAME_MAPPING_H__
#define __SWITCH_PD_P4_NAME_MAPPING_H__

#define LNW_KEY_MATCH_PRIORITY "$MATCH_PRIORITY"

//----------------------------------------------------------------------
// VXLAN_ENCAP_MOD_TABLE
//----------------------------------------------------------------------
#define LNW_VXLAN_ENCAP_MOD_TABLE \
  "linux_networking_control.vxlan_encap_mod_table"

#define LNW_VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define LNW_VXLAN_ENCAP_MOD_TABLE_ACTION_VXLAN_ENCAP \
  "linux_networking_control.vxlan_encap"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR "src_addr"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_DST_ADDR "dst_addr"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_DST_PORT "dst_port"
#define LNW_ACTION_VXLAN_ENCAP_PARAM_VNI "vni"

//----------------------------------------------------------------------
// VXLAN_DECAP_MOD_TABLE
//----------------------------------------------------------------------
#define LNW_VXLAN_DECAP_MOD_TABLE \
  "linux_networking_control.vxlan_decap_mod_table"

#define LNW_VXLAN_DECAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define LNW_VXLAN_DECAP_MOD_TABLE_ACTION_VXLAN_DECAP_OUTER_IPV4 \
  "linux_networking_control.vxlan_decap_outer_ipv4"

//----------------------------------------------------------------------
// IPV4_TUNNEL_TERM_TABLE
//----------------------------------------------------------------------
#define LNW_IPV4_TUNNEL_TERM_TABLE \
  "linux_networking_control.ipv4_tunnel_term_table"
#define LNW_IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_FLAG_TYPE \
  "user_meta.pmeta.tun_flag1_d0"
#define LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC "ipv4_src"
#define LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST "ipv4_dst"

#define LNW_IPV4_TUNNEL_TERM_TABLE_ACTION_DECAP_OUTER_IPV4 \
  "linux_networking_control.decap_outer_ipv4"
#define LNW_ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID "tunnel_id"

//----------------------------------------------------------------------
// L2_FWD_RX_TABLE
//----------------------------------------------------------------------
#define LNW_L2_FWD_RX_TABLE "linux_networking_control.l2_fwd_rx_table"

#define LNW_L2_FWD_RX_TABLE_KEY_DST_MAC "dst_mac"

#define LNW_L2_FWD_RX_TABLE_KEY_BRIDGE_ID "user_meta.pmeta.bridge_id"

#define LNW_L2_FWD_RX_TABLE_KEY_SMAC_LEARNED "user_meta.pmeta.smac_learned"

#define LNW_L2_FWD_RX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"
#define LNW_ACTION_L2_FWD_PARAM_PORT "port"
#define LNW_L2_FWD_RX_TABLE_ACTION_RX_L2_FWD_LAG_AND_RECIRCULATE \
  "linux_networking_control.l2_fwd_lag_and_recirculate"
#define LNW_ACTION_RX_L2_FWD_LAG_PARAM_LAG_ID "lag_group_id"

//----------------------------------------------------------------------
// L2_FWD_RX_WITH_TUNNEL_TABLE
//----------------------------------------------------------------------
#define LNW_L2_FWD_RX_WITH_TUNNEL_TABLE \
  "linux_networking_control.l2_fwd_rx_with_tunnel_table"

#define LNW_L2_FWD_RX_WITH_TUNNEL_TABLE_KEY_DST_MAC "dst_mac"

#define LNW_L2_FWD_RX_WITH_TUNNEL_TABLE_ACTION_L2_FWD \
  "linux_networking_control.l2_fwd"

//----------------------------------------------------------------------
// L2_FWD_TX_TABLE
//----------------------------------------------------------------------
// NOP TODO
#define LNW_L2_FWD_TX_TABLE "linux_networking_control.l2_fwd_tx_table"
#define LNW_L2_FWD_TX_TABLE_KEY_BRIDGE_ID "user_meta.pmeta.bridge_id"
#define LNW_L2_FWD_TX_TABLE_KEY_DST_MAC "dst_mac"

#define LNW_L2_FWD_TX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"

#define LNW_L2_FWD_TX_TABLE_ACTION_L2_FWD_LAG \
  "linux_networking_control.l2_fwd_lag"
#define LNW_ACTION_L2_FWD_LAG_PARAM_LAG_ID "lag_group_id"

#define LNW_L2_FWD_TX_TABLE_ACTION_SET_TUNNEL \
  "linux_networking_control.set_tunnel"
#define LNW_ACTION_SET_TUNNEL_PARAM_TUNNEL_ID "tunnel_id"
#define LNW_ACTION_SET_TUNNEL_PARAM_DST_ADDR "dst_addr"

//----------------------------------------------------------------------
// TX_ACC_VSI_TABLE
//----------------------------------------------------------------------
#define LNW_TX_ACC_VSI_TABLE "linux_networking_control.tx_acc_vsi"

#define LNW_TX_ACC_VSI_TABLE_KEY_META_COMMON_VSI "vmeta.common.vsi"

#define LNW_TX_ACC_VSI_TABLE_KEY_ZERO_PADDING "zero_padding"

#define LNW_TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE \
  "linux_networking_control.l2_fwd_and_bypass_bridge"

#define ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT "port"

//----------------------------------------------------------------------
// SOURCE_PORT_TO_BRIDGE_MAP_TABLE
//----------------------------------------------------------------------
#define LNW_SOURCE_PORT_TO_BRIDGE_MAP_TABLE \
  "linux_networking_control.source_port_to_bridge_map"

#define LNW_SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_SOURCE_PORT \
  "user_meta.cmeta.source_port"

#define LNW_SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_VID \
  "hdrs.vlan_ext[vmeta.common.depth].hdr.vid"

#define LNW_SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_SET_BRIDGE_ID \
  "linux_networking_control.set_bridge_id"

#define LNW_SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_PARAM_BRIDGE_ID "bridge_id"

#endif /* __SWITCH_PD_P4_NAME_MAPPING_H__ */
