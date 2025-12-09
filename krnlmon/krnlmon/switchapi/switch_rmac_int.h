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

#ifndef __SWITCH_RMAC_INT_H__
#define __SWITCH_RMAC_INT_H__

#include "switch_id.h"
#include "switch_rmac.h"
#include "switch_types_int.h"

#ifdef __cplusplus
extern "C" {
#endif

#define switch_rmac_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_RMAC, \
                       sizeof(switch_rmac_info_t))

#define switch_rmac_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_RMAC, _handle)

#define switch_rmac_get(_device, _handle, _info)                 \
  ({                                                             \
    switch_rmac_info_t* _tmp_rmac_info = NULL;                   \
    (void)(_tmp_rmac_info == *_info);                            \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_RMAC, _handle, \
                      (void**)_info);                            \
  })

/** rmac entry */
typedef struct switch_rmac_entry_s {
  /** mac address */
  switch_mac_addr_t mac;

  /** list node */
  switch_node_t node;

  /** RMAC to be programmed or not */
  bool is_rmac_pd_programmed;

} switch_rmac_entry_t;

/** rmac group info */
typedef struct switch_rmac_info_s {
  /** list of rmac entries */
  switch_list_t rmac_list;

  /** rmac type - inner/outer */
  switch_rmac_type_t rmac_type;

} switch_rmac_info_t;

/** rewrite mac info */
typedef struct switch_smac_entry_s {
  /** mac address */
  switch_mac_addr_t mac;

  /** rewrite smac index */
  switch_id_t smac_index;

  /** smac index ref count */
  switch_uint16_t ref_count;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_smac_entry_t;

/** router mac device context */
typedef struct switch_rmac_context_s {
  /** smac index allocator */
  switch_id_allocator_t* smac_allocator;

  /** tunnel smac index allocator */
  switch_id_allocator_t* tunnel_smac_allocator;

} switch_rmac_context_t;

switch_status_t switch_rmac_init(switch_device_t device);

switch_status_t switch_rmac_free(switch_device_t device);

switch_status_t switch_rmac_default_entries_add(switch_device_t device);

switch_status_t switch_rmac_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __SWITCH_RMAC_INT_H__ */
