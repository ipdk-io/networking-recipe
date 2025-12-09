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

#include "switch_pd_lag.h"

#include "switchapi/es2k/switch_pd_p4_name_mapping.h"
#include "switchapi/es2k/switch_pd_p4_name_routing.h"
#include "switchapi/es2k/switch_pd_utils.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_lag.h"
#include "switchapi/switch_tdi.h"
#include "switchutils/switch_log.h"

/**
 * Routine Description:
 *   @brief Program tx_lag_table in ES2K
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_info - LAG info
 *   @param[in] entry_add - true for entry add, false
 *                          for entry delete
 *
 * Return Values:
 *    @return  TDI_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_pd_tx_lag_table_entry(switch_device_t device,
                                             const switch_lag_info_t* lag_info,
                                             bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id_lag_id = 0;
  tdi_id_t field_id_hash = 0;
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

  uint16_t lag_id = -1;
  uint8_t egress_port = -1;
  static uint32_t total_lag_list = 0;
  uint32_t lag_list = 0;
  uint8_t port_count = 0;

  switch_lag_member_info_t* lag_member = NULL;
  switch_handle_t lag_h = SWITCH_API_INVALID_HANDLE;
  switch_handle_t lag_member_h = SWITCH_API_INVALID_HANDLE;

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

  status = tdi_table_from_name_get(info_hdl, LNW_TX_LAG_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_TX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_TX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl, LNW_TX_LAG_TABLE_KEY_LAG_ID,
                                &field_id_lag_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_TX_LAG_TABLE_KEY_LAG_ID, status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_TX_LAG_TABLE_KEY_VMETA_COMMON_HASH, &field_id_hash);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_TX_LAG_TABLE_KEY_VMETA_COMMON_HASH, status);
    goto dealloc_resources;
  }

  status = tdi_action_name_to_id(
      table_info_hdl, LNW_TX_LAG_TABLE_ACTION_SET_EGRESS_PORT, &action_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                      LNW_TX_LAG_TABLE_ACTION_SET_EGRESS_PORT, status);
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
      table_info_hdl, ACTION_SET_EGRESS_PORT_PARAM_EGRESS_PORT, action_id,
      &data_field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                      ACTION_SET_EGRESS_PORT_PARAM_EGRESS_PORT, status);
    goto dealloc_resources;
  }

  lag_h = lag_info->lag_handle;
  lag_member_h = lag_info->active_lag_member;

  status = switch_lag_member_get(device, lag_member_h, &lag_member);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  status = switch_pd_get_physical_port_id(
      device, lag_member->api_lag_member_info.port_id, &egress_port);

  lag_id = lag_h & ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT);

  while ((total_lag_list < LNW_LAG_HASH_SIZE) &&
         (lag_list < LNW_LAG_PER_GROUP_HASH_SIZE)) {
    port_count = 0;

    status = tdi_key_field_set_value_and_mask(key_hdl, field_id_lag_id, lag_id,
                                              0xFF);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to set value for key ID: %d for tx_lag_table"
          ", error: %d",
          field_id_lag_id, status);
      goto dealloc_resources;
    }

    status = tdi_key_field_set_value_and_mask(key_hdl, field_id_hash,
                                              lag_list + port_count, 0x7);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to set value for key ID: %d for tx_lag_table"
          ", error: %d",
          field_id_hash, status);
      goto dealloc_resources;
    }

    if (entry_add) {
      status = tdi_data_field_set_value(data_hdl, data_field_id, egress_port);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }

      status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                   key_hdl, data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to add %s entry, error: %d", LNW_TX_LAG_TABLE,
                          status);
        goto dealloc_resources;
      }
      total_lag_list++;
    } else {
      /* Delete an entry from target */
      krnlmon_log_info("Delete tx_lag_table entry");
      status = tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl,
                                   key_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to delete %s entry, error: %d",
                          LNW_TX_LAG_TABLE, status);
        goto dealloc_resources;
      }
      total_lag_list--;
    }
    port_count++;
    lag_list += port_count;
  }
  krnlmon_log_debug("Total LAG hash entries created are: %d", total_lag_list);

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

