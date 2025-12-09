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

#include <netlink/attr.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>

#include "switchlink.h"
#include "switchlink_globals.h"
#include "switchlink_handlers.h"
#include "switchlink_int.h"
#include "switchutils/switch_log.h"

/*
 * Routine Description:
 *    Process ecmp netlink messages
 *
 * Arguments:
 *    [in] family - INET family
 *    [in] attr - netlink attribute
 *    [in] vrf_h - vrf handle
 *
 * Return Values:
 *    ecmp handle in case of sucess
 *    0 in case of failure
 */

static switchlink_handle_t process_ecmp(uint8_t family, struct nlattr* attr,
                                        switchlink_handle_t vrf_h) {
  switchlink_db_status_t status;

  if ((family != AF_INET) && (family != AF_INET6)) {
    return 0;
  }

  switchlink_db_ecmp_info_t ecmp_info;
  memset(&ecmp_info, 0, sizeof(switchlink_db_ecmp_info_t));

  struct rtnexthop* rnh = (struct rtnexthop*)nla_data(attr);
  int attrlen = nla_len(attr);
  while (RTNH_OK(rnh, attrlen)) {
    struct rtattr* rta = RTNH_DATA(rnh);
    if (rta->rta_type == RTA_GATEWAY) {
      switchlink_ip_addr_t gateway;
      memset(&gateway, 0, sizeof(switchlink_ip_addr_t));
      gateway.family = family;
      if (family == AF_INET) {
        gateway.ip.v4addr.s_addr = ntohl(*((uint32_t*)RTA_DATA(rta)));
        gateway.prefix_len = 32;
      } else {
        memcpy(&(gateway.ip.v6addr), (struct in6_addr*)RTA_DATA(rta),
               sizeof(struct in6_addr));
        gateway.prefix_len = 128;
      }

      switchlink_db_interface_info_t ifinfo;
      memset(&ifinfo, 0, sizeof(switchlink_db_interface_info_t));
      status = switchlink_db_get_interface_info(rnh->rtnh_ifindex, &ifinfo);
      if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
        switchlink_db_nexthop_info_t nexthop_info;
        memset(&nexthop_info, 0, sizeof(switchlink_db_nexthop_info_t));
        memcpy(&(nexthop_info.ip_addr), &gateway, sizeof(switchlink_ip_addr_t));
        nexthop_info.intf_h = ifinfo.intf_h;
        nexthop_info.vrf_h = vrf_h;
        status = switchlink_db_get_nexthop_info(&nexthop_info);
        if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
          krnlmon_log_debug(
              "Fetched nhop 0x%lx handler, update from"
              " route",
              nexthop_info.nhop_h);
          ecmp_info.nhops[ecmp_info.num_nhops] = nexthop_info.nhop_h;
          nexthop_info.using_by |= SWITCHLINK_NHOP_FROM_ROUTE;
          switchlink_db_update_nexthop_using_by(&nexthop_info);
        } else {
          if (!switchlink_create_nexthop(&nexthop_info)) {
            krnlmon_log_debug(
                "Created nhop 0x%lx handler, update from"
                " route",
                nexthop_info.nhop_h);
            ecmp_info.nhops[ecmp_info.num_nhops] = nexthop_info.nhop_h;
            nexthop_info.using_by |= SWITCHLINK_NHOP_FROM_ROUTE;
            switchlink_db_add_nexthop(&nexthop_info);
          } else {
            ecmp_info.nhops[ecmp_info.num_nhops] = g_cpu_rx_nhop_h;
          }
        }
        ecmp_info.num_nhops++;
        krnlmon_assert(ecmp_info.num_nhops < SWITCHLINK_ECMP_NUM_MEMBERS_MAX);
      }
    }
    rnh = RTNH_NEXT(rnh);
  }

  if (!ecmp_info.num_nhops) {
    return 0;
  }

  status = switchlink_db_get_ecmp_info(&ecmp_info);
  if (status == SWITCHLINK_DB_STATUS_ITEM_NOT_FOUND) {
    switchlink_create_ecmp(&ecmp_info);
    switchlink_db_add_ecmp(&ecmp_info);
  }

  return ecmp_info.ecmp_h;
}

/* TODO: P4-OVS: Dummy Processing of Netlink messages received
 * Support IPv4 Routing
 */

/*
 * Routine Description:
 *    Process route netlink messages
 *
 * Arguments:
 *    [in] nlmsg - netlink msg header
 *    [in] type - type of entry (RTM_NEWROUTE/RTM_DELROUTE)
 *
 * Return Values:
 *    void
 */

