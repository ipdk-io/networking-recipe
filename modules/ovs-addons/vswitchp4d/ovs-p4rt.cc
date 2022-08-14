// Copyright 2022 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#include "openvswitch/ovs-p4rt.h"
#if 0
#include "stratum/glue/logging.h"
#else
#include <stdio.h>
#endif

// TODO(dfoster): should we use OVS or Stratum logging?
// If Stratum, should we isolate that code in its own library?

int ovs_p4_add_port(uint64_t device, int64_t port,
                    const struct ovs_p4_port_properties *port_props)
{
#if 0
    LOG(INFO)
        << "ovs_p4_add_port()"
        << " Device: " << device
        << " Port: " << port
        << " Name: " << port_props->port_name
        << " Port Type: " << port_props->port_type;
#else
    printf("ovs_p4_add_port()"
           " Device: %lu"
           " Port: %ld"
           " Name: %s"
           " Port Type: %d\n",
           device,
           port,
           port_props->port_name,
           port_props->port_type);
#endif

    return 0;
}
