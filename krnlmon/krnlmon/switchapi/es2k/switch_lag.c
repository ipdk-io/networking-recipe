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

#include "switchapi/switch_lag.h"

#include "switch_pd_lag.h"
#include "switch_pd_utils.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_internal.h"
#include "switchapi/switch_status.h"

/**
 * Routine Description:
 *   @brief Create allocated LAG handles
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_lag_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status =
      switch_handle_type_init(device, SWITCH_HANDLE_TYPE_LAG, SWITCH_LAG_MAX);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_handle_type_init(device, SWITCH_HANDLE_TYPE_LAG_MEMBER,
                                   SWITCH_LAG_MAX);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  krnlmon_log_debug("LAG init successful on device %d\n", device);

  return status;
}

/**
 * Routine Description:
 *   @brief Free allocated LAG handles
 *
 * Arguments:
 *   @param[in] device - device
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_lag_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_LAG_MEMBER);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  krnlmon_log_debug("LAG free successful on device %d\n", device);

  return status;
}

/**
 * Routine Description:
 *   @brief Create LAG
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] api_lag_info - LAG info
 *   @param[out] lag_h - LAG handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_create(switch_device_t device,
                                      switch_api_lag_info_t* api_lag_info,
                                      switch_handle_t* lag_h) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_lag_info_t* lag_info = NULL;

  if (api_lag_info->rmac_handle == SWITCH_API_INVALID_HANDLE) {
    status = switch_api_device_default_rmac_handle_get(
        device, &api_lag_info->rmac_handle);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  }

  if (!SWITCH_RMAC_HANDLE(api_lag_info->rmac_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    krnlmon_log_error(
        "LAG create: Invalid rmac handle on device:%d error: %s\n", device,
        switch_error_to_string(status));
    return status;
  }

  *lag_h = switch_lag_handle_create(device);
  CHECK_RET(*lag_h == SWITCH_API_INVALID_HANDLE, SWITCH_STATUS_NO_MEMORY);

  status = switch_lag_get(device, *lag_h, &lag_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("LAG create: Failed to get LAG on device:%d error: %s\n",
                      device, switch_error_to_string(status));
    status = switch_lag_handle_delete(device, *lag_h);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
    return status;
  }

  lag_info->lag_handle = *lag_h;

  status = SWITCH_LIST_INIT(&lag_info->lag_members);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_MEMCPY(&lag_info->api_lag_info, api_lag_info,
                sizeof(switch_api_lag_info_t));

  return status;
}

/**
 * Routine Description:
 *   @brief Delete LAG
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[out] lag_h - LAG handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_delete(switch_device_t device,
                                      switch_handle_t lag_h) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_lag_info_t* lag_info = NULL;

  status = switch_lag_get(device, lag_h, &lag_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  /* Delete only when we have an active LAG member*/
  if (lag_info->active_lag_member != 0) {
    //--------------------- Tx Path : Del Case ----------------------//
    status = switch_pd_tx_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete tx_lag_table entry on device:%d error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    //--------------------- Rx Path : Del Case ----------------------//
    status = switch_pd_rx_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete rx_lag_table entry on device:%d error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }
  }
  status = switch_lag_handle_delete(device, lag_h);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  return status;
}

