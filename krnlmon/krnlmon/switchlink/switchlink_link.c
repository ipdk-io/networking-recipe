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

#include <fcntl.h>
#include <linux/version.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>

#include "krnlmon_options.h"
#include "switchlink.h"
#include "switchlink_globals.h"
#include "switchlink_handlers.h"
#include "switchlink_int.h"
#include "switchlink_link_types.h"
#include "switchutils/switch_log.h"

#if defined(ES2K_TARGET)
// ES2K creates netdevs from idpf driver/SR-IOVs.
// These netdevs won't have any Link type.
#define DEFAULT_SWITCHLINK_LINK_TYPE SWITCHLINK_LINK_TYPE_RIF
#else
#define DEFAULT_SWITCHLINK_LINK_TYPE SWITCHLINK_LINK_TYPE_ETH
#endif

struct link_attrs {
  char ifname[IFNAMSIZ];
  // vxlan attributes
  switchlink_ip_addr_t remote_ip_addr;
  switchlink_ip_addr_t src_ip_addr;
  uint32_t vxlan_dst_port;
  uint32_t vni_id;
  uint8_t ttl;
  // lag attributes
  uint8_t bond_mode;
  uint32_t active_slave;
  uint8_t oper_state;
  switchlink_mac_addr_t mac_addr;
  switchlink_mac_addr_t perm_hwaddr;
  // lag member attributes
  uint8_t slave_state;
  uint16_t aggr_id;
};

static const switchlink_mac_addr_t null_mac = {0, 0, 0, 0, 0, 0};

/*
 * Routine Description:
 *    Get the link type
 *
 * Arguments:
 *    [in] info_kind - link related info
 *
 * Return Values:
 *    link type
 */
static switchlink_link_type_t get_link_type(const char* info_kind) {
  switchlink_link_type_t link_type = DEFAULT_SWITCHLINK_LINK_TYPE;

  if (!strcmp(info_kind, "bridge")) {
    link_type = SWITCHLINK_LINK_TYPE_BRIDGE;
  } else if (!strcmp(info_kind, "vxlan")) {
    link_type = SWITCHLINK_LINK_TYPE_VXLAN;
  } else if (!strcmp(info_kind, "bond")) {
    link_type = SWITCHLINK_LINK_TYPE_BOND;
  } else if (!strcmp(info_kind, "tun")) {
    link_type = SWITCHLINK_LINK_TYPE_TUN;
  } else if (!strcmp(info_kind, "team")) {
    link_type = SWITCHLINK_LINK_TYPE_TEAM;
  }

  return link_type;
}

/*
 * Routine Description:
 *    Processes a nested INFO_DATA attribute.
 *
 * Arguments:
 *    [in] infodata - netlink infodata attribute
 *    [inout] attrs - link attributes
 *
 * Return Values:
 *    void
 */
static void process_info_data_attr(const struct nlattr* infodata,
                                   struct link_attrs* attrs) {
  int infodata_attr_type = nla_type(infodata);
  switch (infodata_attr_type) {
    case IFLA_VXLAN_ID:
      attrs->vni_id = nla_get_u32(infodata);
      krnlmon_log_debug("Interface VNI ID: %d\n", attrs->vni_id);
      break;
    case IFLA_VXLAN_PORT:
      attrs->vxlan_dst_port = ntohs(nla_get_u16(infodata));
      krnlmon_log_debug("Interface Dst port: %d\n", attrs->vxlan_dst_port);
      break;
    case IFLA_VXLAN_GROUP:
      attrs->remote_ip_addr.family = AF_INET;
      attrs->remote_ip_addr.ip.v4addr.s_addr = ntohl(nla_get_u32(infodata));
      attrs->remote_ip_addr.prefix_len = 32;
      krnlmon_log_debug("Remote Ipv4 address: 0x%x\n",
                        attrs->remote_ip_addr.ip.v4addr.s_addr);
      break;
    case IFLA_VXLAN_LOCAL:
      attrs->src_ip_addr.family = AF_INET;
      attrs->src_ip_addr.ip.v4addr.s_addr = ntohl(nla_get_u32(infodata));
      attrs->src_ip_addr.prefix_len = 32;
      krnlmon_log_debug("Src Ipv4 address: 0x%x\n",
                        attrs->src_ip_addr.ip.v4addr.s_addr);
      break;
    case IFLA_VXLAN_TTL:
      attrs->ttl = nla_get_u8(infodata);
      krnlmon_log_debug("TTL: %d\n", attrs->ttl);
      break;
    default:
      break;
  }
}

