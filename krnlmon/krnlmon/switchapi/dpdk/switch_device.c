/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
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

#include <linux/if.h>

/* Local header includes */
#include "switchapi/switch_config_int.h"
#include "switchapi/switch_fdb.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_neighbor_int.h"
#include "switchapi/switch_nhop_int.h"
#include "switchapi/switch_rmac_int.h"
#include "switchapi/switch_table.h"
#include "switchapi/switch_vrf.h"
#include "switchutils/switch_log.h"

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_DEVICE

#ifdef __cplusplus
extern "C" {
#endif

#define __FILE_ID__ SWITCH_DEVICE

#define SWITCH_PORT_STATS_POLL_TMR_PERIOD_MS 50

switch_status_t switch_api_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_config_t api_config;

#ifdef THRIFT_ENABLED
  add_to_rpc_server(rpc_server_cookie);
#endif  // THRIFT_ENABLED
  SWITCH_MEMSET(&api_config, 0x0, sizeof(api_config));
  api_config.max_devices = 1;

  status = switch_config_init(&api_config);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("switch api init failed on device %d: %s\n", device,
                      switch_error_to_string(status));
  }

  status = switch_api_device_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("switch api init failed on device %d: %s\n", device,
                      switch_error_to_string(status));
    return status;
  }

#ifdef THRIFT_ENABLED
  status = start_switch_api_rpc_server();
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("start_switch_api_rpc_server on device %d", device,
                      switch_error_to_string(status));
    return status;
  }
#endif  // THRIFT_ENABLED

  return status;
}

switch_status_t switch_device_context_get(
    switch_device_t device, switch_device_context_t** device_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_config_device_context_get(device, device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device context get failed on device %d: "
        "device config context get failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_device_api_init(switch_device_t device) {
  switch_device_context_t* device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device api init failed on device %d: "
        "device context get failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    if (device_ctx->api_inited[api_type]) {
      continue;
    }

    switch (api_type) {
      case SWITCH_API_TYPE_TUNNEL:
        status = switch_tunnel_init(device);
        break;
      case SWITCH_API_TYPE_PORT:
        break;
      case SWITCH_API_TYPE_L2:
        status = switch_l2_init(device);
        break;
      case SWITCH_API_TYPE_BD:
        break;
      case SWITCH_API_TYPE_VRF:
        status = switch_vrf_init(device);
        break;
      case SWITCH_API_TYPE_L3:
        status = switch_l3_init(device);
        break;
      case SWITCH_API_TYPE_RMAC:
        status = switch_rmac_init(device);
        break;
      case SWITCH_API_TYPE_INTERFACE:
      case SWITCH_API_TYPE_LAG:
        break;
      case SWITCH_API_TYPE_NHOP:
        status = switch_nhop_init(device);
        break;
      case SWITCH_API_TYPE_NEIGHBOR:
        status = switch_neighbor_init(device);
        break;
      case SWITCH_API_TYPE_MCAST:
      case SWITCH_API_TYPE_HOSTIF:
      case SWITCH_API_TYPE_ACL:
      case SWITCH_API_TYPE_MIRROR:
      case SWITCH_API_TYPE_METER:
      case SWITCH_API_TYPE_SFLOW:
      case SWITCH_API_TYPE_DTEL:
      case SWITCH_API_TYPE_STP:
      case SWITCH_API_TYPE_VLAN:
      case SWITCH_API_TYPE_QOS:
      case SWITCH_API_TYPE_QUEUE:
      case SWITCH_API_TYPE_LOGICAL_NETWORK:
      case SWITCH_API_TYPE_NAT:
      case SWITCH_API_TYPE_BUFFER:
      case SWITCH_API_TYPE_BFD:
      case SWITCH_API_TYPE_HASH:
      case SWITCH_API_TYPE_WRED:
      case SWITCH_API_TYPE_ILA:
      case SWITCH_API_TYPE_FAILOVER:
      case SWITCH_API_TYPE_LABEL:
      case SWITCH_API_TYPE_RPF:
      case SWITCH_API_TYPE_DEVICE:
        break;
      case SWITCH_API_TYPE_RIF:
        status = switch_rif_init(device);
        break;
      case SWITCH_API_TYPE_PACKET_DRIVER:
      case SWITCH_API_TYPE_SCHEDULER:
      case SWITCH_API_TYPE_MPLS:
      case SWITCH_API_TYPE_MAX:
      case SWITCH_API_TYPE_NONE:
        break;

      default:
        /* Internal error */
        status = SWITCH_STATUS_FAILURE;
        break;
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "device init failed on device %d "
          "for api type %d %s: (%s)",
          device, api_type, switch_api_type_to_string(api_type),
          switch_error_to_string(status));
      goto cleanup;
    }

    device_ctx->api_inited[api_type] = true;
  }

  krnlmon_log_debug("device api init successful on device %d\n", device);

  return status;

