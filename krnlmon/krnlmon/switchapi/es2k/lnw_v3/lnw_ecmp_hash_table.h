/*
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * ECMP_HASH_TABLE for Linux Networking V3.
 */

#ifndef __LNW_ECMP_HASH_TABLE_H__
#define __LNW_ECMP_HASH_TABLE_H__

#define LNW_ECMP_HASH_TABLE "linux_networking_control.ecmp_hash_table"

#define LNW_ECMP_HASH_TABLE_KEY_HOST_INFO_TX_EXT_FLEX "flex"
#define LNW_ECMP_HASH_TABLE_KEY_META_COMMON_HASH "hash"
#define LNW_ECMP_HASH_TABLE_KEY_ZERO_PADDING "zero_padding"

#define LNW_ECMP_HASH_TABLE_ACTION_SET_NEXTHOP_ID \
  "linux_networking_control.set_nexthop_id"

#define LNW_ECMP_HASH_SIZE 65536

/* Only 3 bits are allocated for hash size per group in LNW.p4
 * check LNW_ECMP_HASH_TABLE_KEY_META_COMMON_HASH */
#define LNW_ECMP_PER_GROUP_HASH_SIZE 8

#endif /* __LNW_ECMP_HASH_TABLE_H__ */
