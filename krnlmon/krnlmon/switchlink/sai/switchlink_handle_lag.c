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

#include <linux/if.h>

#include "switchlink/switchlink_db.h"
#include "switchlink_init_sai.h"
#include "switchsde/sde_port_intf.h"
#include "switchsde/sde_status.h"
#include "switchsde/sde_types.h"

#define SWITCH_PD_MAC_STR_LENGTH 18
#define SWITCH_PD_TARGET_VPORT_OFFSET 16

static sai_lag_api_t* sai_lag_api = NULL;

/**
 * Routine Description:
 *    Initialize LAG SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */
sai_status_t sai_init_lag_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_LAG, (void**)&sai_lag_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/**
 * Routine Description:
 *    SAI call to create lag
 *
 * Arguments:
 *    [in] lag - lag info
 *    [out] lag_h - lag handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */
static int create_lag(const switchlink_db_interface_info_t* lag_intf,
                      switchlink_handle_t* lag_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  if (lag_intf->link_type == SWITCHLINK_LINK_TYPE_BOND) {
    sai_attribute_t attr_list[5];
    int ac = 0;
    memset(attr_list, 0, sizeof(attr_list));
    attr_list[ac].id = SAI_LAG_ATTR_CUSTOM_RANGE_START;
    memcpy(attr_list[ac].value.mac, lag_intf->mac_addr, sizeof(sai_mac_t));
    ac++;

    attr_list[ac].id = SAI_LAG_ATTR_INGRESS_ACL;
    attr_list[ac].value.oid = 0;
    if (!(lag_h)) {
      krnlmon_log_error("LAG handle is NULL");
      return -1;
    } else {
      attr_list[ac].value.oid = *lag_h;
    }
    ac++;

    attr_list[ac].id = SAI_LAG_ATTR_PORT_VLAN_ID;
    attr_list[ac].value.u8 = lag_intf->bond_mode;
    ac++;

    status = sai_lag_api->create_lag(lag_h, 0, ac, attr_list);
  }
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/**
 * Routine Description:
 *    SAI call to delete lag
 *
 * Arguments:
 *    [in] lag - lag info
 *    [in] lag_h - lag handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */
static int delete_lag(const switchlink_db_interface_info_t* lag,
                      switchlink_handle_t lag_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  if (lag->link_type == SWITCHLINK_LINK_TYPE_BOND) {
    status = sai_lag_api->remove_lag(lag_h);
  }
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/**
 * Routine Description:
 *    SAI call to set lag attribute
 *
 * Arguments:
 *    [in] lag - lag info
 *    [out] lag_h - lag member handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */
static int set_lag_attribute(const switchlink_db_interface_info_t* lag_info,
                             switchlink_handle_t lag_member_h) {
  sai_attribute_t attr;
  memset(&attr, 0, sizeof(attr));

  attr.id = SAI_LAG_ATTR_EGRESS_ACL;
  attr.value.oid = lag_member_h;
  return sai_lag_api->set_lag_attribute(lag_info->lag_h, &attr);
}

/**
 * Routine Description:
 *    Calls SAI to set lag member attribute
 *
 * Arguments:
 *    [in] lag_member_info - lag member info
 *    [out] oper_state - oper state
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */
static int set_lag_member_attribute(
    const switchlink_db_lag_member_info_t* lag_member_info,
    uint8_t oper_state) {
  sai_attribute_t attr;
  memset(&attr, 0, sizeof(attr));

  attr.id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;
  attr.value.booldata = (oper_state == IF_OPER_DOWN) ? false : true;
  return sai_lag_api->set_lag_member_attribute(lag_member_info->lag_member_h,
                                               &attr);
}

/**
 * Routine Description:
 *    SAI call to create lag member
 *
 * Arguments:
 *    [in] lag - lag member info
 *    [out] lag_member_h - lag member handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */
static int create_lag_member(
    const switchlink_db_lag_member_info_t* lag_member_intf,
    switchlink_handle_t* lag_member_h) {
  sai_attribute_t attr_list[5];
  int ac = 0;
  sde_status_t sde_status;
  uint32_t port_id = 0;
  sde_dev_id_t sde_dev_id = 0;
  static char mac_str[SWITCH_PD_MAC_STR_LENGTH];

  snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
           lag_member_intf->perm_hwaddr[0], lag_member_intf->perm_hwaddr[1],
           lag_member_intf->perm_hwaddr[2], lag_member_intf->perm_hwaddr[3],
           lag_member_intf->perm_hwaddr[4], lag_member_intf->perm_hwaddr[5]);
  mac_str[SWITCH_PD_MAC_STR_LENGTH - 1] = '\0';

  sde_status = sde_pal_get_port_id_from_mac(sde_dev_id, mac_str, &port_id);
  if (sde_status != SDE_SUCCESS) {
    port_id = lag_member_intf->perm_hwaddr[1] + SWITCH_PD_TARGET_VPORT_OFFSET;
    krnlmon_log_info(
        "Failed to get the port ID, error: %d, Deriving "
        "port ID from second byte of MAC address: "
        "%s",
        sde_status, mac_str);
  }

  memset(attr_list, 0, sizeof(attr_list));
  attr_list[ac].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
  attr_list[ac].value.oid = lag_member_intf->lag_h;
  ac++;

  attr_list[ac].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
  attr_list[ac].value.u32 = port_id;
  ac++;

  // this SAI attribute is used to propagate oper state to switchapi module.
  attr_list[ac].id = SAI_LAG_MEMBER_ATTR_EGRESS_DISABLE;
  if (lag_member_intf->oper_state == IF_OPER_DOWN)
    attr_list[ac].value.booldata = false;
  else
    attr_list[ac].value.booldata = true;
  ac++;

  return sai_lag_api->create_lag_member(lag_member_h, 0, ac, attr_list);
}

/**
 * Routine Description:
 *    SAI call to delete lag member
 *
 * Arguments:
 *    [in] lag - lag member info
 *    [in] lag_member_h - lag member handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */
static int delete_lag_member(const switchlink_db_lag_member_info_t* lag_member,
                             switchlink_handle_t lag_member_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  status = sai_lag_api->remove_lag_member(lag_member_h);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/**
 * Routine Description:
 *    Wrapper function to create lag
 *
 * Arguments:
 *    [in/out] lag - lag info
 *
 * Return Values:
 *    void
 */
void switchlink_create_lag(switchlink_db_interface_info_t* lag_intf) {
  switchlink_db_status_t status;
  switchlink_db_interface_info_t lag_info;

  status = switchlink_db_get_interface_info(lag_intf->ifindex, &lag_info);
  if (status == SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    // create the lag
    krnlmon_log_debug("Switchlink LAG Interface Create: %s", lag_intf->ifname);

    status = create_lag(lag_intf, &(lag_intf->lag_h));
    if (status) {
      krnlmon_log_error(
          "newlink: Failed to create switchlink lag interface: %s, error: %d\n",
          lag_intf->ifname, status);
      return;
    }
    // add the mapping to the db
    switchlink_db_add_interface(lag_intf->ifindex, lag_intf);
    switchlink_db_add_mac_lag(lag_intf->mac_addr, lag_intf->lag_h);
    return;
  } else {
    krnlmon_log_debug("Switchlink DB already has LAG config: %s",
                      lag_intf->ifname);
    if (lag_intf->bond_mode == SWITCHLINK_BOND_MODE_ACTIVE_BACKUP) {
      // check if active_slave attribute is updated.
      if ((lag_intf->active_slave != lag_info.active_slave) ||
          (lag_intf->oper_state != lag_info.oper_state)) {
        // need to program MEV-TS with new active_slave info
        // get the lag member handle with ifindex = active_slave
        if ((lag_intf->active_slave != 0) &&
            (lag_intf->oper_state == IF_OPER_UP)) {
          switchlink_db_lag_member_info_t lag_member_info;
          memset(&lag_member_info, 0, sizeof(switchlink_db_lag_member_info_t));
          lag_member_info.ifindex = lag_intf->active_slave;
          status = switchlink_db_get_lag_member_info(&lag_member_info);
          if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
            status = set_lag_attribute(&lag_info, lag_member_info.lag_member_h);
            if (status) {
              krnlmon_log_error(
                  "newlink: Failed to update switchlink lag: %s, error: %d\n",
                  lag_intf->ifname, status);
              return;
            }
          }
        } else if ((lag_intf->active_slave == 0) ||
                   (lag_intf->oper_state == IF_OPER_DOWN)) {
          status = set_lag_attribute(&lag_info, 0);
          if (status) {
            krnlmon_log_error(
                "newlink: Failed to update switchlink lag: %s, error: %d\n",
                lag_intf->ifname, status);
            return;
          }
        }

        lag_info.oper_state = lag_intf->oper_state;
        lag_info.active_slave = lag_intf->active_slave;
      }
    }

    if (memcmp(&lag_info.mac_addr, &lag_intf->mac_addr,
               sizeof(switchlink_mac_addr_t))) {
      memcpy(&lag_info.mac_addr, &lag_intf->mac_addr,
             sizeof(switchlink_mac_addr_t));

      // Delete if RMAC is configured previously, and create this new RMAC.
      status = create_lag(&lag_info, &lag_info.lag_h);
      if (status) {
        krnlmon_log_error(
            "newlink: Failed to create switchlink lag interface,"
            " error: %d\n",
            status);
        return;
      }
      switchlink_db_add_mac_lag(lag_intf->mac_addr, lag_info.lag_h);
    }

    // update the LAG db structure
    switchlink_db_update_interface(lag_intf->ifindex, &lag_info);
    lag_intf->lag_h = lag_info.lag_h;
  }
  return;
}

/**
 * Routine Description:
 *    Wrapper function to delete lag member
 *
 * Arguments:
 *    [in] ifindex - lag member ifindex
 *
 * Return Values:
 *    void
 */
void switchlink_delete_lag_member(uint32_t ifindex) {
  switchlink_db_lag_member_info_t lag_member_info;
  memset(&lag_member_info, 0, sizeof(switchlink_db_lag_member_info_t));
  lag_member_info.ifindex = ifindex;
  if (switchlink_db_get_lag_member_info(&lag_member_info) ==
      SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    return;
  }

  // delete the lag member from backend and DB
  delete_lag_member(&lag_member_info, lag_member_info.lag_member_h);
  switchlink_db_delete_lag_member(&lag_member_info);
}

/**
 * Routine Description:
 *    Wrapper function to delete lag
 *
 * Arguments:
 *    [in] ifindex - lag ifindex
 *
 * Return Values:
 *    void
 */
void switchlink_delete_lag(uint32_t ifindex) {
  switchlink_db_interface_info_t lag_intf;
  if (switchlink_db_get_interface_info(ifindex, &lag_intf) ==
      SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    return;
  }

  /* Delete LAG members for an LACP */
  if (lag_intf.bond_mode == SWITCHLINK_BOND_MODE_LACP) {
    uint32_t member_if_index = 0;
    while (member_if_index != -1) {
      member_if_index = switchlink_db_delete_lacp_member(lag_intf.lag_h);
      if (member_if_index != -1) {
        switchlink_delete_lag_member(member_if_index);
      }
    }
  }

  // delete the lag from backend and DB
  delete_lag(&lag_intf, lag_intf.lag_h);
  switchlink_db_delete_interface(lag_intf.ifindex);
  switchlink_db_delete_mac_lag(lag_intf.mac_addr);
}

/**
 * Routine Description:
 *    Wrapper function to create lag member
 *
 * Arguments:
 *    [in/out] lag - lag member info
 *
 * Return Values:
 *    void
 */
void switchlink_create_lag_member(
    switchlink_db_lag_member_info_t* lag_member_intf) {
  switchlink_db_status_t status;
  switchlink_handle_t lag_h;
  switchlink_db_lag_member_info_t lag_member_info;

  memset(&lag_member_info, 0, sizeof(switchlink_db_lag_member_info_t));
  lag_member_info.ifindex = lag_member_intf->ifindex;

  status = switchlink_db_get_lag_member_info(&lag_member_info);
  if (status == SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    status =
        switchlink_db_get_mac_lag_handle(lag_member_intf->mac_addr, &lag_h);
    if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
      lag_member_intf->lag_h = lag_h;
      krnlmon_log_debug("Parent LAG handle is: 0x%lx", lag_h);
    }

    // create the lag member
    krnlmon_log_debug("Switchlink LAG Member Create: %s",
                      lag_member_intf->ifname);
    status =
        create_lag_member(lag_member_intf, &(lag_member_intf->lag_member_h));
    if (status) {
      krnlmon_log_error(
          "newlink: Failed to create switchlink LAG member: %s, error: %d\n",
          lag_member_intf->ifname, status);
      return;
    }
    // add the mapping to the db
    switchlink_db_add_lag_member(lag_member_intf);
    return;
  }

  lag_member_intf->lag_member_h = lag_member_info.lag_member_h;
  // lag member has already been created
  krnlmon_log_debug("Switchlink DB already has LAG Member config: %s",
                    lag_member_intf->ifname);

  if (lag_member_intf->is_lacp_member) {
    // check if lag member oper_state has changed
    if (lag_member_info.oper_state != lag_member_intf->oper_state) {
      status = set_lag_member_attribute(&lag_member_info,
                                        lag_member_intf->oper_state);
      if (status) {
        krnlmon_log_error(
            "newlink: Failed to update switchlink lag member: %s, error: %d\n",
            lag_member_intf->ifname, status);
        return;
      }
      // update the oper_state in the switchlink db
      lag_member_info.oper_state = lag_member_intf->oper_state;
      status = switchlink_db_update_lag_member_oper_state(&lag_member_info);
      if (status) {
        krnlmon_log_error(
            "newlink: Failed to update oper state of lag member: %s, error: "
            "%d\n",
            lag_member_intf->ifname, status);
        return;
      }
      lag_member_intf->lag_member_h = lag_member_info.lag_member_h;
    }
  }

  return;
}
