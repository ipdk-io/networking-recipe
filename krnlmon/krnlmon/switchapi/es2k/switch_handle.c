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

#include "switchapi/switch_handle.h"

#include "switchapi/switch_internal.h"
#include "switchapi/switch_nhop_int.h"
#include "switchapi/switch_status.h"
#include "switchutils/switch_log.h"

#define __FILE_ID__ SWITCH_HANDLE

switch_handle_type_t switch_handle_type_get(switch_handle_t handle) {
  switch_handle_type_t type = SWITCH_HANDLE_TYPE_NONE;
  type = handle >> SWITCH_HANDLE_TYPE_SHIFT;
  return type;
}

switch_status_t switch_handle_type_init(switch_device_t device,
                                        switch_handle_type_t type,
                                        switch_size_t size) {
  // Modified grow_on_demand to false, for linux_newtorking.p4
  return switch_handle_type_allocator_init(
      device, type, size * 4, false /*grow*/, false /*zero_based*/);
}

switch_status_t switch_handle_type_allocator_init(switch_device_t device,
                                                  switch_handle_type_t type,
                                                  switch_uint32_t num_handles,
                                                  bool grow_on_demand,
                                                  bool zero_based) {
  switch_device_context_t* device_ctx = NULL;
  switch_handle_info_t* handle_info = NULL;
  switch_id_allocator_t* allocator = NULL;
  switch_size_t size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("handle allocator init failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle allocator init: Failed to get device context, error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "handle allocator init failedto get device context, error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  handle_info = SWITCH_MALLOC(device, sizeof(switch_handle_info_t), 1);
  if (!handle_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "handle allocator init: Failed to allocate memory for "
        "handle %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(handle_info, 0x0, sizeof(switch_handle_info_t));

  size = (num_handles + 3) / 4;
  status = switch_api_id_allocator_new(device, size, zero_based, &allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_FREE(device, handle_info);
    status = SWITCH_STATUS_FAILURE;
    krnlmon_log_error(
        "handle allocator init: Faile to allocate new api id "
        "for handle %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }

  handle_info->type = type;
  handle_info->initial_size = size;
  handle_info->allocator = allocator;
  handle_info->num_in_use = 0;
  handle_info->num_handles = num_handles;
  handle_info->grow_on_demand = grow_on_demand;
  handle_info->zero_based = zero_based;
  handle_info->new_allocator = bf_id_allocator_new(size, zero_based);

  status = SWITCH_ARRAY_INSERT(&device_ctx->handle_info_array, type,
                               (void*)handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle allocator init: Failed to insert handle %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    switch_api_id_allocator_destroy(device, handle_info->allocator);
    bf_id_allocator_destroy(handle_info->new_allocator);
    SWITCH_FREE(device, handle_info);
    return status;
  }

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_handle_type_free(switch_device_t device,
                                        switch_handle_type_t type) {
  switch_device_context_t* device_ctx = NULL;
  switch_handle_info_t* handle_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("handle free failed: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle free failed: Failed to get device context, error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("handle free failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_GET(&device_ctx->handle_info_array, type,
                            (void*)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("handle %s free failed: %s\n",
                      switch_handle_type_to_string(type),
                      switch_error_to_string(status));
    return status;
  }

  switch_api_id_allocator_destroy(device, handle_info->allocator);
  bf_id_allocator_destroy(handle_info->new_allocator);
  status = SWITCH_ARRAY_DELETE(&device_ctx->handle_info_array, type);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle free: Failed to destroy allocator for "
        "handle %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, handle_info);
  return status;
}

static switch_handle_t __switch_handle_create(switch_device_t device,
                                              switch_handle_type_t type,
                                              unsigned int count) {
  switch_device_context_t* device_ctx = NULL;
  switch_handle_info_t* handle_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_uint32_t id = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("handle_create failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle_create: Failed to get device context, error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  if (type >= SWITCH_HANDLE_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("handle_create failed, error: %s\n",
                      switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  status = SWITCH_ARRAY_GET(&device_ctx->handle_info_array, type,
                            (void**)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("handle_create: Failed to get handle %s, error: %s\n",
                      switch_handle_type_to_string(type),
                      switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  if (((handle_info->num_in_use + count - 1) < handle_info->num_handles) ||
      handle_info->grow_on_demand) {
    if (count == 1)
      status =
          switch_api_id_allocator_allocate(device, handle_info->allocator, &id);
    else
      status = switch_api_id_allocator_allocate_contiguous(
          device, handle_info->allocator, count, &id);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "handle_create: Failed to allocate contiguous allocator"
          " for handle %s, error: %s\n",
          switch_handle_type_to_string(type), switch_error_to_string(status));
      return SWITCH_API_INVALID_HANDLE;
    }
    handle_info->num_in_use++;
    handle = id_to_handle(type, id);
  }

  return handle;
}

static switch_status_t _switch_handle_delete_contiguous(switch_device_t device,
                                                        switch_handle_t handle,
                                                        uint32_t count) {
  switch_device_context_t* device_ctx = NULL;
  switch_handle_info_t* handle_info = NULL;
  switch_uint32_t id = 0;
  switch_handle_type_t type = SWITCH_HANDLE_TYPE_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device > SWITCH_MAX_DEVICE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("handle delete contiguous failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle delete contiguous: Failed to get device "
        "context, error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  type = switch_handle_type_get(handle);
  status = SWITCH_ARRAY_GET(&device_ctx->handle_info_array, type,
                            (void*)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle delete contiguous: Failed to get handle info for type %s, "
        "error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }

  id = handle_to_id(handle);
  for (unsigned int i = 0; i < count; i++)
    switch_api_id_allocator_release(device, handle_info->allocator, id + i);
  handle_info->num_in_use -= count;
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t _switch_handle_delete(switch_device_t device,
                                             switch_handle_t handle) {
  return _switch_handle_delete_contiguous(device, handle, 1);
}

static switch_handle_t _switch_handle_create(switch_device_t device,
                                             switch_handle_type_t type,
                                             switch_uint32_t size,
                                             unsigned int count) {
  switch_device_context_t* device_ctx = NULL;
  void* i_info = NULL;
  void* handle_array = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle create: Failed to get device context for type %s, "
        "error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  handle_array = &device_ctx->handle_array[type];
  handle = __switch_handle_create(device, type, count);

  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_FAILURE;
    krnlmon_log_error("handle create: Invalid handle for type %s, error: %s\n",
                      switch_handle_type_to_string(type),
                      switch_error_to_string(status));
    return SWITCH_API_INVALID_HANDLE;
  }

  i_info = SWITCH_MALLOC(device, size, 1);
  if (!i_info) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "handle create: Failed to allocate memory for type %s, "
        "error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    _switch_handle_delete(device, handle);
    return SWITCH_API_INVALID_HANDLE;
  }

  SWITCH_MEMSET(i_info, 0, size);

  status = SWITCH_ARRAY_INSERT(handle_array, handle, (void*)i_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle create: Failed to insert handle for type %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    SWITCH_FREE(device, i_info);
    _switch_handle_delete(device, handle);
    return SWITCH_API_INVALID_HANDLE;
  }
  return handle;
}

switch_handle_t switch_handle_create(switch_device_t device,
                                     switch_handle_type_t type,
                                     switch_uint32_t size) {
  return _switch_handle_create(device, type, size, 1);
}

switch_handle_t switch_handle_create_contiguous(switch_device_t device,
                                                switch_handle_type_t type,
                                                switch_uint32_t size,
                                                unsigned int count) {
  return _switch_handle_create(device, type, size, count);
}

switch_status_t switch_handle_get(switch_device_t device,
                                  switch_handle_type_t type,
                                  switch_handle_t handle, void** i_info) {
  switch_device_context_t* device_ctx = NULL;
  void* handle_array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_HANDLE_VALID(handle, type)) {
    krnlmon_log_error("handle get failed due to invalid handle %lx\n", handle);
    return SWITCH_STATUS_INVALID_HANDLE;
  }
  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle get: Failed to get device context for handle "
        "type %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }
  handle_array = &device_ctx->handle_array[type];

  status = SWITCH_ARRAY_GET(handle_array, handle, (void**)i_info);

  type = switch_handle_type_get(handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("handle get: failed to get handle type, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_handle_delete_contiguous(switch_device_t device,
                                                switch_handle_type_t type,
                                                switch_handle_t handle,
                                                uint32_t count) {
  switch_device_context_t* device_ctx = NULL;
  void* i_info = NULL;
  void* handle_array = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle delete contiguous failed for type %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }
  handle_array = &device_ctx->handle_array[type];

  status = SWITCH_ARRAY_GET(handle_array, handle, (void**)&i_info);

  type = switch_handle_type_get(handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle delete contiguous: failed to get handle "
        "type %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_DELETE(handle_array, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle delete contiguous: failed to delete array "
        "for handle %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }

  status = _switch_handle_delete_contiguous(device, handle, count);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle delete contiguous: failed to delete "
        "handle %s, error: %s\n",
        switch_handle_type_to_string(type), switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, i_info);
  return status;
}

switch_status_t switch_handle_delete(switch_device_t device,
                                     switch_handle_type_t type,
                                     switch_handle_t handle) {
  return switch_handle_delete_contiguous(device, type, handle, 1);
}

switch_status_t switch_api_handle_count_get(switch_device_t device,
                                            switch_handle_type_t type,
                                            switch_size_t* num_entries) {
  switch_device_context_t* device_ctx = NULL;
  switch_handle_info_t* handle_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(type < SWITCH_HANDLE_TYPE_MAX);
  SWITCH_ASSERT(num_entries != NULL);
  *num_entries = 0;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("handle count get failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  status = SWITCH_ARRAY_GET(&device_ctx->handle_info_array, type,
                            (void*)&handle_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "handle count get: Failed to get handle info, error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  *num_entries = handle_info->num_in_use;

  return status;
}
