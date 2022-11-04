/* -*- P4_16 -*- */

#include <core.p4>
#include <psa.p4>

/*************************************************************************
 ************* C O N S T A N T S    A N D   T Y P E S  *******************
**************************************************************************/
const bit<16> ETHERTYPE_IPV4 = 0x0800;

/* Table Sizes */
#ifndef IPV4_HOST_SIZE
  #define IPV4_HOST_SIZE 65536
#endif

const int _IPV4_HOST_SIZE = IPV4_HOST_SIZE;
typedef bit<48> mac_addr_t;

struct empty_metadata_t {
}
/*************************************************************************
 ***********************  H E A D E R S  *********************************
 *************************************************************************/

/*  Define all the headers the program will recognize             */
/*  The actual sets of headers processed by each gress can differ */

/* Standard ethernet header */
header ethernet_h {
    bit<48>   dst_addr;
    bit<48>   src_addr;
    bit<16>   ether_type;
}

header ipv4_h {
    bit<8>       version_ihl;
    bit<8>       diffserv;
    bit<16>      total_len;
    bit<16>      identification;
    bit<16>      flags_frag_offset;
    bit<8>       ttl;
    bit<8>       protocol;
    bit<16>      hdr_checksum;
    bit<32>      src_addr;
    bit<32>      dst_addr;
}

/*************************************************************************
 **************  I N G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/

struct my_ingress_headers_t {
    ethernet_h   ethernet;
    ipv4_h       ipv4;
}

struct local_metadata_t {
    bit<48> dst_addr;
    bit<48> src_addr;
}

    /******  G L O B A L   I N G R E S S   M E T A D A T A  *********/

    /***********************  P A R S E R  **************************/
parser packet_parser(packet_in packet, out my_ingress_headers_t headers, inout local_metadata_t local_metadata, in psa_ingress_parser_input_metadata_t standard_metadata, in empty_metadata_t resub_meta, in empty_metadata_t recirc_meta) {
    state start {
        transition parse_ethernet;
    }
    state parse_ethernet {
        packet.extract(headers.ethernet);
        transition parser_ipv4;
    }
    state parser_ipv4 {
        packet.extract(headers.ipv4);
        transition accept;
    }
}

control packet_deparser(packet_out packet, out empty_metadata_t clone_i2e_meta, out empty_metadata_t resubmit_meta, out empty_metadata_t normal_meta, inout my_ingress_headers_t headers, in local_metadata_t local_metadata, in psa_ingress_output_metadata_t istd) {
    apply {
        packet.emit(headers);
/*
        packet.emit(headers.outer_ethernet);
        packet.emit(headers.outer_ipv4);
        packet.emit(headers.outer_udp);
        packet.emit(headers.outer_vxlan);
        packet.emit(headers.ethernet);
        packet.emit(headers.ipv4);
*/
    }
}

    /***************** M A T C H - A C T I O N  *********************/

control ingress(inout my_ingress_headers_t headers, inout local_metadata_t local_metadata1, in psa_ingress_input_metadata_t standard_metadata, inout psa_ingress_output_metadata_t ostd) {
    action send(PortId_t dst_port) {
 ostd.egress_port = (PortId_t)dst_port;
    }

    action drop() {
 ostd.drop = true;
    }

    table ipv4_host_src {
        key = { headers.ipv4.src_addr : exact; } 
        actions = {
            send; drop;
        }

        size = _IPV4_HOST_SIZE;
    }

    table ipv4_host_dst {
        key = { headers.ipv4.dst_addr : exact; }
        actions = {
            send; drop;
        }
        const default_action = drop;
        size = _IPV4_HOST_SIZE;
    }


    apply {
        ipv4_host_src.apply();
        ipv4_host_dst.apply();
/*
        if (headers.ipv4.isValid()) {
                ipv4_host.apply();
        }
*/
    }
}

    /*********************  D E P A R S E R  ************************/


/*************************************************************************
 ****************  E G R E S S   P R O C E S S I N G   *******************
 *************************************************************************/

    /***********************  H E A D E R S  ************************/

struct my_egress_headers_t {
}

    /********  G L O B A L   E G R E S S   M E T A D A T A  *********/

struct my_egress_metadata_t {
}

control egress(inout my_ingress_headers_t headers, inout local_metadata_t local_metadata, in psa_egress_input_metadata_t istd, inout psa_egress_output_metadata_t ostd) {
    apply {
    }
}

parser egress_parser(packet_in buffer, out my_ingress_headers_t headers, inout local_metadata_t local_metadata, in psa_egress_parser_input_metadata_t istd, in empty_metadata_t normal_meta, in empty_metadata_t clone_i2e_meta, in empty_metadata_t clone_e2e_meta) {
    state start {
        transition accept;
    }
}

control egress_deparser(packet_out packet, out empty_metadata_t clone_e2e_meta, out empty_metadata_t recirculate_meta, inout my_ingress_headers_t headers, in local_metadata_t local_metadata, in psa_egress_output_metadata_t istd, in psa_egress_deparser_input_metadata_t edstd) {
    apply {
    }
}

IngressPipeline(packet_parser(), ingress(), packet_deparser()) pipe;

EgressPipeline(egress_parser(), egress(), egress_deparser()) ep;

PSA_Switch(pipe, PacketReplicationEngine(), ep, BufferingQueueingEngine()) main;


