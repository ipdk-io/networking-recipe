/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
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

#ifndef __SWITCH_HANDLE_INT_H__
#define __SWITCH_HANDLE_INT_H__

#include "target-utils/id/id.h"
//#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/** get id from handle */
#define handle_to_id(x) (x & ((1 << SWITCH_HANDLE_TYPE_SHIFT) - 1))

/** get handle from id and handle type */
#define id_to_handle(t, x) (t << SWITCH_HANDLE_TYPE_SHIFT | x)

/** invalid hardware handle default value */
#define SWITCH_PD_INVALID_HANDLE 0x0FFFFFFFF

/** Handle related information */
typedef struct switch_handle_info_s {
  /** handle info initalizied */
  bool inited;

  /** handle type */
  switch_handle_type_t type;

  /** handle id allocator */
  switch_id_allocator_t* allocator;

  /** number of handles allocated */
  switch_uint32_t num_in_use;

  /** size of allocator */
  switch_uint32_t initial_size;

  /** number of handles to allocate */
  switch_uint32_t num_handles;

  /** grow size on demand */
  bool grow_on_demand;

  /** 0 based handles */
  bool zero_based;

  /** new allocator for larger continuous range */
  bf_id_allocator new_allocator;
} switch_handle_info_t;

switch_status_t switch_handle_type_init(switch_device_t device,
                                        switch_handle_type_t type,
                                        switch_size_t size);

switch_status_t switch_handle_type_allocator_init(switch_device_t device,
                                                  switch_handle_type_t type,
                                                  switch_uint32_t num_handles,
                                                  bool grow_on_demand,
                                                  bool zero_based);

switch_status_t switch_handle_type_free(switch_device_t device,
                                        switch_handle_type_t type);

switch_handle_t switch_handle_create(switch_device_t device,
                                     switch_handle_type_t type,
                                     switch_size_t size);

switch_handle_t switch_handle_create_contiguous(switch_device_t device,
                                                switch_handle_type_t type,
                                                switch_uint32_t size,
                                                unsigned int count);

switch_handle_t switch_handle_create_special(switch_device_t device,
                                             switch_handle_type_t type,
                                             switch_uint32_t size,
                                             unsigned int count);

switch_handle_t switch_handle_create_and_set(switch_device_t device,
                                             switch_handle_type_t type,
                                             switch_uint32_t id,
                                             switch_uint32_t size);

switch_status_t switch_handle_get(switch_device_t device,
                                  switch_handle_type_t type,
                                  switch_handle_t handle, void** i_info);

switch_status_t switch_handle_delete(switch_device_t device,
                                     switch_handle_type_t type,
                                     switch_handle_t handle);

switch_status_t switch_handle_delete_contiguous(switch_device_t device,
                                                switch_handle_type_t type,
                                                switch_handle_t handle,
                                                uint32_t count);

switch_status_t switch_handle_delete_special(switch_device_t device,
                                             switch_handle_type_t type,
                                             switch_handle_t handle,
                                             uint32_t count);

switch_status_t switch_api_handle_iterate_internal(switch_device_t device,
                                                   switch_handle_type_t type,
                                                   switch_handle_t old_handle,
                                                   switch_handle_t* new_handle);

switch_status_t switch_api_handle_count_get(switch_device_t device,
                                            switch_handle_type_t type,
                                            switch_size_t* num_entries);

#define SWITCH_HANDLE_VALID(handle, type) \
  (switch_handle_type_get(handle) == type)

#define SWITCH_PORT_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_PORT)

#define SWITCH_RIF_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_RIF)

#define SWITCH_INTERFACE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_INTERFACE)

#define SWITCH_LAG_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_LAG)

#define SWITCH_LAG_MEMBER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_LAG_MEMBER)

#define SWITCH_VRF_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_VRF)

#define SWITCH_BD_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_BD)

#define SWITCH_VLAN_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_VLAN)

#define SWITCH_BD_MEMBER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_BD_MEMBER)

#define SWITCH_LN_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_LOGICAL_NETWORK)

#define SWITCH_NHOP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_NHOP)

//#define SWITCH_ECMP_HANDLE(handle) SWITCH_NHOP_HANDLE(handle)
#define SWITCH_NHOP_GROUP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_NHOP_GROUP)

#define SWITCH_WCMP_HANDLE(handle) SWITCH_NHOP_HANDLE(handle)

#define SWITCH_NEIGHBOR_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_NEIGHBOR)

