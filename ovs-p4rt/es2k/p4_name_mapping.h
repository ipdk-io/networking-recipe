#ifndef __P4_NAME_MAPPING__
#define __P4_NAME_MAPPING__

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Remove 2 copies of this mapping file and maintain only one
 * in p4src/linux_networking and import it here using cmake variable
 */

/* For ES2K when we program VSI ID as part of action, we need to
 * add an OFFSET of 16 for the VSI ID.
 */
#define ES2K_VPORT_ID_OFFSET 16

/* List of tables and corresponding actions */

/* VXLAN_ENCAP_MOD_TABLE */
#define VXLAN_ENCAP_MOD_TABLE "linux_networking_control.vxlan_encap_mod_table"

#define VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_VXLAN_ENCAP "linux_networking_control.vxlan_encap"
#define ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR "src_addr"
#define ACTION_VXLAN_ENCAP_PARAM_DST_ADDR "dst_addr"
#define ACTION_VXLAN_ENCAP_PARAM_DST_PORT "dst_port"
#define ACTION_VXLAN_ENCAP_PARAM_SRC_PORT "src_port"
#define ACTION_VXLAN_ENCAP_PARAM_VNI "vni"
#define TUNNEL_TYPE_VXLAN 2

/* VXLAN_ENCAP_VLAN_POP_MOD_TABLE */
#define VXLAN_ENCAP_VLAN_POP_MOD_TABLE \
  "linux_networking_control.vxlan_encap_vlan_pop_mod_table"

#define VXLAN_ENCAP_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_VXLAN_ENCAP_VLAN_POP \
  "linux_networking_control.vxlan_encap_vlan_pop"
#define ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_SRC_ADDR "src_addr"
#define ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_DST_ADDR "dst_addr"
#define ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_DST_PORT "dst_port"
#define ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_SRC_PORT "src_port"
#define ACTION_VXLAN_ENCAP_VLAN_POP_PARAM_VNI "vni"

/* VXLAN_ENCAP_V6_MOD_TABLE */
#define VXLAN_ENCAP_V6_MOD_TABLE \
  "linux_networking_control.vxlan_encap_v6_mod_table"

#define VXLAN_ENCAP_V6_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_VXLAN_ENCAP_V6 "linux_networking_control.vxlan_encap_v6"
#define ACTION_VXLAN_ENCAP_V6_PARAM_SRC_ADDR "src_addr"
#define ACTION_VXLAN_ENCAP_V6_PARAM_DST_ADDR "dst_addr"
#define ACTION_VXLAN_ENCAP_V6_PARAM_DS "ds"
#define ACTION_VXLAN_ENCAP_V6_PARAM_ECN "ecn"
#define ACTION_VXLAN_ENCAP_V6_PARAM_FLOW_LABEL "flow_label"
#define ACTION_VXLAN_ENCAP_V6_PARAM_hop_limit "hop_limit"
#define ACTION_VXLAN_ENCAP_V6_PARAM_SRC_PORT "src_port"
#define ACTION_VXLAN_ENCAP_V6_PARAM_DST_PORT "dst_port"
#define ACTION_VXLAN_ENCAP_V6_PARAM_VNI "vni"

/* VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE */
#define VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE \
  "linux_networking_control.vxlan_encap_v6_vlan_pop_mod_table"

#define VXLAN_ENCAP_V6_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_VXLAN_ENCAP_V6_VLAN_POP \
  "linux_networking_control.vxlan_encap_v6_vlan_pop"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_SRC_ADDR "src_addr"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DST_ADDR "dst_addr"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DS "ds"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_ECN "ecn"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_FLOW_LABEL "flow_label"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_hop_limit "hop_limit"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_SRC_PORT "src_port"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_DST_PORT "dst_port"
#define ACTION_VXLAN_ENCAP_V6_VLAN_POP_PARAM_VNI "vni"

/* GENEVE_ENCAP_MOD_TABLE */
#define GENEVE_ENCAP_MOD_TABLE "linux_networking_control.geneve_encap_mod_table"

#define GENEVE_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"
#define GENEVE_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_GENEVE_ENCAP "linux_networking_control.geneve_encap"
#define ACTION_GENEVE_ENCAP_PARAM_SRC_ADDR "src_addr"
#define ACTION_GENEVE_ENCAP_PARAM_DST_ADDR "dst_addr"
#define ACTION_GENEVE_ENCAP_PARAM_DST_PORT "dst_port"
#define ACTION_GENEVE_ENCAP_PARAM_SRC_PORT "src_port"
#define ACTION_GENEVE_ENCAP_PARAM_VNI "vni"

