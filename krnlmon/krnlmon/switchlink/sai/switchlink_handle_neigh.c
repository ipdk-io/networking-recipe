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

#include "switchlink/switchlink_globals.h"
#include "switchlink/switchlink_handlers.h"
#include "switchlink_init_sai.h"

static sai_neighbor_api_t* sai_neigh_api = NULL;
static sai_fdb_api_t* sai_fdb_api = NULL;

/*
 * Routine Description:
 *    Initialize Neighbor SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

sai_status_t sai_init_neigh_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_NEIGHBOR, (void**)&sai_neigh_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    Initialize FDB SAI API
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 */

sai_status_t sai_init_fdb_api() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_api_query(SAI_API_FDB, (void**)&sai_fdb_api);
  krnlmon_assert(status == SAI_STATUS_SUCCESS);

  return status;
}

/*
 * Routine Description:
 *    Validate if any other feature is using this NHOP
 *
 * Arguments:
 *    [in] using_by - Flag which has consolidated features using this nhop.
 *    [in] type - Feature enum type, to be checked if this feature is the only
 *                feature referring to this NHOP.
 *
 * Return Values:
 *    true: if this NHOP can be deleted.
 *    false: if this NHOP is referred by other features.
 */

bool validate_delete_nexthop(uint32_t using_by,
                             enum switchlink_nhop_using_by type) {
  return (using_by & ~(type)) ? false : true;
}

/*
 * Routine Description:
 *    SAI call to create FDB entry
 *
 * Arguments:
 *    [in] mac_addr - MAC address
 *    [in] bridge_h - bridge handle
 *    [in] intf_h - interface handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int create_mac(switchlink_mac_addr_t mac_addr,
                      switchlink_handle_t bridge_h,
                      switchlink_handle_t intf_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_fdb_entry_t fdb_entry;
  memset(&fdb_entry, 0, sizeof(fdb_entry));
  memcpy(fdb_entry.mac_address, mac_addr, sizeof(sai_mac_t));
  fdb_entry.bv_id = bridge_h;

  sai_attribute_t attr_list[3];
  memset(&attr_list, 0, sizeof(attr_list));

  attr_list[0].id = SAI_FDB_ENTRY_ATTR_TYPE;
  attr_list[0].value.s32 = SAI_FDB_ENTRY_TYPE_STATIC;
  attr_list[1].id = SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID;
  attr_list[1].value.oid = intf_h;
  attr_list[2].id = SAI_FDB_ENTRY_ATTR_META_DATA;
  attr_list[2].value.u16 = SAI_L2_FWD_LEARN_PHYSICAL_INTERFACE;

  status = sai_fdb_api->create_fdb_entry(&fdb_entry, 3, attr_list);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    SAI call to delete FDB entry
 *
 * Arguments:
 *    [in] mac_addr - MAC address
 *    [in] bridge_h - bridge handle
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int delete_mac(switchlink_mac_addr_t mac_addr,
                      switchlink_handle_t bridge_h) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_fdb_entry_t fdb_entry;
  memset(&fdb_entry, 0, sizeof(fdb_entry));
  memcpy(fdb_entry.mac_address, mac_addr, sizeof(sai_mac_t));
  fdb_entry.bv_id = bridge_h;

  status = sai_fdb_api->remove_fdb_entry(&fdb_entry);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    Create neighbor entry
 *
 * Arguments:
 *    [in] neigh_info - neighbor interface info
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int create_neighbor(const switchlink_db_neigh_info_t* neigh_info) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_attribute_t attr_list[1];
  memset(attr_list, 0, sizeof(attr_list));
  attr_list[0].id = SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS;
  memcpy(attr_list[0].value.mac, neigh_info->mac_addr, sizeof(sai_mac_t));

  sai_neighbor_entry_t neighbor_entry;
  memset(&neighbor_entry, 0, sizeof(neighbor_entry));
  neighbor_entry.rif_id = neigh_info->intf_h;
  if (neigh_info->ip_addr.family == AF_INET) {
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 =
        htonl(neigh_info->ip_addr.ip.v4addr.s_addr);
    krnlmon_log_info("Create a neighbor entry: 0x%x",
                     neigh_info->ip_addr.ip.v4addr.s_addr);
  } else {
    krnlmon_assert(neigh_info->ip_addr.family == AF_INET6);
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(neighbor_entry.ip_address.addr.ip6, &(neigh_info->ip_addr.ip.v6addr),
           sizeof(sai_ip6_t));
    krnlmon_log_info("Create a neighbor entry: 0x%x:0x%x:0x%x:0x%x",
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[0],
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[1],
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[2],
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[3]);
  }

  status = sai_neigh_api->create_neighbor_entry(&neighbor_entry, 1, attr_list);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    Remove neighbor entry
 *
 * Arguments:
 *    [in] neigh_info - neighbor interface info
 *
 * Return Values:
 *    0 on success
 *   -1 in case of error
 */

