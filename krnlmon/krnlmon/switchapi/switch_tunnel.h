/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2023 Intel Corporation.
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

#ifndef __SWITCH_TUNNEL_H__
#define __SWITCH_TUNNEL_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_interface.h"
#include "switch_types_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup Tunnel Tunnel API
 *  API functions create tunnel interfaces
 *  @{
 */  // begin of Tunnel API

/** Maximum srv6 segments supported */
#define SWITCH_SRV6_SEGMENT_MAX 3

/** Srv6 Segment ID Length */
#define SWITCH_SRV6_SID_LENGTH 16

/** tunnel handle wrappers */
#define switch_tunnel_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_TUNNEL, \
                       sizeof(switch_tunnel_info_t))

#define switch_tunnel_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL, _handle)

#define switch_tunnel_get(_device, _handle, _info)                 \
  ({                                                               \
    switch_tunnel_info_t* _tmp_tunnel_info = NULL;                 \
    (void)(_tmp_tunnel_info == *_info);                            \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_TUNNEL, _handle, \
                      (void**)_info);                              \
  })

/** tunnel term handle wrappers */
#define switch_tunnel_term_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_TUNNEL_TERM, \
                       sizeof(switch_tunnel_term_info_t))

#define switch_tunnel_term_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL_TERM, _handle)

#define switch_tunnel_term_get(_device, _handle, _info)                 \
  ({                                                                    \
    switch_tunnel_term_info_t* _tmp_tunnel_term_info = NULL;            \
    (void)(_tmp_tunnel_term_info == *_info);                            \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_TUNNEL_TERM, _handle, \
                      (void**)_info);                                   \
  })

/** vxlan default port */
#define SWITCH_VXLAN_DEFAULT_PORT 4789

#define SWITCH_TUNNEL_VNI_SIZE 24

#define SWITCH_TUNNEL_VNI_VALID(_vni) \
  (_vni < ((1 << SWITCH_TUNNEL_VNI_SIZE) - 1))

//#define SWITCH_TUNNEL_HANDLE_SIZE 16384

// Table name macros
#define VXLAN_ENCAP_MOD_TABLE "vxlan_encap_mod_table"
#define IPV4_TUNNEL_TERM_TABLE "ipv4_tunnel_term_table"

/** tunnel device context */
typedef struct switch_tunnel_context_s {
  /** source ip rewrite hashtable */
  switch_hashtable_t src_ip_hashtable;

  /** destionation ip rewrite haashtable */
  switch_hashtable_t dst_ip_hashtable;

  /**
   * ingress tunnel vni hashtable
   * tunnel type and vni uniquely identifies a
   * bridge domain
   */
  switch_hashtable_t ingress_tunnel_vni_hashtable;

  /**
   * egress tunnel vni hashtable
   * tunnel type and bd uniquely identifies a vni
   */
  switch_hashtable_t egress_tunnel_vni_hashtable;

  /** source vtep hashtable */
  switch_hashtable_t src_vtep_hashtable;

  /** destination vtep hashtable */
  switch_hashtable_t dst_vtep_hashtable;

  /** source ip rewrite index allocator */
  switch_id_allocator_t* src_ip_id_allocator;

  /** destination ip rewrite index allocator */
  switch_id_allocator_t* dst_ip_id_allocator;

  /** tunnel vni allocator */
  switch_id_allocator_t* tunnel_vni_allocator;

  /** mpls transit array indexed by label */
  switch_array_t* mpls_transit_array;

  /** data structure to store tunnel destination IPs */
  Pvoid_t PJLarr_tunnel_dest;

  /** map to avoid duplicate adds to tunnel_mgid table */
  switch_array_t tunnel_mgid_map;

} switch_tunnel_context_t;