cleanup:
  tmp_status = switch_device_api_free(device);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_device_api_free(switch_device_t device) {
  switch_device_context_t* device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device init failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    if (!device_ctx->api_inited[api_type]) {
      continue;
    }

    switch (api_type) {
      case SWITCH_API_TYPE_TUNNEL:
        status = switch_tunnel_free(device);
        break;

      case SWITCH_API_TYPE_PORT:
        break;
      case SWITCH_API_TYPE_L2:
        status = switch_l2_free(device);
        break;
      case SWITCH_API_TYPE_BD:
        break;
      case SWITCH_API_TYPE_VRF:
        status = switch_vrf_free(device);
        break;
      case SWITCH_API_TYPE_L3:
        break;
      case SWITCH_API_TYPE_RMAC:
        status = switch_rmac_free(device);
        break;
      case SWITCH_API_TYPE_INTERFACE:
        break;
      case SWITCH_API_TYPE_LAG:
        break;
      case SWITCH_API_TYPE_NHOP:
        status = switch_nhop_free(device);
        break;
      case SWITCH_API_TYPE_NEIGHBOR:
        status = switch_neighbor_free(device);
        break;
      case SWITCH_API_TYPE_MCAST:
      case SWITCH_API_TYPE_HOSTIF:
      case SWITCH_API_TYPE_ACL:
      case SWITCH_API_TYPE_MIRROR:
      case SWITCH_API_TYPE_METER:
      case SWITCH_API_TYPE_SFLOW:
      case SWITCH_API_TYPE_DTEL:
      case SWITCH_API_TYPE_STP:
      case SWITCH_API_TYPE_VLAN:
      case SWITCH_API_TYPE_QOS:
      case SWITCH_API_TYPE_QUEUE:
      case SWITCH_API_TYPE_LOGICAL_NETWORK:
      case SWITCH_API_TYPE_NAT:
      case SWITCH_API_TYPE_BUFFER:
      case SWITCH_API_TYPE_BFD:
      case SWITCH_API_TYPE_HASH:
      case SWITCH_API_TYPE_WRED:
      case SWITCH_API_TYPE_ILA:
      case SWITCH_API_TYPE_FAILOVER:
      case SWITCH_API_TYPE_LABEL:
      case SWITCH_API_TYPE_RPF:
      case SWITCH_API_TYPE_DEVICE:
        break;
      case SWITCH_API_TYPE_RIF:
        status = switch_rif_free(device);
        break;
      case SWITCH_API_TYPE_PACKET_DRIVER:
      case SWITCH_API_TYPE_SCHEDULER:
      case SWITCH_API_TYPE_MPLS:
      case SWITCH_API_TYPE_MAX:
      case SWITCH_API_TYPE_NONE:
        break;

      default:
        /* Internal error */
        status = SWITCH_STATUS_FAILURE;
        break;
    }

    device_ctx->api_inited[api_type] = false;

    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "device free failed on device %d "
          "for api type %s: (%s)",
          device, switch_api_type_to_string(api_type),
          switch_error_to_string(status));
      continue;
    }
  }

  krnlmon_log_debug("device api free successful on device %d\n", device);

  return status;
}

