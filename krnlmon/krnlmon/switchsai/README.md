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

# SwitchSAI library

## Table of contents
1. [Overview](#overview)
2. [VRF](#vrf)
3. [RIF](#rif)
4. [Tunnel](#tunnel)
5. [FDB](#fdb)
6. [Route](#route)
7. [Nexthop](#nexthop)
8. [NexthopGroup](#nexthopgroup)
5. [Neighbor](#neighbor)

## Overview <a name="overview"></a>
The SwitchSAI library exposes the standard Switch Abstraction Interface (SAI) API on top of the SwitchAPI API library written to program the data plane described in the linux_networking.p4

For each feature, a module is initialized in sai_initialize and the pointer is returned to invoke SwitchSAI calls from the above layer. In our case, switchLink queries for each module pointer and call SwitchSAI accordingly.
```
Eg:
sai_fdb_api_t fdb_api = {.create_fdb_entry = sai_create_fdb_entry,
                                       .remove_fdb_entry = sai_remove_fdb_entry};

sai_status_t sai_fdb_initialize(sai_api_service_t *sai_api_service) {
  sai_api_service->fdb_api = fdb_api;
  return SAI_STATUS_SUCCESS;
}
```
Each SwitchLink message is constructed with attributes defined by opencompute/SAI and appropriate calls to the SwitchSAI is made. The arguments used for the SwitchSAI will be defined by the opencompute/SAI.
Here are each SwitchSAI calls made from SwitchLink and associated SwitchAPI calls.

## VRF <a name="vrf"></a>

**create_virtual_router:** Takes VRF attributes as input, creates a VRF in SwitchAPI with switch_api_vrf_create and returns a handler corresponding to VRF.
**remove_virtual_router:** Takes VRF handler as an input, deletes a VRF in SwitchAPI with switch_api_vrf_delete.

## RIF <a name="rif"></a>

* **create_router_interface:** Takes RIF attributes as input,
  * Creates RMAC group with switch_api_router_mac_group_create, returns the handler to Switchlink layer for this RMAC group to be saved in RIF SwitchAPI structure
  * Add RMAC entries for the group with switch_api_router_mac_add, returns RMAC handler to Switchlink layer for each entry to be added to the list in a RMAC group.
  * Creates RIF interface with switch_api_rif_create, returns handler called as interface handler to Switchlink layer which is used for cross reference like neighbor, nexthop.
* **remove_router_interface:** Takes RIF handler as input,
  * Gets the RMAC group handler from the RIF handler (separate call to SwitchAPI switch_api_device_default_rmac_handle_get)
  * Using RMAC group handler, loop through list of RMAC for the port and delete each RMAC and finally the RMAC group.
  * Deletes RIF with switch_api_rif_delete.

## Tunnel <a name="tunnel"></a>

* **create_tunnel:** Takes tunnel attributes as input, creates tunnel entry in SwitchAPI with switch_api_tunnel_create, and returns tunnel handler to the caller.
* **remove_tunnel:** Takes tunnel handler as an input, deletes tunnel entry in SwitchAPI with switch_api_tunnel_delete.
* **create_tunnel_term_table_entry:** Takes tunnel terminate attributes as input, creates tunnel term entry in SwitchAPI with switch_api_tunnel_term_create and returns the handler to the caller.
* **remove_tunnel_term_table_entry:** Takes tunnel termination handler as an input, deletes tunnel termination entry in SwitchAPI with switch_api_tunnel_term_delete.

## FDB <a name="fdb"></a>

FDB entry is populated from 2 sources, one is from SwitchLink→SwitchSAI→SwitchAPI and other is from OVS Mac Learning→ SwitchAPI.
* **create_fdb_entry:** Takes FDB attributes as input, creates FDB entry for Tx table in SwitchSAI with switch_api_l2_forward_create.
* **remove_fdb_entry:** Takes FDB attributes as input, deletes FDB Tx table entry in SwitchSAI with switch_api_l2_forward_delete

## Route <a name="route"></a>

* **create_route_entry:** Takes route attributes as input, creates Route entry in SwitchSAI with switch_api_l3_route_add
* **remove_route_entry:** Takes route attributes as input, deletes Route entry in SwitchSAI with switch_api_l3_delete_route

## Nexthop <a name="nexthop"></a>

* **create_next_hop:** Takes nexthop attributes as input, creates nexthop entry in SwitchSAI with switch_api_nhop_create and returns handler to the caller.
* **remove_next_hop:** Takes nexthop handler as an input, deletes a nexthop in SwitchAPI with switch_api_nhop_delete

## Nexthopgroup <a name="nexthopgroup"></a>

* **create_next_hop_group:** Takes nexthop attributes as input, creates nexthop group/ecmp group entry in SwitchSAI with switch_api_ecmp_create and returns handler to the caller.
* **remove_next_hop_group:** Takes nexthop group/ecmp group handler as an input, deletes a nexthop in SwitchAPI with switch_api_nhop_delete
* **create_next_hop_group_member:** Takes nexthop group/ecmp group handler as input along with member attributes, creates a member and adds to the nexthop group entry in SwitchSAI with switch_api_ecmp_member_add and returns handler to the caller.
* **remove_next_hop_group_member:** Takes nexthop member handler as an input, deletes a member in SwitchAPI with switch_api_ecmp_member_delete

## Neighbor <a name="neighbor"></a>

* **create_neighbor_entry:** Takes neighbor attributes as input, gets the previously created nexthop handler and creates neighbor entry in SwitchSAI with switch_api_neighbor_create.
* **remove_neighbor_entry:** Takes neighbor attributes as input, deletes neighbor entry in SwitchSAI with switch_api_neighbor_delete.