/* Tunnel types */
typedef enum switch_tunnel_type_s {

  SWITCH_TUNNEL_TYPE_NONE,

  /** QinQ tunnel */
  SWITCH_TUNNEL_TYPE_QINQ,

  /** Vxlan tunnel */
  SWITCH_TUNNEL_TYPE_VXLAN,

  /** Gre Tunnel*/
  SWITCH_TUNNEL_TYPE_GRE,

  /** NvGre Tunnel */
  SWITCH_TUNNEL_TYPE_NVGRE,

  /** Geneve Tunnel */
  SWITCH_TUNNEL_TYPE_GENEVE,

  /** Erspan T3 Tunnel */
  SWITCH_TUNNEL_TYPE_ERSPAN_T3,

  /** IP in IP Tunnel */
  SWITCH_TUNNEL_TYPE_IPIP,

  /** Segment Routing Tunnel */
  SWITCH_TUNNEL_TYPE_SRV6,

  /** Segment Routing L3VPN Tunnel */
  SWITCH_TUNNEL_TYPE_SRV6_L3VPN,

  /** Segment Routing L2VPN Tunnel */
  SWITCH_TUNNEL_TYPE_SRV6_L2VPN,

  /** Telemetry Report */
  SWITCH_TUNNEL_TYPE_DTEL_REPORT,

  SWITCH_TUNNEL_TYPE_MAX

} switch_tunnel_type_t;

typedef enum switch_tunnel_map_type_s {

  /** tunnel map type vni to vlan handle */
  SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE,

  /** tunnel map type vlan handle to vni */
  SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI,

  /** tunnel map type vni to ln handle */
  SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,

  /** tunnel map type ln handle to vni */
  SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,

  /** tunnel map type vrf handle to vni */
  SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,

  /** tunnel map type vni to vrf handle */
  SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE,

} switch_tunnel_map_type_t;

typedef enum switch_tunnel_term_entry_type_s {

  /** point to point */
  SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,

  /** point to multipoint */
  SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2MP

} switch_tunnel_term_entry_type_t;

typedef enum switch_tunnel_ip_addr_type_s {
  SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4 = 0x0,
  SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6 = 0x1
} switch_tunnel_ip_addr_type_t;

/** Tunnel Entry Type */
typedef enum switch_tunnel_entry_type_s {
  SWITCH_TUNNEL_ENTRY_TYPE_UNICAST = 0x0,
  SWITCH_TUNNEL_ENTRY_TYPE_MULTICAST = 0x1
} switch_tunnel_entry_type_t;

/** Segment routing Segments */
typedef struct switch_srv6_segment_s {
  switch_uint8_t sid[SWITCH_SRV6_SID_LENGTH];
} switch_srv6_segment_t;

/** QinQ encapsulation format */
typedef struct switch_qinq_s {
  /** outer vlan tag */
  switch_vlan_t outer;

  /** inner vlan tag */
  switch_vlan_t inner;

} switch_qinq_t;

/** Tunnel information */
typedef struct switch_api_tunnel_info_s {
  /** tunnel type */
  switch_tunnel_type_t tunnel_type;

  /** tunnel entry type */
  switch_tunnel_entry_type_t entry_type;

  /** tunnel ip type */
  switch_tunnel_ip_addr_type_t ip_type;

  /** tunnel direction */
  switch_direction_t direction;

  /** source ip address */
  switch_ip_addr_t src_ip;

  /** source ip address */
  switch_ip_addr_t dst_ip;

  /** time to live */
  switch_uint8_t ttl;

  /** Gre key */
  switch_uint32_t gre_key;

  /** ingress tunnel mapper */
  switch_handle_t decap_mapper_handle;

  /** egress tunnel mapper */
  switch_handle_t encap_mapper_handle;

  /** underlay rif handle */
  switch_handle_t underlay_rif_handle;

  /** overlay rif handle */
  switch_handle_t overlay_rif_handle;

  /** span id */
  switch_uint16_t erspan_span_id;

  /** IPv6 sr segments */
  switch_srv6_segment_t srv6_seg_list[SWITCH_SRV6_SEGMENT_MAX];

  /** IPv6 sr first segment */
  switch_uint8_t srv6_first_seg;

  /** UDP port number **/
  switch_uint16_t udp_port;

} switch_api_tunnel_info_t;

typedef struct switch_tunnel_info_s {
  /** array of tunnel term objects */
  switch_array_t tunnel_term_array;

  /** api tunnel info */
  switch_api_tunnel_info_t api_tunnel_info;

  /** underlay vrf handle */
  switch_handle_t underlay_vrf_handle;

  /** tunnel type */
  switch_tunnel_type_t tunnel_type;

  /** tunnel source ip rewrite index */
  switch_id_t sip_index;

  /** tunnel vni for IPinIP tunnel */
  switch_vni_t tunnel_vni;

  /** interface handle */
  switch_handle_t intf_handle;

} switch_tunnel_info_t;

