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

#ifndef __SWITCH_NEIGHBOR_H__
#define __SWITCH_NEIGHBOR_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_neighbor_int.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum switch_neighbor_type_s {
  SWITCH_NEIGHBOR_TYPE_IP = 0x0,
  SWITCH_NEIGHBOR_TYPE_NHOP = 0x1
} switch_neighbor_type_t;

/** Neighbor identifier */
typedef struct switch_api_neighbor_info_s {
  /** neighbor type */
  switch_neighbor_type_t neighbor_type;

  /** nhop handle */
  switch_handle_t nhop_handle;

  /** rif handle */
  switch_handle_t rif_handle;

  /** ip address */
  switch_ip_addr_t ip_addr;

  /** destination mac address */
  switch_mac_addr_t mac_addr;

} switch_api_neighbor_info_t;

/** neighbor struct */
typedef struct switch_neighbor_info_s {
  /** dmac list node */
  switch_node_t node;

  /** api neighbor info */
  switch_api_neighbor_info_t api_neighbor_info;

  /** bridge domain handle */
  switch_handle_t handle;

  /** neighbor mac address */
  switch_mac_addr_t mac;

  /** nhop handle */
  switch_handle_t nhop_handle;

  /** neighbor handle - self pointer */
  switch_handle_t neighbor_handle;

  /** rif type */
  switch_rif_type_t rif_type;

  switch_pd_routing_info_t switch_pd_routing_info;

} switch_neighbor_info_t;

switch_status_t switch_api_neighbor_attribute_get(
    const switch_device_t device, const switch_handle_t neighbor_handle,
    const switch_uint64_t neighbor_flags,
    switch_api_neighbor_info_t* api_neighbor_info);
/**
ARP entry add
@param device device
@param neighbor - ARP information used to set egress table
*/
switch_status_t switch_api_neighbor_create(
    switch_device_t device, switch_api_neighbor_info_t* api_neighbor,
    switch_handle_t* neighbor_handle);

/**
ARP entry delete
@param device device
@param neighbor_handle - handle of the arp entry
*/
switch_status_t switch_api_neighbor_delete(switch_device_t device,
                                           switch_handle_t neighbor_handle);

#endif
#ifdef __cplusplus
}
#endif /* __SWITCH_NEIGHBOR_H__ */
