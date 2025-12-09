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

#include "switchapi/es2k/switch_pd_routing.h"

#include "switchapi/es2k/lnw_v3/lnw_ecmp_hash_table.h"
#include "switchapi/es2k/lnw_v3/lnw_ecmp_nexthop_table.h"
#include "switchapi/es2k/lnw_v3/lnw_nexthop_table.h"
#include "switchapi/es2k/switch_pd_p4_name_mapping.h"
#include "switchapi/es2k/switch_pd_p4_name_routing.h"
#include "switchapi/es2k/switch_pd_utils.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_nhop_int.h"
#include "switchapi/switch_tdi.h"
#include "switchutils/switch_log.h"

#define MAC_BASE 0
#define MAC_LOW_BYTES 4
#define MAC_HIGH_BYTES 2
#define MAC_LOW_OFFSET MAC_BASE + MAC_HIGH_BYTES

switch_status_t switch_routing_table_entry(
    switch_device_t device, const switch_pd_routing_info_t* api_routing_info,
    bool entry_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  krnlmon_log_debug("%s", __func__);

  // update nexthop table
  status = switch_pd_nexthop_table_entry(device, api_routing_info, entry_type);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("nexthop table update failed, error: %d", status);
    return status;
  }

  // update ecmp_nexthop_table - switch_pd_ecmp_nexthop_table_entry
  status =
      switch_pd_ecmp_nexthop_table_entry(device, api_routing_info, entry_type);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(" ecmp nexthop table update failed, error: %d", status);
    return status;
  }
  return status;
}

switch_status_t switch_pd_rmac_table_entry(switch_device_t device,
                                           switch_rmac_entry_t* rmac_entry,
                                           switch_handle_t rif_handle,
                                           bool entry_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  krnlmon_log_debug("%s", __func__);

  if (!rmac_entry) {
    krnlmon_log_error("Empty router_mac entry, error: %d", status);
    return status;
  }

  // update rif mod table start
  status =
      switch_pd_rif_mod_start_entry(device, rmac_entry, rif_handle, entry_type);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("rid mod table start entry failed, error: %d", status);
    return status;
  }

  // update rif mod table mid
  status =
      switch_pd_rif_mod_mid_entry(device, rmac_entry, rif_handle, entry_type);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("rid mod table mid entry failed, error: %d", status);
    return status;
  }

  // update rif mod table last
  status =
      switch_pd_rif_mod_last_entry(device, rmac_entry, rif_handle, entry_type);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("rid mod table last entry failed, error: %d", status);
    return status;
  }

  return status;
}

switch_status_t switch_pd_nexthop_table_entry(
    switch_device_t device, const switch_pd_routing_info_t* api_nexthop_pd_info,
    bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;
  uint16_t network_byte_order_rif_id = 0;
  uint16_t lag_id = 0;

  krnlmon_log_debug("%s", __func__);

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_table_from_name_get(info_hdl, LNW_NEXTHOP_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_NEXTHOP_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_NEXTHOP_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to get table info handle for table, "
        "error: %d",
        status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl,
                                LNW_NEXTHOP_TABLE_KEY_NEXTHOP_ID, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_NEXTHOP_TABLE_KEY_NEXTHOP_ID, status);
    goto dealloc_resources;
  }

#if NEXTHOP_TABLE_TERNARY_MATCH
  // When Nexthop table is of type ternary Match
  status = tdi_key_field_set_value_and_mask(
      key_hdl, field_id,
      (api_nexthop_pd_info->nexthop_handle &
       ~(SWITCH_HANDLE_TYPE_NHOP << SWITCH_HANDLE_TYPE_SHIFT)),
      0xFFFF);
#else
  // When Nexthop table is of type exact Match
  status = tdi_key_field_set_value(
      key_hdl, field_id,
      (api_nexthop_pd_info->nexthop_handle &
       ~(SWITCH_HANDLE_TYPE_NHOP << SWITCH_HANDLE_TYPE_SHIFT)));