/* GENEVE_ENCAP_VLAN_POP_MOD_TABLE */
#define GENEVE_ENCAP_VLAN_POP_MOD_TABLE \
  "linux_networking_control.geneve_encap_vlan_pop_mod_table"

#define GENEVE_ENCAP_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_GENEVE_ENCAP_VLAN_POP \
  "linux_networking_control.geneve_encap_vlan_pop"

#define ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_SRC_ADDR "src_addr"
#define ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_DST_ADDR "dst_addr"
#define ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_DST_PORT "dst_port"
#define ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_SRC_PORT "src_port"
#define ACTION_GENEVE_ENCAP_VLAN_POP_PARAM_VNI "vni"

/* GENEVE_ENCAP_V6_MOD_TABLE */
#define GENEVE_ENCAP_V6_MOD_TABLE \
  "linux_networking_control.geneve_encap_v6_mod_table"

#define GENEVE_ENCAP_V6_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_GENEVE_ENCAP_V6 "linux_networking_control.geneve_encap_v6"
#define ACTION_GENEVE_ENCAP_V6_PARAM_SRC_ADDR "src_addr"
#define ACTION_GENEVE_ENCAP_V6_PARAM_DST_ADDR "dst_addr"
#define ACTION_GENEVE_ENCAP_V6_PARAM_DS "ds"
#define ACTION_GENEVE_ENCAP_V6_PARAM_ECN "ecn"
#define ACTION_GENEVE_ENCAP_V6_PARAM_FLOW_LABEL "flow_label"
#define ACTION_GENEVE_ENCAP_V6_PARAM_hop_limit "hop_limit"
#define ACTION_GENEVE_ENCAP_V6_PARAM_SRC_PORT "src_port"
#define ACTION_GENEVE_ENCAP_V6_PARAM_DST_PORT "dst_port"
#define ACTION_GENEVE_ENCAP_V6_PARAM_VNI "vni"

/* GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE */
#define GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE \
  "linux_networking_control.geneve_encap_v6_vlan_pop_mod_table"

#define GENEVE_ENCAP_V6_VLAN_POP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_GENEVE_ENCAP_V6_VLAN_POP \
  "linux_networking_control.geneve_encap_v6_vlan_pop"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_SRC_ADDR "src_addr"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DST_ADDR "dst_addr"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DS "ds"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_ECN "ecn"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_FLOW_LABEL "flow_label"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_hop_limit "hop_limit"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_SRC_PORT "src_port"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_DST_PORT "dst_port"
#define ACTION_GENEVE_ENCAP_V6_VLAN_POP_PARAM_VNI "vni"

/* IPV4_TUNNEL_TERM_TABLE */
#define IPV4_TUNNEL_TERM_TABLE "linux_networking_control.ipv4_tunnel_term_table"

#define IPV4_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID "user_meta.pmeta.bridge_id"
#define IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC "ipv4_src"
#define IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST "ipv4_dst"
#define IPV4_TUNNEL_TERM_TABLE_KEY_VNI "vni"

#define ACTION_SET_VXLAN_DECAP_OUTER_HDR \
  "linux_networking_control.set_vxlan_decap_outer_hdr"
#define ACTION_SET_VXLAN_DECAP_OUTER_HDR_AND_PUSH_VLAN \
  "linux_networking_control.set_vxlan_decap_outer_and_push_vlan"
#define ACTION_SET_GENEVE_DECAP_OUTER_HDR \
  "linux_networking_control.set_geneve_decap_outer_hdr"
#define ACTION_SET_GENEVE_DECAP_OUTER_HDR_AND_PUSH_VLAN \
  "linux_networking_control.set_geneve_decap_outer_and_push_vlan"

/* IPV6_TUNNEL_TERM_TABLE */
#define IPV6_TUNNEL_TERM_TABLE "linux_networking_control.ipv6_tunnel_term_table"

#define IPV6_TUNNEL_TERM_TABLE_KEY_BRIDGE_ID "user_meta.pmeta.bridge_id"
#define IPV6_TUNNEL_TERM_TABLE_KEY_IPV6_SRC "ipv6_src"
#define IPV6_TUNNEL_TERM_TABLE_KEY_VNI "vni"

