/*
 * Copyright (c) 2023-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Definitions specific to P4OVS.
 *
 * OVS code that references this file must do so under protection of an
 * #ifdef P4OVS conditional.
 */

#ifndef LIB_P4OVS_H
#define LIB_P4OVS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "openvswitch/thread.h"
#include "openvswitch/util.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct ovs_mutex p4ovs_fdb_entry_lock;

extern char p4ovs_grpc_addr[32];

/* Control OvS offload with an environment variable during runtime.
 * If env variable OVS_P4_OFFLOAD=false, then disable OVS offload, else
 * if OVS_P4_OFFLOAD is not set or OVS_P4_OFFLOAD is any value other
 * than false, then by default enable OVS offload.
 */
static inline bool ovs_p4_offload_enabled(void) {
    const char* offload = getenv("OVS_P4_OFFLOAD");
    return (offload == NULL) || strcmp(offload, "false") != 0;
}

/* OvS creates multiple handler and revalidator threads based on the number of
 * CPU cores. These threading mechanism also associated with bridges that
 * are created in OvS. During multiple bridge scenarios, we are seeing
 * issues when a mutiple MAC's are learnt on different bridges at the same time.
 * Creating a mutex and with this we are controlling p4runtime calls for each
 * MAC learn.
 */
static inline void p4ovs_lock_init(const struct ovs_mutex *p4ovs_lock) {
    return ovs_mutex_init(p4ovs_lock);
}

static inline void p4ovs_lock_destroy(const struct ovs_mutex *p4ovs_lock) {
    return ovs_mutex_destroy(p4ovs_lock);
}

static inline void p4ovs_lock(const struct ovs_mutex *p4ovs_lock) {
    return ovs_mutex_lock(p4ovs_lock);
}

static inline void p4ovs_unlock(const struct ovs_mutex *p4ovs_lock) {
    return ovs_mutex_unlock(p4ovs_lock) OVS_RELEASES(p4ovs_lock);
}

void ovs_set_grpc_addr(const char* optarg);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // LIB_P4OVS_H