switch_status_t switch_device_init(switch_device_t device,
                                   switch_size_t* table_sizes) {
  switch_device_context_t* device_ctx = NULL;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device init failed on device %d: "
        "device context get failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_table_init(device, table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device init failed on device %d: "
        "table init failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  for (api_type = 1; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    device_ctx->api_inited[api_type] = false;
  }

  status = switch_api_id_allocator_new(device, SWITCH_IFINDEX_SIZE, FALSE,
                                       &device_ctx->ifindex_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device init failed on device %d: "
        "ifindex allocator init failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device init failed on device %d: "
        "device api init failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_api_router_mac_group_create(
      device, SWITCH_RMAC_TYPE_ALL, &device_ctx->device_info.rmac_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device init failed on device %d: "
        "rmac group create failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_api_create_nhop_group(device,
                                        &device_ctx->device_info.group_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device init failed on device %d: "
        "nhop group create failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  krnlmon_log_debug("device init done on device %d", device);

  return status;
}

switch_status_t switch_device_free(switch_device_t device) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device free failed on device %d: "
        "device context get failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_table_free(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device free failed on device %d: "
        "table free failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_free(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device free failed on device %d: "
        "device api free failed(%s)",
        device, switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_destroy(device, device_ctx->ifindex_allocator);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device free failed on device %d: "
        "ifindex allocator free failed(%s)",
        device, switch_error_to_string(status));
  }

  krnlmon_log_debug("device free done on device %d", device);

  return status;
}

switch_status_t switch_device_table_get(switch_device_t device,
                                        switch_table_t** table_info) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(table_info != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  *table_info = device_ctx->table_info;
  return status;
}

switch_status_t switch_device_api_context_set(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void* context) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  device_ctx->context[api_type] = context;

  return status;
}

switch_status_t switch_device_api_context_get(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void** context) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(context != NULL);

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  *context = device_ctx->context[api_type];
  return status;
}

switch_status_t switch_api_device_add(switch_device_t device) {
  switch_device_context_t* device_ctx = NULL;
  switch_size_t table_sizes[SWITCH_TABLE_MAX];
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  char cpuif_netdev_name[IFNAMSIZ] = "";

  UNUSED(cpuif_netdev_name);

  if (SWITCH_CONFIG_DEVICE_INITED(device)) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    krnlmon_log_error("device add failed for device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  device_ctx = SWITCH_MALLOC(device, sizeof(switch_device_context_t), 0x1);
  if (!device_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    krnlmon_log_error("device add failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(device_ctx, 0, sizeof(switch_device_context_t));

  status = switch_config_device_context_set(device, device_ctx);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMSET(table_sizes, 0x0, sizeof(switch_size_t) * SWITCH_TABLE_MAX);

  status = switch_config_table_sizes_get(device, table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device add failed for device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device add failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  device_ctx->max_pipes = SWITCH_MAX_PIPES;
  device_ctx->device_id = device;
  device_ctx->warm_init = false;

  status = switch_device_init(device, table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device add failed for device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  return status;
}

static switch_status_t switch_api_device_remove_internal(
    switch_device_t device) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_CONFIG_DEVICE_INITED(device)) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    krnlmon_log_error("device add failed for device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  status = switch_device_free(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, device_ctx);

  status = switch_config_device_context_set(device, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_device_default_rmac_handle_get(
    switch_device_t device, switch_handle_t* rmac_handle) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  *rmac_handle = device_ctx->device_info.rmac_handle;

  return status;
}

switch_status_t switch_api_get_default_nhop_group(
    switch_device_t device, switch_handle_t* nhop_group_handle) {
  switch_device_context_t* device_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("device context get failed on device %d: %s", device,
                      switch_error_to_string(status));
    return status;
  }

  *nhop_group_handle = device_ctx->device_info.group_handle;

  return status;
}

switch_status_t switch_api_device_tunnel_dmac_get(switch_device_t device,
                                                  switch_mac_addr_t* mac_addr) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t* device_ctx = NULL;

  status = switch_device_context_get(device, &device_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "device tunnel dmac get failed: "
        "device context get failed on device %d: %s",
        device, switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(mac_addr, &device_ctx->device_info.tunnel_dmac,
                sizeof(switch_mac_addr_t));

  return status;
}
#ifdef __cplusplus
}  // extern "C"
#endif

switch_status_t switch_api_device_remove(switch_device_t device) {
  return switch_api_device_remove_internal(device);
}