/**
 * Routine Description:
 *   @brief Create LAG member
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] api_lag_member_info - LAG member info
 *   @param[out] lag_member_h - LAG member handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_member_create(
    switch_device_t device, switch_api_lag_member_info_t* api_lag_member_info,
    switch_handle_t* lag_member_h) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_lag_member_info_t* lag_member_info = NULL;

  *lag_member_h = switch_lag_member_handle_create(device);
  CHECK_RET(*lag_member_h == SWITCH_API_INVALID_HANDLE,
            SWITCH_STATUS_NO_MEMORY);

  status = switch_lag_member_get(device, *lag_member_h, &lag_member_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error(
        "LAG member create: Failed to get LAG member on device:%d error: %s\n",
        device, switch_error_to_string(status));
    status = switch_lag_member_handle_delete(device, *lag_member_h);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);
    return status;
  }

  lag_member_info->lag_member_handle = *lag_member_h;

  SWITCH_MEMCPY(&lag_member_info->api_lag_member_info, api_lag_member_info,
                sizeof(switch_api_lag_member_info_t));

  return status;
}

/**
 * Routine Description:
 *   @brief Delete LAG member
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_member_h - LAG member handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_member_delete(switch_device_t device,
                                             switch_handle_t lag_member_h) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_lag_member_info_t* lag_member_info = NULL;

  status = switch_lag_member_get(device, lag_member_h, &lag_member_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  status = switch_lag_member_handle_delete(device, lag_member_h);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  return status;
}

/**
 * Routine Description:
 *   @brief On creation of a LAG member, update the
 *   structure of parent LAG indicating new member
 *   addition in the lag_members list.
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_h - LAG handle
 *   @param[in] lag_member_h - handle of LAG member
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_update(const switch_device_t device,
                                      const switch_handle_t lag_h,
                                      const switch_handle_t lag_member_h) {
  switch_lag_info_t* lag_info = NULL;
  switch_lag_member_info_t* lag_member_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_LAG_HANDLE(lag_h)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "LAG get: Invalid LAG handle on device %d, "
        "LAG handle 0x%lx: "
        "error: %s\n",
        device, lag_h, switch_error_to_string(status));
    return status;
  }
  status = switch_lag_get(device, lag_h, &lag_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  if (lag_info->api_lag_info.bond_mode == SWITCHAPI_BOND_MODE_ACTIVE_BACKUP) {
    if (!SWITCH_LAG_MEMBER_HANDLE(lag_member_h)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      krnlmon_log_error(
          "LAG get: Invalid LAG member handle on device %d, "
          "LAG member handle 0x%lx: "
          "error: %s\n",
          device, lag_member_h, switch_error_to_string(status));
      return status;
    }
    status = switch_lag_member_get(device, lag_member_h, &lag_member_info);
    CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

    if (lag_info->active_lag_member != 0) {
      // delete rx path
      status = switch_pd_rx_lag_table_entry(device, lag_info, false);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to delete rx_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }

      // insert lag member
      status = SWITCH_LIST_INSERT(&lag_info->lag_members,
                                  &lag_member_info->node, lag_member_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

      // create rx path
      status = switch_pd_rx_lag_table_entry(device, lag_info, true);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to create rx_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }
    } else {
      // update lag members list with the lag_member_h
      status = SWITCH_LIST_INSERT(&lag_info->lag_members,
                                  &lag_member_info->node, lag_member_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    }
  }

  return status;
}

/**
 * Routine Description:
 *   @details On change of oper_state of a LAG member,
 *   update the number of active ports of a LAG and
 *   re-populate the Tx and Rx LAG table entries with
 *   active port.
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_member_h - LAG member handle
 *   @param[in] oper_state - oper state of LAG member
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_member_update(const switch_device_t device,
                                             const switch_handle_t lag_member_h,
                                             bool oper_state) {
  switch_lag_info_t* lag_info = NULL;
  switch_lag_member_info_t* lag_member_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_LAG_MEMBER_HANDLE(lag_member_h)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "LAG get: Invalid LAG member handle on device %d, "
        "LAG member handle 0x%lx: "
        "error: %s\n",
        device, lag_member_h, switch_error_to_string(status));
    return status;
  }
  status = switch_lag_member_get(device, lag_member_h, &lag_member_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  lag_member_info->api_lag_member_info.oper_state = oper_state;

  switch_handle_t lag_h = lag_member_info->api_lag_member_info.lag_h;

  if (!SWITCH_LAG_HANDLE(lag_h)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "LAG get: Invalid LAG handle on device %d, "
        "LAG handle 0x%lx: "
        "error: %s\n",
        device, lag_h, switch_error_to_string(status));
    return status;
  }
  status = switch_lag_get(device, lag_h, &lag_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  // update the active num ports of LAG
  if (oper_state)
    lag_info->num_active_ports++;
  else
    lag_info->num_active_ports--;

  if (lag_info->num_active_ports == 0 &&
      (SWITCH_LIST_COUNT(&lag_info->lag_members) != 0)) {
    // delete the existing tx and rx entries
    status = switch_pd_tx_lacp_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete tx_lacp_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = switch_pd_rx_lacp_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete rx_lacp_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    // delete the lag_member from the LAG
    status = SWITCH_LIST_DELETE(&lag_info->lag_members, &lag_member_info->node);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  } else if (lag_info->num_active_ports == 1) {
    /* oper_state false means num_active_ports is decremented from 2 to 1
     and oper_state true means it is incremented from 0 to 1 */
    if (oper_state) {
      // populate the tx and rx entries with single active member
      status = SWITCH_LIST_INSERT(&lag_info->lag_members,
                                  &lag_member_info->node, lag_member_info);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      status = switch_pd_tx_lacp_lag_table_entry(device, lag_info, true);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to create tx_lacp_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }

      status = switch_pd_rx_lacp_lag_table_entry(device, lag_info, true);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to create rx_lacp_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }
    } else {
      // delete the existing entries and populate the tx and rx entries with
      // other active member
      status = switch_pd_tx_lacp_lag_table_entry(device, lag_info, false);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to delete tx_lacp_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }

      status = switch_pd_rx_lacp_lag_table_entry(device, lag_info, false);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to delete rx_lacp_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }

      status =
          SWITCH_LIST_DELETE(&lag_info->lag_members, &lag_member_info->node);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

      status = switch_pd_tx_lacp_lag_table_entry(device, lag_info, true);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to create tx_lacp_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }

      status = switch_pd_rx_lacp_lag_table_entry(device, lag_info, true);
      if (status != SWITCH_STATUS_SUCCESS) {
        krnlmon_log_error(
            "Failed to create rx_lacp_lag_table entry on device %d: "
            "error: %s\n",
            device, switch_error_to_string(status));
        return status;
      }
    }
  } else if (lag_info->num_active_ports == 2) {
    // delete the existing entries and re-distribute
    // table entries between both active ports
    status = switch_pd_tx_lacp_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete tx_lacp_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = switch_pd_rx_lacp_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete rx_lacp_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = SWITCH_LIST_INSERT(&lag_info->lag_members, &lag_member_info->node,
                                lag_member_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    status = switch_pd_tx_lacp_lag_table_entry(device, lag_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to create tx_lacp_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = switch_pd_rx_lacp_lag_table_entry(device, lag_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to create rx_lacp_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

/**
 * Routine Description:
 *   @brief Get LAG attributes
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_h - LAG handle
 *   @param[out] api_lag_info - LAG info struct
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_attribute_get(
    const switch_device_t device, const switch_handle_t lag_h,
    switch_api_lag_info_t* api_lag_info) {
  switch_lag_info_t* lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_LAG_HANDLE(lag_h)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "LAG attribute get: Invalid LAG handle on device %d, "
        "LAG handle 0x%lx: "
        "error: %s\n",
        device, lag_h, switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_h, &lag_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  *api_lag_info = lag_info->api_lag_info;

  return status;
}

/**
 * Routine Description:
 *   @brief Update RMAC handle for a LAG interface
 *
 * Arguments:
 *   @param[in] device - device
 *   @param[in] lag_h - LAG handle
 *   @param[out] rmac_h - RMAC handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_lag_update_rmac_handle(
    const switch_device_t device, const switch_handle_t lag_h,
    const switch_handle_t rmac_h) {
  switch_lag_info_t* lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_LAG_HANDLE(lag_h)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "LAG attribute get: Invalid LAG handle on device %d, "
        "LAG handle 0x%lx: "
        "error: %s\n",
        device, lag_h, switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_h, &lag_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  lag_info->api_lag_info.rmac_handle = rmac_h;

  return status;
}

/**
 * Routine Description:
 *   @brief Program the Tx and Rx LAG tables
 *
 * Arguments:
 *   @param[in] device : device
 *   @param[in] lag_h : LAG handle
 *   @param[in] active_lag_member_h : Active
 *              LAG member handle
 *
 * Return Values:
 *    @return  SWITCH_STATUS_SUCCESS on success
 *             Failure status code on error
 */
switch_status_t switch_api_program_lag_hw(const switch_device_t device,
                                          switch_handle_t lag_h,
                                          switch_handle_t active_lag_member_h) {
  switch_lag_info_t* lag_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!SWITCH_LAG_HANDLE(lag_h)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error(
        "LAG get: Invalid LAG handle on device %d, "
        "LAG handle 0x%lx: "
        "error: %s\n",
        device, lag_h, switch_error_to_string(status));
    return status;
  }

  status = switch_lag_get(device, lag_h, &lag_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  if ((lag_info->active_lag_member == 0) && (active_lag_member_h != 0)) {
    lag_info->active_lag_member = active_lag_member_h;

    //--------------------- Tx Path ----------------------//
    status = switch_pd_tx_lag_table_entry(device, lag_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to create tx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    //--------------------- Rx Path ----------------------//
    status = switch_pd_rx_lag_table_entry(device, lag_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to create rx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }
  } else if ((lag_info->active_lag_member != 0) && (active_lag_member_h != 0) &&
             (lag_info->active_lag_member != active_lag_member_h)) {
    // Delete case
    status = switch_pd_tx_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete tx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = switch_pd_rx_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete rx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    // Create Case
    lag_info->active_lag_member = active_lag_member_h;

    status = switch_pd_tx_lag_table_entry(device, lag_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to create tx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = switch_pd_rx_lag_table_entry(device, lag_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to create rx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }
  } else if ((lag_info->active_lag_member != 0) && (active_lag_member_h == 0)) {
    // Delete case
    status = switch_pd_tx_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete tx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }

    status = switch_pd_rx_lag_table_entry(device, lag_info, false);
    if (status != SWITCH_STATUS_SUCCESS) {
      krnlmon_log_error(
          "Failed to delete rx_lag_table entry on device %d: "
          "error: %s\n",
          device, switch_error_to_string(status));
      return status;
    }
    lag_info->active_lag_member = active_lag_member_h;
  }

  return status;
}
