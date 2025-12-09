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

#include "saifdb.h"

#include <linux/if_ether.h>

#include "saiinternal.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_fdb.h"
#include "switchapi/switch_interface.h"

static switch_l2_learn_from_t switch_type_learn_from(
    sai_l2_learn_from_t sai_learn_from_type) {
  switch (sai_learn_from_type) {
    case SAI_L2_FWD_LEARN_NONE:
      return SWITCH_L2_FWD_LEARN_NONE;
    case SAI_L2_FWD_LEARN_TUNNEL_INTERFACE:
      return SWITCH_L2_FWD_LEARN_TUNNEL_INTERFACE;
    case SAI_L2_FWD_LEARN_VLAN_INTERFACE:
      return SWITCH_L2_FWD_LEARN_VLAN_INTERFACE;
    case SAI_L2_FWD_LEARN_PHYSICAL_INTERFACE:
      return SWITCH_L2_FWD_LEARN_PHYSICAL_INTERFACE;
    case SAI_L2_FWD_LEARN_MAX:
      return SWITCH_L2_FWD_LEARN_MAX;
  }
  return SWITCH_L2_FWD_LEARN_NONE;
}

static void sai_fdb_entry_to_string(_In_ const sai_fdb_entry_t* fdb_entry,
                                    _Out_ char* entry_string) {
  snprintf(entry_string, SAI_MAX_ENTRY_STRING_LEN,
           "fdb entry mac [%02x:%02x:%02x:%02x:%02x:%02x]",
           fdb_entry->mac_address[0], fdb_entry->mac_address[1],
           fdb_entry->mac_address[2], fdb_entry->mac_address[3],
           fdb_entry->mac_address[4], fdb_entry->mac_address[5]);
}

static sai_status_t sai_fdb_entry_parse(const sai_fdb_entry_t* fdb_entry,
                                        switch_api_l2_info_t* mac_entry) {
  memcpy(mac_entry->dst_mac.mac_addr, fdb_entry->mac_address, ETH_ALEN);
  return SAI_STATUS_SUCCESS;
}

static sai_status_t sai_fdb_entry_attribute_parse(
    uint32_t attr_count, const sai_attribute_t* attr_list,
    switch_api_l2_info_t* mac_entry) {
  const sai_attribute_t* attribute;
  uint32_t i = 0;

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID:
        mac_entry->rif_handle = attribute->value.oid;
        mac_entry->port_id = attribute->value.oid;
        break;
      case SAI_FDB_ENTRY_ATTR_META_DATA:
        mac_entry->learn_from = switch_type_learn_from(attribute->value.u16);
        break;
    }
  }

  /* Reset either port_id or rif_handle based on learning
   * When Learning is via Physical interface or tunnel interface we
   * expect to have its corresponding RIF handle
   * When learning is via VLAN interface we expect to have port_id
   */
  if (mac_entry->learn_from == SWITCH_L2_FWD_LEARN_PHYSICAL_INTERFACE ||
      mac_entry->learn_from == SWITCH_L2_FWD_LEARN_TUNNEL_INTERFACE) {
    mac_entry->port_id = -1;
  } else if (mac_entry->learn_from == SWITCH_L2_FWD_LEARN_VLAN_INTERFACE) {
    mac_entry->rif_handle = -1;
  } else {
    krnlmon_log_error("Unrecognized learn from type");
    return SAI_STATUS_NOT_SUPPORTED;
  }
  return SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Create FDB entry
 *
 * Arguments:
 *    [in] fdb_entry - fdb entry
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_create_fdb_entry(
    _In_ const sai_fdb_entry_t* fdb_entry, _In_ uint32_t attr_count,
    _In_ const sai_attribute_t* attr_list) {
  switch_api_l2_info_t mac_entry;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];
  switch_handle_t mac_handle = SWITCH_API_INVALID_HANDLE;

  if (!fdb_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null fdb entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&mac_entry, 0, sizeof(mac_entry));
  sai_fdb_entry_parse(fdb_entry, &mac_entry);
  status = sai_fdb_entry_attribute_parse(attr_count, attr_list, &mac_entry);

  if (status != SAI_STATUS_SUCCESS) {
    sai_fdb_entry_to_string(fdb_entry, entry_string);
    krnlmon_log_error("Failed to create fdb entry %s, error: %s", entry_string,
                      sai_status_to_string(status));
  }

  mac_entry.type = SWITCH_L2_FWD_TX;

  switch_status = switch_api_l2_handle_get(0, &mac_entry.dst_mac, &mac_handle);
  if (mac_handle != SWITCH_API_INVALID_HANDLE) {
    krnlmon_log_debug("MAC entry already programmed");
    return sai_switch_status_to_sai_status(switch_status);
  }

  switch_status = switch_api_l2_forward_create(0, &mac_entry, &mac_handle);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    sai_fdb_entry_to_string(fdb_entry, entry_string);
    krnlmon_log_error("Failed to create fdb entry %s : error: %s", entry_string,
                      sai_status_to_string(status));
    return status;
  }

  return (sai_status_t)SAI_STATUS_SUCCESS;
}

/*
 * Routine Description:
 *    Remove FDB entry
 *
 * Arguments:
 *    [in] fdb_entry - fdb entry
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_remove_fdb_entry(
    _In_ const sai_fdb_entry_t* fdb_entry) {
  switch_api_l2_info_t mac_entry;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];

  if (!fdb_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null fdb entry: %s", sai_status_to_string(status));
    return status;
  }

  memset(&mac_entry, 0, sizeof(mac_entry));
  sai_fdb_entry_parse(fdb_entry, &mac_entry);
  mac_entry.type = SWITCH_L2_FWD_TX;

  switch_status = switch_api_l2_forward_delete(0, &mac_entry);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    sai_fdb_entry_to_string(fdb_entry, entry_string);
    krnlmon_log_error("Failed to remove fdb entry %s, error: %s", entry_string,
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/*
 *  FDB methods table retrieved with sai_api_query()
 */
sai_fdb_api_t fdb_api = {.create_fdb_entry = sai_create_fdb_entry,
                         .remove_fdb_entry = sai_remove_fdb_entry};

sai_status_t sai_fdb_initialize(sai_api_service_t* sai_api_service) {
  sai_api_service->fdb_api = fdb_api;
  return SAI_STATUS_SUCCESS;
}
