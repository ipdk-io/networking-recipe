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

#include "switch_pd_tunnel.h"

#include "switch_pd_p4_name_mapping.h"
#include "switch_pd_utils.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_tdi.h"
#include "switchapi/switch_tunnel.h"
#include "switchutils/switch_log.h"

switch_status_t switch_pd_tunnel_entry(
    switch_device_t device, const switch_api_tunnel_info_t* api_tunnel_info_t,
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
  uint32_t network_byte_order;

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

  status =
      tdi_table_from_name_get(info_hdl, LNW_VXLAN_ENCAP_MOD_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_VXLAN_ENCAP_MOD_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_VXLAN_ENCAP_MOD_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table, error: %d",
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR,
      &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR,
                      status);
    goto dealloc_resources;
  }

  status = tdi_key_field_set_value(key_hdl, field_id, 0 /*vni value*/);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d for vxlan_encap_mod_table"
        ", error: %d",
        field_id, status);
    goto dealloc_resources;
  }

  if (entry_add) {
    /* Add an entry to target */
    krnlmon_log_info(
        "Populate vxlan encap action in vxlan_encap_mod_table for "
        "tunnel interface %x",
        (unsigned int)api_tunnel_info_t->overlay_rif_handle);

    status = tdi_action_name_to_id(table_info_hdl,
                                   LNW_VXLAN_ENCAP_MOD_TABLE_ACTION_VXLAN_ENCAP,
                                   &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                        LNW_VXLAN_ENCAP_MOD_TABLE_ACTION_VXLAN_ENCAP, status);
      goto dealloc_resources;
    }

    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %s, "
          "error: %d",
          LNW_VXLAN_ENCAP_MOD_TABLE_ACTION_VXLAN_ENCAP, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_VXLAN_ENCAP_PARAM_SRC_ADDR, status);
      goto dealloc_resources;
    }

    network_byte_order = ntohl(api_tunnel_info_t->src_ip.ip.v4addr);
    status = tdi_data_field_set_value_ptr(data_hdl, data_field_id,
                                          (const uint8_t*)&network_byte_order,
                                          sizeof(uint32_t));
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_VXLAN_ENCAP_PARAM_DST_ADDR, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_VXLAN_ENCAP_PARAM_DST_ADDR, status);
      goto dealloc_resources;
    }

    network_byte_order = ntohl(api_tunnel_info_t->dst_ip.ip.v4addr);
    status = tdi_data_field_set_value_ptr(data_hdl, data_field_id,
                                          (const uint8_t*)&network_byte_order,
                                          sizeof(uint32_t));
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_VXLAN_ENCAP_PARAM_DST_PORT, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_VXLAN_ENCAP_PARAM_DST_PORT, status);
      goto dealloc_resources;
    }

    uint16_t network_byte_order_udp = ntohs(api_tunnel_info_t->udp_port);
    status = tdi_data_field_set_value_ptr(
        data_hdl, data_field_id, (const uint8_t*)&network_byte_order_udp,
        sizeof(uint16_t));
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(table_info_hdl,
                                               LNW_ACTION_VXLAN_ENCAP_PARAM_VNI,
                                               action_id, &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_VXLAN_ENCAP_PARAM_VNI, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value_ptr(data_hdl, data_field_id, 0,
                                          sizeof(uint32_t));
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d",
                        LNW_VXLAN_ENCAP_MOD_TABLE, status);
      goto dealloc_resources;
    }
  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete vxlan_encap_mod_table entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_VXLAN_ENCAP_MOD_TABLE, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}

