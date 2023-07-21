/*
 * Copyright (c) 2022 Intel Corporation.
 *
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

#ifndef ROUTING_P4_
#define ROUTING_P4_

#include "pna.p4"
#include "headers.p4"
#include "metadata.p4"

// This control block models the L3 routing pipeline.
// LPM                                              LEM
// +-------+   +-------+ wcmp  +---------+       +-----------+
// |  lpm  |-->| group |------>| nexthop |----+->| router    |--> egress_port
// |       |   |       |------>|         |-+  |  | interface |--> src_mac
// +-------+   +-------+       +---------+ |  |  +-----------+
//   |   |                         ^       |  |  +-----------+
//   |   |                         |       |  +->| neighbor  |
//   V   +-------------------------+       +---->|           |--> dst_mac
//  drop                                         +-----------+
//
// The pipeline first performs a longest prefix match on the packet's
// destination IP address. The action associated with the match then either
// drops the packet, points to a nexthop, or points to a wcmp group which uses a
// hash of the packet to choose from a set of nexthops. The nexthop points to a
// router interface, which determines the packet's src_mac and the egress_port
// to forward the packet to. The nexthop also points to a neighbor which,
// together with the router_interface, determines the packet's dst_mac.
//
// Note that this block does not rewrite any header fields directly, but only
// records rewrites in `local_metadata.packet_rewrites`, from where they will be
// read and applied in the egress stage.

control routing(in headers_t hdr,
                inout local_metadata_t local_metadata,
                inout vendor_metadata_t vendormeta,
                in pna_main_input_metadata_t istd)
{
  Hash<bit<16>>(PNA_HashAlgorithm_t.TARGET_DEFAULT) ecmp_hash_fn;

  bool ecmp_group_id_valid = false;

  action drop() {
    drop_packet();
  }

  /* get egress port from rif_mod in control plane */
  action set_nexthop(router_interface_id_t router_interface_id, 
                     neighbor_id_t neighbor_id, PortId_t egress_port) {
    vendormeta.mod_action_ref = NEIGHBOR;
    vendormeta.mod_data_ptr = (ModDataPtr_t) neighbor_id;
    local_metadata.rif_mod_map_id = router_interface_id;
    send_to_port(egress_port); /* handle in control plane */
  }

  /* TODO Andy: Need a nexthop_table.apply() call somewhere in the
   * program, otherwise it is unused and need not be defined. */
  table nexthop_table {
    key = {
      local_metadata.nexthop_id : exact;
    }
    actions = {
      set_nexthop;
    }
    size = 65536;
  }

  action set_nexthop_id (bit<16> nexthop_id) {
    local_metadata.nexthop_id = nexthop_id;
  }
  
  table ecmp_hash_table {
    key = {
        local_metadata.ecmp_group_id :exact;
    local_metadata.ecmp_hash : exact;
    }
    actions = {
      set_nexthop_id;
      @defaultonly NoAction;
    }
    const default_action = NoAction;
    size = 65536;
  }

  action ecmp_hash_action(bit<16> ecmp_group_id) {
    ecmp_group_id_valid = true;
    local_metadata.ecmp_group_id = ecmp_group_id;
  }
  
  /* non-tunnel-LPM - moved to tunnel_encap table for tunneling */
  table ipv4_table {
    key = {
      local_metadata.ipv4_dst_match : lpm;
    }
    actions = {
      set_nexthop_id;
      ecmp_hash_action;
      @defaultonly NoAction;
    }

    const default_action = NoAction;
    size = 65536; 
  }

  apply { 
    /* calculate the ECMP hash */

// Usha - DPDK doesn't currently support hash. Can action selector be used here by converting the 
//fields to be hashed as selector fields. Also this calculated hash is used as a table key in 
// the ecmp_hash_table. This looks like a place to use action selector??
/*    local_metadata.ecmp_hash = ecmp_hash_fn.get_hash({
                // list of input fields to hash function go here
                // TODO Andy: This list of fields should be reviewed
                // to see whether they are truly the desired header
                // fields in all cases we want this code to support.
                // If they must always be inner headers regardless of
                // the header sequence parsed in the packet, for
                // example, then the first three lines will likely
                // need some changes.
        hdr.ipv4.src_addr,
        hdr.ipv4.dst_addr,
        hdr.ipv4.protocol,
        hdr.udp.src_port,
        hdr.udp.dst_port});
*/
    if(hdr.ipv4.isValid()) {
      ipv4_table.apply();
    }

    if (ecmp_group_id_valid) {
      ecmp_hash_table.apply();
    }
      nexthop_table.apply();
  }
} /* Control routing */
#endif  // _ROUTING_P4_