/**
 * Routine Description:
 *    Processes a nested INFO_DATA attribute for LAG.
 *
 * Arguments:
 *    [in] infodata - netlink infodata attribute
 *    [inout] attrs - link attributes
 *
 * Return Values:
 *    void
 */
static void process_info_lag_data_attr(const struct nlattr* infodata,
                                       struct link_attrs* attrs) {
  int infodata_attr_type = nla_type(infodata);
  switch (infodata_attr_type) {
    case IFLA_BOND_MODE:
      attrs->bond_mode = nla_get_u8(infodata);
      krnlmon_log_debug("IFLA Bond Mode: %d\n", attrs->bond_mode);
      break;
    case IFLA_BOND_ACTIVE_SLAVE:
      attrs->active_slave = nla_get_u32(infodata);
      krnlmon_log_debug("IFLA Active Slave: %d\n", attrs->active_slave);
      break;
    default:
      break;
  }
}

/**
 * Routine Description:
 *    Processes a nested INFO_SLAVE_DATA attribute for LAG member.
 *
 * Arguments:
 *    [in] infoslavedata - netlink infoslavedata attribute
 *    [inout] attrs - link attributes
 *
 * Return Values:
 *    void
 */
static void process_info_lag_member_data_attr(
    const struct nlattr* infoslavedata, struct link_attrs* attrs) {
  int infodata_attr_type = nla_type(infoslavedata);
  switch (infodata_attr_type) {
    case IFLA_BOND_SLAVE_STATE:
      attrs->slave_state = nla_get_u8(infoslavedata);
      krnlmon_log_debug("IFLA Bond Slave State: %d\n", attrs->slave_state);
      break;
    case IFLA_BOND_SLAVE_PERM_HWADDR:
      if (nla_len(infoslavedata) == sizeof(switchlink_mac_addr_t)) {
        memcpy(&attrs->perm_hwaddr, nla_data(infoslavedata),
               nla_len(infoslavedata));
      }
      break;
    case IFLA_BOND_SLAVE_AD_AGGREGATOR_ID:
      attrs->aggr_id = nla_get_u16(infoslavedata);
      krnlmon_log_debug("IFLA Bond Slave AD Aggregator ID: %d\n",
                        attrs->aggr_id);
      break;
    default:
      break;
  }
}

/*
 * Routine Description:
 *    Processes netlink link messages.
 *
 * Arguments:
 *    [in] nlmsg - netlink msg header
 *    [in] type - type of netlink messages
 *
 * Return Values:
 *    void
 */
