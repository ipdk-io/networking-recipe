/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2023 Intel Corporation.
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

#include "switchapi/switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define __FILE_ID__ SWITCHAPI_UTILS

static switch_uint32_t MurmurHash(const void* key, switch_uint32_t length,
                                  switch_uint32_t seed) {
// 'm' and 'r' are mixing constants generated offline.
// They're not really 'magic', they just happen to work well.
#define m 0x5bd1e995
#define r 24

  // Initialize the hash to a 'random' value
  switch_uint32_t h = seed ^ length;

  // Mix 4 bytes at a time into the hash
  const unsigned char* data = (const unsigned char*)key;

  while (length >= 4) {
    uint32_t k = *(uint32_t*)data;

    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;

    data += 4;
    length -= 4;
  }

  // Handle the last few bytes of the input array

  switch (length) {
    case 3:
      h ^= data[2] << 16;
    /* fall through */
    case 2:
      h ^= data[1] << 8;
    /* fall through */
    case 1:
      h ^= (switch_uint32_t)data[0];
      h *= m;
  };

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}

switch_status_t SWITCH_ARRAY_INIT(switch_array_t* array) {
  SWITCH_ASSERT(array != NULL);
  array->array = NULL;
  array->num_entries = 0;
  return SWITCH_STATUS_SUCCESS;
}

switch_uint32_t SWITCH_ARRAY_COUNT(switch_array_t* array) {
  SWITCH_ASSERT(array != NULL);
  return array->num_entries;
}

switch_status_t SWITCH_ARRAY_INSERT(switch_array_t* array,
                                    switch_uint64_t index, void* data) {
  SWITCH_ASSERT(array != NULL);
  SWITCH_ASSERT(data != NULL);

  Word_t* p = NULL;
  JLI(p, array->array, (switch_uint64_t)index);
  if (p) {
    *p = (Word_t)data;
    array->num_entries++;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_NO_MEMORY;
  }
}

switch_status_t SWITCH_ARRAY_GET(switch_array_t* array, switch_uint64_t index,
                                 void** data) {
  SWITCH_ASSERT(array != NULL);
  SWITCH_ASSERT(data != NULL);

  Word_t* p = NULL;
  JLG(p, array->array, (switch_uint64_t)index);
  if (p) {
    *(Word_t*)data = *(Word_t*)p;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_ITEM_NOT_FOUND;
  }
}

switch_status_t SWITCH_ARRAY_NEXT(switch_array_t* array, switch_uint64_t* index,
                                  void** data) {
  Word_t tmp_index = (Word_t)*index;

  SWITCH_ASSERT(array != NULL);
  SWITCH_ASSERT(data != NULL);

  Word_t* p = NULL;
  if (tmp_index == 0) {
    JLF(p, array->array, tmp_index);
  } else {
    JLN(p, array->array, tmp_index);
  }
  if (p) {
    *(Word_t*)data = *(Word_t*)p;
    *index = (switch_uint64_t)tmp_index;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_ITEM_NOT_FOUND;
  }
}

switch_status_t SWITCH_ARRAY_DELETE(switch_array_t* array,
                                    switch_uint64_t index) {
  SWITCH_ASSERT(array != NULL);

  switch_int32_t rc = 0;
  JLD(rc, array->array, (switch_uint64_t)index);
  if (rc == 1) {
    array->num_entries--;
    return SWITCH_STATUS_SUCCESS;
  } else {
    return SWITCH_STATUS_ITEM_NOT_FOUND;
  }
}

