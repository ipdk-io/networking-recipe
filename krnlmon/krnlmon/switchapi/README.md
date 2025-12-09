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

# SwitchAPI library

## Table of contents
1. [Overview](#overview)
2. [FDB](#fdb)
3. [RIF](#rif)
4. [VRF](#vrf)
5. [RMAC](#rmac)
6. [Neighbor](#neighbor)
7. [Nexthop](#nexthop)
8. [Route](#route)
9. [Platform Dependent API](#platform_dependent_api)


## Overview <a name="overview"></a>
The Switchapi library is an abstraction layer that exposes higher level API's used to interact with target backend to program the data plane described in the linux_networking.p4.
Modules supported in switchapi are FDB, Nexthop, Neighbor, ECMP, Route, VRF, RMAC, Tunnel.

For each module, as part of initialization, device context is set, hashtable is initialized and a unique handle is allocated and initialized based on the type of module.

## FDB <a name="fdb"></a>
 * During initialization, device context is set, hashtable is initialized and handle is allocated for both Tx (SWITCH_HANDLE_TYPE_L2_FWD_TX) and Rx (SWITCH_HANDLE_TYPE_L2_FWD_RX) and updated in device context
 * **switch_api_l2_forward_create**: 
   * Triggered by SwitchSAI create_fdb_entry API. Creates FDB handle based on type of information received (Tx or Rx). Tx FDB learn can be from tunnel interface, physical interface or VLAN interface.
   * Table entry is created with platform dependent (PD) API (switch_pd_l2_tx_forward_table_entry for Tx entry and switch_pd_l2_rx_forward_table_entry for Rx entry).
   * Once the table entry succeeds, handle along with api_l2_info is cached in l2 hashtable with mac address as the key.
 * **switch_api_l2_forward_delete**:
   * Triggered by SwitchSAI delete_fdb_entry API. Deletes the table entry using platform dependent API based on type of handle(Tx/Rx) and deletes the cache from Judy array

## RIF <a name="rif"></a>
 * **switch_api_rif_create**: Handle is allocated and initialized for handle type SWITCH_HANDLE_TYPE_RIF and the port ID for which this function is triggered, is determined
using platform independent API switch_pd_to_get_port_id and saved in rif_info structure.
 * **switch_api_rif_delete**: Deletes the associated RIF handle

## VRF <a name="vrf"></a>
 * **switch_api_vrf_create**: Handle is allocated and initialized for handle type SWITCH_HANDLE_TYPE_VRF. Based on the VRF id attribute passed by SwitchSAI, check if VRF handle exists for this VRF id. If not, create the VRF handle and cache in Judy array with vrf id as key and return VRF handle to the SwitchSAI
 * **switch_api_vrf_delete**: Retrieve the vrf_info for associated VRF handle, delete the cached information from Judy array and delete the handle

## RMAC <a name="rmac"></a>
 * **switch_api_router_mac_group_create**: Handle is allocated and initialized for handle type SWITCH_HANDLE_TYPE_RMAC.  Retrieves the rmac_info based on rmac handle and initalize the rmac list
 * **switch_api_router_mac_add**: Iterates through all the MAC's in rmac_list and create an entry for each MAC
 * **switch_api_router_mac_group_delete** - Iterates through all entries in rmac list amd delete the rmac entry and handle associated

## Neighbor <a name="neighbor"></a>
 * **switch_api_neighbor_create**: Handle is allocated and initialized for handle type SWITCH_HANDLE_TYPE_NEIGHBOR. Retrieves the nhop_handle from api_neighbor_info attribute passed by SwitchSAI. If nhop handle is valid, create the handle for neighbor and update the nexthop table and neighbor mod table using appropriate platform dependent (PD) API's
 * **switch_api_neighbor_delete**: Retrieve the neighbor_info from neighbor handle and call the platform dependent (PD) API's to delete table entry from target backend and delete the handle

## Nexthop <a name="nexthop"></a>
 * **switch_api_nhop_create**: Handle is allocated and initialized for handle type SWITCH_HANDLE_TYPE_NHOP. Check if the nhop handle exists, if not, create one and get the nhop key based on nhop type. Once the handle is successfully created, cache the nhop info with key as nhop key in the hashtable and return nhop handle to SwitchSAI
 * Backend programming for nhop create is not needed because neighbor_create will call a nexthop_create always before creating neighbor
 * **switch_api_nhop_delete**: Retrieve the nhop_info from nhop handle and if nhop reference count is more than 1, then decrement the count and return. Else, delete the cached information from hastable for this nhop key.

## Route <a name="route"></a>
 * During initialization, device context is set, route hashtable is initialized and handle is allocated for  SWITCH_HANDLE_TYPE_ROUTE and updated in device context
 * **switch_api_l3_route_add**:
    * Create route entry using vrf handle and ip address from api_route_info attribute passed by SwitchSAI and lookup the hashtable to search if this route entry already exists. If yes, then return to the SwitchSAI else, create the handle of type SWITCH_HANDLE_TYPE_ROUTE.
    *  Once the handle is successfully created, if nhop handle is of type SWITCH_HANDLE_TYPE_NHOP,  call the platform dependent(PD) API to add entry to ipv4 table in target backend. If nhop handle is of type SWITCH_HANDLE_TYPE_ECMP_GROUP, call the platform dependent(PD) API to add entry to ip4 table  and ecmp_hash table
    * Once the table entry is successfully added to backend, cache the route_info in the hastable with route entry as the key

 * **switch_api_l3_delete_route**: Retrieve the route handle from hashtable using route entry as the key. Call the platform dependent(PD) API to delete the table entries from target backend and delete the route handle.

## Platform Dependent(PD) API <a name="platform_dependent_api"></a>

All platform dependent (PD) API's allocates a session handle for TDI transactions and retrieves the key and table handle and id's associated for action and all required fields with appropriate TDI API's and program the target backend using tdi_table_entry_add API. Once the table is successfully programed, the session handle is deleted

#### FDB platform dependent API:
 * **switch_pd_l2_tx_forward_table_entry**
   * For FDB learns from vlan interface
    ```
    Table name: LNW_L2_FWD_TX_TABLE
    Key: LNW_L2_FWD_TX_TABLE_KEY_DST_MAC
    Action: LNW_L2_FWD_TX_TABLE_ACTION_L2_FWD with parameters port id of vlan interface
    ```
   * For FDB learns from physical interface
    ```
    Table name: LNW_L2_FWD_TX_TABLE
    Key: LNW_L2_FWD_TX_TABLE_KEY_DST_MAC
    Action: LNW_L2_FWD_TX_TABLE_ACTION_L2_FWD with parameters port id of physical interface
    ```
   * For FDB learns from tunnel interface
    ```
    Table name: LNW_L2_FWD_TX_TABLE
    Key: LNW_L2_FWD_TX_TABLE_KEY_DST_MAC
    Action: LNW_L2_FWD_TX_TABLE_ACTION_SET_TUNNEL with parameters tunnel id, dst addr
    ```

 * **switch_pd_l2_rx_forward_table_entry**
```
    Table name: LNW_L2_FWD_RX_TABLE
    Key: LNW_L2_FWD_TX_TABLE_KEY_DST_MAC
    Action: LNW_L2_FWD_RX_TABLE_ACTION_L2_FWD with parameters port id
```

#### Route Platform dependent API

 * **switch_pd_nexthop_table_entry**
```
    Table name: LNW_NEXTHOP_TABLE
    Key: LNW_NEXTHOP_TABLE_KEY_NEXTHOP_ID
    Action: LNW_NEXTHOP_TABLE_ACTION_SET_NEXTHOP with parameters rif id, neighbor id and egress port id
```
 * **switch_pd_neighbor_table_entry**
```
    Table name: LNW_NEIGHBOR_MOD_TABLE
    Key: LNW_NEIGHBOR_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR
    Action: LNW_NEIGHBOR_MOD_TABLE_ACTION_SET_OUTER_MAC with parameters dst_mac_addr
```

 * **switch_pd_rif_mod_entry**
```
    Table name: LNW_RIF_MOD_TABLE
    Key: LNW_RIF_MOD_TABLE_KEY_RIF_MOD_MAP_ID
    Action: LNW_RIF_MOD_TABLE_ACTION_SET_SRC_MAC with parameters src_mac_addr
```

 * **switch_pd_ipv4_table_entry**
```
    Table name: LNW_IPV4_TABLE
    Key: LNW_IPV4_TABLE_KEY_IPV4_DST_MATCH
    For SWITCH_ACTION_NHOP:
        Action LNW_IPV4_TABLE_ACTION_SET_NEXTHOP_ID with parameters nexthop_id
    For SWITCH_ACTION_ECMP :
        Action LNW_IPV4_TABLE_ACTION_ECMP_HASH_ACTION with parameters ecmp_group_id
```

 * **switch_pd_ecmp_hash_table_entry**
```
    Table name: LNW_ECMP_HASH_TABLE
    Key: LNW_ECMP_HASH_TABLE_KEY_HOST_INFO_TX_EXTENDED_FLEX_0
    Action: LNW_ECMP_HASH_TABLE_ACTION_SET_NEXTHOP_ID with parameters nexthop_id
```

#### Tunnel Platform Dependent API

 * **switch_pd_tunnel_entry**
```
    Table name: LNW_VXLAN_ENCAP_MOD_TABLE
    Key: LNW_VXLAN_ENCAP_MOD_TABLE_KEY_VENDORMETA_MOD_DATA_PTR
    Action: LNW_VXLAN_ENCAP_MOD_TABLE_ACTION_VXLAN_ENCAP with
    parameters src_addr, dst_addr, VNI
```

 * **switch_pd_tunnel_term_entry**
```
    Table name: LNW_IPV4_TUNNEL_TERM_TABLE
    Key: LNW_IPV4_TUNNEL_TERM_TABLE_KEY_TUNNEL_TYPE, LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_SRC and LNW_IPV4_TUNNEL_TERM_TABLE_KEY_IPV4_DST
    Action: LNW_IPV4_TUNNEL_TERM_TABLE_ACTION_DECAP_OUTER_IPV4 with parameters tunnel_id