#endif

  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for nexthop_neigh_table,"
        " error: %d",
        field_id, status);
    goto dealloc_resources;
  }

  if (entry_add && SWITCH_RIF_HANDLE(api_nexthop_pd_info->rif_handle)) {
    /* Add an entry to target */
    // TODO Nupur: Fix the log to print rif id and dmac
    krnlmon_log_info(
        "Populate set_nexthop_neigh_info action with neighbor id: 0x%x in"
        " nexthop_neigh_table for nexthop_id 0x%x",
        (unsigned int)api_nexthop_pd_info->neighbor_handle,
        (unsigned int)api_nexthop_pd_info->nexthop_handle);

    status = tdi_action_name_to_id(
        table_info_hdl, LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP_INFO, &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator ID for: %s, "
          "error: %d",
          LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP_INFO, status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(table_info_hdl,
                                               LNW_ACTION_SET_NEXTHOP_PARAM_RIF,
                                               action_id, &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get data field id param for: %s, "
          "error: %d",
          LNW_ACTION_SET_NEXTHOP_PARAM_RIF, status);
      goto dealloc_resources;
    }

    // For ES2K we need to program RIF_id action in Big endian
    network_byte_order_rif_id =
        api_nexthop_pd_info->rif_handle &
        ~(SWITCH_HANDLE_TYPE_RIF << SWITCH_HANDLE_TYPE_SHIFT);

    status = tdi_data_field_set_value(data_hdl, data_field_id,
                                      network_byte_order_rif_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_NEXTHOP_PARAM_DMAC_HIGH, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_NEXTHOP_PARAM_DMAC_HIGH, status);
      goto dealloc_resources;
    }
    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&api_nexthop_pd_info->dst_mac_addr.mac_addr + MAC_BASE,
        MAC_HIGH_BYTES);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_NEXTHOP_PARAM_DMAC_LOW, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_NEXTHOP_PARAM_DMAC_LOW, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&api_nexthop_pd_info->dst_mac_addr.mac_addr +
            MAC_LOW_OFFSET,
        MAC_LOW_BYTES);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }
    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_NEXTHOP_PARAM_EGRESS_PORT, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_NEXTHOP_PARAM_EGRESS_PORT, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value(data_hdl, data_field_id,
                                      api_nexthop_pd_info->port_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d", LNW_NEXTHOP_TABLE,
                        status);
      goto dealloc_resources;
    }
  } else if (entry_add && SWITCH_LAG_HANDLE(api_nexthop_pd_info->rif_handle)) {
    /* Add an entry to target */
    krnlmon_log_info(
        "Populate set_nexthop_lag with neighbor id: 0x%x"
        " in nexthop_table for nexthop_id 0x%x",
        (unsigned int)api_nexthop_pd_info->neighbor_handle,
        (unsigned int)api_nexthop_pd_info->nexthop_handle);

    status = tdi_action_name_to_id(
        table_info_hdl, LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP_LAG, &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator ID for: %s, "
          "error: %d",
          LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP_LAG, status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    lag_id = api_nexthop_pd_info->rif_handle &
             ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT);

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_NEXTHOP_LAG_PARAM_DMAC_HIGH, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_NEXTHOP_LAG_PARAM_DMAC_HIGH, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&api_nexthop_pd_info->dst_mac_addr.mac_addr + MAC_BASE,
        MAC_HIGH_BYTES);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_NEXTHOP_LAG_PARAM_DMAC_LOW, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_NEXTHOP_LAG_PARAM_DMAC_LOW, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&api_nexthop_pd_info->dst_mac_addr.mac_addr +
            MAC_LOW_OFFSET,
        MAC_LOW_BYTES);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_NEXTHOP_LAG_PARAM_LAG_ID, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_NEXTHOP_LAG_PARAM_LAG_ID, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value(data_hdl, data_field_id, lag_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d", LNW_NEXTHOP_TABLE,
                        status);
      goto dealloc_resources;
    }
  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete nexthop_table entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_NEXTHOP_TABLE, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl, data_hdl,
                                     session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_ecmp_nexthop_table_entry(
    switch_device_t device, const switch_pd_routing_info_t* api_nexthop_pd_info,
    bool entry_add) {
  // New function in lnw_v3
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;
  uint16_t network_byte_order_rif_id = 0;

  krnlmon_log_debug("%s", __func__);

  if (SWITCH_LAG_HANDLE(api_nexthop_pd_info->rif_handle)) {
    // ECMP and LAG are mutually exclusive, we don't need to handle LAG case.
    return SWITCH_STATUS_SUCCESS;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }
  status =
      tdi_table_from_name_get(info_hdl, LNW_ECMP_NEXTHOP_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_NEXTHOP_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_NEXTHOP_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to get table info handle for table, "
        "error: %d",
        status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_ECMP_NEXTHOP_TABLE_KEY_ECMP_NEXTHOP_ID, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_ECMP_NEXTHOP_TABLE_KEY_ECMP_NEXTHOP_ID, status);
    goto dealloc_resources;
  }

  //  ECMP Nexthop table is of type ternary
  status = tdi_key_field_set_value_and_mask(
      key_hdl, field_id,
      (api_nexthop_pd_info->nexthop_handle &
       ~(SWITCH_HANDLE_TYPE_NHOP << SWITCH_HANDLE_TYPE_SHIFT)),
      0xFFFF);

  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for ecmp_nexthop_table,"
        " error: %d",
        field_id, status);
    goto dealloc_resources;
  }

  if (entry_add && SWITCH_RIF_HANDLE(api_nexthop_pd_info->rif_handle)) {
    /* Add an entry to target */
    // TODO Nupur: Fix the log to print rif id and dmac
    krnlmon_log_info(
        "Populate set_nexthop_neigh_info action with neighbor id: 0x%x in"
        " nexthop_neigh_table for nexthop_id 0x%x",
        (unsigned int)api_nexthop_pd_info->neighbor_handle,
        (unsigned int)api_nexthop_pd_info->nexthop_handle);

    status = tdi_action_name_to_id(
        table_info_hdl,
        LNW_ECMP_NEXTHOP_TABLE_ACTION_SET_ECMP_NEXTHOP_INFO_DMAC, &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator ID for: %s, "
          "error: %d",
          LNW_ECMP_NEXTHOP_TABLE_ACTION_SET_ECMP_NEXTHOP_INFO_DMAC, status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_RIF, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get data field id param for: %s, "
          "error: %d",
          LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_RIF, status);
      goto dealloc_resources;
    }

    // For ES2K we need to program RIF_id action in Big endian
    network_byte_order_rif_id =
        api_nexthop_pd_info->rif_handle &
        ~(SWITCH_HANDLE_TYPE_RIF << SWITCH_HANDLE_TYPE_SHIFT);

    status = tdi_data_field_set_value(data_hdl, data_field_id,
                                      network_byte_order_rif_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_DMAC_HIGH, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_DMAC_HIGH, status);
      goto dealloc_resources;
    }
    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&api_nexthop_pd_info->dst_mac_addr.mac_addr + MAC_BASE,
        MAC_HIGH_BYTES);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_DMAC_LOW, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_DMAC_LOW, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&api_nexthop_pd_info->dst_mac_addr.mac_addr +
            MAC_LOW_OFFSET,
        MAC_LOW_BYTES);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }
    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_EGRESS_PORT,
        action_id, &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_ECMP_NEXTHOP_PARAM_EGRESS_PORT, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value(data_hdl, data_field_id,
                                      api_nexthop_pd_info->port_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d",
                        LNW_ECMP_NEXTHOP_TABLE, status);
      goto dealloc_resources;
    }

  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete nexthop_table entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_ECMP_NEXTHOP_TABLE, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl, data_hdl,
                                     session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_rif_mod_start_entry(switch_device_t device,
                                              switch_rmac_entry_t* rmac_entry,
                                              switch_handle_t rif_handle,
                                              bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;

  krnlmon_log_debug("%s", __func__);

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status =
      tdi_table_from_name_get(info_hdl, LNW_RIF_MOD_TABLE_START, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_RIF_MOD_TABLE_START, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_RIF_MOD_TABLE_START, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_RIF_MOD_TABLE_START_KEY_RIF_MOD_MAP_ID0, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RIF_MOD_TABLE_START_KEY_RIF_MOD_MAP_ID0, status);
    goto dealloc_resources;
  }

  if (SWITCH_RIF_HANDLE(rif_handle)) {
    status = tdi_key_field_set_value(
        key_hdl, field_id,
        (rif_handle & ~(SWITCH_HANDLE_TYPE_RIF << SWITCH_HANDLE_TYPE_SHIFT)));
  } else if (SWITCH_LAG_HANDLE(rif_handle)) {
    status = tdi_key_field_set_value(
        key_hdl, field_id,
        (rif_handle & ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT)));
  } else {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for rif_mod_table_start", field_id);
    goto dealloc_resources;
  }

  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for rif_mod_table_start", field_id);
    goto dealloc_resources;
  }

  if (entry_add) {
    /* Add an entry to target */
    krnlmon_log_info(
        "Populate set_src_mac_start action in rif_mod_table_start");

    status = tdi_action_name_to_id(
        table_info_hdl, LNW_RIF_MOD_TABLE_START_ACTION_SET_SRC_MAC_START,
        &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                        LNW_RIF_MOD_TABLE_START_ACTION_SET_SRC_MAC_START,
                        status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&rmac_entry->mac.mac_addr + RMAC_START_OFFSET,
        RMAC_BYTES_OFFSET);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d",
                        LNW_RIF_MOD_TABLE_START, status);
      goto dealloc_resources;
    }
  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete rif_mod_table_start entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_RIF_MOD_TABLE_START, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_rif_mod_mid_entry(switch_device_t device,
                                            switch_rmac_entry_t* rmac_entry,
                                            switch_handle_t rif_handle,
                                            bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;

  krnlmon_log_debug("%s", __func__);

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_table_from_name_get(info_hdl, LNW_RIF_MOD_TABLE_MID, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_RIF_MOD_TABLE_MID, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_RIF_MOD_TABLE_MID, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_RIF_MOD_TABLE_MID_KEY_RIF_MOD_MAP_ID1, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RIF_MOD_TABLE_MID_KEY_RIF_MOD_MAP_ID1, status);
    goto dealloc_resources;
  }

  if (SWITCH_RIF_HANDLE(rif_handle)) {
    status = tdi_key_field_set_value(
        key_hdl, field_id,
        (rif_handle & ~(SWITCH_HANDLE_TYPE_RIF << SWITCH_HANDLE_TYPE_SHIFT)));
  } else if (SWITCH_LAG_HANDLE(rif_handle)) {
    status = tdi_key_field_set_value(
        key_hdl, field_id,
        (rif_handle & ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT)));
  } else {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for rif_mod_table_mid", field_id);
    goto dealloc_resources;
  }

  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for rif_mod_table_mid", field_id);
    goto dealloc_resources;
  }

  if (entry_add) {
    /* Add an entry to target */
    krnlmon_log_info("Populate set_src_mac_start action in rif_mod_table_mid");

    status = tdi_action_name_to_id(table_info_hdl,
                                   LNW_RIF_MOD_TABLE_MID_ACTION_SET_SRC_MAC_MID,
                                   &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                        LNW_RIF_MOD_TABLE_MID_ACTION_SET_SRC_MAC_MID, status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&rmac_entry->mac.mac_addr + RMAC_MID_OFFSET,
        RMAC_BYTES_OFFSET);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d",
                        LNW_RIF_MOD_TABLE_MID, status);
      goto dealloc_resources;
    }
  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete rif_mod_table_start entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_RIF_MOD_TABLE_MID, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_rif_mod_last_entry(switch_device_t device,
                                             switch_rmac_entry_t* rmac_entry,
                                             switch_handle_t rif_handle,
                                             bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;

  krnlmon_log_debug("%s", __func__);

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status =
      tdi_table_from_name_get(info_hdl, LNW_RIF_MOD_TABLE_LAST, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_RIF_MOD_TABLE_LAST, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_RIF_MOD_TABLE_LAST, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_RIF_MOD_TABLE_LAST_KEY_RIF_MOD_MAP_ID2, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RIF_MOD_TABLE_LAST_KEY_RIF_MOD_MAP_ID2, status);
    goto dealloc_resources;
  }

  if (SWITCH_RIF_HANDLE(rif_handle)) {
    status = tdi_key_field_set_value(
        key_hdl, field_id,
        (rif_handle & ~(SWITCH_HANDLE_TYPE_RIF << SWITCH_HANDLE_TYPE_SHIFT)));
  } else if (SWITCH_LAG_HANDLE(rif_handle)) {
    status = tdi_key_field_set_value(
        key_hdl, field_id,
        (rif_handle & ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT)));
  } else {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for rif_mod_table_last", field_id);
    goto dealloc_resources;
  }

  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for rif_mod_table_last", field_id);
    goto dealloc_resources;
  }

  if (entry_add) {
    /* Add an entry to target */
    krnlmon_log_info("Populate set_src_mac_start action in rif_mod_table_last");

    status = tdi_action_name_to_id(
        table_info_hdl, LNW_RIF_MOD_TABLE_LAST_ACTION_SET_SRC_MAC_LAST,
        &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                        LNW_RIF_MOD_TABLE_LAST_ACTION_SET_SRC_MAC_LAST, status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_SET_SRC_MAC_PARAM_SRC_MAC_ADDR, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id,
        (const uint8_t*)&rmac_entry->mac.mac_addr + RMAC_LAST_OFFSET,
        RMAC_BYTES_OFFSET);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d",
                        LNW_RIF_MOD_TABLE_LAST, status);
      goto dealloc_resources;
    }

  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete rif_mod_table_start entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_RIF_MOD_TABLE_LAST, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_ipv6_table_entry(
    switch_device_t device, const switch_api_route_entry_t* api_route_entry,
    bool entry_add, switch_ip_table_action_t action) {
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;
  uint32_t network_byte_order = 0;

  krnlmon_log_debug("%s", __func__);

  if (action == SWITCH_ACTION_NONE && entry_add) {
    krnlmon_log_debug("Ignore NONE action for ipv6 table entry addition");
    return SWITCH_STATUS_SUCCESS;
  }

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_table_from_name_get(info_hdl, LNW_IPV6_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_IPV6_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_IPV6_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl,
                                LNW_IPV6_TABLE_KEY_IPV6_DST_MATCH, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_IPV6_TABLE_KEY_IPV6_DST_MATCH, status);
    goto dealloc_resources;
  }

  /* Use LPM API for LPM match type*/
  status = tdi_key_field_set_value_lpm_ptr(
      key_hdl, field_id,
      (const uint8_t*)&api_route_entry->ip_address.ip.v6addr.u.addr8,
      (const uint16_t)api_route_entry->ip_address.prefix_len, 16);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for ipv6_table, error: %d",
        field_id, status);
    goto dealloc_resources;
  }

  if (entry_add) {
    if (action == SWITCH_ACTION_NHOP) {
      krnlmon_log_info(
          "Populate set_nexthop_id action in ipv6_table for "
          "route handle %x",
          (unsigned int)api_route_entry->route_handle);

      status = tdi_action_name_to_id(table_info_hdl,
                                     LNW_IPV6_TABLE_ACTION_IPV6_SET_NEXTHOP_ID,
                                     &action_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator ID for: %s, error: %d",
            LNW_IPV6_TABLE_ACTION_IPV6_SET_NEXTHOP_ID, status);
        goto dealloc_resources;
      }

      status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator for ID: %d, "
            "error: %d",
            action_id, status);
        goto dealloc_resources;
      }

      status = tdi_data_field_id_with_action_get(
          table_info_hdl, LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID, action_id,
          &data_field_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get data field id param for: %s, error: %d",
            LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID, status);
        goto dealloc_resources;
      }

      network_byte_order =
          api_route_entry->nhop_handle &
          ~(SWITCH_HANDLE_TYPE_NHOP << SWITCH_HANDLE_TYPE_SHIFT);

      status =
          tdi_data_field_set_value(data_hdl, data_field_id, network_byte_order);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }
    }

    if (action == SWITCH_ACTION_NHOP_GROUP) {
      status = tdi_action_name_to_id(table_info_hdl,
                                     LNW_IPV6_TABLE_ACTION_ECMP_V6_HASH_ACTION,
                                     &action_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator ID for: %s, error: %d",
            LNW_IPV6_TABLE_ACTION_ECMP_V6_HASH_ACTION, status);
        goto dealloc_resources;
      }

      status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator for ID: %d, "
            "error: %d",
            action_id, status);
        goto dealloc_resources;
      }

      status = tdi_data_field_id_with_action_get(
          table_info_hdl, LNW_ACTION_ECMP_HASH_ACTION_PARAM_ECMP_GROUP_ID,
          action_id, &data_field_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get data field id param for: %s, error: %d",
            LNW_ACTION_ECMP_HASH_ACTION_PARAM_ECMP_GROUP_ID, status);
        goto dealloc_resources;
      }

      status = tdi_data_field_set_value(
          data_hdl, data_field_id,
          (api_route_entry->nhop_handle &
           ~(SWITCH_HANDLE_TYPE_NHOP_GROUP << SWITCH_HANDLE_TYPE_SHIFT)));
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d", LNW_IPV6_TABLE,
                        status);
      goto dealloc_resources;
    }
  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete ipv6_table entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d", LNW_IPV6_TABLE,
                        status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_ipv4_table_entry(
    switch_device_t device, const switch_api_route_entry_t* api_route_entry,
    bool entry_add, switch_ip_table_action_t action) {
  tdi_status_t status;

  tdi_id_t field_id = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;
  uint32_t network_byte_order = 0;

  krnlmon_log_debug("%s", __func__);

  if (action == SWITCH_ACTION_NONE && entry_add) {
    krnlmon_log_debug("Ignore NONE action for ipv4 table entry addition");
    return SWITCH_STATUS_SUCCESS;
  }

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_table_from_name_get(info_hdl, LNW_IPV4_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_IPV4_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_IPV4_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl,
                                LNW_IPV4_TABLE_KEY_IPV4_DST_MATCH, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_IPV4_TABLE_KEY_IPV4_DST_MATCH, status);
    goto dealloc_resources;
  }

  /* Use LPM API for LPM match type*/
  network_byte_order = ntohl(api_route_entry->ip_address.ip.v4addr);
  status = tdi_key_field_set_value_lpm_ptr(
      key_hdl, field_id, (const uint8_t*)&network_byte_order,
      (const uint16_t)api_route_entry->ip_address.prefix_len, sizeof(uint32_t));
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for ipv4_table, error: %d",
        field_id, status);
    goto dealloc_resources;
  }

  if (entry_add) {
    if (action == SWITCH_ACTION_NHOP) {
      krnlmon_log_info(
          "Populate set_nexthop_id action in ipv4_table for "
          "route handle %x",
          (unsigned int)api_route_entry->route_handle);

      status = tdi_action_name_to_id(table_info_hdl,
                                     LNW_IPV4_TABLE_ACTION_IPV4_SET_NEXTHOP_ID,
                                     &action_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator ID for: %s, error: %d",
            LNW_IPV4_TABLE_ACTION_IPV4_SET_NEXTHOP_ID, status);
        goto dealloc_resources;
      }

      status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator for ID: %d, "
            "error: %d",
            action_id, status);
        goto dealloc_resources;
      }

      status = tdi_data_field_id_with_action_get(
          table_info_hdl, LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID, action_id,
          &data_field_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get data field id param for: %s, error: %d",
            LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID, status);
        goto dealloc_resources;
      }

      network_byte_order =
          api_route_entry->nhop_handle &
          ~(SWITCH_HANDLE_TYPE_NHOP << SWITCH_HANDLE_TYPE_SHIFT);

      status =
          tdi_data_field_set_value(data_hdl, data_field_id, network_byte_order);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }
    }

    if (action == SWITCH_ACTION_NHOP_GROUP) {
      status = tdi_action_name_to_id(
          table_info_hdl, LNW_IPV4_TABLE_ACTION_ECMP_HASH_ACTION, &action_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator ID for: %s, error: %d",
            LNW_IPV4_TABLE_ACTION_ECMP_HASH_ACTION, status);
        goto dealloc_resources;
      }

      status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get action allocator for ID: %d, "
            "error: %d",
            action_id, status);
        goto dealloc_resources;
      }

      status = tdi_data_field_id_with_action_get(
          table_info_hdl, LNW_ACTION_ECMP_HASH_ACTION_PARAM_ECMP_GROUP_ID,
          action_id, &data_field_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to get data field id param for: %s, error: %d",
            LNW_ACTION_ECMP_HASH_ACTION_PARAM_ECMP_GROUP_ID, status);
        goto dealloc_resources;
      }

      status = tdi_data_field_set_value(
          data_hdl, data_field_id,
          (api_route_entry->nhop_handle &
           ~(SWITCH_HANDLE_TYPE_NHOP_GROUP << SWITCH_HANDLE_TYPE_SHIFT)));
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d", LNW_IPV4_TABLE,
                        status);
      goto dealloc_resources;
    }
  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete ipv4_table entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d", LNW_IPV4_TABLE,
                        status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_ecmp_hash_table_entry(
    switch_device_t device, const switch_nhop_group_info_t* ecmp_info,
    bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id_group_id = 0;
  tdi_id_t field_id_meta_bit32_zero = 0;
  tdi_id_t field_id_meta_common_hash = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;

  tdi_dev_id_t dev_id = device;

  tdi_flags_hdl* flags_hdl = NULL;
  tdi_target_hdl* target_hdl = NULL;
  const tdi_device_hdl* dev_hdl = NULL;
  tdi_session_hdl* session = NULL;
  const tdi_info_hdl* info_hdl = NULL;
  tdi_table_key_hdl* key_hdl = NULL;
  tdi_table_data_hdl* data_hdl = NULL;
  const tdi_table_hdl* table_hdl = NULL;
  const tdi_table_info_hdl* table_info_hdl = NULL;

  uint16_t network_byte_order_ecmp_id = -1;
  uint32_t network_byte_order = -1;
  static uint32_t total_ecmp_list = 0;
  switch_list_t nhop_members;
  uint32_t ecmp_list = 0;
  uint8_t nhop_count = 0;

  switch_node_t* node = NULL;
  switch_nhop_member_t* ecmp_member = NULL;
  switch_handle_t ecmp_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;

  krnlmon_log_debug("%s", __func__);

  status = tdi_flags_create(0, &flags_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create flags handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_device_get(dev_id, &dev_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get device handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_target_create(dev_hdl, &target_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create target handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_session_create(dev_hdl, &session);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to create tdi session, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_info_get(dev_id, PROGRAM_NAME, &info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Failed to get tdi info handle, error: %d", status);
    goto dealloc_resources;
  }

  status = tdi_table_from_name_get(info_hdl, LNW_ECMP_HASH_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_ECMP_HASH_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_ECMP_HASH_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl,
                                LNW_ECMP_HASH_TABLE_KEY_HOST_INFO_TX_EXT_FLEX,
                                &field_id_group_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_ECMP_HASH_TABLE_KEY_HOST_INFO_TX_EXT_FLEX, status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl,
                                LNW_ECMP_HASH_TABLE_KEY_META_COMMON_HASH,
                                &field_id_meta_common_hash);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_ECMP_HASH_TABLE_KEY_META_COMMON_HASH, status);
    goto dealloc_resources;
  }

  status = tdi_action_name_to_id(
      table_info_hdl, LNW_ECMP_HASH_TABLE_ACTION_SET_NEXTHOP_ID, &action_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                      LNW_ECMP_HASH_TABLE_ACTION_SET_NEXTHOP_ID, status);
    goto dealloc_resources;
  }

  status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to get action allocator for ID: %d, "
        "error: %d",
        action_id, status);
    goto dealloc_resources;
  }

  status = tdi_data_field_id_with_action_get(
      table_info_hdl, LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID, action_id,
      &data_field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                      LNW_ACTION_SET_NEXTHOP_ID_PARAM_NEXTHOP_ID, status);
    goto dealloc_resources;
  }

  ecmp_handle = ecmp_info->nhop_group_handle;
  nhop_members = (switch_list_t)ecmp_info->members;

  // For ES2K program ECMP Group ID in host byte order
  network_byte_order_ecmp_id = ecmp_handle & ~(SWITCH_HANDLE_TYPE_NHOP_GROUP
                                               << SWITCH_HANDLE_TYPE_SHIFT);

  while ((total_ecmp_list < LNW_ECMP_HASH_SIZE) &&
         (ecmp_list < LNW_ECMP_PER_GROUP_HASH_SIZE)) {
    nhop_count = 0;
    FOR_EACH_IN_LIST(nhop_members, node) {
      ecmp_member = (switch_nhop_member_t*)node->data;
      nhop_handle = ecmp_member->nhop_handle;

      /* Mask for ECMP group ID is 16 bits */
      status = tdi_key_field_set_value_and_mask(
          key_hdl, field_id_group_id, network_byte_order_ecmp_id, 0xFFFF);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to set value for key ID: %d for ecmp_hash_table"
            ", error: %d",
            field_id_group_id, status);
        goto dealloc_resources;
      }

      /* Mask for ECMP hash value is 3 bits */
      status = tdi_key_field_set_value_and_mask(
          key_hdl, field_id_meta_common_hash, ecmp_list + nhop_count, 0x7);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to set value for key ID: %d for ecmp_hash_table"
            ", error: %d",
            field_id_meta_common_hash, status);
        goto dealloc_resources;
      }

      if (entry_add) {
        network_byte_order = nhop_handle & ~(SWITCH_HANDLE_TYPE_NHOP
                                             << SWITCH_HANDLE_TYPE_SHIFT);

        status = tdi_data_field_set_value(data_hdl, data_field_id,
                                          network_byte_order);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                            data_field_id, status);
          goto dealloc_resources;
        }

        status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                     key_hdl, data_hdl);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to add %s entry, error: %d",
                            LNW_ECMP_HASH_TABLE, status);
          goto dealloc_resources;
        }
        total_ecmp_list++;
      } else {
        /* Delete an entry from target */
        krnlmon_log_info("Delete ecmp_hash_table entry");
        status = tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl,
                                     key_hdl);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to delete %s entry, error: %d",
                            LNW_ECMP_HASH_TABLE, status);
          goto dealloc_resources;
        }
        total_ecmp_list--;
      }
      nhop_count++;
    }
    FOR_EACH_IN_LIST_END();
    ecmp_list += nhop_count;
  }
  krnlmon_log_debug("Total ECMP hash entries created are: %d", total_ecmp_list);

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}
