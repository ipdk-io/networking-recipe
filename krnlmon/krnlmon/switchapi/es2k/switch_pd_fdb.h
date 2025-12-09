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

#ifndef __SWITCH_PD_FDB_H__
#define __SWITCH_PD_FDB_H__

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_fdb.h"
#include "switchapi/switch_handle.h"
#include "switchapi/switch_tunnel.h"

#ifdef __cplusplus
extern "C" {
#endif

switch_status_t switch_pd_l2_tx_forward_table_entry(
    switch_device_t device, const switch_api_l2_info_t* api_l2_tx_info,
    const switch_api_tunnel_info_t* api_tunnel_info_t, bool entry_add);

switch_status_t switch_pd_l2_rx_forward_table_entry(
    switch_device_t device, const switch_api_l2_info_t* api_l2_rx_info,
    bool entry_add);

switch_status_t switch_pd_l2_rx_forward_with_tunnel_table_entry(
    switch_device_t device, const switch_api_l2_info_t* api_l2_rx_info,
    bool entry_add);

#ifdef __cplusplus
}
#endif

#endif