/**
 * Routine Description:
 *   @brief Program rx_lag_table in MEV-TS
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_info - LAG info
 *   @param[in] entry_add - true for entry add, false
 *                          for entry delete
 *
 * Return Values:
 *    @return  TDI_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_pd_rx_lag_table_entry(switch_device_t device,
                                             const switch_lag_info_t* lag_info,
                                             bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id_port_id = 0;
  tdi_id_t field_id_lag_id = 0;
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

  switch_list_t lag_members;
  switch_node_t* node = NULL;
  switch_lag_member_info_t* lag_member = NULL;
  switch_handle_t lag_member_h = SWITCH_API_INVALID_HANDLE;
  uint8_t egress_port = -1;

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

  status = tdi_table_from_name_get(info_hdl, LNW_RX_LAG_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_RX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_RX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl, LNW_RX_LAG_TABLE_KEY_PORT_ID,
                                &field_id_port_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RX_LAG_TABLE_KEY_PORT_ID, status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl, LNW_RX_LAG_TABLE_KEY_LAG_ID,
                                &field_id_lag_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RX_LAG_TABLE_KEY_LAG_ID, status);
    goto dealloc_resources;
  }

  status = tdi_action_name_to_id(
      table_info_hdl, LNW_RX_LAG_TABLE_ACTION_FWD_TO_VSI, &action_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                      LNW_RX_LAG_TABLE_ACTION_FWD_TO_VSI, status);
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
                                             LNW_ACTION_FWD_TO_VSI_PARAM_PORT,
                                             action_id, &data_field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                      LNW_ACTION_FWD_TO_VSI_PARAM_PORT, status);
    goto dealloc_resources;
  }

  lag_members = (switch_list_t)lag_info->lag_members;
  FOR_EACH_IN_LIST(lag_members, node) {
    lag_member = (switch_lag_member_info_t*)node->data;
    lag_member_h = lag_member->lag_member_handle;

    status = tdi_key_field_set_value(
        key_hdl, field_id_lag_id,
        (lag_info->lag_handle &
         ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT)));
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to set value for key ID: %d for rx_lag_table"
          ", error: %d",
          field_id_lag_id, status);
      goto dealloc_resources;
    }

    switch_lag_member_info_t* lag_member_info = NULL;
    status = switch_lag_member_get(device, lag_member_h, &lag_member_info);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

    status = switch_pd_get_physical_port_id(
        device, lag_member_info->api_lag_member_info.port_id, &egress_port);

    status = tdi_key_field_set_value(key_hdl, field_id_port_id, egress_port);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to set value for key ID: %d for rx_lag_table"
          ", error: %d",
          field_id_port_id, status);
      goto dealloc_resources;
    }

    if (entry_add) {
      switch_lag_member_info_t* active_lag_member_info = NULL;
      status = switch_lag_member_get(device, lag_info->active_lag_member,
                                     &active_lag_member_info);

      status = tdi_data_field_set_value(
          data_hdl, data_field_id,
          active_lag_member_info->api_lag_member_info.port_id);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }

      status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                   key_hdl, data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to add %s entry, error: %d", LNW_RX_LAG_TABLE,
                          status);
        goto dealloc_resources;
      }
    } else {
      /* Delete an entry from target */
      krnlmon_log_info("Delete rx_lag_table entry");
      status = tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl,
                                   key_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to delete %s entry, error: %d",
                          LNW_RX_LAG_TABLE, status);
        goto dealloc_resources;
      }
    }
  }
  FOR_EACH_IN_LIST_END();

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

