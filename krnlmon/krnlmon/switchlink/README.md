<!--
/*
 * Copyright (c) 2022 Intel Corporation.
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
- -->
# Switchlink library

## Table of contents
1. [Overview](#overview)
2. [RTM_NEWLINK & RTM_DELLINK](#rtm_newlink_and_rtm_dellink)
3. [RTM_NEWADDR & RTM_DELADDR](#rtm_newaddr_and_rtm_deladdr)
4. [RTM_NEWROUTE & RTM_DELROUTE](#rtm_newroute_and_rtm_delroute)
5. [RTM_NEWNEIGH & RTM_DELNEIGH](#rtm_newneigh_and_rtm_delneigh)

## Overview <a name="overview"></a>
The switchlink library provides a Netlink listener that listens to kernel notifications for network events (link, address, neighbor, route, etc.) and uses the SwitchSAI library to program the data plane described in the linux_networking.p4

When P4-OVS is started as part of p4proto_init, a switchlink_main thread is spawned which is responsible for creating netlink socket and listening to netlink notifications from kernel in a loop.
Following callbacks are registered during initialization:
1. nl_socket_modify_cb(
      g_nlsk, NL_CB_VALID, NL_CB_CUSTOM, nl_recv_sock_msg, NULL)
2. nl_socket_modify_cb(
      g_nlsk, NL_CB_FINISH, NL_CB_CUSTOM, nl_recv_sock_msg, NULL)

The netlink socket is continuously polled on, if any FD is set then the registered callback is invoked. eg: nl_recv_sock_msg

Based on the type of the netlink message, a respective process netlink API will be invoked. Following netlink notifications are subscribed to:
1. RTM_NEWLINK & RTM_DELLINK - process_link_msg(nlmsg, nlmsg->nlmsg_type);
2. RTM_NEWADDR & RTM_DELADDR - process_address_msg(nlmsg, nlmsg->nlmsg_type);
3. RTM_NEWROUTE & RTM_DELROUTE - process_route_msg(nlmsg, nlmsg->nlmsg_type);
4. RTM_NEWNEIGH & RTM_DELNEIGH - process_neigh_msg(nlmsg, nlmsg->nlmsg_type);

A default VRF is created and this VRF handler is cached

## RTM_NEWLINK & RTM_DELLINK <a name="rtm_newlink_and_rtm_dellink"></a>
When a TAP port or VxLAN port is created/deleted, RTM_NEWLINK/RTM_DELLINK netlink notification is received for a port.

### TAP port

* Attributes retrieved from the netlink message are used to update the local switchlink_db_interface_info_t. Further, SwitchSAI API with appropriate attributes defined in opencompute/SAI definitions is called.
```
SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID
SAI_ROUTER_INTERFACE_ATTR_TYPE
SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS
SAI_ROUTER_INTERFACE_ATTR_PORT_ID
```
* On a successful execution of SwicthSAI API, a unique handler for this TAP port is received from SAI layer.
* Local cache for this TAP port is created and saved in TommyDS Database for further reference with key as ifindex of the port and the handler received from the SwitchSAI is also updated
* Here, based on the kernel design for a TUNTAP port, we get two different RMAC's for a PORT, where SwitchLink takes the second updated RMAC for the port and uses it for further references
* During delete, we get the local cache from ifindex. This cached data is used to make SwitchSAI call to delete this port and also delete entry from local TommyDS database.

### TUNNEL port

* Attributes retrieved from the netlink message are used to update the local switchlink_db_tunnel_interface_info_t. Further, SwitchSAI API with appropriate attributes defined in opencompute/SAI definitions is called(Only VNI 0 is supported for now).
```
Attributes for tunnel create:
       SAI_TUNNEL_ATTR_ENCAP_SRC_IP
       SAI_TUNNEL_ATTR_ENCAP_DST_IP
       SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT
Attributes for Tunnel termination create:
       SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP
       SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP
       SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID
```
* On a successful execution of SwicthSAI API, a unique handler for this tunnel port and a unique handler for tunnel termination is received from SAI layer
* Local cache for this Tunnel port and tunnel termination is created and saved in TommyDS Database for further reference with key as ifindex of the port and the handler received from the SwitchSAI is also updated.
* During delete, we get the local cache from ifindex. This cached data is used to make SwitchSAI call to delete this tunnel port and tunnel termination entries, and also delete entry from local TommyDS database.

### Dummy port
* Ignores the notification of TEP ports, as these ports are created for TEP termination for ECMP case.

## RTM_NEWADDR & RTM_DELADDR <a name="rtm_newaddr_and_rtm_deladdr"></a>
* When an IP address is assigned to/deleted from a TAP port, this kind of netlink notification is recieved
* Attributes are retrieved from the netlink message and are used to update the local switchlink_db_route_info_t. Further, SwitchSAI API with appropriate attributes defined in opencompute/SAI definitions is called
* For each IP address assigned to the kernel port, a route entry is created with prefix length as 32 and also actual prefix provided by user for the address.
* SAI_IP_ADDR_FAMILY_IPV4 is the only attribute set by switchLink which has IP address and prefix length set. The handler information for the route entry is not cached.
* Local cache for this address is created and saved in TommyDS Database for further reference with key as IPaddress + VRF of the port.
* During delete, we get the local cache based on the key IPaddress + VRF and delete the entry in SwitchSAI and also from local Tommy DS cache.

## RTM_NEWROUTE & RTM_DELROUTE <a name="rtm_newroute_and_rtm_delroute"></a>
* Attributes are retrieved from the netlink message and are used to update the local switchlink_db_route_info_t. Further, SwitchSAI API with appropriate attributes defined in opencompute/SAI definitions is called
* For each route created on the kernel port, a route entry with prefix provided by user for the address is created
   * SAI_IP_ADDR_FAMILY_IPV4 is the attribute set by switchLink which has IP address and prefix length set and is used to make a SwitchSAI call.
   * If a route is created with a gateway and if that gateway is resolved, we update SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID attribute with nexthop_id handler (read below), and make a SwitchSAI call.
* Local cache for this address is created and saved in TommyDS Database for further reference with key as IPaddress + VRF of the port. The handler information for the route entry is not cached.

### For ECMP
* When a kernel route is added with multipath option, this message is received as part of RTM_NEWROUTE, this message contains an attribute RTA_MULTIPATH.
* Based on this attribute, we loop through configured multipath address and configure each nexthop address locally.
 * If nexthop is already not available in local cache:
   * Create an Nexthop entry and populate nexthop entry by updating below attributes in SwitchSAI
```
SAI_NEXT_HOP_ATTR_TYPE
SAI_NEXT_HOP_ATTR_IP
SAI_IP_ADDR_FAMILY_IPV4
SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID
```
   * On successful addition of nexthop entry, a handler for the nexthop table is received, and this will be saved in Nexthop table with key as nexthop IP + VRF + Interface.
   * Along with the saved nexthop entry locally, we also update from where this nexthop is learnt. In this case it is ROUTE, which is of type SWITCHLINK_NHOP_FROM_NEIGHBOR
   * Check if we need to delete this Nexthop entry when neighbor is deleted, we get the local cache from Tommy DS based on the key, check if this nexthop is only referred by neighbor i.e., SWITCHLINK_NHOP_FROM_NEIGHBOR, if yes, then delete the nexthop else remove this SWITCHLINK_NHOP_FROM_NEIGHBOR reference from nexthop entry.

  * If nexthop is already created, then update that existing nexthop by looking at the local cache. Also update which feature is referring this nexthop, in this case SWITCHLINK_NHOP_FROM_ROUTE
* During delete, we get the local cache based on the key IPaddress + VRF and delete the entry in SwitchSAI and also from local TommyDS cache.
* When an ECMP entry is deleted, we go and check if this nexthop is only referred by Route i.e., SWITCHLINK_NHOP_FROM_ROUTE , if yes then delete the nexthop, else remove this SWITCHLINK_NHOP_FROM_ROUTE reference from nexthop entry.

## RTM_NEWNEIGH & RTM_DELNEIGH <a name="rtm_newneigh_and_rtm_delneigh"></a>
* Attributes are retrieved from the netlink message and are used to update the local switchlink_mac_addr_t & switchlink_db_neigh_info_t. Further, SwitchSAI API with appropriate attributes defined in opencompute/SAI definitions is called.
* When a neighbor state reaches to NUD_REACHABLE/NUD_PERMANENT/NUD_FAILED, the netlink message is processed.
As part of neighbor create we also learn the PEER device MAC address, associated PEER device IP to ,create nexthop entry

* MAC:
 * Populate FDB entry with peer MAC address and port on which this MAC is learnt.
```
SAI_FDB_ENTRY_ATTR_TYPE
SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID
```
 * Local cache for this MAC is created and saved in TommyDS Database for further reference with key as MAC address + Bridge.
 * Delete this FDB entry when neighbor is deleted, get the local cache from TommyDS database based on the key and delete the MAC entry.
 * Note: If a NEWNEIGH notification is received for modified PEER mac address, existing FDB entry is deleted and new entry with modified address is created.

* Neighbor:
 * Populate Neighbor entry with peer IP address and MAC address.
```
SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS
```
 * Along with the above attribute we also update interface handler on which this neighbor is learnt and the Neighbor IP address.
 * On successful creation of neighbor entry, a unique handler for this neighbor is received
 * Local cache for this neighbor is created and saved in TommyDS Database for further reference with key as IPaddress + VRF + Interface handler of the port.
 * During delete, we get the local cache based on the key and delete the entry in SwitchSAI and also from local TommyDS cache.

* If nexthop entry is not created already then
 * Create a Nexthop and populate nexthop entry by updating below attributes to SwitchSAI
```
SAI_NEXT_HOP_ATTR_TYPE
SAI_NEXT_HOP_ATTR_IP
SAI_IP_ADDR_FAMILY_IPV4
SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID
```
 * On successful addition of nexthop entry, a handler for the nexthop table is received, and this will be saved in Nexthop table with key as nexthop IP + VRF + Interface.
 * Along with the saved nexthop entry locally, we also update from where this nexthop is learnt in this case it is ROUTE, which is of type SWITCHLINK_NHOP_FROM_NEIGHBOR
 * Check if we need to delete this Nexthop entry when neighbor is deleted, get the local cache from Tommy DS based on the key, check if this nexthop is only referred by neighbor i.e., SWITCHLINK_NHOP_FROM_NEIGHBOR, if yes then go and delete the nexthop else remove this SWITCHLINK_NHOP_FROM_NEIGHBOR reference from nexthop entry.

* If nexthop is already created:
 * Update that existing nexthop by looking at the local cache. Also update which feature if referring this nexthop, in this case SWITCHLINK_NHOP_FROM_NEIGHBOR
 * Check if we need to delete this Nexthop entry when neighbor is deleted, we get the local cache from Tommy DS based on the key, check if this nexthop is only referred by neighbor i.e., SWITCHLINK_NHOP_FROM_NEIGHBOR, if yes then delete the nexthop else remove this SWITCHLINK_NHOP_FROM_NEIGHBOR reference from nexthop entry.
