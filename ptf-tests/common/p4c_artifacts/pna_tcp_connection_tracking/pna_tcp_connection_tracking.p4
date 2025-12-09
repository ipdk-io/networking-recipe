#include <core.p4>
#include "pna.p4"

#define IP_PACKET_FWD_MAX_ENTRIES 0x1000
#define IP_PROTO_UDP    0x11
#define IP_PROTO_TCP    0x06
#define IP_PROTO_ICMP   0x01

///////////////////////////////////////////////////////////////////
// Headers definations                                           //
///////////////////////////////////////////////////////////////////

typedef bit<48>  EthernetAddress_t;
const bit<8> ETHERNET_HEADER_LEN = 14;

header ethernet_t {
    EthernetAddress_t dstAddr;
    EthernetAddress_t srcAddr;
    bit<16>           etherType;
}

header ipv4_t {
    bit<4>  version;
    bit<4>  ihl;
    bit<8>  diffserv;
    bit<16> totalLen;
    bit<16> identification;
    bit<3>  flags;
    bit<13> fragOffset;
    bit<8>  ttl;
    bit<8>  protocol;
    bit<16> hdrChecksum;
    bit<32> srcAddr;
    bit<32> dstAddr;
}

header icmp_t {
    bit<8> type;
    bit<8> code;
    bit<16> checksum;
    bit<16> identifier;
    bit<16> seqNum;
}

header tcp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<32> seqNo;
    bit<32> ackNo;
    bit<4>  dataOffset;
    bit<4>  res;
    bit<8>  flags;
    bit<16> window;
    bit<16> checksum;
    bit<16> urgentPtr;
}

header udp_t {
    bit<16>  srcPort;
    bit<16>  dstPort;
    bit<16>  length;
    bit<16>  checksum;
}

struct headers_t {
    ethernet_t ethernet;
    ipv4_t ipv4;
    icmp_t icmp;
    tcp_t  tcp;
    udp_t  udp;
}

struct hash_params_t {
    bit<32> ipAddr1;
    bit<32> ipAddr2;
    bit <8> protocol;
    bit<16> port1;
    bit<16> port2;
    bit<16> identifier;
}

struct main_metadata_t {
    bit<8> dir;
    bit<8> currentState;
    bit<8> tcpFlagsSet;
    bit<32> hashCode;
    bit<32> replyHashCode;
    bit<32> connHashCode;
    bool doAddOnMiss;
    bool updateExpireTimeoutProfile;
    bool restartExpireTimer;
    bool acceptPacket;
    ExpireTimeProfileId_t timeoutProfileId;
    hash_params_t hashParams;
}

struct tcp_ct_table_hit_params_t {
}

struct udp_ct_table_hit_params_t {
}

struct icmp_ct_table_hit_params_t {
}

///////////////////////////////////////////////////////////////////
// TCP 5-Tuple Hash                                              //
///////////////////////////////////////////////////////////////////
Hash<bit<32>>(PNA_HashAlgorithm_t.TARGET_DEFAULT) tupleHash;

///////////////////////////////////////////////////////////////////
// CT TCP statemachine                                           //
///////////////////////////////////////////////////////////////////
const bit<32> TCP_CT_MAX_FLOW_CNT = 0x1000;
Register<bit<8>, bit<32>>(TCP_CT_MAX_FLOW_CNT) tcpCtCurrentStateReg;

///////////////////////////////////////////////////////////////////
// CT TCP Direction                                              //
///////////////////////////////////////////////////////////////////
Register<bit<8>, bit<32>>(TCP_CT_MAX_FLOW_CNT, 0xFE) tcpCtDirReg;

///////////////////////////////////////////////////////////////////
// CT UDP statemachine                                           //
///////////////////////////////////////////////////////////////////
const bit<32> UDP_CT_MAX_FLOW_CNT = 0x1000;
Register<bit<8>, bit<32>>(UDP_CT_MAX_FLOW_CNT) udpCtCurrentStateReg;

///////////////////////////////////////////////////////////////////
// CT UDP Direction                                              //
///////////////////////////////////////////////////////////////////
Register<bit<8>, bit<32>>(UDP_CT_MAX_FLOW_CNT, 0xFE) udpCtDirReg;

