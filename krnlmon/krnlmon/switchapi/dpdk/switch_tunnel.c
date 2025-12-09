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

#include "switchapi/switch_tunnel.h"

#include "switch_pd_tunnel.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_status.h"

switch_status_t switch_api_tunnel_delete(const switch_handle_t tunnel_handle) {
  switch_device_t device = 0;
  switch_tunnel_info_t* tunnel_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_TUNNEL_HANDLE(tunnel_handle));
  if (!SWITCH_TUNNEL_HANDLE(tunnel_handle)) {
    krnlmon_log_error(
        "Failed to delete tunnel on device %d due to invalid, "
        "tunnel handle 0x%lx: ,error: %s",
        device, tunnel_handle, switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, tunnel_handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS || tunnel_info == NULL) {
    krnlmon_log_error(
        "Failed to get tunnel info on device %d of tunnel handle 0x%lx: "
        "error: %s\n",
        device, tunnel_handle, switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_entry(device, &tunnel_info->api_tunnel_info, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to delete tunnel entry on device %d: ,"
        "error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_handle_delete(device, tunnel_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to delete tunnel handle on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  return status;
}

// Stubbed code to enabe compilation
switch_status_t switch_api_tunnel_term_delete(
    switch_handle_t tunnel_term_handle) {
  switch_tunnel_term_info_t* tunnel_term_info = NULL;
  switch_api_tunnel_term_info_t* api_tunnel_term_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_t device = 0;

  SWITCH_ASSERT(SWITCH_TUNNEL_TERM_HANDLE(tunnel_term_handle));
  status =
      switch_tunnel_term_get(device, tunnel_term_handle, &tunnel_term_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to get tunnel term info on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }
  api_tunnel_term_info = &tunnel_term_info->api_tunnel_term_info;

  status = switch_pd_tunnel_term_entry(device, api_tunnel_term_info, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to delete tunnel term entry on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }
  status = switch_tunnel_term_handle_delete(device, tunnel_term_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to delete tunnel term handle on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_tunnel_create(
    const switch_device_t device,
    const switch_api_tunnel_info_t* api_tunnel_info,
    switch_handle_t* tunnel_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_tunnel_info_t* tunnel_info = NULL;

  handle = switch_tunnel_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "Failed to create tunnel handle on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_get(device, handle, &tunnel_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to get tunnel info on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_entry(device, api_tunnel_info, true);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to create tunnel entry on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&tunnel_info->api_tunnel_info, api_tunnel_info,
                sizeof(switch_api_tunnel_info_t));

  *tunnel_handle = handle;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_tunnel_term_create(
    const switch_device_t device,
    const switch_api_tunnel_term_info_t* api_tunnel_term_info,
    switch_handle_t* tunnel_term_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_tunnel_term_info_t* tunnel_term_info = NULL;

  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;

  handle = switch_tunnel_term_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "Failed to create tunnel term handle on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_tunnel_term_get(device, handle, &tunnel_term_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to get tunnel term info on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_pd_tunnel_term_entry(device, api_tunnel_term_info, true);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to create tunnel term entry on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&tunnel_term_info->api_tunnel_term_info, api_tunnel_term_info,
                sizeof(switch_api_tunnel_term_info_t));

  *tunnel_term_handle = handle;

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_tunnel_init(switch_device_t device) {
  switch_tunnel_context_t* tunnel_ctx = NULL;
  switch_size_t tunnel_table_size = 0;
  switch_size_t tunnel_term_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  tunnel_ctx = SWITCH_MALLOC(device, sizeof(switch_tunnel_context_t), 0x1);
  if (!tunnel_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "Failed to allocate memory for switch_tunnel_context_t on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(tunnel_ctx, 0x0, sizeof(switch_tunnel_context_t));

  status = switch_device_api_context_set(device, SWITCH_API_TYPE_TUNNEL,
                                         (void*)tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to set device api context on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_api_table_size_get(device, SWITCH_TABLE_TUNNEL,
                                     &tunnel_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_table_size_get(device, SWITCH_TABLE_TUNNEL_TERM,
                                     &tunnel_term_table_size);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

#if 0
  status = switch_api_id_allocator_new(device,
                                       SWITCH_TUNNEL_VNI_ALLOCATOR_SIZE,
                                       FALSE,
                                       &tunnel_ctx->tunnel_vni_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to allocate new api id on device %d: "
        ",error: %s\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }
#endif

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_TUNNEL,
                                   tunnel_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to init handle SWITCH_HANDLE_TYPE_TUNNEL on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_TUNNEL_TERM,
                                   tunnel_term_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to init handle SWITCH_HANDLE_TYPE_TUNNEL_TERM on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  tmp_status = switch_tunnel_free(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_tunnel_free(switch_device_t device) {
  switch_tunnel_context_t* tunnel_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_TUNNEL,
                                         (void**)&tunnel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to get device api context on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to free handle SWITCH_HANDLE_TYPE_TUNNEL on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_TUNNEL_TERM);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to free handle SWITCH_HANDLE_TYPE_TUNNEL_TERM on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  SWITCH_FREE(device, tunnel_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_TUNNEL, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}