typedef struct switch_api_tunnel_term_info_s {
  /** tunnel handle */
  switch_handle_t tunnel_handle;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** tunnel type */
  switch_tunnel_type_t tunnel_type;

  /** tunnel term entry type */
  switch_tunnel_term_entry_type_t term_entry_type;

  /** source ip address */
  switch_ip_addr_t src_ip;

  /** destination ip address */
  switch_ip_addr_t dst_ip;

  /** to Hold tunnel ID */
  switch_uint32_t tunnel_id;

} switch_api_tunnel_term_info_t;

/** tunnel termination object */
typedef struct switch_tunnel_term_info_s {
  /** api tunnel term info */
  switch_api_tunnel_term_info_t api_tunnel_term_info;

} switch_tunnel_term_info_t;

/** tunnel mapper */
typedef struct switch_api_tunnel_mapper_s {
  /** tunnel mapper entry type */
  switch_tunnel_map_type_t tunnel_map_type;

} switch_api_tunnel_mapper_t;

/** tunnel mapper entry */
typedef struct switch_api_tunnel_mapper_entry_s {
  /** tunnel mapper entry type */
  switch_tunnel_map_type_t tunnel_map_type;

  /** tunnel mapper handle */
  switch_handle_t tunnel_mapper_handle;

  /** tunne vni */
  switch_vni_t tunnel_vni;

  /** logical network handle */
  switch_handle_t ln_handle;

  /** vlan handle */
  switch_handle_t vlan_handle;

  /** vrf handle */
  switch_handle_t vrf_handle;

} switch_api_tunnel_mapper_entry_t;

switch_status_t switch_api_tunnel_mapper_create(
    const switch_device_t device,
    const switch_api_tunnel_mapper_t* tunnel_mapper,
    switch_handle_t* tunnel_mapper_handle);

switch_status_t switch_api_tunnel_mapper_delete(
    const switch_device_t device, const switch_handle_t tunnel_mapper_handle);

switch_status_t switch_api_tunnel_mapper_entries_get(
    const switch_device_t device, const switch_handle_t tunnel_mapper_handle,
    switch_uint16_t* num_entries, switch_handle_t* tunnel_mapper_entry_handles);

switch_status_t switch_api_tunnel_mapper_entry_create(
    const switch_device_t device,
    const switch_api_tunnel_mapper_entry_t* tunnel_mapper_entry,
    switch_handle_t* tunnel_mapper_entry_handle);

switch_status_t switch_api_tunnel_mapper_entry_delete(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_entry_handle);

switch_status_t switch_api_tunnel_mapper_entry_get(
    const switch_device_t device,
    const switch_handle_t tunnel_mapper_entry_handle,
    switch_api_tunnel_mapper_entry_t* tunnel_mapper_entry);

switch_status_t switch_api_tunnel_create(
    const switch_device_t device, const switch_api_tunnel_info_t* tunnel_info,
    switch_handle_t* tunnel_handle);

switch_status_t switch_api_tunnel_delete(const switch_handle_t tunnel_handle);

switch_status_t switch_api_tunnel_info_get(
    const switch_device_t device, const switch_handle_t tunnel_handle,
    switch_api_tunnel_info_t* api_tunnel_info);

switch_status_t switch_api_tunnel_interface_get(
    const switch_device_t device, const switch_handle_t tunnel_handle,
    switch_handle_t* intf_handle);

switch_status_t switch_api_tunnel_term_entries_get(
    const switch_device_t device, const switch_handle_t tunnel_handle,
    switch_uint16_t* num_entries, switch_handle_t* tunnel_term_handles);

switch_status_t switch_api_tunnel_term_create(
    const switch_device_t device,
    const switch_api_tunnel_term_info_t* tunnel_term_info,
    switch_handle_t* tunnel_handle);

switch_status_t switch_api_tunnel_term_delete(
    const switch_handle_t tunnel_term_handle);

switch_status_t switch_api_tunnel_term_get(
    const switch_device_t device, const switch_handle_t tunnel_term_handle,
    switch_api_tunnel_term_info_t* api_term_info);

switch_status_t switch_tunnel_init(switch_device_t device);

switch_status_t switch_tunnel_free(switch_device_t device);

switch_status_t switch_tunnel_default_entries_add(switch_device_t device);

switch_status_t switch_tunnel_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_TUNNEL_H__ */
