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

#include "sainexthopgroup.h"

#include "saiinternal.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_nhop.h"

/*
 * Routine Description:
 *    Create next hop group
 *
 * Arguments:
 *    [out] next_hop_group_id - next hop group id
 *    [in] switch_id - device ID
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_create_next_hop_group_entry(
    _Out_ sai_object_id_t* next_hop_group_id, _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count, _In_ const sai_attribute_t* attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t attribute;
  sai_next_hop_group_type_t nhgroup_type = -1;
  uint32_t index = 0;
  switch_handle_t next_hop_group_handle = SWITCH_API_INVALID_HANDLE;
  *next_hop_group_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = attr_list[index];
    switch (attribute.id) {
      case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
        nhgroup_type = attribute.value.s32;
        break;
    }
  }

  if (nhgroup_type != SAI_NEXT_HOP_GROUP_TYPE_ECMP) {
    return SAI_STATUS_INVALID_PARAMETER;
  }

  status = switch_api_create_nhop_group(switch_id, &next_hop_group_handle);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("failed to create NHOP group %s",
                      sai_status_to_string(status));
    return status;
  }
  *next_hop_group_id = next_hop_group_handle;

  return (sai_status_t)status;
}

/*
 * Routine Description:
 *    Remove next hop group
 *
 * Arguments:
 *    [in] next_hop_group_id - next hop group id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_remove_next_hop_group_entry(
    _In_ sai_object_id_t next_hop_group_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status =
      switch_api_delete_nhop_group(0, (switch_handle_t)next_hop_group_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("failed to remove NHOP group %lx: %s", next_hop_group_id,
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/**
 * @brief Create next hop group member
 *
 * @param[out] next_hop_group_member_id - next hop group member id
 * @param[in] switch_id - device ID
 * @param[in] attr_count - number of attributes
 * @param[in] attr_list - array of attributes
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
static sai_status_t sai_create_next_hop_group_member(
    _Out_ sai_object_id_t* next_hop_group_member_id,
    _In_ sai_object_id_t switch_id, _In_ uint32_t attr_count,
    _In_ const sai_attribute_t* attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t nhop_group_id = 0;
  switch_handle_t nhop_id = 0;
  sai_attribute_t attribute;
  uint32_t index = 0;
  switch_handle_t member_id = SWITCH_API_INVALID_HANDLE;
  *next_hop_group_member_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    attribute = attr_list[index];
    switch (attribute.id) {
      case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID:
        nhop_group_id = attribute.value.oid;
        break;

      case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID:
        nhop_id = attribute.value.oid;
        break;
      default:
        break;
    }
  }

#if defined(DPDK_TARGET)
  /* If NHOP group is not created, map this member to default group */
  if (!nhop_group_id) {
    switch_status =
        switch_api_get_default_nhop_group(switch_id, &nhop_group_id);
    status = sai_switch_status_to_sai_status(switch_status);
    if ((status != SAI_STATUS_SUCCESS) || !nhop_group_id) {
      krnlmon_log_error("failed to get default NHOP group %s",
                        sai_status_to_string(status));
    }
  }
#endif

  switch_status =
      switch_api_add_nhop_member(switch_id, (switch_handle_t)nhop_group_id, 0x1,
                                 (switch_handle_t*)&nhop_id, &member_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("failed to add member to NHOP group %lx : %s",
                      nhop_group_id, sai_status_to_string(status));
  }

  *next_hop_group_member_id = (sai_object_id_t)member_id;

  return (sai_status_t)status;
}

/**
 * @brief Remove next hop group member
 *
 * @param[in] next_hop_group_member_id - next hop group member id
 *
 * @return SAI_STATUS_SUCCESS on success Failure status code on error
 */
static sai_status_t sai_remove_next_hop_group_member(
    _In_ sai_object_id_t next_hop_group_member_id) {
  switch_handle_t nhop_group_id = 0;
  switch_handle_t nhop_id = 0;

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_nhop_group_get_by_nhop_member(
      0, next_hop_group_member_id, &nhop_group_id, &nhop_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "failed to get NHOP group and nhop for member ID %lx : %s",
        next_hop_group_member_id, sai_status_to_string(status));
  }

  switch_status = switch_api_delete_nhop_member(
      0, (switch_handle_t)nhop_group_id, 0x1, (switch_handle_t*)&nhop_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("failed to remove member from NHOP group %lx : %s",
                      next_hop_group_member_id, sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/*
 *  Next Hop group methods table retrieved with sai_api_query()
 */
sai_next_hop_group_api_t nhop_group_api = {
    .create_next_hop_group = sai_create_next_hop_group_entry,
    .remove_next_hop_group = sai_remove_next_hop_group_entry,
    .create_next_hop_group_member = sai_create_next_hop_group_member,
    .remove_next_hop_group_member = sai_remove_next_hop_group_member};

sai_status_t sai_next_hop_group_initialize(sai_api_service_t* sai_api_service) {
  krnlmon_log_debug("Initializing NHOP group");
  sai_api_service->nhop_group_api = nhop_group_api;
  return SAI_STATUS_SUCCESS;
}