switch_status_t SWITCH_LIST_INIT(switch_list_t* list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);

  if (!list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("list insert failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_list_init(&list->list);
  list->num_entries = 0;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_LIST_SORT(switch_list_t* list,
                                 switch_list_compare_func_t compare_func) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);

  if (!list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("list insert failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_list_sort(&list->list, compare_func);
  return SWITCH_STATUS_SUCCESS;
}

bool SWITCH_LIST_EMPTY(switch_list_t* list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool is_empty = false;

  SWITCH_ASSERT(list != NULL);

  if (!list) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("list empty get failed, error: %s\n",
                      switch_error_to_string(status));
    return FALSE;
  }

  is_empty = tommy_list_empty(&list->list);
  return is_empty;
}

switch_size_t SWITCH_LIST_COUNT(switch_list_t* list) {
  SWITCH_ASSERT(list != NULL);

  if (!list) {
    return 0;
  }

  return list->num_entries;
}

switch_status_t SWITCH_LIST_INSERT(switch_list_t* list, switch_node_t* node,
                                   void* data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);
  SWITCH_ASSERT(node != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!list || !node || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("list insert failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_list_insert_head(&list->list, node, data);
  list->num_entries++;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_LIST_DELETE(switch_list_t* list, switch_node_t* node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(list != NULL);
  SWITCH_ASSERT(node != NULL);

  if (!list || !node) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("list delete failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_list_remove_existing(&list->list, node);
  list->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_INIT(switch_hashtable_t* hashtable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(hashtable->size != 0);

  if (!hashtable || hashtable->size == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("hashtable init failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_init(&hashtable->table, hashtable->size);
  hashtable->num_entries = 0;
  return SWITCH_STATUS_SUCCESS;
}

switch_size_t SWITCH_HASHTABLE_COUNT(switch_hashtable_t* hashtable) {
  SWITCH_ASSERT(hashtable != NULL);

  if (!hashtable) {
    return 0;
  }

  return hashtable->num_entries;
}

switch_status_t SWITCH_HASHTABLE_INSERT(switch_hashtable_t* hashtable,
                                        switch_hashnode_t* node, void* key,
                                        void* data) {
  switch_uint8_t hash_key[SWITCH_API_BUFFER_SIZE];
  switch_uint32_t hash_length = 0;
  switch_uint32_t hash = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(node != NULL);
  SWITCH_ASSERT(key != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!hashtable || !node || !key || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("hashtable insert failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hash_key, 0x0, SWITCH_API_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("hashtable insert failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  tommy_hashtable_insert(&hashtable->table, node, data, hash);
  hashtable->num_entries++;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_DELETE(switch_hashtable_t* hashtable,
                                        void* key, void** data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint8_t hash_key[SWITCH_API_BUFFER_SIZE];
  switch_uint32_t hash_length = 0;
  switch_uint32_t hash = 0;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(key != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!hashtable || !key || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("hashtable delete failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hash_key, 0x0, SWITCH_API_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_error("hashtable delete failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  *data = (void*)tommy_hashtable_remove(
      &hashtable->table, hashtable->compare_func, hash_key, hash);
  if (!(*data)) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    krnlmon_log_error("hashtable delete failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  hashtable->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_DELETE_NODE(switch_hashtable_t* hashtable,
                                             switch_hashnode_t* node) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(node != NULL);

  if (!hashtable || !node) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("hashtable delete node failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_remove_existing(&hashtable->table, node);
  hashtable->num_entries--;
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_SEARCH(switch_hashtable_t* hashtable,
                                        void* key, void** data) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint8_t hash_key[SWITCH_API_BUFFER_SIZE];
  switch_uint32_t hash_length = 0;
  switch_uint32_t hash = 0;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(key != NULL);
  SWITCH_ASSERT(data != NULL);

  if (!hashtable || !key || !data) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_debug("hashtable search failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hash_key, 0x0, SWITCH_API_BUFFER_SIZE);

  status = hashtable->key_func(key, hash_key, &hash_length);
  if (status != SWITCH_STATUS_SUCCESS) {
    krnlmon_log_debug("hashtable search failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  hash = MurmurHash(hash_key, hash_length, hashtable->hash_seed);
  *data = (void*)tommy_hashtable_search(
      &hashtable->table, hashtable->compare_func, hash_key, hash);
  if (!(*data)) {
    status = SWITCH_STATUS_ITEM_NOT_FOUND;
    krnlmon_log_debug("hashtable search failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t SWITCH_HASHTABLE_FOREACH_ARG(switch_hashtable_t* hashtable,
                                             void* func, void* arg) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);
  SWITCH_ASSERT(func != NULL);
  SWITCH_ASSERT(arg != NULL);

  if (!hashtable || !func || !arg) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("hashtable foreach arg failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_foreach_arg(&hashtable->table, func, arg);

  return SWITCH_STATUS_SUCCESS;
}

switch_status_t SWITCH_HASHTABLE_DONE(switch_hashtable_t* hashtable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(hashtable != NULL);

  if (!hashtable) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("hashtable done failed, error: %s\n",
                      switch_error_to_string(status));
    return status;
  }
  tommy_hashtable_done(&hashtable->table);
  return SWITCH_STATUS_SUCCESS;
}

const char* switch_error_to_string(switch_status_t status) {
  switch (status) {
    case SWITCH_STATUS_ITEM_NOT_FOUND:
      return "err: entry not found";
    case SWITCH_STATUS_FAILURE:
      return "err: general failure";
    case SWITCH_STATUS_NO_MEMORY:
      return "err: no memory";
    case SWITCH_STATUS_INSUFFICIENT_RESOURCES:
      return "err: insufficient resources";
    case SWITCH_STATUS_ITEM_ALREADY_EXISTS:
      return "err: item already exists";
    case SWITCH_STATUS_BUFFER_OVERFLOW:
      return "err: buffer overflow";
    case SWITCH_STATUS_INVALID_PORT_NUMBER:
      return "err: invalid port number";
    case SWITCH_STATUS_INVALID_PORT_MEMBER:
      return "err: invalid port member";
    case SWITCH_STATUS_UNINITIALIZED:
      return "err: uninitialized";
    case SWITCH_STATUS_TABLE_FULL:
      return "err: table full";
    case SWITCH_STATUS_INVALID_VLAN_ID:
      return "err: invalid vlan id";
    case SWITCH_STATUS_INVALID_ATTRIBUTE:
      return "err: invalid attribute";
    case SWITCH_STATUS_INVALID_INTERFACE:
      return "err: invalid interface";
    case SWITCH_STATUS_PORT_IN_USE:
      return "err: port in use";
    case SWITCH_STATUS_NOT_IMPLEMENTED:
      return "err: not implemented";
    case SWITCH_STATUS_INVALID_HANDLE:
      return "err: invalid handle";
    case SWITCH_STATUS_PD_FAILURE:
      return "err: pd failure";
    case SWITCH_STATUS_INVALID_PARAMETER:
      return "err: invalid parameter";
    default:
      return "err: unknown failure";
  }
}

char* switch_handle_type_to_string(switch_handle_type_t handle_type) {
  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_NONE:
      return "none";
    case SWITCH_HANDLE_TYPE_PORT:
      return "port";
    case SWITCH_HANDLE_TYPE_LAG:
      return "lag";
    case SWITCH_HANDLE_TYPE_LAG_MEMBER:
      return "lag member";
    case SWITCH_HANDLE_TYPE_INTERFACE:
      return "interface";
    case SWITCH_HANDLE_TYPE_VRF:
      return "vrf";
    case SWITCH_HANDLE_TYPE_BD:
      return "bd";
    case SWITCH_HANDLE_TYPE_NHOP:
      return "nexthop";
    case SWITCH_HANDLE_TYPE_NEIGHBOR:
      return "neighbor";
    case SWITCH_HANDLE_TYPE_RMAC:
      return "rmac";
    case SWITCH_HANDLE_TYPE_VLAN:
      return "vlan";
    case SWITCH_HANDLE_TYPE_STP:
      return "stp";
    case SWITCH_HANDLE_TYPE_MGID:
      return "mgid";
    case SWITCH_HANDLE_TYPE_ACL:
      return "acl";
    case SWITCH_HANDLE_TYPE_MGID_ECMP:
      return "mgid ecmp";
    case SWITCH_HANDLE_TYPE_URPF:
      return "urpf";
    case SWITCH_HANDLE_TYPE_HOSTIF_GROUP:
      return "hostif group";
    case SWITCH_HANDLE_TYPE_HOSTIF:
      return "hostif";
    case SWITCH_HANDLE_TYPE_ACE:
      return "ace";
    case SWITCH_HANDLE_TYPE_MIRROR:
      return "mirror";
    case SWITCH_HANDLE_TYPE_METER:
      return "ingress meter";
    case SWITCH_HANDLE_TYPE_EGRESS_METER:
      return "egress meter";
    case SWITCH_HANDLE_TYPE_SFLOW:
      return "sflow";
    case SWITCH_HANDLE_TYPE_SFLOW_ACE:
      return "sflow ace";
    case SWITCH_HANDLE_TYPE_ACL_COUNTER:
      return "acl counter";
    case SWITCH_HANDLE_TYPE_RACL_COUNTER:
      return "racl counter";
    case SWITCH_HANDLE_TYPE_EGRESS_ACL_COUNTER:
      return "egress_acl counter";
    case SWITCH_HANDLE_TYPE_QOS_MAP:
      return "qos map";
    case SWITCH_HANDLE_TYPE_PRIORITY_GROUP:
      return "priority group";
    case SWITCH_HANDLE_TYPE_QUEUE:
      return "queue";
    case SWITCH_HANDLE_TYPE_SCHEDULER:
      return "scheduler";
    case SWITCH_HANDLE_TYPE_BUFFER_POOL:
      return "buffer pool";
    case SWITCH_HANDLE_TYPE_BUFFER_PROFILE:
      return "buffer profile";
    case SWITCH_HANDLE_TYPE_LABEL:
      return "label";
    case SWITCH_HANDLE_TYPE_BD_MEMBER:
      return "bd member";
    case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:
      return "logical network";
    case SWITCH_HANDLE_TYPE_BFD:
      return "bfd";
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER:
      return "tunnel mapper";
    case SWITCH_HANDLE_TYPE_HASH:
      return "hash";
    case SWITCH_HANDLE_TYPE_WRED:
      return "wred";
    case SWITCH_HANDLE_TYPE_RANGE:
      return "range";
    case SWITCH_HANDLE_TYPE_NHOP_MEMBER:
      return "nhop member";
    case SWITCH_HANDLE_TYPE_STP_PORT:
      return "stp port";
    case SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE:
      return "hostif reason code";
    case SWITCH_HANDLE_TYPE_RPF_GROUP:
      return "mrpf";
    case SWITCH_HANDLE_TYPE_RIF:
      return "rif";
    case SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER:
      return "hostif rx filter";
    case SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER:
      return "hostif tx filter";
    case SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER:
      return "pktdriver rx filter";
    case SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER:
      return "pktdriver tx filter";
    case SWITCH_HANDLE_TYPE_ROUTE:
      return "route";
    case SWITCH_HANDLE_TYPE_DEVICE:
      return "device";
    case SWITCH_HANDLE_TYPE_MTU:
      return "mtu";
    case SWITCH_HANDLE_TYPE_ACL_GROUP:
      return "acl group";
    case SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER:
      return "acl group member";
    case SWITCH_HANDLE_TYPE_WRED_COUNTER:
      return "wred counter";
    case SWITCH_HANDLE_TYPE_SCHEDULER_GROUP:
      return "scheduler group";
    case SWITCH_HANDLE_TYPE_WRED_PROFILE:
      return "wred profile";
    case SWITCH_HANDLE_TYPE_DTEL:
      return "dtel";
    case SWITCH_HANDLE_TYPE_DTEL_QUEUE_ALERT:
      return "dtel queue alert";
    case SWITCH_HANDLE_TYPE_DTEL_INT_SESSION:
      return "dtel int session";
    case SWITCH_HANDLE_TYPE_DTEL_REPORT_SESSION:
      return "dtel report session";
    case SWITCH_HANDLE_TYPE_DTEL_EVENT:
      return "dtel event";
    case SWITCH_HANDLE_TYPE_TUNNEL:
      return "tunnel";
    case SWITCH_HANDLE_TYPE_TUNNEL_ENCAP:
      return "tunnel encap";
    case SWITCH_HANDLE_TYPE_TUNNEL_TERM:
      return "tunnel term";
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY:
      return "tunnel mapper entry";
    case SWITCH_HANDLE_TYPE_MPLS:
      return "mpls";
    case SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK:
      return "mpls label stack";
    case SWITCH_HANDLE_TYPE_SR_SIDLIST:
      return "sr sidlist";
    case SWITCH_HANDLE_TYPE_METER_COLOR_ACTION:
      return "meter color action";
    case SWITCH_HANDLE_TYPE_NHOP_GROUP:
      return "nhop group";
    case SWITCH_HANDLE_TYPE_L2_FWD_RX:
      return "l2 fwd rx";
    case SWITCH_HANDLE_TYPE_L2_FWD_TX:
      return "l2 fwd tx";
    case SWITCH_HANDLE_TYPE_MAC:
      return "mac";
    case SWITCH_HANDLE_TYPE_MAX:
    default:
      return "invalid";
  }
}

// TODO: Remove this function. Only place called is from
// switch_pd_port.c, which might be dead code and needs cleanup.
#if 0
switch_status_t switch_pd_status_to_status(bf_status_t pd_status) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch (pd_status) {
    case BF_SUCCESS:
      status = SWITCH_STATUS_SUCCESS;
      break;

    case BF_NO_SYS_RESOURCES:
      status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
      break;

    case BF_ALREADY_EXISTS:
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      break;

    case BF_IN_USE:
      status = SWITCH_STATUS_RESOURCE_IN_USE;
      break;

    case BF_HW_COMM_FAIL:
      status = SWITCH_STATUS_HW_FAILURE;
      break;

    case BF_OBJECT_NOT_FOUND:
      status = SWITCH_STATUS_ITEM_NOT_FOUND;
      break;

    case BF_NOT_IMPLEMENTED:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;

    case BF_INVALID_ARG:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;

    case BF_NO_SPACE:
      status = SWITCH_STATUS_TABLE_FULL;
      break;

    default:
      status = SWITCH_STATUS_PD_FAILURE;
      break;
  }
  return status;
}
#endif

switch_status_t switch_pd_tdi_status_to_status(tdi_status_t pd_status) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch (pd_status) {
    case TDI_SUCCESS:
      status = SWITCH_STATUS_SUCCESS;
      break;

    case TDI_NO_SYS_RESOURCES:
      status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
      break;

    case TDI_ALREADY_EXISTS:
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      break;

    case TDI_IN_USE:
      status = SWITCH_STATUS_RESOURCE_IN_USE;
      break;

    case TDI_HW_COMM_FAIL:
      status = SWITCH_STATUS_HW_FAILURE;
      break;

    case TDI_OBJECT_NOT_FOUND:
      status = SWITCH_STATUS_ITEM_NOT_FOUND;
      break;

    case TDI_NOT_IMPLEMENTED:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;

    case TDI_INVALID_ARG:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;

    case TDI_NO_SPACE:
      status = SWITCH_STATUS_TABLE_FULL;
      break;

    default:
      status = SWITCH_STATUS_PD_FAILURE;
      break;
  }
  return status;
}

#if 0
static bool switch_l3_host_entry(const switch_ip_addr_t* ip_addr) {
  SWITCH_ASSERT(ip_addr != NULL);

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    return ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH_IN_BITS ? TRUE
                                                                    : FALSE;
  } else {
    return ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS ? TRUE
                                                                    : FALSE;
  }
}
#endif

switch_status_t switch_ipv4_to_string(switch_ip4_t ip4, char* buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t* length) {
  SWITCH_ASSERT(buffer != NULL);

  char tmp_buffer[SWITCH_API_BUFFER_SIZE];
  switch_ip4_t v4addr = htonl(ip4);
  inet_ntop(AF_INET, &v4addr, tmp_buffer, sizeof(tmp_buffer));
  *length = (switch_int32_t)strnlen(tmp_buffer, sizeof(tmp_buffer));
  SWITCH_MEMCPY(buffer, tmp_buffer, *length);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_ipv6_to_string(switch_ip6_t ip6, char* buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t* length) {
  SWITCH_ASSERT(buffer != NULL);

  char tmp_buffer[SWITCH_API_BUFFER_SIZE];
  inet_ntop(AF_INET6, &ip6, tmp_buffer, sizeof(tmp_buffer));
  *length = (switch_int32_t)strnlen(tmp_buffer, sizeof(tmp_buffer));
  SWITCH_MEMCPY(buffer, tmp_buffer, *length);
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_mac_to_string(switch_mac_addr_t* mac, char* buffer,
                                     switch_int32_t buffer_size,
                                     switch_int32_t* length_out) {
  SWITCH_ASSERT(buffer != NULL);
  SWITCH_ASSERT(mac != NULL);

  switch_int32_t length =
      snprintf(buffer, buffer_size, "%02x:%02x:%02x:%02x:%02x:%02x",
               mac->mac_addr[0], mac->mac_addr[1], mac->mac_addr[2],
               mac->mac_addr[3], mac->mac_addr[4], mac->mac_addr[5]);
  if (length_out) {
    *length_out = length;
  }
  return SWITCH_STATUS_SUCCESS;
}

#if 0
static switch_status_t switch_ipv4_prefix_to_mask(switch_uint32_t prefix,
                                                  switch_uint32_t* mask) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!mask) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    return status;
  }

  *mask = 0;
  if (prefix) {
    *mask = (0xFFFFFFFF << (SWITCH_IPV4_PREFIX_LENGTH_IN_BITS - prefix));
    *mask = *mask & 0xFFFFFFFF;
  }
  return status;
}

static switch_status_t switch_ipv6_prefix_to_mask(switch_uint32_t prefix,
                                                  switch_uint8_t* mask) {
  switch_uint32_t prefix_bytes = 0;
  switch_uint32_t index = 0;

  SWITCH_MEMSET(mask, 0xFF, SWITCH_IPV6_PREFIX_LENGTH);

  if (prefix == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS) {
    return SWITCH_STATUS_SUCCESS;
  }

  prefix_bytes = prefix / SWITCH_IPV6_PREFIX_LENGTH;
  for (index = 0; index < prefix_bytes; index++) {
    mask[index] = 0xFF;
  }

  mask[index] = (0xFF << (SWITCH_IPV6_PREFIX_LENGTH_IN_BITS - prefix));
  mask[index] = mask[index] & 0xFF;
  return SWITCH_STATUS_SUCCESS;
}
#endif

switch_direction_t switch_table_id_to_direction(switch_table_id_t table_id) {
  switch (table_id) {
    case SWITCH_TABLE_NONE:
      return 0;

    case SWITCH_TABLE_INGRESS_PORT_MAPPING:
    case SWITCH_TABLE_INGRESS_PORT_PROPERTIES:
    case SWITCH_TABLE_OUTER_RMAC:
    case SWITCH_TABLE_INNER_RMAC:
    case SWITCH_TABLE_SMAC:
    case SWITCH_TABLE_DMAC:
    case SWITCH_TABLE_IPV4_HOST:
    case SWITCH_TABLE_IPV6_HOST:
    case SWITCH_TABLE_IPV4_LPM:
    case SWITCH_TABLE_IPV6_LPM:
    case SWITCH_TABLE_URPF:
    case SWITCH_TABLE_IPV4_LOCAL_HOST:
    case SWITCH_TABLE_NHOP:
    case SWITCH_TABLE_NHOP_GROUP:
    case SWITCH_TABLE_NHOP_MEMBER_SELECT:
    case SWITCH_TABLE_IPV4_SRC_VTEP:
    case SWITCH_TABLE_IPV4_DST_VTEP:
    case SWITCH_TABLE_IPV6_SRC_VTEP:
    case SWITCH_TABLE_IPV6_DST_VTEP:
    case SWITCH_TABLE_IPV4_VTEP:
    case SWITCH_TABLE_IPV6_VTEP:
    case SWITCH_TABLE_TUNNEL:
    case SWITCH_TABLE_TUNNEL_MPLS:
    case SWITCH_TABLE_PORT_VLAN_TO_BD_MAPPING:
    case SWITCH_TABLE_PORT_VLAN_TO_IFINDEX_MAPPING:
    case SWITCH_TABLE_BD:
    case SWITCH_TABLE_BD_FLOOD:
    case SWITCH_TABLE_INGRESS_BD_STATS:
    case SWITCH_TABLE_IPV4_ACL:
    case SWITCH_TABLE_IPV6_ACL:
    case SWITCH_TABLE_IPV4_RACL:
    case SWITCH_TABLE_IPV6_RACL:
    case SWITCH_TABLE_IPV4_QOS_ACL:
    case SWITCH_TABLE_IPV4_MIRROR_ACL:
    case SWITCH_TABLE_IPV6_MIRROR_ACL:
    case SWITCH_TABLE_IPV4_DTEL_ACL:
    case SWITCH_TABLE_IPV6_DTEL_ACL:
    case SWITCH_TABLE_SYSTEM_ACL:
    case SWITCH_TABLE_LOW_PRI_SYSTEM_ACL:
    case SWITCH_TABLE_SYSTEM_REASON_ACL:
    case SWITCH_TABLE_MAC_ACL:
    case SWITCH_TABLE_MAC_QOS_ACL:
    case SWITCH_TABLE_ACL_STATS:
    case SWITCH_TABLE_RACL_STATS:
    case SWITCH_TABLE_OUTER_MCAST_STAR_G:
    case SWITCH_TABLE_OUTER_MCAST_SG:
    case SWITCH_TABLE_OUTER_MCAST_RPF:
    case SWITCH_TABLE_MCAST_RPF:
    case SWITCH_TABLE_IPV4_MCAST_S_G:
    case SWITCH_TABLE_IPV4_MCAST_STAR_G:
    case SWITCH_TABLE_IPV6_MCAST_S_G:
    case SWITCH_TABLE_IPV6_MCAST_STAR_G:
    case SWITCH_TABLE_STP:
    case SWITCH_TABLE_LAG_GROUP:
    case SWITCH_TABLE_LAG_SELECT:
    case SWITCH_TABLE_MIRROR:
    case SWITCH_TABLE_METER_INDEX:
    case SWITCH_TABLE_METER_ACTION:
    case SWITCH_TABLE_DROP_STATS:
    case SWITCH_TABLE_INGRESS_QOS_MAP:
    case SWITCH_TABLE_INGRESS_QOS_MAP_DSCP:
    case SWITCH_TABLE_INGRESS_QOS_MAP_PCP:
      return SWITCH_API_DIRECTION_INGRESS;

    case SWITCH_TABLE_EGRESS_PORT_MAPPING:
    case SWITCH_TABLE_EGRESS_BD_STATS:
    case SWITCH_TABLE_SMAC_REWRITE:
    case SWITCH_TABLE_MTU:
    case SWITCH_TABLE_REWRITE:
    case SWITCH_TABLE_TUNNEL_REWRITE:
    case SWITCH_TABLE_TUNNEL_DECAP:
    case SWITCH_TABLE_TUNNEL_SMAC_REWRITE:
    case SWITCH_TABLE_TUNNEL_DMAC_REWRITE:
    case SWITCH_TABLE_IPV4_TUNNEL_DIP_REWRITE:
    case SWITCH_TABLE_IPV6_TUNNEL_DIP_REWRITE:
    case SWITCH_TABLE_VLAN_DECAP:
    case SWITCH_TABLE_VLAN_XLATE:
    case SWITCH_TABLE_EGRESS_BD:
    case SWITCH_TABLE_EGRESS_IPV4_ACL:
    case SWITCH_TABLE_EGRESS_IPV6_ACL:
    case SWITCH_TABLE_EGRESS_IPV4_MIRROR_ACL:
    case SWITCH_TABLE_EGRESS_IPV6_MIRROR_ACL:
    case SWITCH_TABLE_EGRESS_SYSTEM_ACL:
    case SWITCH_TABLE_EGRESS_ACL_STATS:
    case SWITCH_TABLE_RID:
    case SWITCH_TABLE_REPLICA_TYPE:
    case SWITCH_TABLE_EGRESS_METER_INDEX:
    case SWITCH_TABLE_EGRESS_METER_ACTION:
    case SWITCH_TABLE_EGRESS_PFC_ACL:
    case SWITCH_TABLE_EGRESS_QOS_MAP:
      return SWITCH_API_DIRECTION_EGRESS;
    // TODO: Handle cases below
    case SWITCH_TABLE_TUNNEL_SIP_REWRITE:
    case SWITCH_TABLE_TUNNEL_TERM:
    case SWITCH_TABLE_ECN_ACL:
    case SWITCH_TABLE_PFC_ACL:
    case SWITCH_TABLE_MAX_ACL:
    case SWITCH_TABLE_IPV4_BRIDGE_MCAST_S_G:
    case SWITCH_TABLE_IPV4_BRIDGE_MCAST_STAR_G:
    case SWITCH_TABLE_IPV6_BRIDGE_MCAST_S_G:
    case SWITCH_TABLE_IPV6_BRIDGE_MCAST_STAR_G:
    case SWITCH_TABLE_NAT_DST:
    case SWITCH_TABLE_NAT_SRC:
    case SWITCH_TABLE_NAT_TWICE:
    case SWITCH_TABLE_NAT_FLOW:
    case SWITCH_TABLE_QUEUE:
    case SWITCH_TABLE_WRED:
    case SWITCH_TABLE_METER_COLOR_ACTION:
    case SWITCH_TABLE_NEIGHBOR:
    case SWITCH_TABLE_MAX:
    default:
      return 0;
  }
}

const char* switch_api_type_to_string(switch_api_type_t api_type) {
  switch (api_type) {
    case SWITCH_API_TYPE_PORT:
      return "port";
    case SWITCH_API_TYPE_L2:
      return "l2";
    case SWITCH_API_TYPE_BD:
      return "bd";
    case SWITCH_API_TYPE_VRF:
      return "vrf";
    case SWITCH_API_TYPE_L3:
      return "l3";
    case SWITCH_API_TYPE_RMAC:
      return "rmac";
    case SWITCH_API_TYPE_INTERFACE:
      return "intf";
    case SWITCH_API_TYPE_LAG:
      return "lag";
    case SWITCH_API_TYPE_NHOP:
      return "nhop";
    case SWITCH_API_TYPE_NEIGHBOR:
      return "neighbor";
    case SWITCH_API_TYPE_TUNNEL:
      return "tunnel";
    case SWITCH_API_TYPE_MCAST:
      return "mcast";
    case SWITCH_API_TYPE_HOSTIF:
      return "hostif";
    case SWITCH_API_TYPE_ACL:
      return "acl";
    case SWITCH_API_TYPE_MIRROR:
      return "mirror";
    case SWITCH_API_TYPE_METER:
      return "meter";
    case SWITCH_API_TYPE_SFLOW:
      return "sflow";
    case SWITCH_API_TYPE_DTEL:
      return "dtel";
    case SWITCH_API_TYPE_STP:
      return "stp";
    case SWITCH_API_TYPE_VLAN:
      return "vlan";
    case SWITCH_API_TYPE_QOS:
      return "qos";
    case SWITCH_API_TYPE_QUEUE:
      return "queue";
    case SWITCH_API_TYPE_LOGICAL_NETWORK:
      return "ln";
    case SWITCH_API_TYPE_NAT:
      return "nat";
    case SWITCH_API_TYPE_BUFFER:
      return "buffer";
    case SWITCH_API_TYPE_BFD:
      return "bfd";
    case SWITCH_API_TYPE_HASH:
      return "hash";
    case SWITCH_API_TYPE_WRED:
      return "wred";
    case SWITCH_API_TYPE_ILA:
      return "ila";
    case SWITCH_API_TYPE_FAILOVER:
      return "failover";
    case SWITCH_API_TYPE_LABEL:
      return "label";
    case SWITCH_API_TYPE_RPF:
      return "rpf";
    case SWITCH_API_TYPE_DEVICE:
      return "device";
    case SWITCH_API_TYPE_RIF:
      return "rif";
    case SWITCH_API_TYPE_PACKET_DRIVER:
      return "pktdriver";
    case SWITCH_API_TYPE_SCHEDULER:
      return "scheduler";
    case SWITCH_API_TYPE_MPLS:
      return "mpls";
    case SWITCH_API_TYPE_NONE:
    default:
      return "unknown";
  }
}

#if 0
static char* switch_packet_type_to_string(switch_packet_type_t packet_type) {
  switch (packet_type) {
    case SWITCH_PACKET_TYPE_UNICAST:
      return "unicast";
    case SWITCH_PACKET_TYPE_MULTICAST:
      return "multicast";
    case SWITCH_PACKET_TYPE_BROADCAST:
      return "broadcast";
    case SWITCH_PACKET_TYPE_MAX:
    default:
      return "unknown";
  }
}
#endif

switch_int32_t switch_convert_string_to_ipv4(const char* v4_string,
                                             switch_ip4_t* ip) {
  switch_int32_t ret_val;

  if (!v4_string || !ip) {
    return 0;
  }

  ret_val = inet_pton(AF_INET, v4_string, ip);
  if (ret_val != 1) {
    return 0;
  }

  *ip = htonl(*ip);
  return 1;
}

switch_int32_t switch_convert_string_to_ipv6(const char* v6_string,
                                             switch_ip6_t* ip) {
  switch_int32_t ret_val;

  if (!v6_string || !ip) {
    return 0;
  }

  ret_val = inet_pton(AF_INET6, v6_string, ip->u.addr8);
  if (ret_val != 1) {
    return 0;
  }

  return 1;
}

#ifdef __cplusplus
}
#endif
