/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
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

#include "switchapi/switch_rmac.h"

#include "switch_pd_routing.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_rmac_int.h"
#include "switchutils/switch_log.h"

/*
 * Routine Description:
 *   @brief initilize rmac structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rmac_init(switch_device_t device) {
  switch_rmac_context_t* rmac_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  rmac_ctx = SWITCH_MALLOC(device, sizeof(switch_rmac_context_t), 0x1);
  if (!rmac_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "rmac_init:Failed to allocate memory for switch_rmac_context_t "
        "on device %d ,error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_set(device, SWITCH_API_TYPE_RMAC,
                                         (void*)rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "rmac_init: Failed to set device api context on device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  /* TODO: need to check the table size field */
  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_RMAC, 16384);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "rmac init: Failed to initialize handle SWITCH_HANDLE_TYPE_RMAC on "
        "device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  krnlmon_log_info("rmac init successful on device %d\n", device);
  return status;
}

/*
 * Routine Description:
 *   @brief initilize rmac structs
 *
 * Arguments:
 *   @param[in] device - device id
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_rmac_free(switch_device_t device) {
  switch_rmac_context_t* rmac_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_RMAC,
                                         (void**)&rmac_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "rmac free: Failed to get device api context on device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_RMAC);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "rmac free: Failed to free handle SWITCH_HANDLE_TYPE_RMAC on device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  SWITCH_FREE(device, rmac_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_RMAC, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_router_mac_group_create(
    const switch_device_t device, const switch_rmac_type_t rmac_type,
    switch_handle_t* rmac_handle) {
  switch_rmac_info_t* rmac_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  handle = switch_rmac_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "rmac group create: Failed to create rmac handle on device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, handle, &rmac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "rmac group create: Failed to get rmac info on device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  rmac_info->rmac_type = rmac_type;

  status = SWITCH_LIST_INIT(&rmac_info->rmac_list);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  *rmac_handle = handle;

  krnlmon_log_info(
      "rmac group created on device %d, "
      "rmac handle 0x%lx \n",
      device, handle);

  return status;
}

switch_status_t switch_api_router_mac_group_delete(
    const switch_device_t device, const switch_handle_t rif_handle,
    const switch_handle_t rmac_handle) {
  switch_rmac_info_t* rmac_info = NULL;
  switch_rmac_entry_t* rmac_entry = NULL;
  switch_node_t* node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "rmac group delete: Invalid rmac handle 0x%lx: on device %d "
        ",error: %s\n",
        rmac_handle, device, switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "rmac group delete: Failed to get rmac info on device %d rmac hadle "
        "0x%lx: ,error: %s\n",
        device, rmac_handle, switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = node->data;
    status = switch_api_router_mac_delete(device, rif_handle, rmac_handle,
                                          &rmac_entry->mac);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "rmac group delete: Failed to delete router mac on device %d "
          "rmac handle 0x%lx: ,error: %s\n",
          device, rmac_handle, switch_error_to_string(status));
    }
  }
  FOR_EACH_IN_LIST_END();

  status = switch_rmac_handle_delete(device, rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "rmac group delete: Failed to delete rmac handle 0x%lx: on device %d "
        ",error: %s\n",
        rmac_handle, device, switch_error_to_string(status));
    return status;
  }

  krnlmon_log_info(
      "rmac group deleted on device %d, "
      "rmac handle 0x%lx\n",
      device, rmac_handle);

  return status;
}

switch_status_t switch_api_router_mac_delete(const switch_device_t device,
                                             const switch_handle_t rif_handle,
                                             const switch_handle_t rmac_handle,
                                             const switch_mac_addr_t* mac) {
  switch_rmac_info_t* rmac_info = NULL;
  switch_rmac_entry_t* rmac_entry = NULL;
  switch_node_t* node = NULL;
  bool entry_found = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "router mac delete: Invalid rmac handle 0x%lx, mac %s: on device %d "
        ",error: %s\n",
        rmac_handle, switch_macaddress_to_string(mac), device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "router mac delete: Failed to get rmac info, handle 0x%lx "
        ",error: %s\n",
        rmac_handle, switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = (switch_rmac_entry_t*)node->data;
    if (SWITCH_MEMCMP(&(rmac_entry->mac), mac, sizeof(switch_mac_addr_t)) ==
        0) {
      entry_found = TRUE;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!entry_found) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    krnlmon_log_error(
        "router mac delete: Failed to find rmac entry on device %d, rmac "
        "handle 0x%lx, mac %s: ,error: %s\n",
        device, rmac_handle, switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  if (rmac_entry && rmac_entry->is_rmac_pd_programmed) {
    status = switch_pd_rmac_table_entry(device, rmac_entry, rif_handle, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "router mac delete: Failed to delete PD rmac entry on device %d ,"
          "rmac handle 0x%lx, mac %s: "
          ",error: %s\n",
          device, rmac_handle, switch_macaddress_to_string(mac),
          switch_error_to_string(status));
      return status;
    }
  }

  status = SWITCH_LIST_DELETE(&(rmac_info->rmac_list), node);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "router mac delete: Failed to delete from switch list on device %d ,"
        "rmac handle 0x%lx, mac %s: "
        ",error: %s\n",
        device, rmac_handle, switch_macaddress_to_string(mac),
        switch_error_to_string(status));
    return status;
  }

  krnlmon_log_info("rmac deleted on device %d, rmac handle 0x%lx, mac %s\n",
                   device, rmac_handle, switch_macaddress_to_string(mac));

  if (rmac_entry) {
    SWITCH_FREE(device, rmac_entry);
  }

  return status;
}

switch_status_t switch_api_router_mac_add(const switch_device_t device,
                                          const switch_handle_t rmac_handle,
                                          const switch_mac_addr_t* mac) {
  switch_rmac_info_t* rmac_info = NULL;
  switch_rmac_entry_t* rmac_entry = NULL;
  switch_node_t* node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "router mac add: Invalid rmac handle 0x%lx: on device %d "
        ",error: %s\n",
        rmac_handle, device, switch_error_to_string(status));
    return status;
  }

  status = switch_rmac_get(device, rmac_handle, &rmac_info);
  if (!SWITCH_RMAC_HANDLE(rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "router mac add: Failed to get rmac info on device %d, rmac "
        "handle 0x%lx: ,error: %s\n",
        device, rmac_handle, switch_error_to_string(status));
    return status;
  }

  if (!mac || (!SWITCH_MAC_VALID((*mac)))) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "router mac add: Invalid mac on device %d, rmac handle 0x%lx: "
        ",error: %s\n",
        device, rmac_handle, switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(rmac_info->rmac_list, node) {
    rmac_entry = (switch_rmac_entry_t*)node->data;
    if (SWITCH_MEMCMP(&(rmac_entry->mac), mac, sizeof(switch_mac_addr_t)) ==
        0) {
      return SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    }
  }
  FOR_EACH_IN_LIST_END();

  rmac_entry = SWITCH_MALLOC(device, sizeof(switch_rmac_entry_t), 0x1);
  if (!rmac_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "router mac add: Failed to allocate memory for switch_rmac_entry_t "
        "on device %d rmac handle 0x%lx: ,error: %s\n",
        device, rmac_handle, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(rmac_entry, 0x0, sizeof(switch_rmac_entry_t));
  SWITCH_MEMCPY(&rmac_entry->mac, mac, sizeof(switch_mac_addr_t));
  rmac_entry->is_rmac_pd_programmed = false;

  SWITCH_LIST_INSERT(&(rmac_info->rmac_list), &(rmac_entry->node), rmac_entry);

  krnlmon_log_info("RMAC added on device %d, rmac handle 0x%lx, mac %s\n",
                   device, rmac_handle,
                   switch_macaddress_to_string(&rmac_entry->mac));

  return status;
}
