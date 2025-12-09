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

#ifndef __SWITCH_NHOP_H__
#define __SWITCH_NHOP_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_types_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** nexthop id type */
typedef enum switch_nhop_id_type_s {
  SWITCH_NHOP_ID_TYPE_NONE = 0x0,
  SWITCH_NHOP_ID_TYPE_ONE_PATH = 0x1,
  SWITCH_NHOP_ID_TYPE_ECMP = 0x2,
  SWITCH_NHOP_ID_TYPE_WCMP = 0x3
} switch_nhop_id_type_t;

/** nexthop type */
typedef enum switch_nhop_type_s {
  SWITCH_NHOP_TYPE_NONE = 0x0,
  SWITCH_NHOP_TYPE_IP = 0x1,
  SWITCH_NHOP_TYPE_TUNNEL = 0x2,
  SWITCH_NHOP_TYPE_MPLS = 0x3,
  SWITCH_NHOP_TYPE_GLEAN = 0x4,
  SWITCH_NHOP_TYPE_DROP = 0x5
} switch_nhop_type_t;

typedef enum switch_nhop_tunnel_type_s {
  SWITCH_NHOP_TUNNEL_TYPE_NONE = 0x0,
  SWITCH_NHOP_TUNNEL_TYPE_VLAN = 0x1,
  SWITCH_NHOP_TUNNEL_TYPE_LN = 0x2,
  SWITCH_NHOP_TUNNEL_TYPE_VRF = 0x3
} switch_nhop_tunnel_type_t;

typedef enum switch_nhop_attribute_s {
  SWITCH_NHOP_ATTR_NHOP_TYPE = 1 << 0,
} switch_nhop_attribute_t;

typedef struct PACKED switch_nhop_key_s {
  /** handle */
  switch_handle_t handle;

  /** ip address */
  switch_ip_addr_t ip_addr;

  /** vxlan tunnel vni */
  switch_vni_t vni;

} switch_nhop_key_t;

/** Nexthop Key */
typedef struct switch_api_nhop_info_s {
  /** nhop type */
  switch_nhop_type_t nhop_type;

  /** nhop tunnel type */
  switch_nhop_tunnel_type_t nhop_tunnel_type;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** network handle - vlan/ln */
  switch_handle_t network_handle;

  /** rif handle */
  switch_handle_t rif_handle;

  /** tunnel handle */
  switch_handle_t tunnel_handle;

  /** mpls handle */
  switch_handle_t mpls_handle;

  /** interface handle */
  switch_handle_t intf_handle;

  /** mpls label stack handle */
  switch_handle_t label_stack_handle;

  /** nhop ip address */
  switch_ip_addr_t ip_addr;

  /** tunnel vni */
  switch_vni_t tunnel_vni;

  /** tunnel dmac address */
  switch_mac_addr_t mac_addr;

} switch_api_nhop_info_t;

/** nhop_group info struct */
typedef struct switch_nhop_group_info_s {
  switch_handle_t nhop_group_handle;

  switch_list_t members;

  switch_nhop_id_type_t id_type;

  bool first_insert_complete;

  bool pd_nhop_group_deleted;

} switch_nhop_group_info_t;