///////////////////////////////////////////////////////////////////
// CT ICMP statemachine                                          //
///////////////////////////////////////////////////////////////////
const bit<32> ICMP_CT_MAX_FLOW_CNT = 0x1000;
Register<bit<8>, bit<32>>(ICMP_CT_MAX_FLOW_CNT) icmpCtCurrentStateReg;

///////////////////////////////////////////////////////////////////
// CT ICMP Direction                                             //
///////////////////////////////////////////////////////////////////
Register<bit<8>, bit<32>>(ICMP_CT_MAX_FLOW_CNT, 0xFE) icmpCtDirReg;

/* Connection Direction */
const bit<8> CT_DIR_ORIGINAL = 0x0;
const bit<8> CT_DIR_REPLY    = 0x1;

/* TCP Flags bit masks (Flags- C E U A P R S F) */
const bit<8> TCP_URG_MASK = 0x20;
const bit<8> TCP_ACK_MASK = 0x10;
const bit<8> TCP_PSH_MASK = 0x08;
const bit<8> TCP_RST_MASK = 0x04;
const bit<8> TCP_SYN_MASK = 0x02;
const bit<8> TCP_FIN_MASK = 0x01;

/* What TCP flags are set from RST/SYN/FIN/ACK. */
const bit<8> TCP_SYN_SET    = 0x01;
const bit<8> TCP_SYNACK_SET = 0x02;
const bit<8> TCP_FIN_SET    = 0x03;
const bit<8> TCP_ACK_SET    = 0x04;
const bit<8> TCP_RST_SET    = 0x05;
const bit<8> TCP_NONE_SET   = 0x06;

/* TCP Statemachine Timeout */
///////////////////////////////////////////////////////////////////
// TCP Statemachine Timeout                                      //
// Timer Profile 0 - 10 sec                                      //
// Timer Profile 1 - 30 sec                                      //
// Timer Profile 2 - 60 sec                                      //
// Timer Profile 3 - 120 sec                                     //
// Timer Profile 4 - 86400 sec                                   //
///////////////////////////////////////////////////////////////////
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_SYN_SENT    = (ExpireTimeProfileId_t)3;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_SYN_RECV    = (ExpireTimeProfileId_t)2;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_ESTABLISHED = (ExpireTimeProfileId_t)4;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_FIN_WAIT    = (ExpireTimeProfileId_t)3;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_CLOSE_WAIT  = (ExpireTimeProfileId_t)2;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_LAST_ACK    = (ExpireTimeProfileId_t)1;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_TIME_WAIT   = (ExpireTimeProfileId_t)3;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_CLOSE 	   = (ExpireTimeProfileId_t)0;
const ExpireTimeProfileId_t TCP_CT_TIMEOUT_SYN_SENT2   = (ExpireTimeProfileId_t)3;

/* UDP Statemachine Timeout */
const ExpireTimeProfileId_t UDP_CT_TIMEOUT_UNREPLIED   = (ExpireTimeProfileId_t)1;
const ExpireTimeProfileId_t UDP_CT_TIMEOUT_REPLIED	   = (ExpireTimeProfileId_t)3;

/* ICMP Statemachine Timeout */
const ExpireTimeProfileId_t ICMP_CT_TIMEOUT_UNREPLIED   = (ExpireTimeProfileId_t)1;
const ExpireTimeProfileId_t ICMP_CT_TIMEOUT_REPLIED	   = (ExpireTimeProfileId_t)3;