/* VXLAN_DECAP_MOD TABLE */
#define VXLAN_DECAP_MOD_TABLE "linux_networking_control.vxlan_decap_mod_table"
#define VXLAN_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR "vmeta.common.mod_blob_ptr"
#define ACTION_VXLAN_DECAP_OUTER_HDR \
  "linux_networking_control.vxlan_decap_outer_hdr"

/* VXLAN_DECAP_AND_VLAN_PUSH_MOD TABLE */
#define VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE \
  "linux_networking_control.vxlan_decap_and_push_vlan_mod_table"
#define VXLAN_DECAP_AND_VLAN_PUSH_MOD_TABLE_KEY_MOD_BLOB_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_VXLAN_DECAP_AND_PUSH_VLAN \
  "linux_networking_control.vxlan_decap_and_push_vlan"
#define ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_PCP "pcp"
#define ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_DEI "dei"
#define ACTION_VXLAN_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID "vlan_id"

/* GENEVE_DECAP_MOD TABLE */
#define GENEVE_DECAP_MOD_TABLE "linux_networking_control.geneve_decap_mod_table"
#define GENEVE_DECAP_MOD_TABLE_KEY_MOD_BLOB_PTR "vmeta.common.mod_blob_ptr"
#define ACTION_GENEVE_DECAP_OUTER_HDR \
  "linux_networking_control.geneve_decap_outer_hdr"

/* GENEVE_DECAP_AND_VLAN_PUSH_MOD TABLE */
#define GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE \
  "linux_networking_control.geneve_decap_and_push_vlan_mod_table"
#define GENEVE_DECAP_AND_VLAN_PUSH_MOD_TABLE_KEY_MOD_BLOB_PTR \
  "vmeta.common.mod_blob_ptr"

#define ACTION_GENEVE_DECAP_AND_PUSH_VLAN \
  "linux_networking_control.geneve_decap_and_push_vlan"
#define ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_PCP "pcp"
#define ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_DEI "dei"
#define ACTION_GENEVE_DECAP_AND_PUSH_VLAN_PARAM_VLAN_ID "vlan_id"

/* L2_FWD_RX_TABLE */
#define L2_FWD_RX_TABLE "linux_networking_control.l2_fwd_rx_table"

#define L2_FWD_RX_TABLE_KEY_DST_MAC "dst_mac"
#define L2_FWD_RX_TABLE_KEY_BRIDGE_ID "user_meta.pmeta.bridge_id"
#define L2_FWD_RX_TABLE_KEY_SMAC_LEARNED "user_meta.pmeta.smac_learned"

#define L2_FWD_RX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"
#define ACTION_L2_FWD_PARAM_PORT "port"

/* L2_FWD_RX_WITH_TUNNEL_TABLE */
#define L2_FWD_RX_WITH_TUNNEL_TABLE \
  "linux_networking_control.l2_fwd_rx_with_tunnel_table"

#define L2_FWD_RX_WITH_TUNNEL_TABLE_KEY_DST_MAC "dst_mac"

#define L2_FWD_RX_WITH_TUNNEL_TABLE_ACTION_L2_FWD \
  "linux_networking_control.l2_fwd"

/* L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE */
#define L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE \
  "linux_networking_control.l2_fwd_rx_ipv6_with_tunnel_table"

#define L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE_KEY_DST_MAC "dst_mac"

#define L2_FWD_RX_IPV6_WITH_TUNNEL_TABLE_ACTION_L2_FWD \
  "linux_networking_control.l2_fwd"

/* L2_FWD_TX_TABLE */
#define L2_FWD_TX_TABLE "linux_networking_control.l2_fwd_tx_table"

#define L2_FWD_TX_TABLE_KEY_DST_MAC "dst_mac"
#define L2_FWD_TX_TABLE_KEY_BRIDGE_ID "user_meta.pmeta.bridge_id"
#define L2_FWD_TX_TABLE_KEY_SMAC_LEARNED "user_meta.pmeta.smac_learned"

#define L2_FWD_TX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"

#define L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V4 \
  "linux_networking_control.set_vxlan_underlay_v4"

#define L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V4 \
  "linux_networking_control.set_geneve_underlay_v4"