void switchlink_process_route_msg(const struct nlmsghdr* nlmsg, int msgtype) {
  int hdrlen, attrlen;
  struct nlattr* attr;
  struct rtmsg* rmsg;
  bool dst_valid = false;
  bool gateway_valid = false;
  switchlink_handle_t ecmp_h = 0;
  switchlink_handle_t intf_h = 0;
  switchlink_ip_addr_t src_addr;
  switchlink_ip_addr_t dst_addr;
  switchlink_ip_addr_t gateway_addr;
  switchlink_db_interface_info_t ifinfo;
  uint8_t af = AF_UNSPEC;
  bool oif_valid = false;
  uint32_t oif = 0;

  krnlmon_assert((msgtype == RTM_NEWROUTE) || (msgtype == RTM_DELROUTE));
  rmsg = nlmsg_data(nlmsg);
  hdrlen = sizeof(struct rtmsg);
  krnlmon_log_debug(
      "%sroute: family = %d, dst_len = %d, src_len = %d, tos = %d, "
      "table = %d, proto = %d, scope = %d, type = %d, "
      "flags = 0x%x\n",
      ((msgtype == RTM_NEWROUTE) ? "new" : "del"), rmsg->rtm_family,
      rmsg->rtm_dst_len, rmsg->rtm_src_len, rmsg->rtm_tos, rmsg->rtm_table,
      rmsg->rtm_protocol, rmsg->rtm_scope, rmsg->rtm_type, rmsg->rtm_flags);

  if (rmsg->rtm_family > AF_MAX) {
    krnlmon_assert(rmsg->rtm_type == RTN_MULTICAST);
    if (rmsg->rtm_family == RTNL_FAMILY_IPMR) {
      af = AF_INET;
    } else if (rmsg->rtm_family == RTNL_FAMILY_IP6MR) {
      af = AF_INET6;
    }
  } else {
    af = rmsg->rtm_family;
  }

  if ((af != AF_INET) && (af != AF_INET6)) {
    return;
  }

  memset(&dst_addr, 0, sizeof(switchlink_ip_addr_t));
  memset(&gateway_addr, 0, sizeof(switchlink_ip_addr_t));

  attrlen = nlmsg_attrlen(nlmsg, hdrlen);
  attr = nlmsg_attrdata(nlmsg, hdrlen);
  while (nla_ok(attr, attrlen)) {
    int attr_type = nla_type(attr);
    switch (attr_type) {
      case RTA_SRC:
        memset(&src_addr, 0, sizeof(switchlink_ip_addr_t));
        src_addr.family = af;
        src_addr.prefix_len = rmsg->rtm_src_len;
        if (src_addr.family == AF_INET) {
          src_addr.ip.v4addr.s_addr = ntohl(nla_get_u32(attr));
        } else {
          memcpy(&(src_addr.ip.v6addr), nla_data(attr), nla_len(attr));
        }
        break;
      case RTA_DST:
        dst_valid = true;
        memset(&dst_addr, 0, sizeof(switchlink_ip_addr_t));
        dst_addr.family = af;
        dst_addr.prefix_len = rmsg->rtm_dst_len;
        if (dst_addr.family == AF_INET) {
          dst_addr.ip.v4addr.s_addr = ntohl(nla_get_u32(attr));
        } else {
          memcpy(&(dst_addr.ip.v6addr), nla_data(attr), nla_len(attr));
        }
        break;
      case RTA_GATEWAY:
        gateway_valid = true;
        memset(&gateway_addr, 0, sizeof(switchlink_ip_addr_t));
        gateway_addr.family = rmsg->rtm_family;
        if (rmsg->rtm_family == AF_INET) {
          gateway_addr.ip.v4addr.s_addr = ntohl(nla_get_u32(attr));
          gateway_addr.prefix_len = 32;
        } else {
          memcpy(&(gateway_addr.ip.v6addr), nla_data(attr), nla_len(attr));
          gateway_addr.prefix_len = 128;
        }
        break;
      case RTA_MULTIPATH:
        ecmp_h = process_ecmp(af, attr, g_default_vrf_h);
        break;
      case RTA_OIF:
        oif_valid = true;
        oif = nla_get_u32(attr);
        break;
      default:
        break;
    }
    attr = nla_next(attr, &attrlen);
  }

  if (rmsg->rtm_dst_len == 0) {
    dst_valid = true;
    memset(&dst_addr, 0, sizeof(switchlink_ip_addr_t));
    dst_addr.family = af;
    dst_addr.prefix_len = 0;
  }

  if (msgtype == RTM_NEWROUTE) {
    memset(&ifinfo, 0, sizeof(ifinfo));
    if (oif_valid) {
      switchlink_db_status_t status;
      status = switchlink_db_get_interface_info(oif, &ifinfo);
      if (status == SWITCHLINK_DB_STATUS_SUCCESS) {
        krnlmon_log_debug("Found interface cache for: %s", ifinfo.ifname);
        if (ifinfo.link_type == SWITCHLINK_LINK_TYPE_BOND) {
          intf_h = ifinfo.lag_h;
        } else {
          intf_h = ifinfo.intf_h;
        }
      } else if (status != SWITCHLINK_DB_STATUS_SUCCESS) {
        krnlmon_log_debug(
            "route: Failed to get switchlink DB interface info, "
            "error: %d \n",
            status);
        return;
      }
    }
    krnlmon_log_info("Create route for %s, with addr: 0x%x", ifinfo.ifname,
                     dst_valid ? dst_addr.ip.v4addr.s_addr : 0);
    switchlink_create_route(g_default_vrf_h, (dst_valid ? &dst_addr : NULL),
                            (gateway_valid ? &gateway_addr : NULL), ecmp_h,
                            intf_h);
  } else {
    krnlmon_log_info("Delete route with addr: 0x%x",
                     dst_valid ? dst_addr.ip.v4addr.s_addr : 0);
    switchlink_delete_route(g_default_vrf_h, (dst_valid ? &dst_addr : NULL));
  }
}