///////////////////////////////////////////////////////////////////
// TCP states                                                    //
///////////////////////////////////////////////////////////////////
const bit<8> TCP_CT_NONE        = 0x00;
const bit<8> TCP_CT_SYN_SENT    = 0x01;
const bit<8> TCP_CT_SYN_RECV    = 0x02;
const bit<8> TCP_CT_ESTABLISHED = 0x03;
const bit<8> TCP_CT_FIN_WAIT    = 0x04;
const bit<8> TCP_CT_CLOSE_WAIT  = 0x05;
const bit<8> TCP_CT_LAST_ACK    = 0x06;
const bit<8> TCP_CT_TIME_WAIT   = 0x07;
const bit<8> TCP_CT_CLOSE 	    = 0x08;
const bit<8> TCP_CT_SYN_SENT2    = 0x09;
const bit<8> TCP_CT_LISTEN 	    = 0x09;
const bit<8> TCP_CT_MAX 	    = 0x0A;
const bit<8> TCP_CT_IGNORE 	    = 0x0B;
const bit<8> TCP_CT_RETRANS 	= 0x0C;
const bit<8> TCP_CT_UNACK 	    = 0x0D;
const bit<8> TCP_CT_TIMEOUT_MAX = 0x0E;

///////////////////////////////////////////////////////////////////
// UDP states                                                    //
///////////////////////////////////////////////////////////////////
const bit<8> UDP_CT_NONE        = 0x00;
const bit<8> UDP_CT_UNREPLIED   = 0x11;
const bit<8> UDP_CT_REPLIED	    = 0x12;

///////////////////////////////////////////////////////////////////
// ICMP states                                                    //
///////////////////////////////////////////////////////////////////
const bit<8> ICMP_CT_NONE        = 0x00;
const bit<8> ICMP_CT_UNREPLIED   = 0x11;
const bit<8> ICMP_CT_REPLIED	 = 0x12;

///////////////////////////////////////////////////////////////////
// Helper functions                                              //
///////////////////////////////////////////////////////////////////
bit<8> getTcpFlagSetInfo(in bit<8> tcp_flags) {
    // RST Flag
    if ((tcp_flags & TCP_RST_MASK) != 0) {
        return TCP_RST_SET;
    }

    // SYN or SYN+ACK Flag
    if ((tcp_flags & TCP_SYN_MASK) != 0) {
        if ((tcp_flags & TCP_ACK_MASK) != 0) {
            return TCP_SYNACK_SET;
        } else {
            return TCP_SYN_SET;
        }
    }

    // FIN Flag
    if ((tcp_flags & TCP_FIN_MASK) != 0) {
        return TCP_FIN_SET;
    }

    // ACK Flag
    if ((tcp_flags & TCP_ACK_MASK) != 0) {
        return TCP_ACK_SET;
    }

    return TCP_NONE_SET;
}

bit<8> getTcpDirection(in bit<32> hash, in bit<32> replyHash) {
    if (tcpCtDirReg.read(replyHash) == CT_DIR_ORIGINAL) {
        return CT_DIR_REPLY;
    }

    return CT_DIR_ORIGINAL;
}

bit<8> getUdpDirection(in bit<32> hash, in bit<32> replyHash) {
    if (udpCtDirReg.read(replyHash) == CT_DIR_ORIGINAL) {
        return CT_DIR_REPLY;
    }

    return CT_DIR_ORIGINAL;
}

bit<8> getIcmpDirection(in bit<32> hash, in bit<32> replyHash) {
    if (icmpCtDirReg.read(replyHash) == CT_DIR_ORIGINAL) {
        return CT_DIR_REPLY;
    }

    return CT_DIR_ORIGINAL;
}

parser MainParserImpl(
    packet_in pkt,
    out   headers_t hdr,
    inout main_metadata_t meta,
    in    pna_main_parser_input_metadata_t istd)
{
    state start {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            0x0800: parse_ipv4;
            default: accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.protocol) {
            IP_PROTO_ICMP: parse_icmp;
            IP_PROTO_TCP: parse_tcp;
            IP_PROTO_UDP: parse_udp;
            default: accept;
        }
    }

    state parse_icmp {
        pkt.extract(hdr.icmp);
        transition accept;
    }

    state parse_tcp {
        pkt.extract(hdr.tcp);
        transition accept;
    }

    state parse_udp {
        pkt.extract(hdr.udp);
        transition accept;
    }
}

control PreControlImpl(
        in    headers_t  hdr,
        inout main_metadata_t meta,
        in    pna_pre_input_metadata_t  istd,
        inout pna_pre_output_metadata_t ostd)
{
    apply { }
}


