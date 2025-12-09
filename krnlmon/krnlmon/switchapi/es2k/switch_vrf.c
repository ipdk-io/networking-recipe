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

#include "switchapi/switch_vrf.h"

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_status.h"

switch_status_t switch_vrf_init(switch_device_t device) {
  switch_vrf_context_t* vrf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  vrf_ctx = SWITCH_MALLOC(device, sizeof(switch_vrf_context_t), 0x1);
  if (!vrf_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "vrf init: Failed to allocate memory switch_vrf_context_t on device %d:"
        " , error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(vrf_ctx, 0x0, sizeof(switch_vrf_context_t));

  status = switch_device_api_context_set(device, SWITCH_API_TYPE_VRF,
                                         (void*)vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf init: Failed to get device context on device %d: "
        ", error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_VRF, MAX_VRF);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf init: Failed to init handle on device %d: "
        ", error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  krnlmon_log_info("vrf init successful on device %d\n", device);

  return status;
}

switch_status_t switch_vrf_free(switch_device_t device) {
  switch_vrf_context_t* vrf_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_VRF,
                                         (void**)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf free: Failed to get device context on device %d: "
        ", error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_VRF);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf free: Failed to fre handle on device %d: "
        ", error: %s\n",
        device, switch_error_to_string(status));
  }

  SWITCH_FREE(device, vrf_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_VRF, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  krnlmon_log_info("vrf free successful on device %d\n", device);

  return status;
}

switch_status_t switch_api_vrf_create(const switch_device_t device,
                                      const switch_vrf_t vrf_id,
                                      switch_handle_t* vrf_handle) {
  switch_vrf_context_t* vrf_ctx = NULL;
  switch_vrf_info_t* vrf_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t* tmp_vrf_handle = NULL;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_VRF,
                                         (void**)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf create:: Failed to get device context on device %d, vrf id %d: "
        ", error: %s\n",
        device, vrf_id, switch_error_to_string(status));
    return status;
  }

  *vrf_handle = SWITCH_API_INVALID_HANDLE;

  if (vrf_id) {
    status = SWITCH_ARRAY_GET(&vrf_ctx->vrf_id_array, vrf_id,
                              (void**)&tmp_vrf_handle);
    if (status != SWITCH_STATUS_ITEM_NOT_FOUND &&
        status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "vrf create Failed to get vrf handle on device %d, vrf id %d: "
          ", error: %s\n",
          device, vrf_id, switch_error_to_string(status));
      return status;
    }

    if (status == SWITCH_STATUS_SUCCESS) {
      *vrf_handle = *tmp_vrf_handle;
      krnlmon_log_info("vrf id %x, (handle: 0x%lx) already exists on device %d",
                       vrf_id, *vrf_handle, device);
      return status;
    }
  }

  handle = switch_vrf_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "vrf create: Failed to create handle on device %d, vrf id %d: "
        ", error: %s\n",
        device, vrf_id, switch_error_to_string(status));
    return status;
  }

  *vrf_handle = handle;

  status =
      SWITCH_ARRAY_INSERT(&vrf_ctx->vrf_id_array, vrf_id, (void*)(vrf_handle));

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf create Failed to get vrf handle on device %d, vrf id %d: "
        ", error: %s\n",
        device, vrf_id, switch_error_to_string(status));
    status = switch_vrf_handle_delete(device, handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    return status;
  }

  switch_vrf_get(device, handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf create: Failed to get vrf info on device %d, vrf id %d: "
        ", error: %s\n",
        device, vrf_id, switch_error_to_string(status));
    status = switch_vrf_handle_delete(device, handle);
    return status;
  }

  vrf_info->vrf_id = vrf_id;

  krnlmon_log_info("vrf created on device %d vrf id %d handle 0x%lx ", device,
                   vrf_id, handle);

  return status;
}

switch_status_t switch_api_vrf_delete(const switch_device_t device,
                                      const switch_handle_t vrf_handle) {
  switch_vrf_context_t* vrf_ctx = NULL;
  switch_vrf_info_t* vrf_info = NULL;
  switch_handle_t* tmp_vrf_handle = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "vrf delete failed on device %d, vrf handle 0x%lx: "
        ", error: %s\n",
        device, vrf_handle, switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_VRF,
                                         (void**)&vrf_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf delete: Failed to get device context on device %d, vrf "
        "handle 0x%lx: , error: %s\n",
        device, vrf_handle, switch_error_to_string(status));
    return status;
  }

  switch_vrf_get(device, vrf_handle, &vrf_info, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf delete: Failed to get vrf info on device %d, vrf handle 0x%lx: "
        ", error: %s\n",
        device, vrf_handle, switch_error_to_string(status));
    return status;
  }

  if (vrf_info->vrf_id) {
    SWITCH_ARRAY_GET(&vrf_ctx->vrf_id_array, vrf_info->vrf_id,
                     (void**)&tmp_vrf_handle);
    status = SWITCH_ARRAY_DELETE(&vrf_ctx->vrf_id_array, vrf_info->vrf_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "vrf delete: Failed to get vrf info for vrf handle 0x%lx vrf id %d: "
          "on device %d, error: %s\n",
          vrf_handle, vrf_info->vrf_id, device, switch_error_to_string(status));
      return status;
    }

    if (tmp_vrf_handle) {
      SWITCH_FREE(device, tmp_vrf_handle);
    }
  }

  status = switch_vrf_handle_delete(device, vrf_info->vrf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "vrf delete: Failed to delete vrf handle 0x%lx: on device %d "
        ", error: %s\n",
        vrf_handle, device, switch_error_to_string(status));
    return status;
  }

  krnlmon_log_info("vrf deleted on device %d, vrf handle 0x%lx\n", device,
                   vrf_handle);

  return status;
}