static int delete_neighbor(const switchlink_db_neigh_info_t* neigh_info) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  sai_neighbor_entry_t neighbor_entry;
  memset(&neighbor_entry, 0, sizeof(neighbor_entry));
  neighbor_entry.rif_id = neigh_info->intf_h;

  if (neigh_info->ip_addr.family == AF_INET) {
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    neighbor_entry.ip_address.addr.ip4 =
        htonl(neigh_info->ip_addr.ip.v4addr.s_addr);
    krnlmon_log_info("Delete a neighbor entry: 0x%x",
                     neigh_info->ip_addr.ip.v4addr.s_addr);
  } else {
    neighbor_entry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(neighbor_entry.ip_address.addr.ip6, &(neigh_info->ip_addr.ip.v6addr),
           sizeof(sai_ip6_t));
    krnlmon_log_info("Delete a neighbor entry: 0x%x:0x%x:0x%x:0x%x",
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[0],
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[1],
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[2],
                     neigh_info->ip_addr.ip.v6addr.__in6_u.__u6_addr32[3]);
  }

  status = sai_neigh_api->remove_neighbor_entry(&neighbor_entry);
  return ((status == SAI_STATUS_SUCCESS) ? 0 : -1);
}

/*
 * Routine Description:
 *    Delete MAC entry
 *
 * Arguments:
 *    [in] mac_addr - MAC address associated with entry
 *    [in] bridge_h - bridge handle
 *
 * Return Values:
 *    void
 */

void switchlink_delete_mac(switchlink_mac_addr_t mac_addr,
                           switchlink_handle_t bridge_h) {
  switchlink_handle_t intf_h;
  switchlink_db_status_t status;
  status = switchlink_db_get_mac_intf(mac_addr, bridge_h, &intf_h);
  if (status != SWITCHLINK_DB_STATUS_SUCCESS) {
    return;
  }
  krnlmon_log_info("Delete a FDB entry: %x:%x:%x:%x:%x:%x", mac_addr[0],
                   mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                   mac_addr[5]);
  delete_mac(mac_addr, bridge_h);
  switchlink_db_delete_mac(mac_addr, bridge_h);
}

/*
 * Routine Description:
 *    Create MAC entry
 *
 * Arguments:
 *    [in] mac_addr - MAC address associated with entry
 *    [in] bridge_h - bridge handle
 *    [in] intf_h - interface handle
 *
 * Return Values:
 *    void
 */

void switchlink_create_mac(switchlink_mac_addr_t mac_addr,
                           switchlink_handle_t bridge_h,
                           switchlink_handle_t intf_h) {
  switchlink_handle_t old_intf_h;
  switchlink_db_status_t status;
  status = switchlink_db_get_mac_intf(mac_addr, bridge_h, &old_intf_h);
  if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
    if (old_intf_h != intf_h) {
      delete_mac(mac_addr, bridge_h);
    } else {
      krnlmon_log_debug("FDB entry already exist");
      return;
    }
  }
  krnlmon_log_info("Create a FDB entry: %x:%x:%x:%x:%x:%x", mac_addr[0],
                   mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                   mac_addr[5]);

  create_mac(mac_addr, bridge_h, intf_h);
  switchlink_db_add_mac(mac_addr, bridge_h, intf_h);
}

/*
 * Routine Description:
 *    Wrapper function to create neighbor, nexthop, route entry
 *
 * Arguments:
 *    [in] vrf_h - vrf handle
 *    [in] ipaddr - IP address associated with neighbor entry
 *    [in] mac_addr - MAC address associated with neighbor entry
 *    [in] intf_h - interface handle
 *
 * Return Values:
 *    void
 */

