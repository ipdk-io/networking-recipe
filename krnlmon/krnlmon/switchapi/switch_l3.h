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

#ifndef __SWITCH_L3_H__
#define __SWITCH_L3_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_l3_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file switch_l2.h
 * brief This file contains API to program ipv4/ipv6 fib and lpm tables. The
 *       basic routing APIs are controlled by the manipulation of these routing
 *       tables – fib and lpm.
 */

/** @defgroup L3 L3 Routing API
 *  API functions to configure Routing tables
 *  @{
 */

/** route type */
typedef enum switch_route_type_s {
  SWITCH_ROUTE_TYPE_HOST = 0,
  SWITCH_ROUTE_TYPE_MYIP = 1
} switch_route_type_t;

/** route entry */
typedef struct switch_api_route_entry_s {
  /** router interface handle */
  switch_handle_t rif_handle;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** ip address */
  switch_ip_addr_t ip_address;

  /** nexthop handle */
  switch_handle_t nhop_handle;

  /** nexthop member handle */
  switch_handle_t nhop_member_handle;

  /**ecmp group handle */
  switch_handle_t ecmp_group_id;

  /** route type */
  switch_route_type_t route_type;

  /** route installed by neighbor */
  bool neighbor_installed;

  /** L3 address label */
  switch_uint8_t label;

  switch_handle_t route_handle;

} switch_api_route_entry_t;

/** stores route info and associated hardware handles */
typedef struct switch_route_info_s {
  /**
   * route entry programmed by the application and
   * acts as the key for route hashtable. This should
   * be the first entry in this struct for hashing
   */
  switch_route_entry_t route_entry;

  /** nexhop handle */
  switch_handle_t nhop_handle;

  /** route hashtable node */
  switch_hashnode_t node;

  /** route info */
  switch_api_route_entry_t api_route_info;

} switch_route_info_t;

/**
 Get the handle of the interface that route lookup returns for a host addr
 @param device device
 @param vrf virtual domain identifier
 @param ip_addr IP address (host or prefix)
 @param intf_handle pointer to return Interface handle
*/
switch_status_t switch_api_l3_route_nhop_get(switch_device_t device,
                                             switch_handle_t vrf_handle,
                                             switch_ip_addr_t* ip_addr,
                                             switch_handle_t* intf_handle);

/**
 * @brief route add - add an entry to host or lpm table to match based on
 *        the prefix length
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_add(
    switch_device_t device, switch_api_route_entry_t* api_route_entry);

/**
 * @brief route update - update an entry to host or lpm table to match based on
 *        the prefix length
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_route_update(
    switch_device_t device, switch_api_route_entry_t* api_route_entry);

/**
 * @brief route delete - delete an entry to host or lpm table to match based on
 *        the prefix length
 *
 * @param[in] device device
 * @param[in] api_route_entry route entry
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_l3_delete_route(
    switch_device_t device, switch_api_route_entry_t* api_route_entry);
/**
 Lookup FIB table (host or LPM) for a given host address
 Return nexthop handle (single path or ECMP group)
 Return INVALID_HANDLE if lookup fails
 @param device device
 @param vrf virtual domain identifier
 @param ip_addr IP address
 @param nhop_handle pointer to return Nexthop  Handle
*/
switch_status_t switch_api_l3_route_lookup(
    switch_device_t device, switch_api_route_entry_t* api_route_entry,
    switch_handle_t* nhop_handle);

switch_status_t switch_api_rif_ipv4_unicast_enabled_set(
    switch_device_t device, switch_handle_t intf_handle, bool set);

switch_status_t switch_api_rif_ipv6_unicast_enabled_set(
    switch_device_t device, switch_handle_t intf_handle, bool set);

switch_status_t switch_api_l3_route_handle_lookup(
    const switch_device_t device,
    const switch_api_route_entry_t* api_route_entry,
    switch_handle_t* route_handle);

switch_status_t switch_api_route_table_size_get(switch_device_t device,
                                                switch_size_t* tbl_size);

switch_status_t switch_route_hashtable_insert(switch_device_t device,
                                              switch_handle_t route_handle);

switch_status_t switch_route_table_hash_lookup(
    switch_device_t device, switch_route_entry_t* route_entry,
    switch_handle_t* route_handle);

switch_status_t switch_route_hashtable_remove(switch_device_t device,
                                              switch_handle_t route_handle);
/** @} */  // end of L3 API

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_L3_H__ */