control MainControlImpl(
    inout headers_t       hdr,
    inout main_metadata_t meta,
    in    pna_main_input_metadata_t  istd,
    inout pna_main_output_metadata_t ostd)
{
    bool add_succeeded;

    action drop() {
        drop_packet();
    }

    action tcp_ct_syn_sent() {
        // Update Connection new state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_SYN_SENT);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_SYN_SENT;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Enable Connection Autolearning Flag
        meta.doAddOnMiss = true;

        // Updating direction register
        tcpCtDirReg.write(meta.hashCode, CT_DIR_ORIGINAL);
    }

    action tcp_ct_syn_recv() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_SYN_RECV);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_SYN_RECV;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }

    action tcp_ct_established() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_ESTABLISHED);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_ESTABLISHED;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }
    action tcp_ct_fin_wait() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_FIN_WAIT);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_FIN_WAIT;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }

    action tcp_ct_close_wait() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_CLOSE_WAIT);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_CLOSE_WAIT;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }

    action tcp_ct_last_ack() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_LAST_ACK);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_LAST_ACK;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }

    action tcp_ct_time_wait() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_TIME_WAIT);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_TIME_WAIT;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }

    action tcp_ct_close() {
        // Update Connection state
        // tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_CLOSE);
        // Reset the current state register to NONE when the TCP state transition
        // to the close state So that the next TCP state machine is processed
        // correctly.
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_NONE);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_CLOSE;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;

    }

    action tcp_ct_syn_sent2() {
        // Update Connection state
        tcpCtCurrentStateReg.write(meta.connHashCode, TCP_CT_SYN_SENT2);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)TCP_CT_TIMEOUT_SYN_SENT2;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Disable Connection Autolearning Flag
        meta.doAddOnMiss = false;
    }

    action tcp_ct_ignore() {
        meta.updateExpireTimeoutProfile = false;
        meta.restartExpireTimer = false;
        meta.doAddOnMiss = false;
    }

    table tcp_ct_state_table {
         key = {
            meta.dir : exact;
            meta.tcpFlagsSet : exact;
            meta.currentState : exact;
        }

        actions = {
            drop;
            tcp_ct_syn_sent;
            tcp_ct_syn_recv;
            tcp_ct_established;
            tcp_ct_fin_wait;
            tcp_ct_close_wait;
            tcp_ct_last_ack;
            tcp_ct_time_wait;
            tcp_ct_close;
            tcp_ct_syn_sent2;
            tcp_ct_ignore;
        }

        const default_action = drop;
    }

    action udp_ct_unreplied() {
        // Update Connection state
        udpCtCurrentStateReg.write(meta.connHashCode, UDP_CT_UNREPLIED);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)UDP_CT_TIMEOUT_UNREPLIED;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Enable Connection Autolearning Flag
        meta.doAddOnMiss = true;

        // Updating direction register
        udpCtDirReg.write(meta.hashCode, CT_DIR_ORIGINAL);
	}

	action udp_ct_replied() {
        // Update Connection state
        udpCtCurrentStateReg.write(meta.connHashCode, UDP_CT_REPLIED);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)UDP_CT_TIMEOUT_REPLIED;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Enable Connection Autolearning Flag
        meta.doAddOnMiss = false;
	}

	action udp_ct_ignore() {
	    meta.updateExpireTimeoutProfile = false;
	    meta.restartExpireTimer = false;
	    meta.doAddOnMiss = false;
	}
    action icmp_ct_unreplied() {
        // Update Connection state
        icmpCtCurrentStateReg.write(meta.connHashCode, ICMP_CT_UNREPLIED);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)ICMP_CT_TIMEOUT_UNREPLIED;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Enable Connection Autolearning Flag
        meta.doAddOnMiss = true;

        // Updating direction register
        icmpCtDirReg.write(meta.hashCode, CT_DIR_ORIGINAL);
	}

	action icmp_ct_replied() {
        // Update Connection state
        icmpCtCurrentStateReg.write(meta.connHashCode, ICMP_CT_REPLIED);

        // Update timer profile
        meta.updateExpireTimeoutProfile = true;
        meta.timeoutProfileId = (ExpireTimeProfileId_t)ICMP_CT_TIMEOUT_REPLIED;

        // Restart Expire Timer
        meta.restartExpireTimer = true;

        // Enable Connection Autolearning Flag
        meta.doAddOnMiss = false;
	}
	action icmp_ct_ignore() {
            meta.updateExpireTimeoutProfile = false;
            meta.restartExpireTimer = false;
            meta.doAddOnMiss = false;
        }

    table udp_ct_state_table {
         key = {
            meta.dir : exact;
            meta.currentState : exact;
        }

        actions = {
            drop;
            udp_ct_unreplied;
            udp_ct_replied;
            udp_ct_ignore;
        }

        const default_action = drop;
    }

    table icmp_ct_state_table {
         key = {
            meta.dir : exact;
            meta.currentState : exact;
        }

        actions = {
            drop;
            icmp_ct_unreplied;
            icmp_ct_replied;
            icmp_ct_ignore;
        }

        const default_action = drop;
    }

    action tcp_ct_table_hit () {
        if (meta.restartExpireTimer) {
            if (meta.updateExpireTimeoutProfile) {
                set_entry_expire_time(meta.timeoutProfileId);
            } else {
                restart_expire_timer();
            }
        }

        meta.acceptPacket = true;
    }

    action tcp_ct_table_miss() {
        if (meta.doAddOnMiss) {
            add_succeeded =
                add_entry(action_name = "tcp_ct_table_hit",
                          action_params = (tcp_ct_table_hit_params_t)
                                          {},
                          expire_time_profile_id = meta.timeoutProfileId);

            meta.acceptPacket = true;
        } else {
            drop_packet();
            meta.acceptPacket = false;
        }
    }

    table tcp_ct_table {
        key = {
            hdr.ipv4.protocol : exact;
            SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr):
                exact @name("ipv4_addr_0");
            SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr):
                exact @name("ipv4_addr_1");
            SelectByDirection(istd.direction, hdr.tcp.srcPort, hdr.tcp.dstPort):
                exact @name("tcp_port_0");
            SelectByDirection(istd.direction, hdr.tcp.dstPort, hdr.tcp.srcPort):
                exact @name("tcp_port_1");
        }
        actions = {
            @tableonly   tcp_ct_table_hit;
            @defaultonly tcp_ct_table_miss;
        }

        add_on_miss = true;

        default_idle_timeout_for_data_plane_added_entries = 1;

        idle_timeout_with_auto_delete = true;
        const default_action = tcp_ct_table_miss;
    }

    action udp_ct_table_hit () {
        if (meta.restartExpireTimer) {
            if (meta.updateExpireTimeoutProfile) {
                set_entry_expire_time(meta.timeoutProfileId);
            } else {
                restart_expire_timer();
            }
        }

        meta.acceptPacket = true;
    }

    action udp_ct_table_miss() {
        if (meta.doAddOnMiss) {
            add_succeeded =
                add_entry(action_name = "udp_ct_table_hit",
                          action_params = (udp_ct_table_hit_params_t)
                                          {},
                          expire_time_profile_id = meta.timeoutProfileId);

            meta.acceptPacket = true;
        } else {
            drop_packet();
            meta.acceptPacket = false;
        }
    }

    table udp_ct_table {
        key = {
            hdr.ipv4.protocol : exact;
            SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr):
                exact @name("ipv4_addr_0");
            SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr):
                exact @name("ipv4_addr_1");
            SelectByDirection(istd.direction, hdr.udp.srcPort, hdr.udp.dstPort):
                exact @name("port_0");
            SelectByDirection(istd.direction, hdr.udp.dstPort, hdr.udp.srcPort):
                exact @name("port_1");
        }
        actions = {
            @tableonly   udp_ct_table_hit;
            @defaultonly udp_ct_table_miss;
        }

        add_on_miss = true;

        default_idle_timeout_for_data_plane_added_entries = 1;

        idle_timeout_with_auto_delete = true;
        const default_action = udp_ct_table_miss;
    }

    action send(PortId_t port) {
        send_to_port(port);
    }

    action icmp_ct_table_hit () {
        if (meta.restartExpireTimer) {
            if (meta.updateExpireTimeoutProfile) {
                set_entry_expire_time(meta.timeoutProfileId);
            } else {
                restart_expire_timer();
            }
        }

        meta.acceptPacket = true;
    }

    action icmp_ct_table_miss() {
        if (meta.doAddOnMiss) {
            add_succeeded =
                add_entry(action_name = "icmp_ct_table_hit",
                          action_params = (icmp_ct_table_hit_params_t)
                                          {},
                          expire_time_profile_id = meta.timeoutProfileId);

            meta.acceptPacket = true;
        } else {
            drop_packet();
            meta.acceptPacket = false;
        }
    }

    table icmp_ct_table {
        key = {
            hdr.ipv4.protocol : exact;
            SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr):
                exact @name("ipv4_addr_0");
            SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr):
                exact @name("ipv4_addr_1");
            hdr.icmp.identifier : exact;
        }

        actions = {
            @tableonly   icmp_ct_table_hit;
            @defaultonly icmp_ct_table_miss;
        }

        add_on_miss = true;

        default_idle_timeout_for_data_plane_added_entries = 1;

        idle_timeout_with_auto_delete = true;
        const default_action = icmp_ct_table_miss;
    }

    table ip_packet_fwd {
        key = {
            hdr.ipv4.dstAddr : lpm;
        }

        actions = {
            drop;
            send;
        }

        const default_action = drop;

        size = IP_PACKET_FWD_MAX_ENTRIES;
    }

    apply {
        // Initial values
        meta.doAddOnMiss = false;
        meta.restartExpireTimer = false;
        meta.updateExpireTimeoutProfile = false;

        // By default reject the packet
        meta.acceptPacket = false;

        if (hdr.tcp.isValid()) {
            // Generate 5-Tuple Hash of received flow and reverse flow
            meta.hashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    hdr.ipv4.srcAddr,
                    hdr.ipv4.dstAddr,
                    hdr.tcp.srcPort,
                    hdr.tcp.dstPort} );

            meta.replyHashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    hdr.ipv4.dstAddr,
                    hdr.ipv4.srcAddr,
                    hdr.tcp.dstPort,
                    hdr.tcp.srcPort} );

            meta.hashParams.ipAddr1 = SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr);
            meta.hashParams.ipAddr2 = SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr);
            meta.hashParams.protocol = hdr.ipv4.protocol;
            meta.hashParams.port1 = SelectByDirection(istd.direction, hdr.tcp.srcPort, hdr.tcp.dstPort);
            meta.hashParams.port2 = SelectByDirection(istd.direction, hdr.tcp.dstPort, hdr.tcp.srcPort);

            meta.connHashCode = tupleHash.get_hash( {meta.hashParams.ipAddr1, meta.hashParams.ipAddr2, meta.hashParams.protocol,
                                                    meta.hashParams.port1, meta.hashParams.port2});

