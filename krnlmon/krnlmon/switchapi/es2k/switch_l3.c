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

#include "switchapi/switch_l3.h"

#include "switchapi/switch_internal.h"
#include "switchapi/switch_nhop_int.h"
#include "switchapi/switch_pd_routing.h"
#include "switchutils/switch_log.h"

switch_status_t switch_route_table_entry_key_init(void* args,
                                                  switch_uint8_t* key,
                                                  switch_uint32_t* len) {
  switch_route_entry_t* route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *len = 0;
  route_entry = (switch_route_entry_t*)args;

  SWITCH_MEMCPY(key, &route_entry->vrf_handle, sizeof(switch_handle_t));
  *len += sizeof(switch_handle_t);

  SWITCH_MEMCPY((key + *len), &route_entry->ip, sizeof(switch_ip_addr_t));
  *len += sizeof(switch_ip_addr_t);

  SWITCH_MEMCPY((key + *len), &route_entry->neighbor_installed, sizeof(bool));
  *len += sizeof(bool);

  SWITCH_ASSERT(*len == SWITCH_ROUTE_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_route_entry_hash_compare(const void* key1,
                                               const void* key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_ROUTE_HASH_KEY_SIZE);
}

switch_status_t switch_l3_init(switch_device_t device) {
  switch_l3_context_t* l3_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  l3_ctx = SWITCH_MALLOC(device, sizeof(switch_l3_context_t), 0x1);
  if (!l3_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error(
        "l3 init: Failed to allocate memory for switch_l3_context_t "
        "on device %d ,error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status =
      switch_device_api_context_set(device, SWITCH_API_TYPE_L3, (void*)l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 init: Failed to set device context device %d "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  l3_ctx->route_hashtable.size = IPV4_TABLE_SIZE;
  l3_ctx->route_hashtable.compare_func = switch_route_entry_hash_compare;
  l3_ctx->route_hashtable.key_func = switch_route_table_entry_key_init;
  l3_ctx->route_hashtable.hash_seed = SWITCH_ROUTE_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&l3_ctx->route_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 init: Failed to init hashtable on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_ROUTE,
                                   IPV4_TABLE_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 init: Failed to init SWITCH_HANDLE_TYPE_ROUTE on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_l3_free(switch_device_t device) {
  switch_l3_context_t* l3_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_L3,
                                         (void**)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 free: Failed to get device context on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  status = SWITCH_HASHTABLE_DONE(&l3_ctx->route_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 free: SWITCH_HASHTABLE_DONE failed on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_ROUTE);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 free: Failed to free SWITCH_HANDLE_TYPE_ROUTE on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
  }

  SWITCH_FREE(device, l3_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_L3, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_route_table_hash_lookup(
    switch_device_t device, switch_route_entry_t* route_entry,
    switch_handle_t* route_handle) {
  switch_l3_context_t* l3_ctx = NULL;
  switch_route_info_t* route_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "route table lookup failed on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_L3,
                                         (void**)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route table lookup: Failed to get device context on device %d: "
        ",error: %s\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_SEARCH(&l3_ctx->route_hashtable, (void*)route_entry,
                                   (void**)&route_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    *route_handle = route_info->api_route_info.route_handle;
  }

  return status;
}

switch_status_t switch_route_hashtable_insert(switch_device_t device,
                                              switch_handle_t route_handle) {
  switch_l3_context_t* l3_ctx = NULL;
  switch_route_info_t* route_info = NULL;
  switch_route_entry_t* route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "route hashtable insert failed on device %d "
        ",route handle 0x%lx, error: %s\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route hashtable insert: Failed to get route info on device %d "
        ",route handle 0x%lx, error: %s\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_L3,
                                         (void**)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route hashtable insert failed on device %d "
        "route handle 0x%lx: l3 context get failed(%s)\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_HASHTABLE_INSERT(&l3_ctx->route_hashtable, &((route_info)->node),
                              (void*)route_entry, (void*)(route_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route hashtable insert failed on device %d "
        "route handle 0x%lx: hashtable insert failed(%s)\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_route_hashtable_remove(switch_device_t device,
                                              switch_handle_t route_handle) {
  switch_l3_context_t* l3_ctx = NULL;
  switch_route_info_t* route_info = NULL;
  switch_route_entry_t* route_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_ROUTE_HANDLE(route_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: route handle invalid(%s)\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: route get failed(%s)\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  route_entry = &route_info->route_entry;

  status = switch_device_api_context_get(device, SWITCH_API_TYPE_L3,
                                         (void**)&l3_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: l3 context get failed(%s)\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_DELETE(&l3_ctx->route_hashtable, (void*)route_entry,
                                   (void**)&route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "route hashtable delete failed on device %d "
        "route handle 0x%lx: l3 hashtable delete failed(%s)\n",
        device, route_handle, switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_l3_route_add(
    switch_device_t device, switch_api_route_entry_t* api_route_entry) {
  switch_handle_t nhop_group_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_route_info_t* route_info = NULL;
  switch_nhop_group_info_t* nhop_group_info = NULL;
  switch_route_entry_t route_entry;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;

  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "l3 route table add failed on device %d "
        "parameters invalid(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  vrf_handle = api_route_entry->vrf_handle;
  if (!SWITCH_VRF_HANDLE(vrf_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx "
        "vrf handle invalid(%s)\n",
        device, vrf_handle, switch_error_to_string(status));
    return status;
  }

  memset(&route_entry, 0, sizeof(route_entry));
  route_entry.vrf_handle = vrf_handle;
  SWITCH_MEMCPY(&route_entry.ip, &api_route_entry->ip_address,
                sizeof(switch_ip_addr_t));

  status = switch_route_table_hash_lookup(device, &route_entry, &route_handle);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    krnlmon_log_error(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx "
        "route table lookup failed(%s)\n",
        device, vrf_handle, switch_error_to_string(status));
    return status;
  }

  handle = switch_route_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    krnlmon_log_error(
        "l3 route table add failed on device %d "
        "route handle create failed(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 route table add failed on device %d "
        "route get failed(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  switch_ip_table_action_t ip_table_action = SWITCH_ACTION_NONE;

  if (switch_handle_type_get(api_route_entry->nhop_handle) ==
      SWITCH_HANDLE_TYPE_NHOP) {
    ip_table_action = SWITCH_ACTION_NHOP;
  } else if (switch_handle_type_get(api_route_entry->nhop_handle) ==
             SWITCH_HANDLE_TYPE_NHOP_GROUP) {
    ip_table_action = SWITCH_ACTION_NHOP_GROUP;
    nhop_group_handle = api_route_entry->nhop_handle;

    status = switch_nhop_get_group(device, nhop_group_handle, &nhop_group_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to get ecmp info on device: %d, handle: 0x%lx, error: %s",
          device, nhop_group_handle, switch_error_to_string(status));
      return status;
    }

    /* As action is ECMP, program ECMP hash table as well */
    status = switch_pd_ecmp_hash_table_entry(device, nhop_group_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to update ECMP hash table for NHOP group "
          "action, error: %s\n",
          switch_error_to_string(status));
      return status;
    }
  }

  if (api_route_entry->ip_address.type == SWITCH_API_IP_ADDR_V4) {
    /* Configure IPv4_table */
    status = switch_pd_ipv4_table_entry(device, api_route_entry, true,
                                        ip_table_action);
  } else if (api_route_entry->ip_address.type == SWITCH_API_IP_ADDR_V6) {
    /* Configure IPv6_table */
    status = switch_pd_ipv6_table_entry(device, api_route_entry, true,
                                        ip_table_action);
  } else {
    krnlmon_log_error("Invalid IP address type: %d",
                      api_route_entry->ip_address.type);
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to update IP table for NHOP action: %d,"
        "error: %s\n",
        ip_table_action, switch_error_to_string(status));
    return status;
  }

  api_route_entry->route_handle = handle;
  SWITCH_MEMCPY(&route_info->api_route_info, api_route_entry,
                sizeof(switch_api_route_entry_t));
  route_info->route_entry.vrf_handle = vrf_handle;
  route_info->nhop_handle = api_route_entry->nhop_handle;
  SWITCH_MEMCPY(&route_info->route_entry.ip, &api_route_entry->ip_address,
                sizeof(switch_ip_addr_t));

  status = switch_route_hashtable_insert(device, handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 route table add failed on device %d "
        "vrf handle 0x%lx "
        "route table insert failed(%s)\n",
        device, vrf_handle, switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_l3_delete_route(
    switch_device_t device, switch_api_route_entry_t* api_route_entry) {
  switch_route_entry_t route_entry;
  switch_nhop_group_info_t* nhop_group_info = NULL;
  switch_route_info_t* route_info = NULL;
  switch_api_route_entry_t api_route_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_handle_t nhop_group_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t route_handle = SWITCH_API_INVALID_HANDLE;

  if (!api_route_entry) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "l3 route table delete failed on device %d "
        "parameters invalid(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  route_entry.vrf_handle = api_route_entry->vrf_handle;
  SWITCH_MEMCPY(&route_entry.ip, &api_route_entry->ip_address,
                sizeof(switch_ip_addr_t));
  route_entry.neighbor_installed = api_route_entry->neighbor_installed;

  status = switch_route_table_hash_lookup(device, &route_entry, &route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 route table delete failed on device %d "
        "route entry hash find failed(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_route_get(device, route_handle, &route_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 route table delete failed on device %d "
        "route get failed(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  api_route_info = route_info->api_route_info;
  if (route_info->nhop_handle) {
    if (switch_handle_type_get(api_route_info.nhop_handle) ==
        SWITCH_HANDLE_TYPE_NHOP_GROUP) {
      nhop_group_handle = api_route_info.nhop_handle;
      status =
          switch_nhop_get_group(device, nhop_group_handle, &nhop_group_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "nhop_group info get failed on device: %d, nhop_group handle: "
            "0x%lx, nhop_group get Failed:(%s)\n",
            device, nhop_group_handle, switch_error_to_string(status));
        return status;
      }

      status = switch_pd_ecmp_hash_table_entry(device, nhop_group_info, false);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to delete ECMP hash table for NHOP group"
            " action");
        return status;
      }
    }

    if (api_route_info.ip_address.type == SWITCH_API_IP_ADDR_V4) {
      /* Delete IPv4_table entry */
      status = switch_pd_ipv4_table_entry(device, &api_route_info, false,
                                          SWITCH_ACTION_NONE);
    } else if (api_route_info.ip_address.type == SWITCH_API_IP_ADDR_V6) {
      /* Delete IPv6_table entry */
      status = switch_pd_ipv6_table_entry(device, api_route_entry, false,
                                          SWITCH_ACTION_NONE);
    } else {
      krnlmon_log_error("Invalid IP address type: %d",
                        api_route_info.ip_address.type);
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to Delete IP table entry, error: %s",
                        switch_error_to_string(status));
      return status;
    }
  }

  status = switch_route_hashtable_remove(device, route_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "l3 route table delete failed on device %d "
        "route table delete failed(%s)\n",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_route_handle_delete(device, route_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}