void switchlink_process_link_msg(const struct nlmsghdr* nlmsg, int msgtype) {
  int hdrlen, attrlen;
  const struct nlattr *attr, *linkinfo, *infodata, *infoslavedata;
  const struct ifinfomsg* ifmsg;
  switchlink_link_type_t link_type = SWITCHLINK_LINK_TYPE_NONE;
  switchlink_link_type_t slave_link_type = SWITCHLINK_LINK_TYPE_NONE;
  int linkinfo_attr_type;

  switchlink_db_interface_info_t intf_info = {0};
  switchlink_db_tunnel_interface_info_t tnl_intf_info = {0};
  struct link_attrs attrs = {0};
#ifdef LAG_OPTION
  bool create_lag_member = false;
#endif

  krnlmon_assert((msgtype == RTM_NEWLINK) || (msgtype == RTM_DELLINK));
  ifmsg = nlmsg_data(nlmsg);
  hdrlen = sizeof(struct ifinfomsg);

  krnlmon_log_debug(
      "%slink: family = %d, type = %d, ifindex = %d, flags = 0x%x, "
      "change = 0x%x\n",
      ((msgtype == RTM_NEWLINK) ? "new" : "del"), ifmsg->ifi_family,
      ifmsg->ifi_type, ifmsg->ifi_index, ifmsg->ifi_flags, ifmsg->ifi_change);

  attrlen = nlmsg_attrlen(nlmsg, hdrlen);
  attr = nlmsg_attrdata(nlmsg, hdrlen);

  while (nla_ok(attr, attrlen)) {
    int attr_type = nla_type(attr);
    switch (attr_type) {
      case IFLA_IFNAME:
        snprintf(attrs.ifname, sizeof(attrs.ifname), "%s",
                 nla_get_string(attr));
        krnlmon_log_debug("Interface name is %s\n", attrs.ifname);
        break;
      case IFLA_OPERSTATE:
        attrs.oper_state = nla_get_u8(attr);
        krnlmon_log_debug("IFLA Operstate: %d\n", attrs.oper_state);
        break;
      case IFLA_LINKINFO:
        // IFLA_LINKINFO is a container type
        nla_for_each_nested(linkinfo, attr, attrlen) {
          linkinfo_attr_type = nla_type(linkinfo);
          switch (linkinfo_attr_type) {
            case IFLA_INFO_KIND:
              link_type = get_link_type(nla_get_string(linkinfo));
              break;
            case IFLA_INFO_DATA:
              // IFLA_INFO_DATA is a container type
              if (link_type == SWITCHLINK_LINK_TYPE_VXLAN) {
                nla_for_each_nested(infodata, linkinfo, attrlen) {
                  process_info_data_attr(infodata, &attrs);
                }
              } else if (link_type == SWITCHLINK_LINK_TYPE_BOND) {
                nla_for_each_nested(infodata, linkinfo, attrlen) {
                  process_info_lag_data_attr(infodata, &attrs);
                }
              }
              break;
            case IFLA_INFO_SLAVE_KIND:
              slave_link_type = get_link_type(nla_get_string(linkinfo));
              break;
            case IFLA_INFO_SLAVE_DATA:
              if (slave_link_type == SWITCHLINK_LINK_TYPE_BOND) {
#ifdef LAG_OPTION
                create_lag_member = true;
#endif
                nla_for_each_nested(infoslavedata, linkinfo, attrlen) {
                  process_info_lag_member_data_attr(infoslavedata, &attrs);
                }
              }
              break;
            default:
              break;
          }
        }
        break;
      case IFLA_ADDRESS:
        // IFLA_ADDRESS for kind "sit" is 4 octets
        if (nla_len(attr) == sizeof(switchlink_mac_addr_t)) {
          memcpy(&intf_info.mac_addr, nla_data(attr), nla_len(attr));
          memcpy(&attrs.mac_addr, nla_data(attr), nla_len(attr));

          krnlmon_log_debug("Interface Mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            intf_info.mac_addr[0], intf_info.mac_addr[1],
                            intf_info.mac_addr[2], intf_info.mac_addr[3],
                            intf_info.mac_addr[4], intf_info.mac_addr[5]);
        }
        break;
      default:
        break;
    }
    attr = nla_next(attr, &attrlen);
  }

  if (msgtype == RTM_NEWLINK) {
    switch (link_type) {
      case SWITCHLINK_LINK_TYPE_BRIDGE:
        break;
      case SWITCHLINK_LINK_TYPE_TEAM:
        krnlmon_log_info("LAG via teaming driver isn't supported\n");
        break;
#ifdef LAG_OPTION
      case SWITCHLINK_LINK_TYPE_BOND:
        snprintf(intf_info.ifname, sizeof(intf_info.ifname), "%s",
                 attrs.ifname);
        intf_info.ifindex = ifmsg->ifi_index;
        intf_info.bond_mode = attrs.bond_mode;
        intf_info.oper_state = attrs.oper_state;
        intf_info.active_slave = attrs.active_slave;
        intf_info.link_type = SWITCHLINK_LINK_TYPE_BOND;
        intf_info.intf_type = SWITCHLINK_INTF_TYPE_L3;
        if (intf_info.bond_mode == SWITCHLINK_BOND_MODE_ACTIVE_BACKUP ||
            intf_info.bond_mode == SWITCHLINK_BOND_MODE_LACP) {
          switchlink_create_lag(&intf_info);
        } else {
          krnlmon_log_debug("bond mode:%d isn't supported\n",
                            intf_info.bond_mode);
        }
        break;
#endif
      case SWITCHLINK_LINK_TYPE_ETH:
        break;

#if !defined(OVSP4RT_SUPPORT)
      case SWITCHLINK_LINK_TYPE_VXLAN: {
        snprintf(tnl_intf_info.ifname, sizeof(tnl_intf_info.ifname), "%s",
                 attrs.ifname);
        tnl_intf_info.dst_ip = attrs.remote_ip_addr;
        tnl_intf_info.src_ip = attrs.src_ip_addr;
        tnl_intf_info.link_type = link_type;
        tnl_intf_info.ifindex = ifmsg->ifi_index;
        tnl_intf_info.vni_id = attrs.vni_id;
        tnl_intf_info.dst_port = attrs.vxlan_dst_port;
        tnl_intf_info.ttl = attrs.ttl;

        switchlink_create_tunnel_interface(&tnl_intf_info);
        break;
      }
#endif

      case SWITCHLINK_LINK_TYPE_TUN:
      case SWITCHLINK_LINK_TYPE_NONE:
      case SWITCHLINK_LINK_TYPE_RIF:
        if (!memcmp(intf_info.mac_addr, null_mac, sizeof(null_mac))) {
          krnlmon_log_info("Ignoring interfaces: %s with NULL MAC address",
                           intf_info.ifname);
          break;
        }

#if defined(ES2K_TARGET)
        if (!switchlink_validate_driver(attrs.ifname)) {
          krnlmon_log_info(
              "Ignoring interface: %s which is not created"
              " by openvswitch or idpf driver",
              intf_info.ifname);
          break;
        }
#endif
        snprintf(intf_info.ifname, sizeof(intf_info.ifname), "%s",
                 attrs.ifname);
        intf_info.ifindex = ifmsg->ifi_index;
        intf_info.vrf_h = g_default_vrf_h;
        intf_info.intf_type = SWITCHLINK_INTF_TYPE_L3;
        intf_info.link_type = SWITCHLINK_LINK_TYPE_RIF;

        switchlink_create_interface(&intf_info);
        break;
      default:
        break;
    }
    switch (slave_link_type) {
#ifdef LAG_OPTION
      case SWITCHLINK_LINK_TYPE_BOND: {
        switchlink_db_lag_member_info_t lag_member_info = {0};
        snprintf(lag_member_info.ifname, sizeof(lag_member_info.ifname), "%s",
                 attrs.ifname);
        lag_member_info.ifindex = ifmsg->ifi_index;
        lag_member_info.oper_state = attrs.oper_state;
        lag_member_info.slave_state = attrs.slave_state;
        if (attrs.aggr_id != 0) lag_member_info.is_lacp_member = true;
        memcpy(&(lag_member_info.mac_addr), &(attrs.mac_addr),
               sizeof(switchlink_mac_addr_t));
        memcpy(&(lag_member_info.perm_hwaddr), &(attrs.perm_hwaddr),
               sizeof(switchlink_mac_addr_t));
        if (create_lag_member) {
          switchlink_create_lag_member(&lag_member_info);
        }
        break;
      }
#endif
      default:
        break;
    }
  } else {
    krnlmon_assert(msgtype == RTM_DELLINK);

#if !defined(OVSP4RT_SUPPORT)
    if (link_type == SWITCHLINK_LINK_TYPE_VXLAN) {
      switchlink_delete_tunnel_interface(ifmsg->ifi_index);
      return;
    }
#endif

    if (link_type == SWITCHLINK_LINK_TYPE_TUN ||
        link_type == SWITCHLINK_LINK_TYPE_RIF) {
      switchlink_delete_interface(ifmsg->ifi_index);
    }
#ifdef LAG_OPTION
    if (link_type == SWITCHLINK_LINK_TYPE_BOND) {
      switchlink_delete_lag(ifmsg->ifi_index);
      return;
    }

    if (slave_link_type == SWITCHLINK_LINK_TYPE_BOND) {
      switchlink_delete_lag_member(ifmsg->ifi_index);
      return;
    }
#endif
  }
  return;
}

void switchlink_init_link(void) {
  /* P4OVS: create default vrf */
  switchlink_create_vrf(&g_default_vrf_h);
}
