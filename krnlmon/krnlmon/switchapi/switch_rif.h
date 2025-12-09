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

#ifndef __SWITCH_RIF_H__
#define __SWITCH_RIF_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_tunnel.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_rif_type_s {
  SWITCH_RIF_TYPE_NONE = 0,
  SWITCH_RIF_TYPE_VLAN = 1,
  SWITCH_RIF_TYPE_LN = 2,
  SWITCH_RIF_TYPE_INTF = 3,
  SWITCH_RIF_TYPE_LOOPBACK = 4,
  SWITCH_RIF_TYPE_MAX
} switch_rif_type_t;

/** rif information */
typedef struct switch_api_rif_info_s {
  uint32_t rif_ifindex;
  switch_rif_type_t rif_type;

  switch_handle_t intf_handle;
  switch_handle_t rmac_handle; /**< rmac group id */
  switch_handle_t nh_handle;   /**< shared nexthop if a route points to RIF */

  switch_port_t port_id;     /** target dp index used when packet need to tx */
  switch_port_t phy_port_id; /** target dp index used when packet need to rx */

} switch_api_rif_info_t;

switch_status_t switch_rif_init(switch_device_t device);

switch_status_t switch_rif_free(switch_device_t device);

switch_status_t switch_api_rif_attribute_get(
    const switch_device_t device, const switch_handle_t rif_handle,
    const switch_uint64_t rif_flags, switch_api_rif_info_t* api_rif_info);

#if defined(ES2K_TARGET)
switch_status_t switch_api_update_rif_rmac_handle(
    const switch_device_t device, const switch_handle_t rif_handle,
    const switch_handle_t rmac_handle);
#endif

switch_status_t switch_api_rif_create(switch_device_t device,
                                      switch_api_rif_info_t* api_rif_info,
                                      switch_handle_t* rif_handle);
switch_status_t switch_api_rif_delete(switch_device_t device,
                                      switch_handle_t rif_handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_RIF_H__ */
