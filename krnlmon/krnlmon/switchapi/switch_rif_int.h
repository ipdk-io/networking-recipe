/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
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
#ifndef __SWITCH_RIF_INT_H__
#define __SWITCH_RIF_INT_H__

#include "switch_rif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWITCH_RIF_MAX 16384

typedef struct switch_rif_info_s {
  /** application interface info */
  switch_api_rif_info_t api_rif_info;

  /** list of ip addresses */
  switch_list_t ip_list;

  /** list of vrf */
  switch_array_t vrf_array;

  switch_handle_t rif_handle;

} switch_rif_info_t;

#define switch_rif_handle_create(_device)               \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_RIF, \
                       sizeof(switch_rif_info_t))

#define switch_rif_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_RIF, _handle)

#define switch_rif_get(_device, _handle, _info)                 \
  ({                                                            \
    switch_rif_info_t* _tmp_rif_info = NULL;                    \
    (void)(_tmp_rif_info == *_info);                            \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_RIF, _handle, \
                      (void**)_info);                           \
  })

#define SWITCH_RIF_TYPE(_rif_info) _rif_info->api_rif_info.rif_type

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __SWITCH_RIF_INT_H__ */
