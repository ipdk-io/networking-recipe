/*
 * Copyright (c) 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Defines the public interface that permits OvS to send p4runtime
 * messages to infrap4d.
 *
 * ---------------------------------------------------------------------
 * IMPORTANT:
 *
 * This file is moving from the ovs repository to the ovs-p4rt directory
 * in the networking-recipe repository.
 *
 * To maintain backward compatibility during the transition, there are
 * currently TWO copies of the file, one in each repository.
 *
 * If you make any changes, they must be made to BOTH copies.
 * ---------------------------------------------------------------------
 */

#ifndef OVSP4RT_OVS_P4RT_H_
#define OVSP4RT_OVS_P4RT_H_

#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------

/* When VSI ID is used as an action, we need to add an offset of 16 and
 * populate the action. */
#define VSI_ID_OFFSET 16

/* As the p4 program uses 8 bits for bridge ID, we are currently limited
 * to a maximum of 256 bridges (0-255). */
#define MAX_P4_BRIDGE_ID 255

/* Source ports for VxLAN should start from 2048. Ports 0 through 2047
 * are reserved for VSI/phy ports. */
#define P4_VXLAN_SOURCE_PORT_OFFSET 2048

//----------------------------------------------------------------------
// Data types
//----------------------------------------------------------------------

/* This is a replica of port_vlan_mode in ofproto.h */
enum p4_vlan_mode {
  P4_PORT_VLAN_ACCESS,
  P4_PORT_VLAN_TRUNK,
  P4_PORT_VLAN_NATIVE_TAGGED,
  P4_PORT_VLAN_NATIVE_UNTAGGED,
  P4_PORT_VLAN_DOT1Q_TUNNEL,
  P4_PORT_VLAN_UNSUPPORTED
};

enum ovs_tunnel_type {
  OVS_TUNNEL_UNKNOWN = 0,
  OVS_TUNNEL_VXLAN,
  OVS_TUNNEL_GENEVE
};

struct p4_ipaddr {
  uint8_t family;
  uint8_t prefix_len;
  union {
    struct in_addr v4addr;
    struct in6_addr v6addr;
  } ip;
};

struct port_vlan_info {
  enum p4_vlan_mode port_vlan_mode;
  int port_vlan;
};

struct tunnel_info {
  uint32_t ifindex;
  uint32_t port_id;
  uint32_t src_port;
  struct p4_ipaddr local_ip;
  struct p4_ipaddr remote_ip;
  uint16_t dst_port;
  uint16_t vni;
  struct port_vlan_info vlan_info;
  uint8_t bridge_id;
  uint8_t tunnel_type;
};

struct src_port_info {
  uint8_t bridge_id;
  uint16_t vlan_id;
  uint32_t src_port;
};

struct vlan_info {
  uint32_t vlan_id;
};

struct mac_learning_info {
  bool is_tunnel;
  bool is_vlan;
  uint8_t mac_addr[6];
  uint8_t bridge_id;
  uint32_t src_port;
  uint32_t rx_src_port;
  struct port_vlan_info vlan_info;
  union {
    struct tunnel_info tnl_info;
    struct vlan_info vln_info;
  };
};

struct ip_mac_map_info {
  uint8_t src_mac_addr[6];
  uint8_t dst_mac_addr[6];
  struct p4_ipaddr src_ip_addr;
  struct p4_ipaddr dst_ip_addr;
};

//----------------------------------------------------------------------
// Function prototypes
//----------------------------------------------------------------------

extern void ovsp4rt_config_fdb_entry(struct mac_learning_info learn_info,
                                     bool insert_entry, const char* grpc_addr);

extern void ovsp4rt_config_ip_mac_map_entry(struct ip_mac_map_info learn_info,
                                            bool insert_entry,
                                            const char* grpc_addr);

extern void ovsp4rt_config_rx_tunnel_src_entry(struct tunnel_info tunnel_info,
                                               bool insert_entry,
                                               const char* grpc_addr);

extern void ovsp4rt_config_src_port_entry(struct src_port_info vsi_sp,
                                          bool insert_entry,
                                          const char* grpc_addr);

extern void ovsp4rt_config_tunnel_src_port_entry(struct src_port_info tnl_sp,
                                                 bool insert_entry,
                                                 const char* grpc_addr);

extern void ovsp4rt_config_tunnel_entry(struct tunnel_info tunnel_info,
                                        bool insert_entry,
                                        const char* grpc_addr);

extern void ovsp4rt_config_vlan_entry(uint16_t vlan_id, bool insert_entry,
                                      const char* grpc_addr);

extern enum ovs_tunnel_type ovsp4rt_str_to_tunnel_type(const char* tnl_type);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // OVSP4RT_OVS_P4RT_H_