/**
 * Routine Description:
 *   @brief Program tx_lag_table in MEV-TS for LACP mode
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_info - LAG info
 *   @param[in] entry_add - true for entry add, false
 *                          for entry delete
 *
 * Return Values:
 *    @return  TDI_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_pd_tx_lacp_lag_table_entry(
    switch_device_t device, const switch_lag_info_t* lag_info, bool entry_add) {
  tdi_status_t status;
  tdi_id_t field_id_lag_id = 0;
  tdi_id_t field_id_hash = 0;
  tdi_id_t action_id = 0;
  tdi_id_t data_field_id = 0;
  tdi_id_t rif_data_field_id = 0;

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

  uint16_t lag_id = -1;
  uint8_t egress_port = -1;
  static uint32_t total_lag_list = 0;
  uint32_t lag_list = 0;
  uint8_t port_count = 0;

  switch_list_t lag_members;
  switch_node_t* node = NULL;
  switch_lag_member_info_t* lag_member = NULL;
  switch_handle_t lag_h = SWITCH_API_INVALID_HANDLE;
  switch_handle_t lag_member_h = SWITCH_API_INVALID_HANDLE;

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

  status = tdi_table_from_name_get(info_hdl, LNW_TX_LAG_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_TX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_TX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl, LNW_TX_LAG_TABLE_KEY_LAG_ID,
                                &field_id_lag_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_TX_LAG_TABLE_KEY_LAG_ID, status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_TX_LAG_TABLE_KEY_VMETA_COMMON_HASH, &field_id_hash);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_TX_LAG_TABLE_KEY_VMETA_COMMON_HASH, status);
    goto dealloc_resources;
  }

  status = tdi_action_name_to_id(
      table_info_hdl, LNW_TX_LAG_TABLE_ACTION_SET_EGRESS_PORT, &action_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                      LNW_TX_LAG_TABLE_ACTION_SET_EGRESS_PORT, status);
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
      table_info_hdl, ACTION_SET_EGRESS_PORT_PARAM_ROUTER_INTF_ID, action_id,
      &rif_data_field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                      ACTION_SET_EGRESS_PORT_PARAM_ROUTER_INTF_ID, status);
    goto dealloc_resources;
  }

  status = tdi_data_field_id_with_action_get(
      table_info_hdl, ACTION_SET_EGRESS_PORT_PARAM_EGRESS_PORT, action_id,
      &data_field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                      ACTION_SET_EGRESS_PORT_PARAM_EGRESS_PORT, status);
    goto dealloc_resources;
  }

  lag_h = lag_info->lag_handle;
  lag_members = (switch_list_t)lag_info->lag_members;
  lag_id = lag_h & ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT);

  while ((total_lag_list < LNW_LAG_HASH_SIZE) &&
         (lag_list < LNW_LAG_PER_GROUP_HASH_SIZE)) {
    port_count = 0;
    FOR_EACH_IN_LIST(lag_members, node) {
      lag_member = (switch_lag_member_info_t*)node->data;
      lag_member_h = lag_member->lag_member_handle;
      switch_lag_member_info_t* lag_member_info = NULL;
      status = switch_lag_member_get(device, lag_member_h, &lag_member_info);
      CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
      status = switch_pd_get_physical_port_id(
          device, lag_member_info->api_lag_member_info.port_id, &egress_port);
      status = tdi_key_field_set_value_and_mask(key_hdl, field_id_lag_id,
                                                lag_id, 0xFF);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to set value for key ID: %d for tx_lag_table"
            ", error: %d",
            field_id_lag_id, status);
        goto dealloc_resources;
      }

      status = tdi_key_field_set_value_and_mask(key_hdl, field_id_hash,
                                                lag_list + port_count, 0x7);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error(
            "Unable to set value for key ID: %d for tx_lag_table"
            ", error: %d",
            field_id_hash, status);
        goto dealloc_resources;
      }

      if (entry_add) {
        status = tdi_data_field_set_value(data_hdl, rif_data_field_id, lag_id);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                            rif_data_field_id, status);
          goto dealloc_resources;
        }

        status = tdi_data_field_set_value(data_hdl, data_field_id, egress_port);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                            data_field_id, status);
          goto dealloc_resources;
        }

        status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                     key_hdl, data_hdl);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to add %s entry, error: %d",
                            LNW_TX_LAG_TABLE, status);
          goto dealloc_resources;
        }
        total_lag_list++;
      } else {
        /* Delete an entry from target */
        krnlmon_log_info("Delete tx_lag_table entry");
        status = tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl,
                                     key_hdl);
        if (status != TDI_SUCCESS) {
          krnlmon_log_error("Unable to delete %s entry, error: %d",
                            LNW_TX_LAG_TABLE, status);
          goto dealloc_resources;
        }
        total_lag_list--;
      }
      port_count++;
    }
    FOR_EACH_IN_LIST_END();
    lag_list += port_count;
  }

  krnlmon_log_debug("Total LAG hash entries created are: %d", total_lag_list);

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

