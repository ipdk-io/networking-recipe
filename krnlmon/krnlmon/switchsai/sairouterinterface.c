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

#include "sairouterinterface.h"

#include "saiinternal.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_interface.h"
#include "switchapi/switch_l3.h"
#include "switchapi/switch_rif.h"
#include "switchapi/switch_rmac.h"

static sai_status_t sai_create_rmac_internal(sai_object_id_t switch_id,
                                             uint32_t attr_count,
                                             const sai_attribute_t* attr_list,
                                             switch_handle_t* rmac_h) {
  uint32_t index = 0;
  switch_mac_addr_t mac;
  const sai_attribute_t* attribute;
  sai_status_t sai_status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_device_default_rmac_handle_get(switch_id, rmac_h);
  sai_status = sai_switch_status_to_sai_status(switch_status);
  if (sai_status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to get default RMAC handle, error: %s",
                      sai_status_to_string(sai_status));
    return sai_status;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS:

        switch_status = switch_api_router_mac_group_create(
            switch_id, SWITCH_RMAC_TYPE_ALL, rmac_h);

        if (switch_status == SWITCH_STATUS_SUCCESS) {
          memcpy(&mac.mac_addr, &attribute->value.mac, 6);
          krnlmon_log_debug("MAC: %02x:%02x:%02x:%02x:%02x:%02x, add to group",
                            mac.mac_addr[0], mac.mac_addr[1], mac.mac_addr[2],
                            mac.mac_addr[3], mac.mac_addr[4], mac.mac_addr[5]);
          switch_status = switch_api_router_mac_add(switch_id, *rmac_h, &mac);
        }
        sai_status = sai_switch_status_to_sai_status(switch_status);
        break;

      default:
        break;
    }
  }
  return sai_status;
}

static sai_status_t sai_delete_rmac_internal(switch_handle_t rif_handle,
                                             switch_handle_t rmac_handle) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t tmp_rmac_handle = SWITCH_API_INVALID_HANDLE;

  switch_api_device_default_rmac_handle_get(0, &tmp_rmac_handle);
  if (tmp_rmac_handle != rmac_handle) {
    switch_status =
        switch_api_router_mac_group_delete(0, rif_handle, rmac_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to remove router interface, error: %s",
                        sai_status_to_string(status));
    }
    return status;
  }
  return status;
}

/*
 * Routine Description:
 *    Create router interface.
 *
 * Arguments:
 *    [out] rif_id - router interface id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_create_router_interface(
    _Out_ sai_object_id_t* rif_id, _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count, _In_ const sai_attribute_t* attr_list) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_rif_info_t api_rif_info = {0};

  const sai_attribute_t* attribute;
  switch_handle_t rmac_handle = 0;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t rif_handle = SWITCH_API_INVALID_HANDLE;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  attribute =
      get_attr_from_list(SAI_ROUTER_INTERFACE_ATTR_TYPE, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
    return status;
  }

  intf_handle = attribute->value.oid;

  // This means interface already created, adding RMAC now
  if (intf_handle) {
    switch_api_rif_attribute_get(switch_id, intf_handle,
                                 (switch_uint64_t)UINT64_MAX, &api_rif_info);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to get router interface, error: %s",
                        sai_status_to_string(status));
      return status;
    }

    if (!api_rif_info.rmac_handle) {
      return status;
    }

    status = sai_delete_rmac_internal(intf_handle, api_rif_info.rmac_handle);
    if (status != SAI_STATUS_SUCCESS) {
      return status;
    }

    status = sai_create_rmac_internal(switch_id, attr_count, attr_list,
                                      &rmac_handle);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to create RMAC, error: %s",
                        sai_status_to_string(status));
      return status;
    }

#if defined(ES2K_TARGET)
    status =
        switch_api_update_rif_rmac_handle(switch_id, intf_handle, rmac_handle);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to update RMAC handle, error: %s",
                        sai_status_to_string(status));
      return status;
    }
#endif
  } else {
    *rif_id = SAI_NULL_OBJECT_ID;

    status = sai_create_rmac_internal(0, attr_count, attr_list, &rmac_handle);
    if (status != SAI_STATUS_SUCCESS) {
      return status;
    }
    api_rif_info.rmac_handle = rmac_handle;

    attribute = get_attr_from_list(SAI_ROUTER_INTERFACE_ATTR_PORT_ID, attr_list,
                                   attr_count);
    if (attribute == NULL) {
      status = SAI_STATUS_INVALID_PARAMETER;
      krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
      return status;
    }

    api_rif_info.rif_ifindex = attribute->value.u32;

    switch_status =
        switch_api_rif_create(switch_id, &api_rif_info, &rif_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to create router interface, error: %s",
                        sai_status_to_string(status));
      return status;
    }
    *rif_id = rif_handle;
  }
  return (sai_status_t)status;
}

/*
 * Routine Description:
 *    Remove router interface.
 *
 * Arguments:
 *    [in] rif_id - router interface id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_remove_router_interface(_In_ sai_object_id_t rif_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_rif_info_t api_rif_info;
  switch_handle_t rmac_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_rif_attribute_get(
      0, rif_id, (switch_uint64_t)UINT64_MAX, &api_rif_info);
  if ((status = sai_switch_status_to_sai_status(switch_status)) !=
      SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to remove router interface, error: %s",
                      sai_status_to_string(status));
    return status;
  }

  rmac_handle = api_rif_info.rmac_handle;
  status = sai_delete_rmac_internal((switch_handle_t)rif_id, rmac_handle);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to remove RMA, erorr: %s",
                      sai_status_to_string(status));
  }

  switch_status = switch_api_rif_delete(0, (switch_handle_t)rif_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to remove router interface, error: %s",
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/*
 *  Routing interface methods table retrieved with sai_api_query()
 */
sai_router_interface_api_t rif_api = {
    .create_router_interface = sai_create_router_interface,
    .remove_router_interface = sai_remove_router_interface};

sai_status_t sai_router_interface_initialize(
    sai_api_service_t* sai_api_service) {
  sai_api_service->rif_api = rif_api;
  return SAI_STATUS_SUCCESS;
}
