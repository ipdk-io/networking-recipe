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

static sai_next_hop_api_t* sai_nhop_api = NULL;
static sai_next_hop_group_api_t* sai_nhop_group_api = NULL;

/*
 * Routine Description:
 *    Initialize Nexthop SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

sai_status_t sai_init_nhop_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_NEXT_HOP, (void**)&sai_nhop_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);
  status = sai_api_query(SAI_API_NEXT_HOP_GROUP, (void**)&sai_nhop_group_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    SAI call to create nexthop entry
 *
 * Arguments:
 *    [in] nexthop_info - nexthop interface info
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

int switchlink_create_nexthop(switchlink_db_nexthop_info_t* nexthop_info) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_attribute_t attr_list[3];
  memset(attr_list, 0, sizeof(attr_list));
  attr_list[0].id = SAI_NEXT_HOP_ATTR_TYPE;
  attr_list[0].value.s32 = SAI_NEXT_HOP_TYPE_IP;
  attr_list[1].id = SAI_NEXT_HOP_ATTR_IP;
  if (nexthop_info->ip_addr.family == AF_INET) {
    attr_list[1].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr_list[1].value.ipaddr.addr.ip4 =
        htonl(nexthop_info->ip_addr.ip.v4addr.s_addr);
  } else {
    attr_list[1].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(attr_list[1].value.ipaddr.addr.ip6,
           &(nexthop_info->ip_addr.ip.v6addr), sizeof(sai_ip6_t));
  }
  attr_list[2].id = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID;
  attr_list[2].value.oid = nexthop_info->intf_h;
  status =
      sai_nhop_api->create_next_hop(&(nexthop_info->nhop_h), 0, 3, attr_list);
  if (status != SAI_STATUS_SUCCESS) {
    return -1;
  }
#if defined(DPDK_TARGET)
  memset(attr_list, 0x0, sizeof(attr_list));
  attr_list[0].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
  attr_list[0].value.oid = nexthop_info->nhop_h;
  status = sai_nhop_group_api->create_next_hop_group_member(
      &(nexthop_info->nhop_member_h), 0, 1, attr_list);
  if (status != SAI_STATUS_SUCCESS) {
    return -1;
  }
#endif
  return status;
}

/*
 * Routine Description:
 *    SAI call to delete nexthop entry
 *
 * Arguments:
 *    [in] nhop handler
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

int switchlink_delete_nexthop(switchlink_handle_t nhop_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  status = sai_nhop_api->remove_next_hop(nhop_h);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}
