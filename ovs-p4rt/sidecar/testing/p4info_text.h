// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef TESTING_P4INFO_TEXT_H_
#define TESTING_P4INFO_TEXT_H_

// P4Info for ES2K Linux Networking V3.
constexpr char P4INFO_TEXT[] =
    R"p4(pkg_info {
  arch: "pna"
}
tables {
  preamble {
    id: 48773578
    name: "MainControlDecrypt.lem_exception"
    alias: "lem_exception"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 38116127
    name: "MainControlDecrypt.lem_clear"
    alias: "lem_clear"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 45068641
    name: "MainControlDecrypt.ipsec_rx_sa_classification_table"
    alias: "ipsec_rx_sa_classification_table"
  }
  match_fields {
    id: 1
    name: "hdrs.ipv4[vmeta.common.depth].src_ip"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "hdrs.ipv4[vmeta.common.depth].dst_ip"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 3
    name: "hdrs.esp.spi"
    bitwidth: 32
    match_type: EXACT
  }
  action_refs {
    id: 33177492
  }
  action_refs {
    id: 30285547
  }
  const_default_action_id: 30285547
  size: 1024
}
tables {
  preamble {
    id: 41667918
    name: "linux_networking_control.ipv4_lpm_root_lut"
    alias: "ipv4_lpm_root_lut"
    annotations: "@intel_lut_type(\"lpm_table\")"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.bit16_zeros"
    bitwidth: 16
    match_type: TERNARY
  }
  action_refs {
    id: 17061464
  }
  size: 1
}
tables {
  preamble {
    id: 43524029
    name: "linux_networking_control.ipv6_lpm_root_lut"
    alias: "ipv6_lpm_root_lut"
    annotations: "@intel_lut_type(\"lpm_table\")"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.bit16_zeros"
    bitwidth: 16
    match_type: TERNARY
  }
  action_refs {
    id: 25701894
  }
  size: 1
}
tables {
  preamble {
    id: 42605135
    name: "linux_networking_control.rif_mod_table_start"
    alias: "rif_mod_table_start"
  }
  match_fields {
    id: 1
    name: "rif_mod_map_id0"
    bitwidth: 11
    match_type: EXACT
  }
  action_refs {
    id: 23093409
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 43970557
    name: "linux_networking_control.rif_mod_table_mid"
    alias: "rif_mod_table_mid"
  }
  match_fields {
    id: 1
    name: "rif_mod_map_id1"
    bitwidth: 11
    match_type: EXACT
  }
  action_refs {
    id: 30315892
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 47499906
    name: "linux_networking_control.rif_mod_table_last"
    alias: "rif_mod_table_last"
  }
  match_fields {
    id: 1
    name: "rif_mod_map_id2"
    bitwidth: 11
    match_type: EXACT
  }
  action_refs {
    id: 32740970
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 47372228
    name: "linux_networking_control.always_recirculate_table"
    alias: "always_recirculate_table"
  }
  match_fields {
    id: 1
    name: "hdrs.inval.data"
    bitwidth: 16
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "hdrs.inval.data"
    bitwidth: 16
    match_type: EXACT
  }
  action_refs {
    id: 33275382
  }
  const_default_action_id: 33275382
  size: 1024
}
tables {
  preamble {
    id: 40312237
    name: "linux_networking_control.vlan_push_mod_table"
    alias: "vlan_push_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 24137163
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 38884256
    name: "linux_networking_control.vlan_pop_mod_table"
    alias: "vlan_pop_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 25667542
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 40763773
    name: "linux_networking_control.vxlan_encap_mod_table"
    alias: "vxlan_encap_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 20733968
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 39675860
    name: "linux_networking_control.vxlan_encap_vlan_pop_mod_table"
    alias: "vxlan_encap_vlan_pop_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 26114242
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 46225003
    name: "linux_networking_control.vxlan_encap_v6_mod_table"
    alias: "vxlan_encap_v6_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 30345128
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 34318005
    name: "linux_networking_control.vxlan_encap_v6_vlan_pop_mod_table"
    alias: "vxlan_encap_v6_vlan_pop_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 28284062
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 40980035
    name: "linux_networking_control.vxlan_decap_mod_table"
    alias: "vxlan_decap_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 28308555
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 37624107
    name: "linux_networking_control.vxlan_decap_and_push_vlan_mod_table"
    alias: "vxlan_decap_and_push_vlan_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 31953264
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 41319073
    name: "linux_networking_control.geneve_encap_mod_table"
    alias: "geneve_encap_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 25818889
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 47977422
    name: "linux_networking_control.geneve_encap_vlan_pop_mod_table"
    alias: "geneve_encap_vlan_pop_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 26665268
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 42283616
    name: "linux_networking_control.geneve_encap_v6_mod_table"
    alias: "geneve_encap_v6_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 29610186
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 47370391
    name: "linux_networking_control.geneve_encap_v6_vlan_pop_mod_table"
    alias: "geneve_encap_v6_vlan_pop_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 27347188
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 34173274
    name: "linux_networking_control.geneve_decap_mod_table"
    alias: "geneve_decap_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 27842378
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 34525815
    name: "linux_networking_control.geneve_decap_and_push_vlan_mod_table"
    alias: "geneve_decap_and_push_vlan_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 25073902
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 47484925
    name: "linux_networking_control.ipv4_tunnel_term_table"
    alias: "ipv4_tunnel_term_table"
  }
  match_fields {
    id: 1
    name: "ipv4_src"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "vni"
    bitwidth: 16
    match_type: EXACT
  }
  action_refs {
    id: 31163719
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 25342798
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 26749366
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 31208923
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 21790705
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 21790705
  size: 1024
}
tables {
  preamble {
    id: 34601307
    name: "linux_networking_control.ipv6_tunnel_term_table"
    alias: "ipv6_tunnel_term_table"
  }
  match_fields {
    id: 1
    name: "ipv6_src"
    bitwidth: 128
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "vni"
    bitwidth: 16
    match_type: EXACT
  }
  action_refs {
    id: 31163719
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 25342798
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 26749366
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 31208923
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 44842501
    name: "linux_networking_control.l2_fwd_rx_table"
    alias: "l2_fwd_rx_table"
  }
  match_fields {
    id: 2
    name: "user_meta.pmeta.bridge_id"
    bitwidth: 8
    match_type: EXACT
  }
  match_fields {
    id: 1
    name: "dst_mac"
    annotations: "@format(MAC_ADDRESS)"
    bitwidth: 48
    match_type: EXACT
  }
  action_refs {
    id: 19169916
  }
  action_refs {
    id: 21862855
  }
  action_refs {
    id: 21283156
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 21283156
  size: 1024
}
tables {
  preamble {
    id: 40240205
    name: "linux_networking_control.l2_fwd_tx_table"
    alias: "l2_fwd_tx_table"
  }
  match_fields {
    id: 1
    name: "user_meta.pmeta.bridge_id"
    bitwidth: 8
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "dst_mac"
    annotations: "@format(MAC_ADDRESS)"
    bitwidth: 48
    match_type: EXACT
  }
  action_refs {
    id: 19169916
  }
  action_refs {
    id: 24440746
  }
  action_refs {
    id: 26412051
  }
  action_refs {
    id: 19193142
  }
  action_refs {
    id: 31983357
  }
  action_refs {
    id: 23849990
  }
  action_refs {
    id: 23736116
  }
  action_refs {
    id: 17483375
  }
  action_refs {
    id: 24842266
  }
  action_refs {
    id: 33478945
  }
  action_refs {
    id: 24108243
  }
  action_refs {
    id: 25690116
  }
  action_refs {
    id: 20441854
  }
  direct_resource_ids: 333810446
  size: 1024
}
tables {
  preamble {
    id: 37894008
    name: "linux_networking_control.nexthop_table"
    alias: "nexthop_table"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.nexthop_id"
    bitwidth: 16
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "bit16_zeros"
    bitwidth: 8
    match_type: EXACT
  }
  action_refs {
    id: 16795029
  }
  action_refs {
    id: 25951413
  }
  action_refs {
    id: 29238102
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  direct_resource_ids: 323002307
  size: 1024
}
tables {
  preamble {
    id: 47712712
    name: "linux_networking_control.ecmp_nexthop_table"
    alias: "ecmp_nexthop_table"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.nexthop_id"
    bitwidth: 16
    match_type: TERNARY
  }
  action_refs {
    id: 27181644
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 40757978
    name: "linux_networking_control.tx_lag_table"
    alias: "tx_lag_table"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.lag_group_id"
    bitwidth: 8
    match_type: TERNARY
  }
  match_fields {
    id: 2
    name: "hash"
    bitwidth: 3
    match_type: TERNARY
  }
  action_refs {
    id: 27998997
  }
  action_refs {
    id: 24954025
  }
  action_refs {
    id: 29238102
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 44444143
    name: "linux_networking_control.rx_lag_table"
    alias: "rx_lag_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.port_id"
    bitwidth: 2
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "user_meta.cmeta.lag_group_id"
    bitwidth: 8
    match_type: EXACT
  }
  action_refs {
    id: 26092385
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 37566509
    name: "linux_networking_control.ipv4_table"
    alias: "ipv4_table"
  }
  match_fields {
    id: 1
    name: "ipv4_table_lpm_root"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "ipv4_dst_match"
    annotations: "@intel_byte_order(\"NETWORK\")"
    bitwidth: 32
    match_type: LPM
  }
  action_refs {
    id: 22009623
  }
  action_refs {
    id: 16874810
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 49661222
    name: "linux_networking_control.ecmp_hash_table"
    alias: "ecmp_hash_table"
  }
  match_fields {
    id: 1
    name: "flex"
    bitwidth: 16
    match_type: TERNARY
  }
  match_fields {
    id: 2
    name: "hash"
    bitwidth: 3
    match_type: TERNARY
  }
  action_refs {
    id: 29883644
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 46496254
    name: "linux_networking_control.ipv6_table"
    alias: "ipv6_table"
  }
  match_fields {
    id: 1
    name: "ipv6_table_lpm_root"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "ipv6_dst_match"
    annotations: "@intel_byte_order(\"NETWORK\")"
    bitwidth: 128
    match_type: LPM
  }
  action_refs {
    id: 21321477
  }
  action_refs {
    id: 27000185
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 40351839
    name: "linux_networking_control.rx_source_port"
    alias: "rx_source_port"
  }
  match_fields {
    id: 1
    name: "vmeta.common.port_id"
    bitwidth: 2
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "zero_padding"
    bitwidth: 16
    match_type: EXACT
  }
  action_refs {
    id: 32890467
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 49097500
    name: "linux_networking_control.rx_ipv4_tunnel_source_port"
    alias: "rx_ipv4_tunnel_source_port"
  }
  match_fields {
    id: 1
    name: "ipv4_src"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "vni"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 32890467
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 40981078
    name: "linux_networking_control.rx_ipv6_tunnel_source_port"
    alias: "rx_ipv6_tunnel_source_port"
  }
  match_fields {
    id: 1
    name: "ipv6_src"
    bitwidth: 128
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "vni"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 32890467
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 33606489
    name: "linux_networking_control.source_port_to_bridge_map"
    alias: "source_port_to_bridge_map"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.source_port"
    bitwidth: 16
    match_type: TERNARY
  }
  match_fields {
    id: 2
    name: "hdrs.vlan_ext[vmeta.common.depth].hdr.vid"
    bitwidth: 12
    match_type: TERNARY
  }
  action_refs {
    id: 31939992
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 46342225
    name: "linux_networking_control.l2_fwd_smac_table"
    alias: "l2_fwd_smac_table"
  }
  match_fields {
    id: 1
    name: "hdrs.mac[vmeta.common.depth].sa"
    bitwidth: 48
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "user_meta.pmeta.bridge_id"
    bitwidth: 8
    match_type: EXACT
  }
  action_refs {
    id: 21257015
  }
  action_refs {
    id: 21283156
  }
  const_default_action_id: 21283156
  size: 1024
}
tables {
  preamble {
    id: 42508227
    name: "linux_networking_control.tx_acc_vsi"
    alias: "tx_acc_vsi"
  }
  match_fields {
    id: 1
    name: "vmeta.common.vsi"
    bitwidth: 11
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "zero_padding"
    bitwidth: 16
    match_type: EXACT
  }
  action_refs {
    id: 31624713
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 33785091
    name: "linux_networking_control.tx_source_port_v4"
    alias: "tx_source_port_v4"
  }
  match_fields {
    id: 1
    name: "vmeta.common.vsi"
    bitwidth: 11
    match_type: TERNARY
  }
  action_refs {
    id: 32890467
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 43337754
    name: "linux_networking_control.l2_to_tunnel_v4"
    alias: "l2_to_tunnel_v4"
  }
  match_fields {
    id: 1
    name: "hdrs.mac[vmeta.common.depth].da"
    bitwidth: 48
    match_type: EXACT
  }
  action_refs {
    id: 23805991
  }
  action_refs {
    id: 29238102
  }
  action_refs {
    id: 33275382
  }
  const_default_action_id: 33275382
  size: 1024
}
tables {
  preamble {
    id: 36796227
    name: "linux_networking_control.l2_to_tunnel_v6"
    alias: "l2_to_tunnel_v6"
  }
  match_fields {
    id: 1
    name: "hdrs.mac[vmeta.common.depth].da"
    bitwidth: 48
    match_type: EXACT
  }
  action_refs {
    id: 23953453
  }
  action_refs {
    id: 33275382
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 33275382
  size: 1024
}
tables {
  preamble {
    id: 38721082
    name: "linux_networking_control.rx_phy_port_to_pr_map"
    alias: "rx_phy_port_to_pr_map"
  }
  match_fields {
    id: 1
    name: "vmeta.common.port_id"
    bitwidth: 2
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "zero_padding"
    bitwidth: 16
    match_type: EXACT
  }
  action_refs {
    id: 26092385
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 35934350
    name: "linux_networking_control.source_port_to_pr_map"
    alias: "source_port_to_pr_map"
  }
  match_fields {
    id: 1
    name: "user_meta.cmeta.source_port"
    bitwidth: 16
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "zero_padding"
    bitwidth: 8
    match_type: EXACT
  }
  action_refs {
    id: 26092385
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 45509113
    name: "linux_networking_control.vsi_to_vsi_loopback"
    alias: "vsi_to_vsi_loopback"
  }
  match_fields {
    id: 1
    name: "vmeta.common.vsi"
    bitwidth: 11
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "target_vsi"
    bitwidth: 11
    match_type: EXACT
  }
  action_refs {
    id: 26092385
  }
  action_refs {
    id: 29238102
  }
  const_default_action_id: 29238102
  size: 1024
}
tables {
  preamble {
    id: 44641644
    name: "linux_networking_control.hash_ipv4_tcp"
    alias: "hash_ipv4_tcp"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 46678878
    name: "linux_networking_control.hash_ipv4_udp"
    alias: "hash_ipv4_udp"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 45972289
    name: "linux_networking_control.hash_ipv4"
    alias: "hash_ipv4"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 33896331
    name: "linux_networking_control.hash_l2"
    alias: "hash_l2"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 46852435
    name: "linux_networking_control.hash_ipv6_tcp"
    alias: "hash_ipv6_tcp"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 46359163
    name: "linux_networking_control.hash_ipv6_udp"
    alias: "hash_ipv6_udp"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 34506208
    name: "linux_networking_control.hash_ipv6"
    alias: "hash_ipv6"
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 49349790
    name: "linux_networking_control.ipsec_tunnel_table"
    alias: "ipsec_tunnel_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.saidx"
    bitwidth: 24
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "bit16_zeros"
    bitwidth: 13
    match_type: EXACT
  }
  action_refs {
    id: 26423957
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  size: 1024
}
tables {
  preamble {
    id: 47756035
    name: "linux_networking_control.ipsec_spd"
    alias: "ipsec_spd"
  }
  match_fields {
    id: 1
    name: "hdrs.ipv4[vmeta.common.depth].dst_ip"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "hdrs.ipv4[vmeta.common.depth].protocol"
    bitwidth: 8
    match_type: EXACT
  }
  action_refs {
    id: 20682322
  }
  action_refs {
    id: 19192368
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  direct_resource_ids: 330899638
  size: 1024
}
tables {
  preamble {
    id: 45022218
    name: "linux_networking_control.ipsec_tx_sa_classification_table"
    alias: "ipsec_tx_sa_classification_table"
  }
  match_fields {
    id: 1
    name: "hdrs.ipv4[vmeta.common.depth].dst_ip"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "hdrs.ipv4[vmeta.common.depth].protocol"
    bitwidth: 8
    match_type: EXACT
  }
  match_fields {
    id: 3
    name: "user_meta.cmeta.is_tunnel"
    bitwidth: 1
    match_type: EXACT
  }
  action_refs {
    id: 24935652
  }
  action_refs {
    id: 17885953
  }
  action_refs {
    id: 30154712
  }
  action_refs {
    id: 29527637
  }
  action_refs {
    id: 29238102
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  direct_resource_ids: 327348295
  size: 1024
}
tables {
  preamble {
    id: 37326952
    name: "linux_networking_control.ipsec_tunnel_encap_mod_table"
    alias: "ipsec_tunnel_encap_mod_table"
  }
  match_fields {
    id: 1
    name: "vmeta.common.mod_blob_ptr"
    bitwidth: 24
    match_type: EXACT
  }
  action_refs {
    id: 30081157
  }
  action_refs {
    id: 21257015
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 43475471
    name: "linux_networking_control.ipv4_ipsec_tunnel_term_table"
    alias: "ipv4_ipsec_tunnel_term_table"
  }
  match_fields {
    id: 1
    name: "ipv4_src"
    bitwidth: 32
    match_type: EXACT
  }
  match_fields {
    id: 2
    name: "ipv4_dst"
    bitwidth: 32
    match_type: EXACT
  }
  action_refs {
    id: 25138331
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 33275382
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 33275382
  size: 1024
}
tables {
  preamble {
    id: 37403444
    name: "linux_networking_control.vm_src_ip4_mac_map_table"
    alias: "vm_src_ip4_mac_map_table"
  }
  match_fields {
    id: 1
    name: "ipv4_src"
    bitwidth: 32
    match_type: EXACT
  }
  action_refs {
    id: 28718673
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 21257015
  size: 1024
}
tables {
  preamble {
    id: 50085764
    name: "linux_networking_control.vm_dst_ip4_mac_map_table"
    alias: "vm_dst_ip4_mac_map_table"
  }
  match_fields {
    id: 1
    name: "ipv4_dst"
    bitwidth: 32
    match_type: EXACT
  }
  action_refs {
    id: 25018769
    annotations: "@tableonly"
    scope: TABLE_ONLY
  }
  action_refs {
    id: 21257015
    annotations: "@defaultonly"
    scope: DEFAULT_ONLY
  }
  const_default_action_id: 21257015
  size: 1024
}
actions {
  preamble {
    id: 21257015
    name: "NoAction"
    alias: "NoAction"
    annotations: "@noWarn(\"unused\")"
  }
}
actions {
  preamble {
    id: 30285547
    name: "MainControlDecrypt.ipsec_bypass"
    alias: "MainControlDecrypt.ipsec_bypass"
  }
}
actions {
  preamble {
    id: 33177492
    name: "MainControlDecrypt.ipsec_decrypt"
    alias: "ipsec_decrypt"
  }
  params {
    id: 1
    name: "saidx"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 19173135
    name: "linux_networking_control.no_modify"
    alias: "no_modify"
  }
}
actions {
  preamble {
    id: 31851898
    name: "linux_networking_control.dummycall"
    alias: "dummycall"
  }
}
actions {
  preamble {
    id: 33275382
    name: "linux_networking_control.do_recirculate"
    alias: "do_recirculate"
  }
}
actions {
  preamble {
    id: 19169916
    name: "linux_networking_control.l2_fwd"
    alias: "l2_fwd"
  }
  params {
    id: 1
    name: "port"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
}
actions {
  preamble {
    id: 31624713
    name: "linux_networking_control.l2_fwd_and_bypass_bridge"
    alias: "l2_fwd_and_bypass_bridge"
  }
  params {
    id: 1
    name: "port"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
}
actions {
  preamble {
    id: 24440746
    name: "linux_networking_control.l2_fwd_lag"
    alias: "l2_fwd_lag"
  }
  params {
    id: 1
    name: "lag_group_id"
    bitwidth: 8
  }
}
actions {
  preamble {
    id: 21862855
    name: "linux_networking_control.l2_fwd_lag_and_recirculate"
    alias: "l2_fwd_lag_and_recirculate"
  }
  params {
    id: 1
    name: "lag_group_id"
    bitwidth: 8
  }
}
actions {
  preamble {
    id: 24954025
    name: "linux_networking_control.bypass"
    alias: "bypass"
  }
}
actions {
  preamble {
    id: 29238102
    name: "linux_networking_control.drop"
    alias: "drop"
  }
}
actions {
  preamble {
    id: 24137163
    name: "linux_networking_control.vlan_push"
    alias: "vlan_push"
  }
  params {
    id: 1
    name: "pcp"
    bitwidth: 3
  }
  params {
    id: 2
    name: "dei"
    bitwidth: 1
  }
  params {
    id: 3
    name: "vlan_id"
    bitwidth: 12
  }
}
actions {
  preamble {
    id: 25667542
    name: "linux_networking_control.vlan_pop"
    alias: "vlan_pop"
  }
}
actions {
  preamble {
    id: 20733968
    name: "linux_networking_control.vxlan_encap"
    alias: "vxlan_encap"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 32
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 32
  }
  params {
    id: 3
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 5
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 26114242
    name: "linux_networking_control.vxlan_encap_vlan_pop"
    alias: "vxlan_encap_vlan_pop"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 32
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 32
  }
  params {
    id: 3
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 5
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 30345128
    name: "linux_networking_control.vxlan_encap_v6"
    alias: "vxlan_encap_v6"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 128
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 128
  }
  params {
    id: 3
    name: "ds"
    bitwidth: 6
  }
  params {
    id: 4
    name: "ecn"
    bitwidth: 2
  }
  params {
    id: 5
    name: "flow_label"
    bitwidth: 20
  }
  params {
    id: 6
    name: "hop_limit"
    bitwidth: 8
  }
  params {
    id: 7
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 8
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 9
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 28284062
    name: "linux_networking_control.vxlan_encap_v6_vlan_pop"
    alias: "vxlan_encap_v6_vlan_pop"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 128
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 128
  }
  params {
    id: 3
    name: "ds"
    bitwidth: 6
  }
  params {
    id: 4
    name: "ecn"
    bitwidth: 2
  }
  params {
    id: 5
    name: "flow_label"
    bitwidth: 20
  }
  params {
    id: 6
    name: "hop_limit"
    bitwidth: 8
  }
  params {
    id: 7
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 8
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 9
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 25818889
    name: "linux_networking_control.geneve_encap"
    alias: "geneve_encap"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 32
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 32
  }
  params {
    id: 3
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 5
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 26665268
    name: "linux_networking_control.geneve_encap_vlan_pop"
    alias: "geneve_encap_vlan_pop"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 32
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 32
  }
  params {
    id: 3
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 5
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 29610186
    name: "linux_networking_control.geneve_encap_v6"
    alias: "geneve_encap_v6"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 128
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 128
  }
  params {
    id: 3
    name: "ds"
    bitwidth: 6
  }
  params {
    id: 4
    name: "ecn"
    bitwidth: 2
  }
  params {
    id: 5
    name: "flow_label"
    bitwidth: 20
  }
  params {
    id: 6
    name: "hop_limit"
    bitwidth: 8
  }
  params {
    id: 7
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 8
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 9
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 27347188
    name: "linux_networking_control.geneve_encap_v6_vlan_pop"
    alias: "geneve_encap_v6_vlan_pop"
  }
  params {
    id: 1
    name: "src_addr"
    bitwidth: 128
  }
  params {
    id: 2
    name: "dst_addr"
    bitwidth: 128
  }
  params {
    id: 3
    name: "ds"
    bitwidth: 6
  }
  params {
    id: 4
    name: "ecn"
    bitwidth: 2
  }
  params {
    id: 5
    name: "flow_label"
    bitwidth: 20
  }
  params {
    id: 6
    name: "hop_limit"
    bitwidth: 8
  }
  params {
    id: 7
    name: "src_port"
    bitwidth: 16
  }
  params {
    id: 8
    name: "dst_port"
    bitwidth: 16
  }
  params {
    id: 9
    name: "vni"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 28308555
    name: "linux_networking_control.vxlan_decap_outer_hdr"
    alias: "vxlan_decap_outer_hdr"
  }
}
actions {
  preamble {
    id: 27842378
    name: "linux_networking_control.geneve_decap_outer_hdr"
    alias: "geneve_decap_outer_hdr"
  }
}
actions {
  preamble {
    id: 31953264
    name: "linux_networking_control.vxlan_decap_and_push_vlan"
    alias: "vxlan_decap_and_push_vlan"
  }
  params {
    id: 1
    name: "pcp"
    bitwidth: 3
  }
  params {
    id: 2
    name: "dei"
    bitwidth: 1
  }
  params {
    id: 3
    name: "vlan_id"
    bitwidth: 12
  }
}
actions {
  preamble {
    id: 25073902
    name: "linux_networking_control.geneve_decap_and_push_vlan"
    alias: "geneve_decap_and_push_vlan"
  }
  params {
    id: 1
    name: "pcp"
    bitwidth: 3
  }
  params {
    id: 2
    name: "dei"
    bitwidth: 1
  }
  params {
    id: 3
    name: "vlan_id"
    bitwidth: 12
  }
}
actions {
  preamble {
    id: 23093409
    name: "linux_networking_control.set_src_mac_start"
    alias: "set_src_mac_start"
  }
  params {
    id: 1
    name: "arg"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 30315892
    name: "linux_networking_control.set_src_mac_mid"
    alias: "set_src_mac_mid"
  }
  params {
    id: 1
    name: "arg"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 32740970
    name: "linux_networking_control.set_src_mac_last"
    alias: "set_src_mac_last"
  }
  params {
    id: 1
    name: "arg"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 31671750
    name: "linux_networking_control.set_outer_mac"
    alias: "set_outer_mac"
  }
}
actions {
  preamble {
    id: 31163719
    name: "linux_networking_control.set_vxlan_decap_outer_hdr"
    alias: "set_vxlan_decap_outer_hdr"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 25342798
    name: "linux_networking_control.set_vxlan_decap_outer_and_push_vlan"
    alias: "set_vxlan_decap_outer_and_push_vlan"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 26412051
    name: "linux_networking_control.set_vxlan_underlay_v4"
    alias: "set_vxlan_underlay_v4"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 19193142
    name: "linux_networking_control.set_vxlan_underlay_v6"
    alias: "set_vxlan_underlay_v6"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 31983357
    name: "linux_networking_control.pop_vlan_set_vxlan_underlay_v4"
    alias: "pop_vlan_set_vxlan_underlay_v4"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 23849990
    name: "linux_networking_control.pop_vlan_set_vxlan_underlay_v6"
    alias: "pop_vlan_set_vxlan_underlay_v6"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 26749366
    name: "linux_networking_control.set_geneve_decap_outer_hdr"
    alias: "set_geneve_decap_outer_hdr"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 31208923
    name: "linux_networking_control.set_geneve_decap_outer_and_push_vlan"
    alias: "set_geneve_decap_outer_and_push_vlan"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 23736116
    name: "linux_networking_control.set_geneve_underlay_v4"
    alias: "set_geneve_underlay_v4"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 17483375
    name: "linux_networking_control.set_geneve_underlay_v6"
    alias: "set_geneve_underlay_v6"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 24842266
    name: "linux_networking_control.pop_vlan_set_geneve_underlay_v4"
    alias: "pop_vlan_set_geneve_underlay_v4"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 33478945
    name: "linux_networking_control.pop_vlan_set_geneve_underlay_v6"
    alias: "pop_vlan_set_geneve_underlay_v6"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 20
  }
}
actions {
  preamble {
    id: 27998997
    name: "linux_networking_control.set_egress_port"
    alias: "set_egress_port"
  }
  params {
    id: 1
    name: "egress_port"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
}
actions {
  preamble {
    id: 16795029
    name: "linux_networking_control.set_nexthop_info_dmac"
    alias: "set_nexthop_info_dmac"
  }
  params {
    id: 1
    name: "router_interface_id"
    bitwidth: 16
  }
  params {
    id: 2
    name: "egress_port"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
  params {
    id: 3
    name: "dmac_high"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dmac_low"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 25951413
    name: "linux_networking_control.set_nexthop_lag"
    alias: "set_nexthop_lag"
  }
  params {
    id: 1
    name: "router_interface_id"
    bitwidth: 16
  }
  params {
    id: 2
    name: "lag_group_id"
    bitwidth: 8
  }
  params {
    id: 3
    name: "dmac_high"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dmac_low"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 27181644
    name: "linux_networking_control.ecmp_set_nexthop_info_dmac"
    alias: "ecmp_set_nexthop_info_dmac"
  }
  params {
    id: 1
    name: "router_interface_id"
    bitwidth: 16
  }
  params {
    id: 2
    name: "egress_port"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
  params {
    id: 3
    name: "dmac_high"
    bitwidth: 16
  }
  params {
    id: 4
    name: "dmac_low"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 29883644
    name: "linux_networking_control.set_nexthop_id"
    alias: "set_nexthop_id"
  }
  params {
    id: 1
    name: "nexthop_id"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 16874810
    name: "linux_networking_control.ecmp_hash_action"
    alias: "ecmp_hash_action"
  }
  params {
    id: 1
    name: "ecmp_group_id"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 22009623
    name: "linux_networking_control.ipv4_set_nexthop_id"
    alias: "ipv4_set_nexthop_id"
  }
  params {
    id: 1
    name: "nexthop_id"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 21321477
    name: "linux_networking_control.ipv6_set_nexthop_id"
    alias: "ipv6_set_nexthop_id"
  }
  params {
    id: 1
    name: "nexthop_id"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 27000185
    name: "linux_networking_control.ecmp_v6_hash_action"
    alias: "ecmp_v6_hash_action"
  }
  params {
    id: 1
    name: "ecmp_group_id"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 24108243
    name: "linux_networking_control.add_vlan_and_fwd"
    alias: "add_vlan_and_fwd"
  }
  params {
    id: 1
    name: "vlan_ptr"
    bitwidth: 24
  }
  params {
    id: 2
    name: "port_id"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
}
actions {
  preamble {
    id: 25690116
    name: "linux_networking_control.remove_vlan_and_fwd"
    alias: "remove_vlan_and_fwd"
  }
  params {
    id: 1
    name: "vlan_ptr"
    bitwidth: 24
  }
  params {
    id: 2
    name: "port_id"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
}
actions {
  preamble {
    id: 32890467
    name: "linux_networking_control.set_source_port"
    alias: "set_source_port"
  }
  params {
    id: 1
    name: "source_port"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 31939992
    name: "linux_networking_control.set_bridge_id"
    alias: "set_bridge_id"
  }
  params {
    id: 1
    name: "bridge_id"
    bitwidth: 8
  }
}
actions {
  preamble {
    id: 23805991
    name: "linux_networking_control.set_tunnel_v4"
    alias: "set_tunnel_v4"
  }
  params {
    id: 1
    name: "dst_addr"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 23953453
    name: "linux_networking_control.set_tunnel_v6"
    alias: "set_tunnel_v6"
  }
  params {
    id: 1
    name: "ipv6_1"
    bitwidth: 32
  }
  params {
    id: 2
    name: "ipv6_2"
    bitwidth: 32
  }
  params {
    id: 3
    name: "ipv6_3"
    bitwidth: 32
  }
  params {
    id: 4
    name: "ipv6_4"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 26092385
    name: "linux_networking_control.fwd_to_vsi"
    alias: "fwd_to_vsi"
  }
  params {
    id: 1
    name: "port"
    bitwidth: 32
    type_name {
      name: "PortId_t"
    }
  }
}
actions {
  preamble {
    id: 21790705
    name: "linux_networking_control.trap_enable"
    alias: "trap_enable"
  }
}
actions {
  preamble {
    id: 20441854
    name: "linux_networking_control.fwd_to_cp_tx"
    alias: "fwd_to_cp_tx"
  }
}
actions {
  preamble {
    id: 21283156
    name: "linux_networking_control.fwd_to_cp"
    alias: "fwd_to_cp"
  }
}
actions {
  preamble {
    id: 26423957
    name: "linux_networking_control.set_ipsec_tunnel"
    alias: "set_ipsec_tunnel"
  }
  params {
    id: 1
    name: "tunnel_id"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 19192368
    name: "linux_networking_control.ipsec_bypass"
    alias: "linux_networking_control.ipsec_bypass"
  }
}
actions {
  preamble {
    id: 29527637
    name: "linux_networking_control.tx_ipsec_tunnel_v6"
    alias: "tx_ipsec_tunnel_v6"
  }
  params {
    id: 1
    name: "dst_addr_1"
    bitwidth: 32
  }
  params {
    id: 2
    name: "dst_addr_2"
    bitwidth: 32
  }
  params {
    id: 3
    name: "dst_addr_3"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 30154712
    name: "linux_networking_control.tx_ipsec_tunnel"
    alias: "tx_ipsec_tunnel"
  }
  params {
    id: 1
    name: "dst_addr"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 24935652
    name: "linux_networking_control.tx_ipsec_transport"
    alias: "tx_ipsec_transport"
  }
}
actions {
  preamble {
    id: 17885953
    name: "linux_networking_control.tx_ipsec_transport_with_underlay"
    alias: "tx_ipsec_transport_with_underlay"
  }
}
actions {
  preamble {
    id: 25138331
    name: "linux_networking_control.decap_ipsec_tunnel_hdr"
    alias: "decap_ipsec_tunnel_hdr"
  }
}
actions {
  preamble {
    id: 20682322
    name: "linux_networking_control.ipsec_protect_set_metadata"
    alias: "ipsec_protect_set_metadata"
  }
  params {
    id: 1
    name: "saidx"
    bitwidth: 24
  }
}
actions {
  preamble {
    id: 30081157
    name: "linux_networking_control.ipsec_tunnel_encap_mod"
    alias: "ipsec_tunnel_encap_mod"
  }
  params {
    id: 1
    name: "ipsec_src_addr"
    bitwidth: 32
  }
  params {
    id: 2
    name: "ipsec_dst_addr"
    bitwidth: 32
  }
  params {
    id: 3
    name: "proto"
    bitwidth: 8
  }
}
actions {
  preamble {
    id: 33290754
    name: "linux_networking_control.ipsec_transport_mod_action"
    alias: "ipsec_transport_mod_action"
  }
}
actions {
  preamble {
    id: 24657351
    name: "linux_networking_control.ipsec_transport_with_underlay_mod_action"
    alias: "ipsec_transport_with_underlay_mod_action"
  }
}
actions {
  preamble {
    id: 28758942
    name: "linux_networking_control.ipsec_tunnel_decap"
    alias: "ipsec_tunnel_decap"
  }
}
actions {
  preamble {
    id: 28718673
    name: "linux_networking_control.vm_src_ip4_mac_map_action"
    alias: "vm_src_ip4_mac_map_action"
  }
  params {
    id: 1
    name: "smac_high"
    bitwidth: 16
  }
  params {
    id: 2
    name: "smac_mid"
    bitwidth: 16
  }
  params {
    id: 3
    name: "smac_low"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 25018769
    name: "linux_networking_control.vm_dst_ip4_mac_map_action"
    alias: "vm_dst_ip4_mac_map_action"
  }
  params {
    id: 1
    name: "dmac_high"
    bitwidth: 16
  }
  params {
    id: 2
    name: "dmac_mid"
    bitwidth: 16
  }
  params {
    id: 3
    name: "dmac_low"
    bitwidth: 16
  }
}
actions {
  preamble {
    id: 17061464
    name: "linux_networking_control.ipv4_lpm_root_lut_action"
    alias: "ipv4_lpm_root_lut_action"
  }
  params {
    id: 1
    name: "ipv4_table_lpm_root"
    bitwidth: 32
  }
}
actions {
  preamble {
    id: 25701894
    name: "linux_networking_control.ipv6_lpm_root_lut_action"
    alias: "ipv6_lpm_root_lut_action"
  }
  params {
    id: 1
    name: "ipv6_table_lpm_root"
    bitwidth: 32
  }
}
direct_counters {
  preamble {
    id: 333810446
    name: "linux_networking_control.ddcounter_pool_tx2"
    alias: "ddcounter_pool_tx2"
  }
  spec {
    unit: BOTH
  }
}
direct_counters {
  preamble {
    id: 323002307
    name: "linux_networking_control.ddcounter_pool_tx3"
    alias: "ddcounter_pool_tx3"
  }
  spec {
    unit: BOTH
  }
}
direct_counters {
  preamble {
    id: 327348295
    name: "linux_networking_control.ddcounter_pool_tx4"
    alias: "ddcounter_pool_tx4"
  }
  spec {
    unit: BOTH
  }
}
direct_counters {
  preamble {
    id: 330899638
    name: "linux_networking_control.ddcounter_pool_tx5"
    alias: "ddcounter_pool_tx5"
  }
  spec {
    unit: BOTH
  }
}
type_info {
  new_types {
    key: "PortId_t"
    value {
      translated_type {
        uri: "p4.org/pna/v1/PortId_t"
        sdn_bitwidth: 32
      }
    }
  }
})p4";

#endif  // TESTING_P4INFO_TEXT_H_
