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

#ifndef __SWITCH_DEVICE_INT_H__
#define __SWITCH_DEVICE_INT_H__
#include "switch_device.h"
#include "switch_id.h"
#include "switch_table.h"
#include "switch_types_int.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_IFINDEX_SIZE 16382

#define SWITCH_MAX_PORTS 512

#define SWITCH_MAX_RECIRC_PORTS 4

#define SWITCH_MAX_DEVICE 256

typedef void* switch_api_mutex_t;

typedef switch_status_t (*switch_device_stats_poll_interval_set_fn)(
    switch_device_t device, switch_uint32_t interval);

/** device context information */
typedef struct switch_device_context_s {
  /** p4 table sizes */
  switch_table_t table_info[SWITCH_TABLE_MAX];

  /**
   * handle info array indexed based on handle type
   * which provides the handle allocator
   */
  switch_array_t handle_info_array;

  /**
   * handle array is to allocate and store the
   * contents of every handle's associaated struct.
   */
  switch_array_t handle_array[128];

  switch_api_device_info_t device_info;

  /** device context for every module */
  void* context[SWITCH_API_TYPE_MAX];

  /** flag to indicate if module is initialized */
  bool api_inited[SWITCH_API_TYPE_MAX];

  /** ifindex allocator */
  switch_id_allocator_t* ifindex_allocator;

  /** maximum pipes */
  switch_uint32_t max_pipes;

  /** boolean for warm init */
  bool warm_init;

  /** device id */
  switch_device_t device_id;

} switch_device_context_t;

switch_status_t switch_device_init(switch_device_t device,
                                   switch_size_t* table_sizes);
switch_status_t switch_device_deinit(switch_device_t device);

switch_status_t switch_device_free(switch_device_t device);

switch_status_t switch_device_api_init(switch_device_t device);

switch_status_t switch_device_api_free(switch_device_t device);

switch_status_t switch_device_table_get(switch_device_t device,
                                        switch_table_t** table_info);

switch_status_t switch_device_api_context_get(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void** context);

switch_status_t switch_device_api_context_set(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void* context);

switch_status_t switch_device_context_get(
    switch_device_t device, switch_device_context_t** context_get);

switch_status_t switch_device_ifindex_allocate(switch_device_t device,
                                               switch_ifindex_t* ifindex);

switch_status_t switch_device_ifindex_deallocate(switch_device_t device,
                                                 switch_ifindex_t ifindex);

switch_status_t switch_api_device_vrf_max_get(switch_device_t device,
                                              switch_uint16_t* max_vrf);

switch_status_t switch_device_max_pipes_get(switch_device_t device,
                                            switch_int32_t* max_pipes);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_DEVICE_INT_H__ */
