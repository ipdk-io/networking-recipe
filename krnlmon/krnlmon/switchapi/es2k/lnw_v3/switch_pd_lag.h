/*
 * Copyright 2023-2024 Intel Corporation.
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

#ifndef __SWITCH_PD_LAG_H__
#define __SWITCH_PD_LAG_H__

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_handle.h"
#include "switchapi/switch_lag.h"

#ifdef __cplusplus
extern "C" {
#endif

// Method to program the tx_lag_table on Tx side
switch_status_t switch_pd_tx_lag_table_entry(switch_device_t device,
                                             const switch_lag_info_t* lag_info,
                                             bool entry_add);

// Method to program the rx_lag_table on Rx side
switch_status_t switch_pd_rx_lag_table_entry(switch_device_t device,
                                             const switch_lag_info_t* lag_info,
                                             bool entry_add);

// Method to program the tx_lag_table on Tx side for LACP mode
switch_status_t switch_pd_tx_lacp_lag_table_entry(
    switch_device_t device, const switch_lag_info_t* lag_info, bool oper_state);

// Method to program the rx_lag_table on Rx side for LACP mode
switch_status_t switch_pd_rx_lacp_lag_table_entry(
    switch_device_t device, const switch_lag_info_t* lag_info, bool oper_state);

#ifdef __cplusplus
}
#endif

#endif