#define SWITCH_RMAC_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_RMAC)

#define SWITCH_STP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_STP)

#define SWITCH_MGID_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_MGID)

#define SWITCH_MGID_ECMP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_MGID_ECMP)

#define SWITCH_ACL_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_ACL)

#define SWITCH_ACE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_ACE)

#define SWITCH_HOSTIF_GROUP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_HOSTIF_GROUP)

#define SWITCH_HOSTIF_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_HOSTIF)

#define SWITCH_HOSTIF_RCODE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE)

#define SWITCH_HOSTIF_RX_FILTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER)

#define SWITCH_HOSTIF_TX_FILTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER)

#define SWITCH_PKTDRIVER_RX_FILTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER)

#define SWITCH_PKTDRIVER_TX_FILTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER)

#define SWITCH_ACL_COUNTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_ACL_COUNTER)

#define SWITCH_RACL_COUNTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_RACL_COUNTER)

#define SWITCH_EGRESS_ACL_COUNTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_EGRESS_ACL_COUNTER)

#define SWITCH_MIRROR_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_MIRROR)

#define SWITCH_METER_HANDLE(handle)                         \
  (SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_METER) || \
   SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_EGRESS_METER))

#define SWITCH_SFLOW_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_SFLOW)

#define SWITCH_PPG_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_PRIORITY_GROUP)

#define SWITCH_QOS_MAP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_QOS_MAP)

#define SWITCH_QUEUE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_QUEUE)

#define SWITCH_BUFFER_POOL_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_BUFFER_POOL)

#define SWITCH_BUFFER_PROFILE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_BUFFER_PROFILE)

#define SWITCH_BFD_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_BFD)

#define SWITCH_TUNNEL_MAPPER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER)

#define SWITCH_TUNNEL_MAPPER_ENTRY_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY)

#define SWITCH_TUNNEL_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_TUNNEL)

#define SWITCH_TUNNEL_TERM_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_TUNNEL_TERM)

#define SWITCH_TUNNEL_ENCAP_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_TUNNEL_ENCAP)

#define SWITCH_HASH_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_HASH)

#define SWITCH_WRED_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_WRED)

#define SWITCH_WRED_COUNTER_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_WRED_COUNTER)

#define SWITCH_RANGE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_RANGE)

#define SWITCH_NHOP_MEMBER_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_NHOP_MEMBER)

#define SWITCH_LABEL_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_LABEL)

#define SWITCH_MAC_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_MAC)

#define SWITCH_ROUTE_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_ROUTE)

#define SWITCH_NETWORK_HANDLE(_handle) \
  (SWITCH_VLAN_HANDLE(_handle) || SWITCH_LN_HANDLE(_handle))

#define SWITCH_RPF_GROUP_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_RPF_GROUP)

#define SWITCH_DEVICE_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_DEVICE)

#define SWITCH_MTU_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_MTU)

#define SWITCH_ACL_GROUP_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_ACL_GROUP)

#define SWITCH_ACL_GROUP_MEMBER_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER)

#define SWITCH_SCHEDULER_GROUP_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_SCHEDULER_GROUP)

#define SWITCH_SCHEDULER_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_SCHEDULER)

#define SWITCH_WRED_PROFILE_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_WRED_PROFILE)

#define SWITCH_MPLS_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_MPLS)

#define SWITCH_MPLS_LABEL_STACK_HANDLE(_handle) \
  SWITCH_HANDLE_VALID(_handle, SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK)

#define FOR_EACH_HANDLE_BEGIN(__device, __type, __handle)                     \
  {                                                                           \
    switch_status_t __status = SWITCH_STATUS_SUCCESS;                         \
    switch_handle_t __o_handle = SWITCH_API_INVALID_HANDLE;                   \
    switch_handle_t __n_handle = SWITCH_API_INVALID_HANDLE;                   \
    do {                                                                      \
      __status = switch_api_handle_iterate_internal(__device, __type,         \
                                                    __o_handle, &__n_handle); \
      SWITCH_ASSERT(__status == SWITCH_STATUS_SUCCESS);                       \
      __handle = __n_handle;                                                  \
      __o_handle = __n_handle;

#define FOR_EACH_HANDLE_END()                     \
  }                                               \
  while (__n_handle != SWITCH_API_INVALID_HANDLE) \
    ;                                             \
  }

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __SWITCH_HANDLE_INT_H__ */