void switchlink_create_neigh(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* ipaddr,
                             switchlink_mac_addr_t mac_addr,
                             switchlink_handle_t intf_h) {
  bool nhop_available = false;
  switchlink_db_status_t status;
  switchlink_db_neigh_info_t neigh_info;
  switchlink_db_nexthop_info_t nexthop_info;

  if ((ipaddr->family == AF_INET6) &&
      IN6_IS_ADDR_MULTICAST(&(ipaddr->ip.v6addr))) {
    return;
  }

  memset(&neigh_info, 0, sizeof(switchlink_db_neigh_info_t));
  neigh_info.vrf_h = vrf_h;
  neigh_info.intf_h = intf_h;
  memcpy(&(neigh_info.ip_addr), ipaddr, sizeof(switchlink_ip_addr_t));

  status = switchlink_db_get_neighbor_info(&neigh_info);
  if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
    if (memcmp(neigh_info.mac_addr, mac_addr, sizeof(switchlink_mac_addr_t)) ==
        0) {
      // no change
      return;
    }

    // update, currently handled as a delete followed by add
    switchlink_delete_neigh(vrf_h, ipaddr, intf_h);
  }

  memcpy(neigh_info.mac_addr, mac_addr, sizeof(switchlink_mac_addr_t));

  memset(&nexthop_info, 0, sizeof(switchlink_db_nexthop_info_t));
  nexthop_info.vrf_h = vrf_h;
  nexthop_info.intf_h = intf_h;
  memcpy(&(nexthop_info.ip_addr), ipaddr, sizeof(switchlink_ip_addr_t));

  status = switchlink_db_get_nexthop_info(&nexthop_info);
  if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
    nhop_available = true;
  }

  if (!nhop_available && switchlink_create_nexthop(&nexthop_info) == -1) {
    return;
  }

  if (create_neighbor(&neigh_info) == -1) {
    if (!nhop_available) {
      switchlink_delete_nexthop(nexthop_info.nhop_h);
    }
    return;
  }

  switchlink_db_add_neighbor(&neigh_info);

  nexthop_info.using_by |= SWITCHLINK_NHOP_FROM_NEIGHBOR;
  if (!nhop_available) {
    switchlink_db_add_nexthop(&nexthop_info);
  } else {
    switchlink_db_update_nexthop_using_by(&nexthop_info);
  }

#if defined(DPDK_TARGET)
  // add a host route
  switchlink_create_route(g_default_vrf_h, ipaddr, ipaddr, 0, intf_h);
#endif
}

/*
 * Routine Description:
 *    Wrapper function to delete neighbor, nexthop, route entry
 *
 * Arguments:
 *    [in] vrf_h - vrf handle
 *    [in] ipaddr - IP address associated with neighbor entry
 *    [in] intf_h - interface handle
 *
 * Return Values:
 *    void
 */

void switchlink_delete_neigh(switchlink_handle_t vrf_h,
                             const switchlink_ip_addr_t* ipaddr,
                             switchlink_handle_t intf_h) {
  switchlink_db_nexthop_info_t nexthop_info;
  switchlink_db_neigh_info_t neigh_info;
  switchlink_db_status_t status;

  memset(&neigh_info, 0, sizeof(switchlink_db_neigh_info_t));
  neigh_info.vrf_h = vrf_h;
  neigh_info.intf_h = intf_h;
  memcpy(&(neigh_info.ip_addr), ipaddr, sizeof(switchlink_ip_addr_t));
  status = switchlink_db_get_neighbor_info(&neigh_info);
  if (status != SWITCHLINK_DB_STATUS_SUCCESS) {
    return;
  }

  memset(&nexthop_info, 0, sizeof(switchlink_db_nexthop_info_t));
  nexthop_info.vrf_h = vrf_h;
  nexthop_info.intf_h = intf_h;
  memcpy(&(nexthop_info.ip_addr), ipaddr, sizeof(switchlink_ip_addr_t));

  switchlink_delete_mac(neigh_info.mac_addr, g_default_bridge_h);
  krnlmon_log_info("Delete a neighbor entry: 0x%x", ipaddr->ip.v4addr.s_addr);
  delete_neighbor(&neigh_info);
  switchlink_db_delete_neighbor(&neigh_info);

  status = switchlink_db_get_nexthop_info(&nexthop_info);
  if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
    if (validate_delete_nexthop(nexthop_info.using_by,
                                SWITCHLINK_NHOP_FROM_NEIGHBOR)) {
      krnlmon_log_debug("Deleting nhop with neighbor delete 0x%lx",
                        nexthop_info.nhop_h);
      switchlink_delete_nexthop(nexthop_info.nhop_h);
      switchlink_db_delete_nexthop(&nexthop_info);
    } else {
      krnlmon_log_debug("Removing Neighbor learn from nhop");
      nexthop_info.using_by &= ~SWITCHLINK_NHOP_FROM_NEIGHBOR;
      switchlink_db_update_nexthop_using_by(&nexthop_info);
    }
  }

#if defined(DPDK_TARGET)
  // delete the host route
  switchlink_delete_route(g_default_vrf_h, ipaddr);
#endif
}
