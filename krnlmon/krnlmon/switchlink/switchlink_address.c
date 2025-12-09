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

#include <netlink/msg.h>

#include "switchlink_globals.h"
#include "switchlink_handlers.h"
#include "switchlink_int.h"
#include "switchutils/switch_log.h"

/*
 * Routine Description:
 *    Process address netlink messages
 *
 * Arguments:
 *    [in] nlmsg - netlink msg header
 *    [in] type - netlink msg type
 *
 * Return Values:
 *    void
 */

void switchlink_process_address_msg(const struct nlmsghdr* nlmsg, int msgtype) {
  int hdrlen, attrlen;
  struct nlattr* attr;
  struct ifaddrmsg* addrmsg;
  bool addr_valid = false;
  switchlink_ip_addr_t addr;

  krnlmon_assert((msgtype == RTM_NEWADDR) || (msgtype == RTM_DELADDR));
  addrmsg = nlmsg_data(nlmsg);
  hdrlen = sizeof(struct ifaddrmsg);
  krnlmon_log_debug(
      "%saddr: family = %d, prefixlen = %d, flags = 0x%x, "
      "scope = 0x%x ifindex = %d\n",
      ((msgtype == RTM_NEWADDR) ? "new" : "del"), addrmsg->ifa_family,
      addrmsg->ifa_prefixlen, addrmsg->ifa_flags, addrmsg->ifa_scope,
      addrmsg->ifa_index);

  if ((addrmsg->ifa_family != AF_INET) && (addrmsg->ifa_family != AF_INET6)) {
    // an address family that we are not interested in, skip
    return;
  }

  switchlink_db_status_t status;
  switchlink_handle_t intf_h = 0;

  switchlink_db_interface_info_t ifinfo;
  status = switchlink_db_get_interface_info(addrmsg->ifa_index, &ifinfo);
  if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
    krnlmon_log_debug("Found interface cache for: %s", ifinfo.ifname);
    if (ifinfo.link_type == SWITCHLINK_LINK_TYPE_BOND) {
      intf_h = ifinfo.lag_h;
    } else {
      intf_h = ifinfo.intf_h;
    }
  } else {
    // TODO P4-OVS, for now we ignore these notifications.
    // Needed when, we get address add before port create notification
    krnlmon_log_debug("Ignoring interface address notification ifindex: %d",
                      addrmsg->ifa_index);
    return;
  }

  attrlen = nlmsg_attrlen(nlmsg, hdrlen);
  attr = nlmsg_attrdata(nlmsg, hdrlen);
  while (nla_ok(attr, attrlen)) {
    int attr_type = nla_type(attr);
    switch (attr_type) {
      case IFA_ADDRESS:
        addr_valid = true;
        memset(&addr, 0, sizeof(switchlink_ip_addr_t));
        addr.family = addrmsg->ifa_family;
        addr.prefix_len = addrmsg->ifa_prefixlen;
        if (addrmsg->ifa_family == AF_INET) {
          addr.ip.v4addr.s_addr = ntohl(nla_get_u32(attr));
        } else {
          memcpy(&(addr.ip.v6addr), nla_data(attr), nla_len(attr));
        }
        break;
      default:
        break;
    }
    attr = nla_next(attr, &attrlen);
  }

  if (msgtype == RTM_NEWADDR) {
    if (addr_valid) {
      switchlink_ip_addr_t null_gateway;
      memset(&null_gateway, 0, sizeof(null_gateway));
      null_gateway.family = addr.family;

      // add the subnet route for prefix_len, derived from IFA_ADDRESS
      switchlink_create_route(g_default_vrf_h, &addr, &null_gateway, 0, intf_h);

      // add the interface route
      if (addrmsg->ifa_family == AF_INET) {
        addr.prefix_len = 32;
      } else {
        addr.prefix_len = 128;
      }
      // Add a route with new prefix_len
      switchlink_create_route(g_default_vrf_h, &addr, &null_gateway, 0, intf_h);
    }
  } else {
    if (addr_valid) {
      // remove the subnet route
      switchlink_delete_route(g_default_vrf_h, &addr);

      // remove the interface route
      if (addrmsg->ifa_family == AF_INET) {
        addr.prefix_len = 32;
      } else {
        addr.prefix_len = 128;
      }
      switchlink_delete_route(g_default_vrf_h, &addr);
    }
  }
}
