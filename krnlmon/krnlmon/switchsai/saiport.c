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

#include "saiport.h"

#include "saiinternal.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_port.h"

#define SAI_PORT_DEFAULT_MTU 9400
/*
 * Routine Description:
 *   Set port attribute value.
 *
 * Arguments:
 *    [in] port_id - port id
 *    [in] attr - attribute
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t sai_set_port_attribute(_In_ sai_object_id_t port_id,
                                           _In_ const sai_attribute_t* attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/*
 * Routine Description:
 *   Get port attribute value.
 *
 * Arguments:
 *    [in] port_id - port id
 *    [in] attr_count - number of attributes
 *    [inout] attr_list - array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t sai_get_port_attribute(_In_ sai_object_id_t port_id,
                                           _In_ uint32_t attr_count,
                                           _Inout_ sai_attribute_t* attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/*
 * Routine Description:
 *   Create port
 *
 * Arguments:
 *    [out] port_id - port id
 *    [in] switch_id - switch id
 *    [in] attr_count - Number of attributes
 *    [in] attr_list - Array of attributes
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t sai_create_port(_Out_ sai_object_id_t* port_id,
                                    _In_ sai_object_id_t switch_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t* attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  const sai_attribute_t* attribute = NULL;
  uint64_t portid = 0;
  uint32_t mtu = 0;

  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_port_info_t api_port_info = {0};

  krnlmon_log_info("[SAI_CREATE_PORT] called ..\n");

  for (uint32_t index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_PORT_ATTR_HW_LANE_LIST:
        portid = attribute->value.oid;
        krnlmon_log_info("[SAI_CREATE_PORT]: Port ID = %" PRIu64, portid);
        break;
      case SAI_PORT_ATTR_MTU:
        mtu = attribute->value.u32;
        krnlmon_log_info("[SAI_CREATE_PORT]: MTU = %d\n", mtu);
        break;
      default:
        status = SAI_STATUS_NOT_IMPLEMENTED;
    }
  }

  // UDIT: Re-create strucutre and populate only required parametes ??
  // Filling some commmon attrs with above values for now
  api_port_info.port = portid;
  api_port_info.tx_mtu = mtu;
  api_port_info.rx_mtu = mtu;
  // api_port_info.port_speed = SWITCH_PORT_SPEED_25G;
  // api_port_info.fec_mode = fec_mode;
  // api_port_info.initial_admin_state = admin_state;
  // api_port_info.non_default_ppgs = switch_sai_port_non_default_ppgs();

  status = switch_api_port_add(1, &api_port_info, &port_handle);
  return status;
}

static sai_status_t sai_remove_port(_In_ sai_object_id_t port_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/*
 * Routine Description:
 *   Get port statistics counters.
 *
 * Arguments:
 *    [in] port_id - port id
 *    [in] counter_ids - specifies the array of counter ids
 *    [in] number_of_counters - number of counters in the array
 *    [out] counters - array of resulting counter values.
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t sai_get_port_stats(_In_ sai_object_id_t port_id,
                                       _In_ uint32_t number_of_counters,
                                       _In_ const sai_stat_id_t* counter_ids,
                                       _Out_ uint64_t* counters) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/*
 * Routine Description:
 *   Clear port statistics counters.
 *
 * Arguments:
 *    [in] port_id - port id
 *    [in] number_of_counters - Number of counters in the array
 *    [in] counter_ids - Specifies the array of counter ids
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t sai_clear_port_stats(
    _In_ sai_object_id_t port_id, _In_ uint32_t number_of_counters,
    _In_ const sai_stat_id_t* counter_ids) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/*
 * Routine Description:
 *   Clear all port statistics counters.
 *
 * Arguments:
 *    [in] port_id - port id
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t sai_clear_port_all_stats(_In_ sai_object_id_t port_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  return status;
}

/*
 * Port methods table retrieved with sai_api_query()
 */
sai_port_api_t port_api = {.create_port = sai_create_port,
                           .remove_port = sai_remove_port,
                           .set_port_attribute = sai_set_port_attribute,
                           .get_port_attribute = sai_get_port_attribute,
                           .get_port_stats = sai_get_port_stats,
                           .clear_port_stats = sai_clear_port_stats,
                           .clear_port_all_stats = sai_clear_port_all_stats};

sai_status_t sai_port_initialize(sai_api_service_t* sai_api_service) {
  sai_api_service->port_api = port_api;
  return SAI_STATUS_SUCCESS;
}
