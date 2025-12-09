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

#ifndef __SWITCH_VRF_H__
#define __SWITCH_VRF_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_VRF 1024

#define switch_vrf_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_VRF, \
                       sizeof(switch_vrf_info_t))

#define switch_vrf_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_VRF, _handle)

#define switch_vrf_get(_device, _handle, _info, _status)                  \
  do {                                                                    \
    switch_vrf_context_t* _vrf_ctx = NULL;                                \
    _status = switch_device_api_context_get(_device, SWITCH_API_TYPE_VRF, \
                                            (void**)&_vrf_ctx);           \
    if (_status == SWITCH_STATUS_SUCCESS) {                               \
      switch_handle_get(_device, SWITCH_HANDLE_TYPE_VRF, _handle,         \
                        (void**)_info);                                   \
    }                                                                     \
  } while (0);

typedef struct switch_vrf_context_s {
  /** vrf id to handle array */
  switch_array_t vrf_id_array;

  /** bd vrf handle to vrf handle */
  switch_array_t vrf_array;

} switch_vrf_context_t;

/** stores vrf info and its associated hardware handles */
typedef struct switch_vrf_info_s {
  /**
   * rmac handle used by vrf.
   * used for l3 interfaces with no rmac handles
   */
  switch_handle_t rmac_handle;

  /** vrf id */
  switch_vrf_t vrf_id;

  /** vrf handle - self pointer */
  switch_handle_t vrf_handle;

} switch_vrf_info_t;

switch_status_t switch_vrf_init(switch_device_t device);

switch_status_t switch_vrf_free(switch_device_t device);

switch_status_t switch_api_vrf_create(const switch_device_t device,
                                      const switch_vrf_t vrf_id,
                                      switch_handle_t* vrf_handle);

switch_status_t switch_api_vrf_delete(const switch_device_t device,
                                      const switch_handle_t vrf_handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_VRF_H__ */
