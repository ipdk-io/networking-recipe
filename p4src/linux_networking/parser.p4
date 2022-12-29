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

#ifndef LINUX_NETWORKING_PARSER_P4_
#define LINUX_NETWORKING_PARSER_P4_

#include "pna.p4"
#include "headers.p4"
#include "metadata.p4"

parser packet_parser(packet_in packet, out headers_t headers,
                     inout local_metadata_t local_metadata,
                     in    pna_main_parser_input_metadata_t istd) {

    state start {
        transition parse_outer_ethernet;
    }

    state parse_outer_ethernet {
        packet.extract(headers.outer_ethernet);
        local_metadata.control_packet = 0;
        transition select(headers.outer_ethernet.ether_type) {
            ETHERTYPE_VLAN: parse_outer_vlan_0;
            ETHERTYPE_ARP:  parse_outer_arp;
            ETHERTYPE_IPV4: parse_outer_ipv4;
            _:              accept;
        }
    }

    state parse_outer_vlan_0 {
        packet.extract(headers.outer_vlan[0]);
            local_metadata.vlan_id = headers.outer_vlan[0].pcp_cfi_vid;
            transition select(headers.outer_vlan[0].ether_type) {
                ETHERTYPE_VLAN:  parse_outer_vlan_1;
                ETHERTYPE_ARP:   parse_outer_arp;
                ETHERTYPE_IPV4:  parse_outer_ipv4;
                default: accept;
            }
    }

    state parse_outer_vlan_1 {
        packet.extract(headers.outer_vlan[1]);
            local_metadata.vlan_id = headers.outer_vlan[1].pcp_cfi_vid;
            transition select(headers.outer_vlan[1].ether_type) {
                ETHERTYPE_ARP:   parse_outer_arp;
                ETHERTYPE_IPV4:  parse_outer_ipv4;
                default: accept;
            }
    }

    state parse_outer_arp {
        packet.extract(headers.outer_arp);
        local_metadata.control_packet = 1;
        transition accept;
    }

    state parse_outer_ipv4 {
        packet.extract(headers.outer_ipv4);
        transition select(headers.outer_ipv4.protocol) {
            IP_PROTOCOL_ICMP: parse_outer_icmp;
            IP_PROTOCOL_TCP:  parse_outer_tcp;
            IP_PROTOCOL_UDP:  parse_outer_udp;
            _:                accept;
        }
    }

    state parse_outer_udp {
        packet.extract(headers.outer_udp);
        transition select(headers.outer_udp.dst_port) {
            IP_PROTOCOLS_UDP_PORT_VXLAN: parse_vxlan;
            _:                           accept;
        }
    }

    state parse_outer_icmp {
        packet.extract(headers.outer_icmp);
        transition accept;
    }

    state parse_outer_tcp {
        packet.extract(headers.outer_tcp);
        transition accept;
    }

    state parse_vxlan {
        packet.extract(headers.vxlan);
        local_metadata.tunnel.tun_type = TUNNEL_TYPE_VXLAN;
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(headers.ethernet);
        transition select(headers.ethernet.ether_type) {
            ETHERTYPE_VLAN: parse_vlan_0;
            ETHERTYPE_ARP:  parse_arp;
            ETHERTYPE_IPV4: parse_ipv4;
            _:              accept;
        }
    }

    state parse_vlan_0 {
        packet.extract(headers.vlan[0]);
        local_metadata.vlan_id = headers.vlan[0].pcp_cfi_vid;
            transition select(headers.vlan[0].ether_type) {
                ETHERTYPE_VLAN:  parse_vlan_1;
                ETHERTYPE_ARP:   parse_arp;
                ETHERTYPE_IPV4:  parse_ipv4;
                default: accept;
            }
    }

    state parse_vlan_1 {
        packet.extract(headers.vlan[1]);
        local_metadata.vlan_id = headers.vlan[1].pcp_cfi_vid;
            transition select(headers.vlan[1].ether_type) {
                ETHERTYPE_ARP:   parse_arp;
                ETHERTYPE_IPV4:  parse_ipv4;
                default: accept;
            }
    }

    state parse_arp {
        packet.extract(headers.arp);
        local_metadata.control_packet = 1;
        transition accept;
    }

    state parse_ipv4 {
        packet.extract(headers.ipv4);
        transition select(headers.ipv4.protocol) {
            IP_PROTOCOL_ICMP: parse_icmp;
            IP_PROTOCOL_TCP:  parse_tcp;
            IP_PROTOCOL_UDP:  parse_udp;
            _:                accept;
        }
    }

    state parse_icmp {
        packet.extract(headers.icmp);
        transition accept;
    }

    state parse_tcp {
        packet.extract(headers.tcp);
        transition accept;
    }

    state parse_udp {
        packet.extract(headers.udp);
        transition accept;
    }

}  // parser packet_parser

control packet_deparser(packet_out packet, in headers_t headers,
                        in local_metadata_t local_metadata,
                        in    pna_main_output_metadata_t ostd) {
    apply {
        packet.emit(headers.outer_ethernet);
        packet.emit(headers.outer_vlan);
        packet.emit(headers.outer_arp);
        packet.emit(headers.outer_ipv4);
        packet.emit(headers.outer_icmp);
        packet.emit(headers.outer_udp);
        packet.emit(headers.outer_tcp);
        packet.emit(headers.vxlan);
        packet.emit(headers.ethernet);
        packet.emit(headers.vlan);
        packet.emit(headers.arp);
        packet.emit(headers.ipv4);
        packet.emit(headers.icmp);
        packet.emit(headers.tcp);
        packet.emit(headers.udp);
    }
}  // control packet_deparser

#endif  // LINUX_NETWORKING_PARSER_P4_