switch_status_t switch_pd_tunnel_term_entry(
    switch_device_t device,
    const switch_api_tunnel_term_info_t* api_tunnel_term_info_t,
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
  uint32_t network_byte_order;

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

  status =
      tdi_table_from_name_get(info_hdl, LNW_IPV4_TUNNEL_TERM_TABLE, &table_hdl);
  if (status != TDI_SUCCESS || !table_hdl) {
    krnlmon_log_error("Unable to get table handle for: %s, error: %d",
                      LNW_IPV4_TUNNEL_TERM_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_key_allocate(table_hdl, &key_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to allocate key handle for: %s, error: %d",
                      LNW_IPV4_TUNNEL_TERM_TABLE, status);
    goto dealloc_resources;
  }

  status = tdi_table_info_get(table_hdl, &table_info_hdl);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get table info handle for table");
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s, error: %d",
                      LNW_IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE, status);
    goto dealloc_resources;
  }

  /* From p4 file the value expected is TUNNEL_TYPE_VXLAN=2 */
  status = tdi_key_field_set_value(key_hdl, field_id, 2);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error(
        "Unable to set value for key ID: %d of ipv4_tunnel_term_table"
        ", error: %d",
        field_id, status);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s",
                      LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC);
    goto dealloc_resources;
  }

  /* This refers to incoming packet fields, where SIP will be the remote_ip
   * configured while creating tunnel */
  network_byte_order = ntohl(api_tunnel_term_info_t->dst_ip.ip.v4addr);
  status = tdi_key_field_set_value_ptr(
      key_hdl, field_id, (const uint8_t*)&network_byte_order, sizeof(uint32_t));
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to set value for key ID: %d", field_id);
    goto dealloc_resources;
  }

  status = tdi_key_field_id_get(
      table_info_hdl, LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST, &field_id);
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to get field ID for key: %s",
                      LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST);
    goto dealloc_resources;
  }

  /* This refers to incoming packet fields, where DIP will be the local_ip
   * configured while creating tunnel */
  network_byte_order = ntohl(api_tunnel_term_info_t->src_ip.ip.v4addr);
  status = tdi_key_field_set_value_ptr(
      key_hdl, field_id, (const uint8_t*)&network_byte_order, sizeof(uint32_t));
  if (status != TDI_SUCCESS) {
    krnlmon_log_error("Unable to set value for key ID: %d", field_id);
    goto dealloc_resources;
  }

  if (entry_add) {
    krnlmon_log_info(
        "Populate decap_outer_ipv4 action in ipv4_tunnel_term_table "
        "for tunnel interface %x",
        (unsigned int)api_tunnel_term_info_t->tunnel_handle);

    status = tdi_action_name_to_id(
        table_info_hdl, LNW_IPV4_TUNNEL_TERM_TABLE_ACTION_DECAP_OUTER_IPV4,
        &action_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get action allocator ID for: %s, error: %d",
                        LNW_IPV4_TUNNEL_TERM_TABLE_ACTION_DECAP_OUTER_IPV4,
                        status);
      goto dealloc_resources;
    }

    /* Add an entry to target */
    status = tdi_table_action_data_allocate(table_hdl, action_id, &data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error(
          "Unable to get action allocator for ID: %d, "
          "error: %d",
          action_id, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_id_with_action_get(
        table_info_hdl, LNW_ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID, action_id,
        &data_field_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to get data field id param for: %s, error: %d",
                        LNW_ACTION_DECAP_OUTER_IPV4_PARAM_TUNNEL_ID, status);
      goto dealloc_resources;
    }

    status = tdi_data_field_set_value(data_hdl, data_field_id,
                                      api_tunnel_term_info_t->tunnel_id);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to set action value for ID: %d, error: %d",
                        data_field_id, status);
      goto dealloc_resources;
    }

    status = tdi_table_entry_add(table_hdl, session, target_hdl, flags_hdl,
                                 key_hdl, data_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to add %s entry, error: %d",
                        LNW_IPV4_TUNNEL_TERM_TABLE, status);
      goto dealloc_resources;
    }

  } else {
    /* Delete an entry from target */
    krnlmon_log_info("Delete ipv4_tunnel_term_table entry");
    status =
        tdi_table_entry_del(table_hdl, session, target_hdl, flags_hdl, key_hdl);
    if (status != TDI_SUCCESS) {
      krnlmon_log_error("Unable to delete %s entry, error: %d",
                        LNW_IPV4_TUNNEL_TERM_TABLE, status);
      goto dealloc_resources;
    }
  }

dealloc_resources:
  status = tdi_switch_pd_deallocate_resources(flags_hdl, target_hdl, key_hdl,
                                              data_hdl, session, entry_add);
  return switch_pd_tdi_status_to_status(status);
}
