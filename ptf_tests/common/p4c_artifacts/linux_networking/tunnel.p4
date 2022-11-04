#ifndef LINUX_NETWORKING_TUNNEL_P4_
#define LINUX_NETWORKING_TUNNEL_P4_

#include "pna.p4"
#include "headers.p4"
#include "metadata.p4"

/* Tunnel Decap */
control tunnel_decap(inout headers_t hdr,
                     inout local_metadata_t local_metadata,
                     inout vendor_metadata_t vendormeta)
{
   /* TODO: decttl, copy (ecn_marker to inner) */
  action decap_outer_ipv4(tunnel_id_t tunnel_id) {
    vendormeta.mod_data_ptr = tunnel_id; /* ptr can be tunnel_id */
    vendormeta.mod_action_ref = VXLAN_DECAP_OUTER_IPV4;
    local_metadata.ipv4_dst_match = hdr.ipv4.dst_addr;
  }

  table ipv4_tunnel_term_table {
    key = {
      hdr.outer_ipv4.src_addr : exact @name("ipv4_src");
      hdr.outer_ipv4.dst_addr : exact @name("ipv4_dst");
      local_metadata.tunnel.tun_type : exact; /* base it on udp dst port - check parser for tunnel type */
    }
    actions = {
      decap_outer_ipv4;
      @defaultonly NoAction;
    }
    const default_action = NoAction;
  }

  apply {
    ipv4_tunnel_term_table.apply();
  }
}  // control tunnel_decap

/* Tunnel Encap */
control tunnel_encap(inout headers_t hdr,
               inout local_metadata_t local_metadata,
               inout vendor_metadata_t vendormeta,
               in pna_main_input_metadata_t istd)
{
  action set_tunnel(ModDataPtr_t tunnel_id, ipv4_addr_t dst_addr) {
    vendormeta.mod_action_ref = VXLAN_ENCAP;
    vendormeta.mod_data_ptr = tunnel_id; /* ptr can be tunnel_id */
    local_metadata.ipv4_dst_match = dst_addr;
  }

  table set_tunnel_encap {
    key = {
      istd.input_port: exact;
    }
    actions = {
      set_tunnel;
      @defaultonly NoAction;
    }
    const default_action = NoAction;
    size = MIN_TABLE_SIZE;
  }

  apply {
    set_tunnel_encap.apply();
  }
}

#endif  // LINUX_NETWORKING_TUNNEL_P4_
