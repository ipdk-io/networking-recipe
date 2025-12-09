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

#include <linux/if_ether.h>
#include <net/if.h>
#include <netlink/msg.h>

#include "switchlink.h"
#include "switchlink_globals.h"
#include "switchlink_handlers.h"
#include "switchlink_int.h"
#include "switchutils/switch_log.h"

/*
 * Routine Description:
 *    Process neighbor netlink messages
 *
 * Arguments:
 *    [in] nlmsg - netlink msg header
 *    [in] type - type of entry (RTM_NEWNEIGH/RTM_DELNEIGH)
 *
 * Return Values:
 *    void
 */

void switchlink_process_neigh_msg(const struct nlmsghdr* nlmsg, int msgtype) {
  int hdrlen, attrlen;
  struct nlattr* attr;
  struct ndmsg* nbh;
  switchlink_mac_addr_t mac_addr;
  bool mac_addr_valid = false;
  bool ipaddr_valid = false;
  switchlink_ip_addr_t ipaddr;
  switchlink_handle_t intf_h = SWITCH_LINK_INVALID_HANDLE;

  krnlmon_assert((msgtype == RTM_NEWNEIGH) || (msgtype == RTM_DELNEIGH));
  nbh = nlmsg_data(nlmsg);
  hdrlen = sizeof(struct ndmsg);

  krnlmon_log_debug(
      "%sneigh: family = %d, ifindex = %d, state = 0x%x, \
       flags = 0x%x, type = %u\n",
      ((msgtype == RTM_NEWNEIGH) ? "new" : "del"), nbh->ndm_family,
      nbh->ndm_ifindex, nbh->ndm_state, nbh->ndm_flags, nbh->ndm_type);

  switchlink_db_interface_info_t ifinfo;
  if (switchlink_db_get_interface_info(nbh->ndm_ifindex, &ifinfo) !=
      SWITCHLINK_DB_STATUS_SUCCESS) {
    char intf_name[16] = {0};
    if (!if_indextoname(nbh->ndm_ifindex, intf_name)) {
      krnlmon_log_error("Failed to get ifname for the index: %d",
                        nbh->ndm_ifindex);
      return;
    }
    if_indextoname(nbh->ndm_ifindex, intf_name);
    krnlmon_log_debug(
        "neigh: Failed to get switchlink database interface info "
        "for :%s\n",
        intf_name);
    return;
  }

  memset(&ipaddr, 0, sizeof(switchlink_ip_addr_t));
  attrlen = nlmsg_attrlen(nlmsg, hdrlen);
  attr = nlmsg_attrdata(nlmsg, hdrlen);
  while (nla_ok(attr, attrlen)) {
    int attr_type = nla_type(attr);
    switch (attr_type) {
      case NDA_DST:
        if ((nbh->ndm_state == NUD_REACHABLE) ||
            (nbh->ndm_state == NUD_PERMANENT) ||
            (nbh->ndm_state == NUD_STALE) || (nbh->ndm_state == NUD_FAILED)) {
          ipaddr_valid = true;
          ipaddr.family = nbh->ndm_family;
          if (nbh->ndm_family == AF_INET) {
            ipaddr.ip.v4addr.s_addr = ntohl(nla_get_u32(attr));
            ipaddr.prefix_len = 32;
          } else if (nbh->ndm_family == AF_INET6) {
            memcpy(&(ipaddr.ip.v6addr), nla_data(attr), nla_len(attr));
            ipaddr.prefix_len = 128;
          } else {
            krnlmon_log_error("Invalid NDM family type, for attribute %d\n",
                              attr_type);
            return;
          }
        } else {
          krnlmon_log_debug(
              "Ignoring unused neighbor states for attribute "
              "type %d\n",
              attr_type);
          return;
        }
        break;
      case NDA_LLADDR: {
        krnlmon_assert(nla_len(attr) == sizeof(switchlink_mac_addr_t));
        mac_addr_valid = true;
        memcpy(mac_addr, nla_data(attr), nla_len(attr));
        break;
      }
      default:
        break;
    }
    attr = nla_next(attr, &attrlen);
  }

  if (ifinfo.link_type == SWITCHLINK_LINK_TYPE_BOND) {
    intf_h = ifinfo.lag_h;
  } else {
    intf_h = ifinfo.intf_h;
  }
  switchlink_handle_t bridge_h = g_default_bridge_h;
  if (ifinfo.intf_type == SWITCHLINK_INTF_TYPE_L2_ACCESS) {
    bridge_h = ifinfo.bridge_h;
    krnlmon_assert(bridge_h != 0);
  }

  if (msgtype == RTM_NEWNEIGH) {
    if (ipaddr_valid) {
      if (mac_addr_valid) {
        switchlink_create_neigh(g_default_vrf_h, &ipaddr, mac_addr, intf_h);
      } else {
        /* mac address is not valid, remove the neighbor entry */
        switchlink_delete_neigh(g_default_vrf_h, &ipaddr, intf_h);
      }
    }
  } else {
    if (ipaddr_valid) {
      switchlink_delete_neigh(g_default_vrf_h, &ipaddr, intf_h);
    }
  }
}
