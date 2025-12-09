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

#ifndef __SWITCH_L3_INT_H__
#define __SWITCH_L3_INT_H__

#include "switch_types_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_ROUTE_HASH_KEY_SIZE \
  sizeof(switch_handle_t) + sizeof(switch_ip_addr_t) + sizeof(bool)

#define SWITCH_ROUTE_HASH_SEED 0x123456

#define SWITCH_IP_TYPE_NONE 0
#define SWITCH_IP_TYPE_IPv4 1
#define SWITCH_IP_TYPE_IPv6 2

/** route handle wrappers */
#define switch_route_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_ROUTE, \
                       sizeof(switch_route_info_t))

#define switch_route_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_ROUTE, _handle)

#define switch_route_get(_device, _handle, _info)                 \
  ({                                                              \
    switch_route_info_t* _tmp_route_info = NULL;                  \
    (void)(_tmp_route_info == *_info);                            \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_ROUTE, _handle, \
                      (void**)_info);                             \
  })

/** stores route information */
typedef struct switch_route_entry_s {
  /** vrf handle to identify the vrf id */
  switch_handle_t vrf_handle;

  /** ip address */
  switch_ip_addr_t ip;

  /** neighbor installed */
  bool neighbor_installed;
} switch_route_entry_t;

/* l3 device context */
typedef struct switch_l3_context_s {
  /* route hashtable */
  switch_hashtable_t route_hashtable;

} switch_l3_context_t;

#define SWITCH_L3_IP_TYPE(ip_info) ip_info.type

#define SWITCH_L3_IP_IPV4_ADDRESS(ip_info) ip_info->ip.v4addr

#define SWITCH_L3_IP_IPV6_ADDRESS(ip_info) ip_info->ip.v6addr

switch_status_t switch_route_table_entry_key_init(void* args,
                                                  switch_uint8_t* key,
                                                  switch_uint32_t* len);

switch_int32_t switch_route_entry_hash_compare(const void* key1,
                                               const void* key2);

switch_status_t switch_l3_init(switch_device_t device);

switch_status_t switch_l3_free(switch_device_t device);

switch_status_t switch_route_table_hash_lookup(
    switch_device_t device, switch_route_entry_t* route_entry,
    switch_handle_t* route_handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_L3_INT_H__ */
