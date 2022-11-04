#ifndef LINUX_NETWORKING_METADATA_P4_
#define LINUX_NETWORKING_METADATA_P4_

#include "pna.p4"
#include "headers.p4"

typedef bit<16> ActionRef_t;
typedef bit<24> ModDataPtr_t;

struct vendor_metadata_t {
    // The modify action to be done at the end of a pass
    ActionRef_t mod_action_ref;
    // A pointer to any data needed by the action mod_action_ref
    ModDataPtr_t mod_data_ptr;
}

const ActionRef_t VXLAN_ENCAP = (ActionRef_t) 1;
const ActionRef_t VXLAN_DECAP_OUTER_IPV4 = (ActionRef_t) 2;
const ActionRef_t NEIGHBOR = (ActionRef_t) 3;

#define MIN_TABLE_SIZE 64

typedef bit<24> tunnel_id_t;
typedef bit<24> vni_id_t;
typedef bit<8> vrf_id_t;
typedef bit<16> router_interface_id_t;
typedef bit<24> neighbor_id_t;

struct headers_t {
    ethernet_t outer_ethernet;
    vlan_t[2] outer_vlan;
    arp_t outer_arp;
    ipv4_t outer_ipv4;
    tcp_t outer_tcp;
    udp_t outer_udp;
    icmp_t outer_icmp;
    vxlan_t vxlan;
    ethernet_t ethernet;
    vlan_t[2] vlan;
    arp_t arp;
    ipv4_t ipv4;
    udp_t udp;
    tcp_t tcp;
    icmp_t icmp;
}

struct port_metadata_t {
    bool admin_state;  /* SAI_PORT_ATTR_ADMIN_STATE              */
    bool ingress_filtering;  /* SAI_PORT_ATTR_INGRESS_FILTERING        */
    bool drop_untagged;  /* SAI_PORT_ATTR_DROP_UNTAGGED            */
    bool drop_tagged;  /* SAI_PORT_ATTR_DROP_UNTAGGED            */
    bit<12> port_vlan_id; /* SAI_PORT_ATTR_VLAN_ID                  */
    bit<16> mtu; /* SAI_PORT_ATTR_MTU                      */
    bit<3> default_vlan_priority;  /* SAI_PORT_ATTR_DEFAULT_VLAN_PRIORITY    */
}

struct tunnel_metadata_t {
    tunnel_id_t id;
    vni_id_t vni;
    bit<8> tun_type;
    bit<16> hash;
}

// Local metadata for each packet being processed.
struct local_metadata_t {
    bit<8> exception_packet;
    bit<8> control_packet;
    bool admit_to_l3;
    vrf_id_t vrf_id;
    bit<16> ecmp_group_id;
    bit<16> ecmp_hash;
    bit<16> nexthop_id;
    router_interface_id_t rif_mod_map_id;
    bit<16> vlan_id;
    bit<8> is_tunnel;
    bit<16> hash;
    ipv4_addr_t ipv4_dst_match;
    // Tunnel metadata
    tunnel_metadata_t tunnel;
    bit<16> host_info_tx_extended_flex_0;
    bit<16> host_info_tx_extended_flex_1;
    bit<16> host_info_tx_extended_flex_2;
    bit<16> host_info_tx_extended_flex_3;
    bit<16> host_info_tx_extended_flex_4;
    bit<16> host_info_tx_extended_flex_5;
}
#endif // LINUX_NETWORKING_METADATA_P4_
