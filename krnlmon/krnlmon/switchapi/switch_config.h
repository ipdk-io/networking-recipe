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

#ifndef _switch_config_h
#define _switch_config_h

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_table.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct switch_config_s {
  bool use_pcie;

  bool add_ports;

  bool enable_ports;

  bool program_smac;

  switch_char_t cpu_interface[SWITCH_HOSTIF_NAME_SIZE];

  switch_uint16_t max_devices;

  switch_table_t table_info[SWITCH_TABLE_MAX];

  bool acl_group_optimization;

} switch_config_t;

switch_status_t switch_config_init(switch_config_t* switch_config);

switch_status_t switch_config_free(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _switch_config_h */
