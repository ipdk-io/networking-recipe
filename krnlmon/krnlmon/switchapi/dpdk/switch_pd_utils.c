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

#include "switch_pd_utils.h"

#include <net/if.h>

#include "bf_rt/bf_rt_common.h"
#include "bf_types/bf_types.h"
#include "port_mgr/dpdk/bf_dpdk_port_if.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_tdi.h"
#include "switchutils/switch_log.h"

void switch_pd_to_get_port_id(switch_api_rif_info_t* port_rif_info) {
  char if_name[16] = {0};
  int i = 0;
  bf_dev_id_t bf_dev_id = 0;
  bf_dev_port_t bf_dev_port;

  if (!if_indextoname(port_rif_info->rif_ifindex, if_name)) {
    krnlmon_log_error("Failed to get ifname for the index: %d",
                      port_rif_info->rif_ifindex);
    return;
  }

  for (i = 0; i < MAX_NO_OF_PORTS; i++) {
    struct port_info_t* port_info = NULL;
    bf_dev_port = (bf_dev_port_t)i;
    bf_pal_port_info_get(bf_dev_id, bf_dev_port, &port_info);
    if (port_info == NULL) continue;

    if (!strcmp((port_info)->port_attrib.port_name, if_name)) {
      // With multi-pipeline support, return target dp index
      // for both direction.
      krnlmon_log_debug("found the target dp index %d for sdk port id %d",
                        port_info->port_attrib.port_in_id, i);
      port_rif_info->port_id = port_info->port_attrib.port_in_id;
      if (i > CONFIG_PORT_INDEX) {
        bf_dev_port_t bf_dev_port_control = i - CONFIG_PORT_INDEX;
        port_info = NULL;
        bf_pal_port_info_get(bf_dev_id, bf_dev_port_control, &port_info);
        if (port_info == NULL) {
          krnlmon_log_error(
              "Failed to find the target dp index for "
              "physical port associated with : %s",
              if_name);
          return;
        }
        krnlmon_log_debug(
            "Found phy port target dp index %d for sdk port id %d",
            port_info->port_attrib.port_in_id, bf_dev_port_control);
        port_rif_info->phy_port_id = port_info->port_attrib.port_in_id;
      }
      return;
    }
  }

  krnlmon_log_error("Failed to find the target dp index for ifname : %s",
                    if_name);

  return;
}

tdi_status_t tdi_switch_pd_deallocate_resources(tdi_flags_hdl* flags_hdl,
                                                tdi_target_hdl* target_hdl,
                                                tdi_table_key_hdl* key_hdl,
                                                tdi_table_data_hdl* data_hdl,
                                                tdi_session_hdl* session,
                                                bool entry_type) {
  tdi_status_t status = TDI_SUCCESS;

  status = tdi_deallocate_flag(flags_hdl);

  status = tdi_deallocate_target(target_hdl);

  if (entry_type) {
    // Data handle is created only when entry is added to backend
    status = tdi_deallocate_table_data(data_hdl);
  }

  status = tdi_deallocate_table_key(key_hdl);

  status = tdi_deallocate_session(session);

  return status;
}

tdi_status_t tdi_deallocate_flag(tdi_flags_hdl* flags_hdl) {
  tdi_status_t status = TDI_SUCCESS;
  if (flags_hdl) {
    status = tdi_flags_delete(flags_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to deallocate flags handle, error: %d", status);
    }
  }
  return status;
}

tdi_status_t tdi_deallocate_target(tdi_target_hdl* target_hdl) {
  tdi_status_t status = TDI_SUCCESS;
  if (target_hdl) {
    status = tdi_target_delete(target_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to deallocate target handle, error: %d",
                        status);
    }
  }
  return status;
}

tdi_status_t tdi_deallocate_table_data(tdi_table_data_hdl* data_hdl) {
  tdi_status_t status = TDI_SUCCESS;
  if (data_hdl) {
    status = tdi_table_data_deallocate(data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Failed to deallocate data handle, error: %d", status);
    }
  }
  return status;
}

tdi_status_t tdi_deallocate_table_key(tdi_table_key_hdl* key_hdl) {
  tdi_status_t status = TDI_SUCCESS;
  if (key_hdl) {
    status = tdi_table_key_deallocate(key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Failed to deallocate key handle, error: %d", status);
    }
  }
  return status;
}

tdi_status_t tdi_deallocate_session(tdi_session_hdl* session) {
  tdi_status_t status = TDI_SUCCESS;
  if (session) {
    status = tdi_session_destroy(session);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Failed to destroy session, error: %d", status);
    }
  }
  return status;
}
