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

#ifndef __SWITCH_RMAC_H__
#define __SWITCH_RMAC_H__

#include "switch_base_types.h"
#include "switch_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup RMAC Router MAC API
 *  API functions define and manipulate router mac groups
 *  @{
 */  // begin of Router MAC API

/***************************************************************************
 * ENUMS
 ***************************************************************************/

/**
 * @brief Router mac type
 *
 * Outer or inner rmac table is programmed based on the rmac type.
 * source mac rewrite index is allocated only for the first mac entry.
 * smac index is not reallocated or reprogrammed when the first mac is
 * deleted.
 */
typedef enum switch_rmac_type_s {
  SWITCH_RMAC_TYPE_OUTER = (1 << 0),
  SWITCH_RMAC_TYPE_INNER = (1 << 1),
  SWITCH_RMAC_TYPE_ALL = 0x3
} switch_rmac_type_t;

/***************************************************************************
 * APIS
 ***************************************************************************/

/**
 * @brief Create a router mac group
 *
 * @param[in] device device id
 * @param[in] rmac_type router mac type - outer/inner/all
 * @param[out] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_group_create(
    const switch_device_t device, const switch_rmac_type_t rmac_type,
    switch_handle_t* rmac_handle);

/**
 * @brief Delete a router mac group
 *
 * @param[in] device device id
 * @param[in] rif_handle RIF  handle
 * @param[in] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_group_delete(
    const switch_device_t device, const switch_handle_t rif_handle,
    const switch_handle_t rmac_handle);

/**
 * @brief Create a router mac group with rmacs
 *
 * @param[in] device device id
 * @param[in] num_macs number of router mac addresses
 * @param[in] mac router mac array
 * @param[out] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_group_create_with_macs(
    const switch_device_t device, const switch_size_t num_macs,
    const switch_mac_addr_t* mac, switch_handle_t* rmac_handle);

/**
 * @brief Add a router mac to rmac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] mac mac address
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
/*switch_status_t switch_api_router_mac_add(const switch_device_t device,
                                          const switch_handle_t rmac_handle,
                                          const switch_mac_addr_t *mac,
                                          const bool resv);
*/
switch_status_t switch_api_vrrp_rmac_add(const switch_device_t device,
                                         const switch_handle_t rmac_handle,
                                         const switch_mac_addr_t* vrrp_mac);

/**
 * @brief Delete a router mac from rmac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] mac mac address
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_delete(const switch_device_t device,
                                             const switch_handle_t rif_handle,
                                             const switch_handle_t rmac_handle,
                                             const switch_mac_addr_t* mac);

switch_status_t switch_api_vrrp_rmac_delete(const switch_device_t device,
                                            const switch_handle_t rmac_handle,
                                            const switch_mac_addr_t* vrrp_mac);

/**
 * @brief Returns all router macs belonging to a router mac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[out] num_entries number of mac entries
 * @param[out] macs mac array
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_rmac_macs_get(const switch_device_t device,
                                         const switch_handle_t rmac_handle,
                                         switch_uint16_t* num_entries,
                                         switch_mac_addr_t** macs);

/**
 * @brief Dumps router mac info using clish context
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] cli_ctx cli context
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_rmac_handle_dump(const switch_device_t device,
                                            const switch_handle_t rmac_handle,
                                            const void* cli_ctx);

/**
 * @brief Add a tunnel dmac to rmac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] mac tunnel dmac address
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_add(const switch_device_t device,
                                          const switch_handle_t rmac_handle,
                                          const switch_mac_addr_t* mac);

/** @} */  // end of Router MAC API

#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_RMAC_H__ */
