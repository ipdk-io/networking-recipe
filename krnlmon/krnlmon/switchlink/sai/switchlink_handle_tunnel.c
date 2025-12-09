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

static sai_tunnel_api_t* sai_tunnel_intf_api = NULL;

sai_status_t sai_init_tunnel_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_TUNNEL, (void**)&sai_tunnel_intf_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    SAI call to create tunnel
 *
 * Arguments:
 *    [in] tnl_intf - tunnel interface info
 *    [in] tnl_intf_h - tunnel interface handle
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t create_tunnel(
    const switchlink_db_tunnel_interface_info_t* tnl_intf,
    switchlink_handle_t* tnl_intf_h) {
  sai_attribute_t attr_list[10];
  int ac = 0;
  memset(attr_list, 0, sizeof(attr_list));

  // TODO looks like remote is valid only for PEER_MODE = P2P
  if (tnl_intf->src_ip.family == AF_INET) {
    attr_list[ac].id = SAI_TUNNEL_ATTR_ENCAP_SRC_IP;
    attr_list[ac].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr_list[ac].value.ipaddr.addr.ip4 =
        htonl(tnl_intf->src_ip.ip.v4addr.s_addr);
    ac++;
  }

  // TODO looks like remote is valid only for PEER_MODE = P2P
  if (tnl_intf->dst_ip.family == AF_INET) {
    attr_list[ac].id = SAI_TUNNEL_ATTR_ENCAP_DST_IP;
    attr_list[ac].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr_list[ac].value.ipaddr.addr.ip4 =
        htonl(tnl_intf->dst_ip.ip.v4addr.s_addr);
    ac++;
  }

  attr_list[ac].id = SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT;
  attr_list[ac].value.u16 = tnl_intf->dst_port;
  ac++;

  return sai_tunnel_intf_api->create_tunnel(tnl_intf_h, 0, ac, attr_list);
}

/*
 * Routine Description:
 *    SAI call to create tunnel termination table entry
 *
 * Arguments:
 *    [in] tnl_intf - tunnel term interface info
 *    [in] tnl_term_intf_h - tunnel term interface handle
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

static sai_status_t create_term_table_entry(
    const switchlink_db_tunnel_interface_info_t* tnl_intf,
    switchlink_handle_t* tnl_term_intf_h) {
  sai_attribute_t attr_list[10];
  memset(attr_list, 0, sizeof(attr_list));
  int ac = 0;

  if (tnl_intf->dst_ip.family == AF_INET) {
    attr_list[ac].id = SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP;
    attr_list[ac].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr_list[ac].value.ipaddr.addr.ip4 =
        htonl(tnl_intf->dst_ip.ip.v4addr.s_addr);
    ac++;
  }

  if (tnl_intf->src_ip.family == AF_INET) {
    attr_list[ac].id = SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP;
    attr_list[ac].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attr_list[ac].value.ipaddr.addr.ip4 =
        htonl(tnl_intf->src_ip.ip.v4addr.s_addr);
    ac++;
  }

  attr_list[ac].id = SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID;
  attr_list[ac].value.u32 = tnl_intf->vni_id;
  ac++;

  return sai_tunnel_intf_api->create_tunnel_term_table_entry(tnl_term_intf_h, 0,
                                                             ac, attr_list);
}

/*
 * Routine Description:
 *    Wrapper function to create tunnel interface and create tunnel
 *    term table entry
 *
 * Arguments:
 *    [in] tnl_intf - tunnel interface info
 *    [in] tnl_intf_h - tunnel interface handle
 *    [in] tnl_term_h - tunnel term handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int create_tunnel_interface(
    const switchlink_db_tunnel_interface_info_t* tnl_intf,
    switchlink_handle_t* tnl_intf_h, switchlink_handle_t* tnl_term_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = create_tunnel(tnl_intf, tnl_intf_h);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Cannot create tunnel for interface: %s",
                      tnl_intf->ifname);
    return -1;
  }
  krnlmon_log_info("Created tunnel interface: %s", tnl_intf->ifname);

  status = create_term_table_entry(tnl_intf, tnl_term_h);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Cannot create tunnel termination table entry for "
        "interface: %s",
        tnl_intf->ifname);
    return -1;
  }
  krnlmon_log_info(
      "Created tunnel termination entry for "
      "interface: %s",
      tnl_intf->ifname);

  return 0;
}

/*
 * Routine Description:
 *    Wrapper function to delete tunnel interface and delete tunnel
 *    term table entry
 *
 * Arguments:
 *    [in] tnl_intf - tunnel interface info
 *    [in] tnl_intf_h - tunnel interface handle
 *    [in] tnl_term_h - tunnel term handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int delete_tunnel_interface(
    const switchlink_db_tunnel_interface_info_t* tnl_intf) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status =
      sai_tunnel_intf_api->remove_tunnel_term_table_entry(tnl_intf->tnl_term_h);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Cannot remove tunnel termination entry for "
        "interface: %s",
        tnl_intf->ifname);
    return -1;
  }
  krnlmon_log_info(
      "Removed tunnel termination entry for "
      "interface: %s",
      tnl_intf->ifname);

  status = sai_tunnel_intf_api->remove_tunnel(tnl_intf->orif_h);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Cannot remove tunnel entry for "
        "interface: %s",
        tnl_intf->ifname);
    return -1;
  }

  krnlmon_log_info("Removed tunnel entry for interface: %s", tnl_intf->ifname);
  // Add further code to remove tunnel dependent params here.

  return 0;
}

/*
 * Routine Description:
 *    Create tunnel interface
 *
 * Arguments:
 *    [in] tnl_intf - tunnel interface info
 *
 * Return Values:
 *    void
 */

void switchlink_create_tunnel_interface(
    switchlink_db_tunnel_interface_info_t* tnl_intf) {
  switchlink_db_status_t status;
  switchlink_db_tunnel_interface_info_t tnl_ifinfo;

  status =
      switchlink_db_get_tunnel_interface_info(tnl_intf->ifindex, &tnl_ifinfo);
  if (status == SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    krnlmon_log_debug("Switchlink tunnel interface: %s", tnl_intf->ifname);
    status = create_tunnel_interface(tnl_intf, &(tnl_intf->orif_h),
                                     &(tnl_intf->tnl_term_h));
    if (status) {
      krnlmon_log_error(
          "newlink: Failed to create switchlink tunnel interface :%s, "
          "error: %d",
          tnl_intf->ifname, status);
      return;
    }

    // add the mapping to the db
    switchlink_db_add_tunnel_interface(tnl_intf->ifindex, tnl_intf);
    return;
  }
  krnlmon_log_debug(
      "Switchlink DB already has tunnel config for "
      "interface: %s",
      tnl_intf->ifname);
  return;
}

/*
 * Routine Description:
 *    Delete tunnel interface
 *
 * Arguments:
 *    [in] ifindex - interface index
 *
 * Return Values:
 *    void
 */

void switchlink_delete_tunnel_interface(uint32_t ifindex) {
  switchlink_db_tunnel_interface_info_t tnl_intf;
  if (switchlink_db_get_tunnel_interface_info(ifindex, &tnl_intf) ==
      SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    krnlmon_log_info(
        "Trying to delete a tunnel which is not "
        "available");
    return;
  }

  krnlmon_log_debug("Switchlink tunnel interface: %s", tnl_intf.ifname);

  // delete the interface from backend and in DB
  delete_tunnel_interface(&tnl_intf);
  switchlink_db_delete_tunnel_interface(ifindex);

  return;
}
