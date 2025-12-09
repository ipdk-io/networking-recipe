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

#include "switchapi/switch_id.h"

#include "switchapi/switch_internal.h"
#include "switchutils/switch_log.h"

#define __FILE_ID__ SWITCH_ID

static switch_status_t switch_api_id_allocator_new_internal(
    switch_device_t device, switch_uint32_t initial_size, bool zero_based,
    switch_id_allocator_t** allocator) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *allocator = SWITCH_MALLOC(device, sizeof(switch_id_allocator_t), 1);
  if (*allocator == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "id alloc: Failed to allocate memory for switch_id_allocator_t, "
        "error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  (*allocator)->data =
      SWITCH_MALLOC(device, sizeof(switch_uint32_t), initial_size);
  if ((*allocator)->data == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_FREE(device, *allocator);
    krnlmon_log_error(
        "id alloc: Failed to allocate memory for allocator data, "
        "error: %s\n",
        switch_error_to_string(status));
    return status;
  }

  (*allocator)->n_words = initial_size;
  (*allocator)->zero_based = zero_based;
  SWITCH_MEMSET((*allocator)->data, 0, initial_size * sizeof(switch_uint32_t));
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_id_allocator_destroy_internal(
    switch_device_t device, switch_id_allocator_t* allocator) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("id destroy failed, error: %s",
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, allocator->data);
  SWITCH_FREE(device, allocator);
  return SWITCH_STATUS_SUCCESS;
}

static inline switch_int32_t switch_api_id_fit_width(switch_uint32_t val,
                                                     switch_uint32_t width) {
  switch_uint32_t offset = 32;
  switch_uint32_t mask = 0;
  switch_uint32_t b = 0;

  while (offset > width) {
    mask = (((switch_uint32_t)1 << width) - 1) << (offset - width);
    b = val & mask;
    if (!b) {
      return offset;
    }
    offset = __builtin_ctz(b);
  }
  return -1;
}

static switch_status_t switch_api_id_allocator_allocate_contiguous_internal(
    switch_device_t device, switch_id_allocator_t* allocator,
    switch_uint8_t count, switch_uint32_t* id) {
  switch_uint32_t n_words = 0;
  switch_uint32_t i = 0;
  switch_int32_t pos = -1;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("id alloc contiguous failed, error: %s",
                      switch_error_to_string(status));
    return status;
  }

  for (i = 0; i < allocator->n_words; i++) {
    if (allocator->data[i] != 0xFFFFFFFF) {
      pos = switch_api_id_fit_width(allocator->data[i], count);
      if (pos > 0) {
        // set the bitmap to 1s
        allocator->data[i] |= (0xFFFFFFFF << (pos - count)) & 0xFFFFFFFF;
        *id = 32 * i + (32 - pos) + (allocator->zero_based ? 0 : 1);
        return status;
      }
    }
  }

  n_words = allocator->n_words;
  allocator->data = SWITCH_REALLOC(device, allocator->data,
                                   n_words * 2 * sizeof(switch_uint32_t));

  SWITCH_MEMSET(&allocator->data[n_words], 0,
                n_words * sizeof(switch_uint32_t));
  allocator->n_words = n_words * 2;
  allocator->data[n_words] |= (0xFFFFFFFF << (32 - count)) & 0xFFFFFFFF;
  *id = 32 * n_words + (allocator->zero_based ? 0 : 1);
  return status;
}

static switch_status_t switch_api_id_allocator_allocate_internal(
    switch_device_t device, switch_id_allocator_t* allocator, switch_id_t* id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("id alloc internal failed, error: %s",
                      switch_error_to_string(status));
    return status;
  }

  return switch_api_id_allocator_allocate_contiguous(device, allocator, 1, id);
}

static switch_status_t switch_api_id_allocator_release_internal(
    switch_device_t device, switch_id_allocator_t* allocator, switch_id_t id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("id release failed, error: %s",
                      switch_error_to_string(status));
    return status;
  }

  if (allocator->zero_based != true) {
    id = id > 0 ? id - 1 : 0;
  }
  allocator->data[id >> 5] &= ~(1U << (31 - id % 32));
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_id_allocator_destroy(
    switch_device_t device, switch_id_allocator_t* allocator) {
  return switch_api_id_allocator_destroy_internal(device, allocator);
}

switch_status_t switch_api_id_allocator_new(switch_device_t device,
                                            switch_uint32_t initial_size,
                                            bool zero_based,
                                            switch_id_allocator_t** allocator) {
  return switch_api_id_allocator_new_internal(device, initial_size, zero_based,
                                              allocator);
}

switch_status_t switch_api_id_allocator_allocate(
    switch_device_t device, switch_id_allocator_t* allocator, switch_id_t* id) {
  return switch_api_id_allocator_allocate_internal(device, allocator, id);
}

switch_status_t switch_api_id_allocator_release(
    switch_device_t device, switch_id_allocator_t* allocator, switch_id_t id) {
  return switch_api_id_allocator_release_internal(device, allocator, id);
}

switch_status_t switch_api_id_allocator_allocate_contiguous(
    switch_device_t device, switch_id_allocator_t* allocator,
    switch_uint8_t count, switch_id_t* id) {
  return switch_api_id_allocator_allocate_contiguous_internal(device, allocator,
                                                              count, id);
}
