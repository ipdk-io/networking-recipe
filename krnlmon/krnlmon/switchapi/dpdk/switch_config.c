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

#include "switchapi/switch_config_int.h"
#include "switchapi/switch_internal.h"
#include "switchutils/switch_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __FILE_ID__ SWITCH_CONFIG
switch_config_info_t config_info;

switch_status_t switch_config_init(switch_config_t* switch_config) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (config_info.config_inited) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    krnlmon_log_error("config init failed, error: %s",
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&config_info, 0x0, sizeof(config_info));

  config_info.api_switch_config.max_devices = SWITCH_MAX_DEVICE;
  config_info.api_switch_config.add_ports = FALSE;

  if (switch_config) {
    SWITCH_ASSERT(switch_config->max_devices < SWITCH_MAX_DEVICE);
    if (switch_config->max_devices) {
      config_info.api_switch_config.max_devices = switch_config->max_devices;
    }
  }

  SWITCH_ASSERT(config_info.api_switch_config.max_devices != 0);

  config_info.config_inited = TRUE;

  return status;
}

switch_status_t switch_config_free(void) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!config_info.config_inited) {
    return status;
  }

  config_info.config_inited = FALSE;

  SWITCH_MEMSET(&config_info, 0x0, sizeof(config_info));

  return status;
}

switch_status_t switch_config_device_context_set(
    switch_device_t device, switch_device_context_t* device_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device_ctx && config_info.device_inited[device]) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    krnlmon_log_error("config free failed for device %d, error: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  if (device_ctx) {
    config_info.device_ctx[device] = device_ctx;
    config_info.device_inited[device] = TRUE;
  } else {
    config_info.device_ctx[device] = NULL;
    config_info.device_inited[device] = FALSE;
  }

  return status;
}

switch_status_t switch_config_device_context_get(
    switch_device_t device, switch_device_context_t** device_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!config_info.device_inited[device]) {
    status = SWITCH_STATUS_UNINITIALIZED;
    krnlmon_log_error("Failed to get device context for device %d, error: %s\n",
                      device, switch_error_to_string(status));
    return status;
  }

  *device_ctx = config_info.device_ctx[device];

  return status;
}

switch_status_t switch_config_table_sizes_get(switch_device_t device,
                                              switch_size_t* table_sizes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_table_default_sizes_get(table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to get config table sizes for device %d, error: %s\n", device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

#ifdef __cplusplus
}  // extern "C"
#endif
