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

#ifndef __SWITCHLINK_INIT_SAI_H__
#define __SWITCHLINK_INIT_SAI_H__

#include "krnlmon_options.h"
#include "sai.h"
#include "switchsai/saiinternal.h"

// Init SAI API
sai_status_t sai_init_tunnel_api();
sai_status_t sai_init_rintf_api();
sai_status_t sai_init_vrf_api();
sai_status_t sai_init_fdb_api();
sai_status_t sai_init_neigh_api();
sai_status_t sai_init_route_api();
sai_status_t sai_init_nhop_api();
sai_status_t sai_init_nhop_group_api();
#ifdef LAG_OPTION
sai_status_t sai_init_lag_api();
#endif

#endif /* __SWITCHLINK_INIT_SAI_H__ */
