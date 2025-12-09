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

#include "saineighbor.h"

#include <arpa/inet.h>

#include "saiinternal.h"
#include "switchapi/switch_l3.h"
#include "switchapi/switch_neighbor.h"
#include "switchapi/switch_nhop.h"
#include "switchapi/switch_rif.h"

static void sai_neighbor_entry_to_string(
    _In_ const sai_neighbor_entry_t* neighbor_entry, _Out_ char* entry_string) {
  int count = 0;
  int entry_length = 0;
  count = snprintf(entry_string, SAI_MAX_ENTRY_STRING_LEN,
                   "neighbor:  rif %" PRIx64 " ", neighbor_entry->rif_id);
  sai_ipaddress_to_string(neighbor_entry->ip_address,
                          SAI_MAX_ENTRY_STRING_LEN - count,
                          entry_string + count, &entry_length);
  return;
}

static void sai_neighbor_entry_parse(const sai_neighbor_entry_t* neighbor_entry,
                                     switch_api_neighbor_info_t* api_neighbor) {
  api_neighbor->rif_handle = (switch_handle_t)neighbor_entry->rif_id;
  sai_ip_addr_to_switch_ip_addr(&neighbor_entry->ip_address,
                                &api_neighbor->ip_addr);
}

static void sai_neighbor_entry_attribute_parse(
    uint32_t attr_count, const sai_attribute_t* attr_list,
    switch_api_neighbor_info_t* api_neighbor) {
  const sai_attribute_t* attribute;
  uint32_t index = 0;
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
        memcpy(&api_neighbor->mac_addr, attribute->value.mac,
               sizeof(switch_mac_addr_t));
        break;
      case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
      case SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
        break;
    }
  }
}

static void sai_neighbor_entry_nexthop_get(
    switch_api_neighbor_info_t* api_neighbor) {
  switch_ip_addr_t ip_addr;
  switch_nhop_key_t nhop_key;
  memset(&ip_addr, 0, sizeof(switch_ip_addr_t));
  memset(&nhop_key, 0, sizeof(switch_nhop_key_t));
  nhop_key.handle = api_neighbor->rif_handle;
  memcpy(&nhop_key.ip_addr, &api_neighbor->ip_addr, sizeof(switch_ip_addr_t));
  switch_api_nhop_handle_get(0, &nhop_key, &api_neighbor->nhop_handle);
}

/*
 * Routine Description:
 *    Create neighbor entry
 *
 * Arguments:
 *    [in] neighbor_entry - neighbor entry
 *    [in] attr_count - number of attributes
 *    [in] attrs - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
static sai_status_t sai_create_neighbor_entry(
    _In_ const sai_neighbor_entry_t* neighbor_entry, _In_ uint32_t attr_count,
    _In_ const sai_attribute_t* attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t neighbor_handle = SWITCH_API_INVALID_HANDLE;
  char entry_string[SAI_MAX_ENTRY_STRING_LEN];
  switch_api_neighbor_info_t api_neighbor = {0};

  if (!neighbor_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null neighbor entry: %s", sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&api_neighbor, 0, sizeof(switch_api_neighbor_info_t));
  sai_neighbor_entry_attribute_parse(attr_count, attr_list, &api_neighbor);
  sai_neighbor_entry_parse(neighbor_entry, &api_neighbor);
  sai_neighbor_entry_nexthop_get(&api_neighbor);
  sai_neighbor_entry_to_string(neighbor_entry, entry_string);

  status = switch_api_neighbor_create(0, &api_neighbor, &neighbor_handle);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to create neighbor entry, error: %s",
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/*
 * Routine Description:
 *    Remove neighbor entry
 *
 * Arguments:
 *    [in] neighbor_entry - neighbor entry
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 * Note: IP address expected in Network Byte Order.
 */
static sai_status_t sai_remove_neighbor_entry(
    _In_ const sai_neighbor_entry_t* neighbor_entry) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_neighbor_info_t api_neighbor = {0};
  switch_handle_t neighbor_handle = SWITCH_API_INVALID_HANDLE;

  if (!neighbor_entry) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null neighbor entry: %s", sai_status_to_string(status));
    return status;
  }

  memset(&api_neighbor, 0, sizeof(switch_api_neighbor_info_t));
  sai_neighbor_entry_parse(neighbor_entry, &api_neighbor);
  sai_neighbor_entry_nexthop_get(&api_neighbor);

  switch_status = switch_api_neighbor_handle_get(0, api_neighbor.nhop_handle,
                                                 &neighbor_handle);

  switch_status = switch_api_neighbor_delete(0, neighbor_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to remove neighbor entry, error: %s",
                      sai_status_to_string(status));
    status = SAI_STATUS_SUCCESS;
  }

  return (sai_status_t)status;
}

/*
 *  Neighbor methods table retrieved with sai_api_query()
 */
sai_neighbor_api_t neighbor_api = {
    .create_neighbor_entry = sai_create_neighbor_entry,
    .remove_neighbor_entry = sai_remove_neighbor_entry};

sai_status_t sai_neighbor_initialize(sai_api_service_t* sai_api_service) {
  sai_api_service->neighbor_api = neighbor_api;
  return SAI_STATUS_SUCCESS;
}