#define L2_FWD_TX_TABLE_ACTION_SET_VXLAN_UNDERLAY_V6 \
  "linux_networking_control.set_vxlan_underlay_v6"

#define L2_FWD_TX_TABLE_ACTION_SET_GENEVE_UNDERLAY_V6 \
  "linux_networking_control.set_geneve_underlay_v6"

#define L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V4 \
  "linux_networking_control.pop_vlan_set_vxlan_underlay_v4"

#define L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V4 \
  "linux_networking_control.pop_vlan_set_geneve_underlay_v4"

#define L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_VXLAN_UNDERLAY_V6 \
  "linux_networking_control.pop_vlan_set_vxlan_underlay_v6"

#define L2_FWD_TX_TABLE_ACTION_POP_VLAN_SET_GENEVE_UNDERLAY_V6 \
  "linux_networking_control.pop_vlan_set_geneve_underlay_v6"

#define ACTION_PARAM_TUNNEL_ID "tunnel_id"

#define L2_FWD_TX_TABLE_ACTION_ADD_VLAN_AND_FWD \
  "linux_networking_control.add_vlan_and_fwd"

#define ACTION_ADD_VLAN_AND_FWD_PARAM_VLAN_PTR "vlan_ptr"

#define ACTION_ADD_VLAN_AND_FWD_PARAM_PORT_ID "port_id"

#define L2_FWD_TX_TABLE_ACTION_REMOVE_VLAN_AND_FWD \
  "linux_networking_control.remove_vlan_and_fwd"

#define ACTION_REMOVE_VLAN_AND_FWD_PARAM_VLAN_PTR "vlan_ptr"

#define ACTION_REMOVE_VLAN_AND_FWD_PARAM_PORT_ID "port_id"

/* L2_FWD_TX_IPV6_TABLE */
#define L2_FWD_TX_IPV6_TABLE "linux_networking_control.l2_fwd_tx_ipv6_table"

#define L2_FWD_TX_IPV6_TABLE_KEY_DST_MAC "dst_mac"

#define L2_FWD_TX_IPV6_TABLE_KEY_TUN_FLAG "user_meta.pmeta.tun_flag1_d0"

#define L2_FWD_TX_IPV6_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"

#define L2_FWD_TX_IPV6_TABLE_ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6 \
  "linux_networking_control.set_tunnel_underlay_v6_overlay_v6"

#define ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_TUNNEL_ID "tunnel_id"

#define ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_1 "ipv6_1"

#define ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_2 "ipv6_2"

#define ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_3 "ipv6_3"

#define ACTION_SET_TUNNEL_UNDERLAY_V6_OVERLAY_V6_PARAM_IPV6_4 "ipv6_4"

/* SEM_BYPASS TABLE */
#define SEM_BYPASS_TABLE "linux_networking_control.sem_bypass"

#define SEM_BYPASS_TABLE_KEY_DST_MAC "dst_mac"

#define SEM_BYPASS_TABLE_ACTION_SET_DEST "linux_networking_control.set_dest"

#define ACTION_SET_DEST_PARAM_PORT_ID "port_id"

/* SOURCE_PORT_TO_BRIDGE_MAP TABLE */
#define SOURCE_PORT_TO_BRIDGE_MAP_TABLE \
  "linux_networking_control.source_port_to_bridge_map"

#define SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_SRC_PORT \
  "user_meta.cmeta.source_port"

#define SOURCE_PORT_TO_BRIDGE_MAP_TABLE_KEY_VID \
  "hdrs.vlan_ext[vmeta.common.depth].hdr.vid"

#define SOURCE_PORT_TO_BRIDGE_MAP_TABLE_ACTION_SET_BRIDGE_ID \
  "linux_networking_control.set_bridge_id"

#define ACTION_SET_BRIDGE_ID_PARAM_BRIDGE_ID "bridge_id"

/* RX_IPV4_TUNNEL_SOURCE_PORT TABLE */
#define RX_IPV4_TUNNEL_SOURCE_PORT_TABLE \
  "linux_networking_control.rx_ipv4_tunnel_source_port"

#define RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_KEY_IPV4_SRC "ipv4_src"

#define RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_KEY_VNI "vni"

#define RX_IPV4_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT \
  "linux_networking_control.set_source_port"

#define ACTION_SET_SRC_PORT "source_port"

/* RX_IPV6_TUNNEL_SOURCE_PORT TABLE */
#define RX_IPV6_TUNNEL_SOURCE_PORT_TABLE \
  "linux_networking_control.rx_ipv6_tunnel_source_port"