#ifdef COMPILIER_HASH_FIX
            // Generate 5-Tuple Hash for connection
            meta.connHashCode = tupleHash.get_hash( {
                    SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr),
                    SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr),
                    hdr.ipv4.protocol,
                    SelectByDirection(istd.direction, hdr.tcp.srcPort, hdr.tcp.dstPort),
                    SelectByDirection(istd.direction, hdr.tcp.dstPort, hdr.tcp.srcPort)} );
#endif

            // Get the direction of the connection
            meta.dir = getTcpDirection(meta.hashCode, meta.replyHashCode);

             // Get TCP flags set in packet
            meta.tcpFlagsSet = getTcpFlagSetInfo(hdr.tcp.flags);

            // Read current state of the connection
            meta.currentState = tcpCtCurrentStateReg.read(meta.connHashCode);

            // Run protocol state machine
            tcp_ct_state_table.apply();

            // Run packet forwarding classification
            tcp_ct_table.apply();
        }

        if (hdr.udp.isValid()) {
            // Generate 5-Tuple Hash of received flow and reverse flow
            meta.hashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    hdr.ipv4.srcAddr,
                    hdr.ipv4.dstAddr,
                    hdr.udp.srcPort,
                    hdr.udp.dstPort} );

            meta.replyHashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    hdr.ipv4.dstAddr,
                    hdr.ipv4.srcAddr,
                    hdr.udp.dstPort,
                    hdr.udp.srcPort} );

            meta.hashParams.ipAddr1 = SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr);
            meta.hashParams.ipAddr2 = SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr);
            meta.hashParams.protocol = hdr.ipv4.protocol;
            meta.hashParams.port1 = SelectByDirection(istd.direction, hdr.udp.srcPort, hdr.udp.dstPort);
            meta.hashParams.port2 = SelectByDirection(istd.direction, hdr.udp.dstPort, hdr.udp.srcPort);

            meta.connHashCode = tupleHash.get_hash( {meta.hashParams.ipAddr1, meta.hashParams.ipAddr2, meta.hashParams.protocol,
                                                    meta.hashParams.port1, meta.hashParams.port2});
