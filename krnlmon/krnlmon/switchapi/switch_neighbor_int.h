/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License_Identifier: Apache-2.0
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
#ifndef __SWITCH_NEIGHBOR_INT_H__
#define __SWITCH_NEIGHBOR_INT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "switch_pd_routing.h"
#include "switch_rif.h"
#include "switch_types_int.h"

#define switch_neighbor_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_NEIGHBOR, \
                       sizeof(switch_neighbor_info_t))

#define switch_neighbor_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_NEIGHBOR, _handle)

#define switch_neighbor_get(_device, _handle, _info)                 \
  ({                                                                 \
    switch_neighbor_info_t* _tmp_neighbor_info = NULL;               \
    (void)(_tmp_neighbor_info == *_info);                            \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_NEIGHBOR, _handle, \
                      (void**)_info);                                \
  })

/** neighbor device context */
typedef struct switch_neighbor_context_s {
  /** tunnel dmac rewrite hashtable */
  //  switch_hashtable_t tunnel_dmac_rewrite_hashtable;

  /** dmac rewrite hashtable */
  //  switch_hashtable_t neighbor_dmac_hashtable;

  /** tunnel dmac rewrite index allocator */
  switch_id_allocator_t* dmac_rewrite_index;

} switch_neighbor_context_t;

switch_status_t switch_neighbor_init(switch_device_t device);

switch_status_t switch_neighbor_free(switch_device_t device);

switch_status_t switch_neighbor_default_entries_add(switch_device_t device);

switch_status_t switch_neighbor_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif  // __SWITCH_NEIGHBOR_INT_H__
