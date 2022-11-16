/*
 * Copyright (c) 2022 Intel Corporation.
 *
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

#ifndef LINUX_NETWORKING_HEADERS_P4_
#define LINUX_NETWORKING_HEADERS_P4_

#define ETHERTYPE_IPV4  0x0800
#define ETHERTYPE_IPV6  0x86dd
#define ETHERTYPE_ARP   0x0806
#define ETHERTYPE_LLDP  0x88cc
#define ETHERTYPE_VLAN  0x8100
#define ETHERTYPE_MAC   0x88e7

#define IP_PROTOCOL_IP_IP  0x04
#define IP_PROTOCOL_IPV6   0x29
#define IP_PROTOCOL_TCP    0x06
#define IP_PROTOCOL_UDP    0x11
#define IP_PROTOCOL_ICMP   0x01
#define IP_PROTOCOL_ICMPV6 0x3a
#define IP_PROTOCOLS_GRE   0x2f
#define IP_PROTOCOLS_UDP_PORT_VXLAN 4789

#define GRE_PROTOCOL_ERSPAN 0x88be

#define ERSPAN_VERSION_TYPE_II 1

#define TUNNEL_TYPE_VXLAN      2

typedef bit<48> ethernet_addr_t;
typedef bit<32> ipv4_addr_t;
typedef bit<12> vlan_id_t;

// -- Protocol headers ---------------------------------------------------------

#define ETHERNET_HEADER_BYTES 14

header ethernet_t {
    ethernet_addr_t dst_addr;
    ethernet_addr_t src_addr;
    bit<16> ether_type;
}

header vlan_t {
    bit<16> pcp_cfi_vid;
    bit<16> ether_type;
}

#define IPV4_HEADER_BYTES 20

header ipv4_t {
    bit<8> version_ihl;
    bit<8> dscp_ecn;
    bit<16> total_len;
    bit<16> identification;
    bit<16> flags_frag_offset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> header_checksum;
    ipv4_addr_t src_addr;
    ipv4_addr_t dst_addr;
}

#define IPV6_HEADER_BYTES 40

header ipv6_t {
    bit <32> version_dscp_ecn_flowlabel;
    bit<16> payload_length;
    bit<8> next_header;
    bit<8> hop_limit;
    bit <64> src_addr_hi;
    bit <64> src_addr_lo;
    bit <64> dst_addr_hi;
    bit <64> dst_addr_lo;
}


#define UDP_HEADER_BYTES 8

header udp_t {
    bit<16> src_port;
    bit<16> dst_port;
    bit<16> hdr_length;
    bit<16> checksum;
}

header tcp_t {
    bit<16> src_port;
    bit<16> dst_port;
    bit<32> seq_no;
    bit<32> ack_no;
    bit <8> data_offset_res;
    bit<8> flags;
    bit<16> window;
    bit<16> checksum;
    bit<16> urgent_ptr;
}

header icmp_t {
    bit<8> type;
    bit<8> code;
    bit<16> checksum;
}

header arp_t {
    bit<16> hw_type;
    bit<16> proto_type;
    bit<8> hw_addr_len;
    bit<8> proto_addr_len;
    bit<16> opcode;
    bit<48> sender_hw_addr;
    bit<32> sender_proto_addr;
    bit<48> target_hw_addr;
    bit<32> target_proto_addr;
}

#define VXLAN_HEADER_BYTES 8

// VXLAN -- RFC 7348
header vxlan_t {
    bit<8> flags;
    bit<24> reserved;
    bit<24> vni;
    bit<8> reserved2;
}

#endif  // LINUX_NETWORKING_HEADERS_P4_