#ifdef COMPILIER_HASH_FIX
            // Generate 5-Tuple Hash for connection
            meta.connHashCode = tupleHash.get_hash( {
                    SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr),
                    SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr),
                    hdr.ipv4.protocol,
                    SelectByDirection(istd.direction, hdr.udp.srcPort, hdr.udp.dstPort),
                    SelectByDirection(istd.direction, hdr.udp.dstPort, hdr.udp.srcPort)} );
#endif

            // Get the direction of the connection
            meta.dir = getUdpDirection(meta.hashCode, meta.replyHashCode);

            // Read current state of the connection
            meta.currentState = udpCtCurrentStateReg.read(meta.connHashCode);

            udp_ct_state_table.apply();

            udp_ct_table.apply();
        }

        if (hdr.icmp.isValid()) {
            // Generate 5-Tuple Hash of received flow and reverse flow
            meta.hashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    hdr.ipv4.srcAddr,
                    hdr.ipv4.dstAddr,
                    hdr.icmp.identifier} );

            meta.replyHashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    hdr.ipv4.dstAddr,
                    hdr.ipv4.srcAddr,
                    hdr.icmp.identifier} );

            meta.hashParams.ipAddr1 = SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr);
            meta.hashParams.ipAddr2 = SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr);
            meta.hashParams.protocol = hdr.ipv4.protocol;
            meta.hashParams.identifier = hdr.icmp.identifier;

            meta.connHashCode = tupleHash.get_hash( {meta.hashParams.protocol, meta.hashParams.ipAddr1, meta.hashParams.ipAddr2,
                                                    meta.hashParams.identifier});

