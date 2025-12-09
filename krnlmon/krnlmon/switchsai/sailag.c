/*
 * Copyright 2023 Intel Corporation.
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

#include "saiinternal.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_lag.h"
#include "switchapi/switch_rmac.h"
#include "switchapi/switch_status.h"

/**
 * Routine Description:
 *    Create RMAC for LAG interface.
 *
 * Arguments:
 *    [in] switch_id - switch id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *    [out] rmac_h - RMAC handle
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
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
      case SAI_LAG_ATTR_CUSTOM_RANGE_START:
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

/**
 * Routine Description:
 *    Delete RMAC for LAG interface.
 *
 * Arguments:
 *    [in] lag_h - LAG handle
 *    [in] rmac_handle - RMAC handle
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_delete_rmac_internal(switch_handle_t lag_h,
                                             switch_handle_t rmac_handle) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t tmp_rmac_handle = SWITCH_API_INVALID_HANDLE;

  switch_api_device_default_rmac_handle_get(0, &tmp_rmac_handle);
  if (tmp_rmac_handle != rmac_handle) {
    switch_status = switch_api_router_mac_group_delete(0, lag_h, rmac_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to delete rmac, error: %s",
                        sai_status_to_string(status));
    }
    return status;
  }
  return status;
}

/**
 * Routine Description:
 *    Create LAG
 *
 * Arguments:
 *    [out] lag_id - LAG id
 *    [in] switch_id - Switch Id
 *    [in] attr_count - Number of attributes
 *    [in] attr_list - Array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 */
static sai_status_t sai_create_lag(_Out_ sai_object_id_t* lag_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t* attr_list) {
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_api_lag_info_t api_lag_info = {0};

  const sai_attribute_t* attribute;
  switch_handle_t rmac_handle = 0;
  switch_handle_t lag_h = SWITCH_API_INVALID_HANDLE;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  attribute =
      get_attr_from_list(SAI_LAG_ATTR_INGRESS_ACL, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
    return status;
  }

  lag_h = attribute->value.oid;

  attribute =
      get_attr_from_list(SAI_LAG_ATTR_PORT_VLAN_ID, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
    return status;
  }

  api_lag_info.bond_mode = attribute->value.u8;

  // This means lag interface already created, adding RMAC now
  if (lag_h) {
    switch_api_lag_attribute_get(switch_id, lag_h, &api_lag_info);
    if ((status = sai_switch_status_to_sai_status(switch_status)) !=
        SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to get router interface, error: %s",
                        sai_status_to_string(status));
      return status;
    }

    if (!api_lag_info.rmac_handle) {
      return status;
    }

    status = sai_delete_rmac_internal(lag_h, api_lag_info.rmac_handle);
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
    status = switch_api_lag_update_rmac_handle(switch_id, lag_h, rmac_handle);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to update RMAC handle, error: %s",
                        sai_status_to_string(status));
      return status;
    }
#endif

  } else {
    *lag_id = SAI_NULL_OBJECT_ID;

    status = sai_create_rmac_internal(0, attr_count, attr_list, &rmac_handle);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to create RMAC, error: %s",
                        sai_status_to_string(status));
      return status;
    }

    api_lag_info.rmac_handle = rmac_handle;

    switch_status = switch_api_lag_create(switch_id, &api_lag_info, &lag_h);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Failed to create lag interface, error: %s",
                        sai_status_to_string(status));
      return status;
    }
    *lag_id = lag_h;
  }
  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Remove LAG
 *
 * Arguments:
 *    [in] lag_id - LAG id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_remove_lag(_In_ sai_object_id_t lag_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_lag_delete(0, (switch_handle_t)lag_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to remove lag, error: %s",
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Create LAG member
 *
 * Arguments:
 *    [out] lag_member_id - LAG member id
 *    [in] switch_id - Switch Id
 *    [in] attr_count - Number of attributes
 *    [in] attr_list - Array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 */
static sai_status_t sai_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id, _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count, _In_ const sai_attribute_t* attr_list) {
  switch_api_lag_member_info_t api_lag_member_info = {0};
  switch_handle_t lag_member_h = SWITCH_API_INVALID_HANDLE;
  switch_handle_t lag_h = SWITCH_API_INVALID_HANDLE;

  const sai_attribute_t* attribute;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  attribute =
      get_attr_from_list(SAI_LAG_MEMBER_ATTR_LAG_ID, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
    return status;
  }

  lag_h = attribute->value.oid;
  api_lag_member_info.lag_h = lag_h;

  attribute =
      get_attr_from_list(SAI_LAG_MEMBER_ATTR_PORT_ID, attr_list, attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
    return status;
  }

  api_lag_member_info.port_id = attribute->value.u32;

  attribute = get_attr_from_list(SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE, attr_list,
                                 attr_count);
  if (attribute == NULL) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("missing attribute %s", sai_status_to_string(status));
    return status;
  }

  api_lag_member_info.oper_state = attribute->value.booldata;

  switch_status = switch_api_lag_member_create(switch_id, &api_lag_member_info,
                                               &lag_member_h);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to create lag member, error: %s \n",
                      sai_status_to_string(status));
    return status;
  }

  *lag_member_id = lag_member_h;
  krnlmon_log_debug("LAG member created for handle : 0x%lx", lag_member_h);

  // update the lag_info with the new lag member handle to
  // establish relationship between lag and lag members
  if (lag_h != SWITCH_API_INVALID_HANDLE) {
    switch_api_lag_update(switch_id, lag_h, lag_member_h);
  }

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Remove LAG member
 *
 * Arguments:
 *    [in] lag_member_id - LAG member id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_remove_lag_member(_In_ sai_object_id_t lag_member_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status =
      switch_api_lag_member_delete(0, (switch_handle_t)lag_member_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to remove lag member, error: %s",
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Set LAG attribute.
 *
 * Arguments:
 *    [in] lag_id - LAG ID
 *    [in] attr - SAI attribute of LAG object
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_set_lag_attribute(_In_ sai_object_id_t lag_id,
                                          _In_ const sai_attribute_t* attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t lag_h = lag_id;
  switch_handle_t active_lag_member_h = attr->value.oid;

  switch_status = switch_api_program_lag_hw(0, lag_h, active_lag_member_h);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to program LAG in HW, error: %s",
                      sai_status_to_string(status));
  }

  krnlmon_log_debug("LAG HW Tx/Rx block programmed in MEV-TS");
  return (sai_status_t)status;
}

/**
 * Routine Description:
 *    Set LAG member attribute.
 *
 * Arguments:
 *    [in] lag_member_id - LAG member ID
 *    [in] attr - SAI attribute of LAG object
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_set_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id, _In_ const sai_attribute_t* attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t lag_member_h = lag_member_id;
  bool oper_state = attr->value.booldata;

  switch_status = switch_api_lag_member_update(0, lag_member_h, oper_state);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to update lag member, error: %s",
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/**
 *  LAG methods table retrieved with sai_api_query()
 */
sai_lag_api_t lag_api = {
    .create_lag = sai_create_lag,
    .remove_lag = sai_remove_lag,
    .create_lag_member = sai_create_lag_member,
    .remove_lag_member = sai_remove_lag_member,
    .set_lag_attribute = sai_set_lag_attribute,
    .set_lag_member_attribute = sai_set_lag_member_attribute,
};

sai_status_t sai_lag_initialize(sai_api_service_t* sai_api_service) {
  sai_api_service->lag_api = lag_api;
  return SAI_STATUS_SUCCESS;
}
