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

#include "switchlink_init_sai.h"

#include "krnlmon_options.h"
#include "sai.h"

extern sai_status_t sai_initialize(void);

/*
 * Routine Description:
 *    Initialize SAI API's
 *
 * Arguments:
 *    void
 *
 * Return Values:
 *    void
 */

void switchlink_init_api(void) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_initialize();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  status = sai_init_vrf_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_fdb_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_rintf_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_neigh_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_nhop_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_route_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_tunnel_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_init_nhop_group_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
#ifdef LAG_OPTION
  status = sai_init_lag_api();
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
#endif
  return;
}