#ifdef COMPILIER_HASH_FIX
			// Generate 5-Tuple Hash for connection
            meta.connHashCode = tupleHash.get_hash( {
                    hdr.ipv4.protocol,
                    SelectByDirection(istd.direction, hdr.ipv4.srcAddr, hdr.ipv4.dstAddr),
                    SelectByDirection(istd.direction, hdr.ipv4.dstAddr, hdr.ipv4.srcAddr),
                    hdr.icmp.identifier} );
#endif
            // Get the direction of the connection
            meta.dir = getIcmpDirection(meta.hashCode, meta.replyHashCode);

            // Read current state of the connection
            meta.currentState = icmpCtCurrentStateReg.read(meta.connHashCode);

            icmp_ct_state_table.apply();

            icmp_ct_table.apply();
        }

        // Forward packet if packet is accepted by CT TCP state machine
        if (meta.acceptPacket) {
            ip_packet_fwd.apply();
        }
    }
}

control MainDeparserImpl(
    packet_out pkt,
    in    headers_t hdr,
    in    main_metadata_t meta,
    in    pna_main_output_metadata_t ostd)
{
    apply {
        pkt.emit(hdr.ethernet);
        pkt.emit(hdr.ipv4);
        pkt.emit(hdr.tcp);
        pkt.emit(hdr.udp);
        pkt.emit(hdr.icmp);
    }
}

PNA_NIC(MainParserImpl(), PreControlImpl(), MainControlImpl(), MainDeparserImpl()) main;
