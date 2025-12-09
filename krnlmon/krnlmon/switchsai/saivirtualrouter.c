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

#include "saivirtualrouter.h"

#include "saiinternal.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_vrf.h"

static void sai_vrf_entry_attribute_parse(uint32_t attr_count,
                                          const sai_attribute_t* attr_list) {
  const sai_attribute_t* attribute;
  uint32_t i = 0;

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE:  // TODO
        break;
      case SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE:  // TODO
        break;
      case SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS:  // TODO
        break;
      default:
        break;
    }
  }
}

/*
 * Routine Description:
 *    Create virtual router
 *
 * Arguments:
 *    [out] vr_id - virtual router id
 *    [in] attr_count - number of attributes
 *    [in] attr_list - array of attributes
 *
 * Return Values:
 *  - SAI_STATUS_SUCCESS on success
 *  - SAI_STATUS_ADDR_NOT_FOUND if neither SAI_SWITCH_ATTR_SRC_MAC_ADDRESS nor
 *    SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS is set.
 */
static sai_status_t sai_create_virtual_router_entry(
    _Out_ sai_object_id_t* vr_id, _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count, _In_ const sai_attribute_t* attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_vrf_t vrf_id = 0;
  switch_handle_t vrf_handle = SWITCH_API_INVALID_HANDLE;
  *vr_id = SAI_NULL_OBJECT_ID;

  if (attr_list) {
    sai_vrf_entry_attribute_parse(attr_count, attr_list);
  }

  status = (sai_object_id_t)switch_api_vrf_create(0, vrf_id, &vrf_handle);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("failed to create virtual router entry : %s",
                      sai_status_to_string(status));
  }
  *vr_id = vrf_handle;

  return (sai_status_t)status;
}

/*
 * Routine Description:
 *    Remove virtual router
 *
 * Arguments:
 *    [in] vr_id - virtual router id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
static sai_status_t sai_remove_virtual_router_entry(
    _In_ sai_object_id_t vr_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_ASSERT(sai_object_type_query(vr_id) == SAI_OBJECT_TYPE_VIRTUAL_ROUTER);
  switch_status = switch_api_vrf_delete(0, vr_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("failed to remove virtual router entry %lx : %s", vr_id,
                      sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/*
 *  Virtual router methods table retrieved with sai_api_query()
 */
sai_virtual_router_api_t vr_api = {
    .create_virtual_router = sai_create_virtual_router_entry,
    .remove_virtual_router = sai_remove_virtual_router_entry};

sai_status_t sai_virtual_router_initialize(sai_api_service_t* sai_api_service) {
  sai_api_service->vr_api = vr_api;
  return SAI_STATUS_SUCCESS;
}