/**
 * Routine Description:
 *   @brief Program rx_lag_table in MEV-TS for LACP mode
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_info - LAG info
 *   @param[in] entry_add - true for entry add, false
 *                          for entry delete
 *
 * Return Values:
 *    @return  TDI_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_pd_rx_lacp_lag_table_entry(
    switch_device_t device, const switch_lag_info_t* lag_info, bool entry_add) {
  tdi_status_t status;

  tdi_id_t field_id_port_id = 0;
  tdi_id_t field_id_lag_id = 0;
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

  switch_list_t lag_members;
  switch_node_t* node = NULL;
  switch_lag_member_info_t* lag_member = NULL;
  switch_handle_t lag_member_h = SWITCH_API_INVALID_HANDLE;
  uint8_t egress_port = -1;

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

  status = tdi_table_from_name_get(info_hdl, LNW_RX_LAG_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_RX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_RX_LAG_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl, LNW_RX_LAG_TABLE_KEY_PORT_ID,
                                &field_id_port_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RX_LAG_TABLE_KEY_PORT_ID, status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(table_info_hdl, LNW_RX_LAG_TABLE_KEY_LAG_ID,
                                &field_id_lag_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_RX_LAG_TABLE_KEY_LAG_ID, status);
    goto dealloc_resources;
  }

  status = tdi_action_name_to_id(
      table_info_hdl, LNW_RX_LAG_TABLE_ACTION_FWD_TO_VSI, &action_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                      LNW_RX_LAG_TABLE_ACTION_FWD_TO_VSI, status);
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
                                             LNW_ACTION_FWD_TO_VSI_PARAM_PORT,
                                             action_id, &data_field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                      LNW_ACTION_FWD_TO_VSI_PARAM_PORT, status);
    goto dealloc_resources;
  }

  lag_members = (switch_list_t)lag_info->lag_members;
  FOR_EACH_IN_LIST(lag_members, node) {
    lag_member = (switch_lag_member_info_t*)node->data;
    lag_member_h = lag_member->lag_member_handle;

    status = tdi_key_field_set_value(
        key_hdl, field_id_lag_id,
        (lag_info->lag_handle &
         ~(SWITCH_HANDLE_TYPE_LAG << SWITCH_HANDLE_TYPE_SHIFT)));
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to set value for key ID: %d for rx_lag_table"
          ", error: %d",
          field_id_lag_id, status);
      goto dealloc_resources;
    }

    switch_lag_member_info_t* lag_member_info = NULL;
    status = switch_lag_member_get(device, lag_member_h, &lag_member_info);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

    status = switch_pd_get_physical_port_id(
        device, lag_member_info->api_lag_member_info.port_id, &egress_port);

    status = tdi_key_field_set_value(key_hdl, field_id_port_id, egress_port);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to set value for key ID: %d for rx_lag_table"
          ", error: %d",
          field_id_port_id, status);
      goto dealloc_resources;
    }

    if (entry_add) {
      status =
          tdi_data_field_set_value(data_hdl, data_field_id, egress_port + 16);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                          data_field_id, status);
        goto dealloc_resources;
      }

      status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                   key_hdl, data_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to add %s entry, error: %d", LNW_RX_LAG_TABLE,
                          status);
        goto dealloc_resources;
      }
    } else {
      /* Delete an entry from target */
      krnlmon_log_info("Delete rx_lag_table entry");
      status = tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl,
                                   key_hdl);
      if (status != TDI_SUCCESS) {
        krnlmon_log_error("Unable to delete %s entry, error: %d",
                          LNW_RX_LAG_TABLE, status);
        goto dealloc_resources;
      }
    }
  }
  FOR_EACH_IN_LIST_END();

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}