typedef struct switch_nhop_member_s {
  /** nhop member handle */
  switch_handle_t member_handle;

  /** nhop group handle */
  switch_handle_t nhop_group_handle;

  /** list node */
  switch_node_t node;

  /** nhop member nhop handle */
  switch_handle_t nhop_handle;

  /** member active bit */
  bool active;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_nhop_member_t;

/**
 * initialize nhop member handle
 */
switch_status_t switch_nhop_member_handle_init(switch_device_t device);

/**
 Create a Nexthop
 @param device - device to program the nexthop
 @param nhop_key- Interface to be associated with the nexthop and nexthop ip
*/
switch_status_t switch_api_nhop_create(
    const switch_device_t device, const switch_api_nhop_info_t* api_nhop_info,
    switch_handle_t* nhop_handle);

/**
 Update a Nexthop
 @param device - device to program the nexthop
 @param handle - handle of the next hop to update
 @param nhop_key - nhop key with new info
*/
switch_status_t switch_api_nhop_update(switch_device_t device,
                                       switch_handle_t nhop_handle,
                                       switch_uint64_t flags,
                                       switch_api_nhop_info_t* api_nhop_info);

/**
 Get attributes of a Nexthop
 @param device - device to program the nexthop
 @param handle - handle of the next hop to get
 @param [out]nhop_key - pointer to the attribute obj
*/
switch_status_t switch_api_nhop_get(switch_device_t device,
                                    switch_handle_t nhop_handle,
                                    switch_api_nhop_info_t* api_nhop_info);

/**
 Delete a Nexthop
 @param device device on which to create nhop group
 @param nhop_handle - Handle that identifies nexthop uniquely
*/
switch_status_t switch_api_nhop_delete(const switch_device_t device,
                                       const switch_handle_t nhop_handle);

/**
 Create a nhop Group
 @param device - device to create the nhop_group group
 @param nhop_handle - Handle that identifies nexthop uniquely
*/
switch_status_t switch_api_create_nhop_group(
    const switch_device_t device, switch_handle_t* nhop_group_handle);

/**
 Delete a nhop Group
 @param nhop_group_handle - Handle that identifies nhop group uniquely
*/
switch_status_t switch_api_delete_nhop_group(
    const switch_device_t device, const switch_handle_t nhop_group_handle);

/**
 Add nexthop member to nhop group
 @param device - device to program the nexthop
 @param nhop_group_handle - handle that identifies nhop group uniquely
 @param nhop_count - number of nexthops
 @param nhop_handle_list - List of nexthops to be added to the nhop Group
*/
switch_status_t switch_api_add_nhop_member(
    const switch_device_t device, const switch_handle_t nhop_group_handle,
    const switch_uint32_t num_nhops, const switch_handle_t* nhop_handles,
    switch_handle_t* member_handle);

/**
 Delete nexthop member from nhop group
 @param device - device to program the nexthop
 @param nhop_group_handle - handle that identifies nhop group uniquely
 @param nhop_count - number of nexthops
 @param nhop_handle_list - List of nexthops to be added to the nhop Group
*/
switch_status_t switch_api_delete_nhop_member(
    const switch_device_t device, const switch_handle_t nhop_group_handle,
    const switch_uint32_t num_nhops, const switch_handle_t* nhop_handles);

/*
 Return nexthop handle from (intf_handle, ip address)
 @param nhop_key- Interface to be associated with the nexthop and nexthop ip
 */
switch_status_t switch_api_nhop_handle_get(const switch_device_t device,
                                           const switch_nhop_key_t* nhop_key,
                                           switch_handle_t* nhop_handle);

/*
 Get neighbor handle from nexthop handle
 @param nhop_handle nexthop handle
 */
switch_status_t switch_api_neighbor_handle_get(
    const switch_device_t device, const switch_handle_t nhop_handle,
    switch_handle_t* neighbor_handle);

/*
 Get to know whether nhop is single path or ecmp
 @param nhop_handle nexthop handle
*/
switch_status_t switch_api_nhop_id_type_get(const switch_device_t device,
                                            const switch_handle_t nhop_handle,
                                            switch_nhop_id_type_t* nhop_type);

switch_status_t switch_api_delete_nhop_members(
    switch_device_t device, switch_handle_t nhop_group_handle);

switch_status_t switch_api_nhop_members_get(
    const switch_device_t device, const switch_handle_t nhop_group_handle,
    switch_uint16_t* num_nhops, switch_handle_t** nhop_handles);

switch_status_t switch_api_nhop_handle_dump(const switch_device_t device,
                                            const switch_handle_t nhop_handle,
                                            const void* cli_ctx);
switch_status_t switch_api_nhop_table_size_get(switch_device_t device,
                                               switch_size_t* tbl_size);

switch_status_t switch_api_nhop_group_get_by_nhop_member(
    const switch_device_t device, const switch_handle_t nhop_member_handle,
    switch_handle_t* nhop_group_handle, switch_handle_t* nhop_handle);

switch_status_t switch_api_rif_nhop_get(switch_device_t device,
                                        switch_handle_t vrf_handle,
                                        switch_handle_t* nhop_handle);

/**
 * function to initialize nhop key
 * */
switch_status_t switch_nhop_hash_key_init(void* args, switch_uint8_t* key,
                                          switch_uint32_t* len);

/**
 * function to compare 2 nhop keys
 * */
switch_int32_t switch_nhop_hash_compare(const void* key1, const void* key2);

switch_status_t nhop_member_get_from_nhop(
    const switch_device_t device, const switch_handle_t nhop_group_handle,
    const switch_handle_t nhop_handle, switch_nhop_member_t** nhop_member);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_NHOP_H__ */
