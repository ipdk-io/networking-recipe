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

#include <string.h>

#include "switchlink/switchlink_handlers.h"
#include "switchlink_init_sai.h"

static sai_virtual_router_api_t* sai_vrf_api = NULL;
static sai_router_interface_api_t* sai_rintf_api = NULL;
/*
 * Routine Description:
 *    Initialize Router Interface SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

sai_status_t sai_init_rintf_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_ROUTER_INTERFACE, (void**)&sai_rintf_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    Initialize Virtual Router SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

sai_status_t sai_init_vrf_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_VIRTUAL_ROUTER, (void**)&sai_vrf_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    SAI call to create VRF entry
 *
 * Arguments:
 *    [in] fdb_entry - fdb entry
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

int switchlink_create_vrf(switchlink_handle_t* vrf_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t attr_list[2];

  memset(attr_list, 0, sizeof(attr_list));
  attr_list[0].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V4_STATE;
  attr_list[0].value.booldata = true;
  attr_list[1].id = SAI_VIRTUAL_ROUTER_ATTR_ADMIN_V6_STATE;
  attr_list[1].value.booldata = true;

  status = sai_vrf_api->create_virtual_router(vrf_h, 0, 2, attr_list);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    SAI call to create router interface
 *
 * Arguments:
 *    [in] intf - router interface info
 *    [out] intf_h - router interface handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int create_interface(const switchlink_db_interface_info_t* intf,
                            switchlink_handle_t* intf_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  if (intf->intf_type == SWITCHLINK_INTF_TYPE_L3) {
    sai_attribute_t attr_list[10];
    int ac = 0;
    memset(attr_list, 0, sizeof(attr_list));
    attr_list[ac].id = SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID;
    attr_list[ac].value.oid = intf->vrf_h;
    ac++;
    attr_list[ac].id = SAI_ROUTER_INTERFACE_ATTR_TYPE;
    attr_list[ac].value.oid = 0;
    if (!(intf_h)) {
      krnlmon_log_error("Interface handle is NULL");
      return -1;
    } else {
      attr_list[ac].value.oid = *intf_h;
    }
    ac++;
    attr_list[ac].id = SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS;
    memcpy(attr_list[ac].value.mac, intf->mac_addr, sizeof(sai_mac_t));
    ac++;
    attr_list[ac].id = SAI_ROUTER_INTERFACE_ATTR_PORT_ID;
    attr_list[ac].value.u32 = intf->ifindex;
    ac++;

    status = sai_rintf_api->create_router_interface(intf_h, 0, ac++, attr_list);
  }
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    SAI call to delete router interface
 *
 * Arguments:
 *    [in] intf - router interface info
 *    [in] intf_h - router interface handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int delete_interface(const switchlink_db_interface_info_t* intf,
                            switchlink_handle_t intf_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  if (intf->intf_type == SWITCHLINK_INTF_TYPE_L3) {
    status = sai_rintf_api->remove_router_interface(intf_h);
  }
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    Wrapper function to create interface
 *
 * Arguments:
 *    [in/out] intf - interface info
 *
 * Return Values:
 *    void
 */

void switchlink_create_interface(switchlink_db_interface_info_t* intf) {
  switchlink_db_status_t status;
  switchlink_db_interface_info_t ifinfo;

  status = switchlink_db_get_interface_info(intf->ifindex, &ifinfo);
  if (status == SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    // create the interface
    krnlmon_log_debug("Switchlink Interface Create: %s", intf->ifname);

    status = create_interface(intf, &(intf->intf_h));
    if (status) {
      krnlmon_log_error(
          "newlink: Failed to create switchlink interface, error: %d\n",
          status);
      return;
    }

    // add the mapping to the db
    switchlink_db_add_interface(intf->ifindex, intf);
  } else {
    // interface has already been created
    if (memcmp(&(ifinfo.mac_addr), &(intf->mac_addr),
               sizeof(switchlink_mac_addr_t))) {
      memcpy(&(ifinfo.mac_addr), &(intf->mac_addr),
             sizeof(switchlink_mac_addr_t));

      // Delete if RMAC is configured previously, and create this new RMAC.
      status = create_interface(&ifinfo, &ifinfo.intf_h);
      if (status) {
        krnlmon_log_error(
            "newlink: Failed to create switchlink interface,"
            " error: %d\n",
            status);
        return;
      }

      switchlink_db_update_interface(intf->ifindex, &ifinfo);
    }
    intf->intf_h = ifinfo.intf_h;
  }
}

/*
 * Routine Description:
 *    Wrapper function to delete interface
 *
 * Arguments:
 *    [in] ifindex - interface index
 *
 * Return Values:
 *    void
 */

void switchlink_delete_interface(uint32_t ifindex) {
  switchlink_db_interface_info_t intf;
  if (switchlink_db_get_interface_info(ifindex, &intf) ==
      SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    return;
  }

  // delete the interface from backend and DB
  delete_interface(&intf, intf.intf_h);
  switchlink_db_delete_interface(intf.ifindex);
}
