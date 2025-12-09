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

#include "switchlink/switchlink_handlers.h"
#include "switchlink_init_sai.h"

static sai_next_hop_group_api_t* sai_nhop_group_api = NULL;

/*
 * Routine Description:
 *    Initialize Nexthop group SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

sai_status_t sai_init_nhop_group_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&sai_nhop_group_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    SAI call to delete ecmp by deleting nexthop group
 *
 * Arguments:
 *    [in] ecmp_info - ecmp_info
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int delete_ecmp(switchlink_db_ecmp_info_t* ecmp_info) {
  sai_status_t retval = SAI_STATUS_SUCCESS;
  sai_status_t status;

#if defined(ES2K_TARGET)
  uint8_t index = 0;
  for (index = 0; index < ecmp_info->num_nhops; index++) {
    status = sai_nhop_group_api->remove_next_hop_group_member(
        ecmp_info->nhop_member_handles[index]);
    if (!retval) retval = status;
  }
#endif

  /* While deleting group, we loop through list of members and delete
   * member entries as well
   */
  status = sai_nhop_group_api->remove_next_hop_group(ecmp_info->ecmp_h);
  if (!retval) retval = status;

  return ((retval == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    Delete ecmp and remove entry from the database
 *
 * Arguments:
 *    [in] ecmp_h - route interface handle
 *
 * Return Values:
 *    void
 */

void switchlink_delete_ecmp(switchlink_handle_t ecmp_h) {
  int32_t ref_count;
  uint32_t index = 0;
  uint8_t num_nhops = 0;
  switchlink_db_status_t status;
  switchlink_db_nexthop_info_t nexthop_info;
  switchlink_handle_t nhops[SWITCHLINK_ECMP_NUM_MEMBERS_MAX] = {0};

  status = switchlink_db_dec_ecmp_ref(ecmp_h, &ref_count);
  krnlmon_assert(status == SWITCHLINK_DB_STATUS_SUCCESS);

  if (ref_count == 0) {
    switchlink_db_ecmp_info_t ecmp_info;
    memset(&ecmp_info, 0, sizeof(switchlink_db_ecmp_info_t));
    status = switchlink_db_ecmp_handle_get_info(ecmp_h, &ecmp_info);
    krnlmon_assert(status == SWITCHLINK_DB_STATUS_SUCCESS);
    num_nhops = ecmp_info.num_nhops;
    for (index = 0; index < num_nhops; index++) {
      nhops[index] = ecmp_info.nhops[index];
    }
    krnlmon_log_info("Deleting ecmp handler 0x%lx", ecmp_h);
    delete_ecmp(&ecmp_info);
    switchlink_db_delete_ecmp(ecmp_h);

    for (index = 0; index < num_nhops; index++) {
      memset(&nexthop_info, 0, sizeof(switchlink_db_nexthop_info_t));
      status =
          switchlink_db_get_nexthop_handle_info(nhops[index], &nexthop_info);
      if (status != SWITCHLINK_DB_STATUS_SUCCESS) {
        krnlmon_log_error("Cannot get nhop info for nhop handle 0x%lx",
                          nhops[index]);
        continue;
      }

      if (validate_delete_nexthop(nexthop_info.using_by,
                                  SWITCHLINK_NHOP_FROM_ROUTE)) {
        krnlmon_log_debug("Deleting nhop 0x%lx, from delete_ecmp",
                          nexthop_info.nhop_h);
        switchlink_delete_nexthop(nexthop_info.nhop_h);
        switchlink_db_delete_nexthop(&nexthop_info);
      } else {
        krnlmon_log_debug("Removing Route learn from nhop");
        nexthop_info.using_by &= ~SWITCHLINK_NHOP_FROM_ROUTE;
        switchlink_db_update_nexthop_using_by(&nexthop_info);
      }
    }
  }
}

/*
 * Routine Description:
 *    SAI call to create ecmp by creating nexthop group
 *
 * Arguments:
 *    [in] ecmp_info - ecmp_info
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

int switchlink_create_ecmp(switchlink_db_ecmp_info_t* ecmp_info) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint8_t index = 0;
  sai_attribute_t attr_list[1];
  sai_attribute_t attr_member_list[2];

  memset(attr_list, 0, sizeof(attr_list));
  attr_list[0].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
  attr_list[0].value.s32 = SAI_NEXT_HOP_GROUP_TYPE_ECMP;

  status = sai_nhop_group_api->create_next_hop_group(&(ecmp_info->ecmp_h), 0,
                                                     0x1, attr_list);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Unable to create nexthop group for ECMP");
    return -1;
  }

  for (index = 0; index < ecmp_info->num_nhops; index++) {
    memset(attr_member_list, 0x0, sizeof(attr_member_list));
    attr_member_list[0].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    attr_member_list[0].value.oid = ecmp_info->ecmp_h;
    attr_member_list[1].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
    attr_member_list[1].value.oid = ecmp_info->nhops[index];
    status = sai_nhop_group_api->create_next_hop_group_member(
        &ecmp_info->nhop_member_handles[index], 0, 0x2, attr_member_list);
    if (status != SAI_STATUS_SUCCESS) {
      krnlmon_log_error("Unable to add members to nexthop group for ECMP");
      return -1;
    }
  }

  return status;
}