#define RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_KEY_IPV6_SRC "ipv6_src"

#define RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_KEY_VNI "vni"

#define RX_IPV6_TUNNEL_SOURCE_PORT_TABLE_ACTION_SET_SRC_PORT \
  "linux_networking_control.set_source_port"

/* L2_TO_TUNNEL_V4 TABLE */
#define L2_TO_TUNNEL_V4_TABLE "linux_networking_control.l2_to_tunnel_v4"

#define L2_TO_TUNNEL_V4_KEY_DA "hdrs.mac[vmeta.common.depth].da"

#define L2_TO_TUNNEL_V4_ACTION_SET_TUNNEL_V4 \
  "linux_networking_control.set_tunnel_v4"

#define ACTION_SET_TUNNEL_V4_PARAM_DST_ADDR "dst_addr"

/* L2_TO_TUNNEL_V6 TABLE */
#define L2_TO_TUNNEL_V6_TABLE "linux_networking_control.l2_to_tunnel_v6"

#define L2_TO_TUNNEL_V6_KEY_DA "hdrs.mac[vmeta.common.depth].da"

#define L2_TO_TUNNEL_V6_ACTION_SET_TUNNEL_V6 \
  "linux_networking_control.set_tunnel_v6"

#define ACTION_SET_TUNNEL_V6_PARAM_IPV6_1 "ipv6_1"

#define ACTION_SET_TUNNEL_V6_PARAM_IPV6_2 "ipv6_2"

#define ACTION_SET_TUNNEL_V6_PARAM_IPV6_3 "ipv6_3"

#define ACTION_SET_TUNNEL_V6_PARAM_IPV6_4 "ipv6_4"

/* VLAN_PUSH_MOD Table */
#define VLAN_PUSH_MOD_TABLE "linux_networking_control.vlan_push_mod_table"

#define VLAN_PUSH_MOD_KEY_MOD_BLOB_PTR "vmeta.common.mod_blob_ptr"

#define VLAN_PUSH_MOD_ACTION_VLAN_PUSH "linux_networking_control.vlan_push"

#define ACTION_VLAN_PUSH_PARAM_PCP "pcp"

#define ACTION_VLAN_PUSH_PARAM_DEI "dei"

#define ACTION_VLAN_PUSH_PARAM_VLAN_ID "vlan_id"

/* VLAN_POP_MOD Table */
#define VLAN_POP_MOD_TABLE "linux_networking_control.vlan_pop_mod_table"

#define VLAN_POP_MOD_KEY_MOD_BLOB_PTR "vmeta.common.mod_blob_ptr"

#define VLAN_POP_MOD_ACTION_VLAN_POP "linux_networking_control.vlan_pop"

/* TX_ACC_VSI TABLE */
#define TX_ACC_VSI_TABLE "linux_networking_control.tx_acc_vsi"

#define TX_ACC_VSI_TABLE_KEY_VSI "vmeta.common.vsi"

#define TX_ACC_VSI_TABLE_KEY_ZERO_PADDING "zero_padding"

#define TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE \
  "linux_networking_control.l2_fwd_and_bypass_bridge"

#define ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT "port"

/* VSI_TO_VSI_LOOPBACK TABLE */
#define VSI_TO_VSI_LOOPBACK_TABLE "linux_networking_control.vsi_to_vsi_loopback"

#define VSI_TO_VSI_LOOPBACK_KEY_VSI "vmeta.common.vsi"

#define VSI_TO_VSI_LOOPBACK_KEY_ZERO_PADDING "zero_padding"

#define VSI_TO_VSI_LOOPBACK_ACTION_FWD_TO_VSI \
  "linux_networking_control.fwd_to_vsi"

#define ACTION_L2_FWD_PORT "port"

/* L2_FWD_SMAC TABLE */
#define L2_FWD_SMAC_TABLE "linux_networking_control.l2_fwd_smac_table"

#define L2_FWD_SMAC_TABLE_KEY_SA "hdrs.mac[vmeta.common.depth].sa"

#define L2_FWD_SMAC_TABLE_ACTION_SMAC_LEARN \
  "linux_networking_control.set_smac_learn"

#ifdef __cplusplus
};  // extern "C"
#endif

#endif /* __P4_NAME_MAPPING__ */
