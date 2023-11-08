#ifndef __P4_NAME_MAPPING__
#define __P4_NAME_MAPPING__

#ifdef __cplusplus
extern "C" {
#endif

/* TODO: Remove 2 copies of this mapping file and maintain only one
 * in p4src/linux_networking and import it here using cmake variable
 */

/* List of tables and corresponding actions */

/* VXLAN_ENCAP_MOD_TABLE */
#define VXLAN_ENCAP_MOD_TABLE "linux_networking_control.vxlan_encap_mod_table"

#define VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR \
  "vendormeta_mod_data_ptr"

#define ACTION_VXLAN_ENCAP "linux_networking_control.vxlan_encap"
#define ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR "src_addr"
#define ACTION_VXLAN_ENCAP_PARAM_DST_ADDR "dst_addr"
#define ACTION_VXLAN_ENCAP_PARAM_DST_PORT "dst_port"
#define ACTION_VXLAN_ENCAP_PARAM_VNI "vni"
#define TUNNEL_TYPE_VXLAN 2

/* IPV4_TUNNEL_TERM_TABLE */
#define IPV4_TUNNEL_TERM_TABLE "linux_networking_control.ipv4_tunnel_term_table"

#define IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE "tunnel_type"
#define IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC "ipv4_src"
#define IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST "ipv4_dst"

#define ACTION_DECAP_OUTER_IPV4 "linux_networking_control.decap_outer_ipv4"
#define ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID "tunnel_id"

/* L2_FWD_RX_TABLE */
#define L2_FWD_RX_TABLE "linux_networking_control.l2_fwd_rx_table"

#define L2_FWD_RX_TABLE_KEY_DST_MAC "dst_mac"

#define L2_FWD_RX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"
#define ACTION_L2_FWD_PARAM_PORT "port"

/* L2_FWD_RX_WITH_TUNNEL_TABLE */
#define L2_FWD_RX_WITH_TUNNEL_TABLE \
  "linux_networking_control.l2_fwd_rx_with_tunnel_table"

#define L2_FWD_RX_WITH_TUNNEL_TABLE_KEY_DST_MAC "dst_mac"

#define L2_FWD_RX_WITH_TUNNEL_TABLE_ACTION_L2_FWD \
  "linux_networking_control.l2_fwd"

/* L2_FWD_TX_TABLE */
#define L2_FWD_TX_TABLE "linux_networking_control.l2_fwd_tx_table"

#define L2_FWD_TX_TABLE_KEY_DST_MAC "dst_mac"

#define L2_FWD_TX_TABLE_ACTION_L2_FWD "linux_networking_control.l2_fwd"

#define L2_FWD_TX_TABLE_ACTION_SET_TUNNEL "linux_networking_control.set_tunnel"

#define ACTION_SET_TUNNEL_PARAM_TUNNEL_ID "tunnel_id"

#define ACTION_SET_TUNNEL_PARAM_DST_ADDR "dst_addr"

#ifdef __cplusplus
};  // extern "C"
#endif

#endif /* __P4_NAME_MAPPING__ */
