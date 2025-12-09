/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2023 Intel Corporation.
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

#ifndef _switch_types_int_h_
#define _switch_types_int_h_

#include <arpa/inet.h>
#include <netinet/in.h>

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef tommy_node switch_node_t;
typedef switch_id_t switch_xid_t;
typedef switch_id_t switch_rid_t;
typedef tommy_hashtable_node switch_hashnode_t;

#define switch_pipe pipe
#define switch_fcntl fcntl
#define switch_fd_read read
#define switch_fd_write write
#define switch_fd_close close
#define switch_fd_send sendto
#define switch_ntohs ntohs
#define switch_htons htons
#define switch_socket socket
#define switch_bind bind
#define switch_ioctl ioctl
#define switch_open open
#define switch_fd_set fd_set
#define switch_select select
#define switch_snprintf snprintf

typedef switch_status_t (*switch_key_func_t)(void* args, switch_uint8_t* key,
                                             switch_uint32_t* len);

typedef switch_int32_t (*switch_hash_compare_func_t)(const void* key1,
                                                     const void* key2);

typedef switch_int32_t (*switch_list_compare_func_t)(const void* key1,
                                                     const void* key2);

typedef struct switch_array_ {
  void* array;
  switch_size_t num_entries;
} switch_array_t;

typedef struct switch_hashtable_ {
  tommy_hashtable table;
  switch_hash_compare_func_t compare_func;
  switch_key_func_t key_func;
  switch_size_t num_entries;
  switch_size_t size;
  switch_size_t hash_seed;
} switch_hashtable_t;

typedef struct switch_list_ {
  tommy_list list;
  switch_size_t num_entries;
} switch_list_t;

static inline const char* switch_macaddress_to_string(
    const switch_mac_addr_t* mac) {
  static char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac->mac_addr[0], mac->mac_addr[1], mac->mac_addr[2],
           mac->mac_addr[3], mac->mac_addr[4], mac->mac_addr[5]);
  return mac_str;
}

static inline const char* switch_ipaddress_to_string(
    const switch_ip_addr_t* ip_addr) {
  static char ipv4_str[INET_ADDRSTRLEN + 10];
  static char ipv6_str[INET6_ADDRSTRLEN + 10];
  int len = 0;
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
    uint32_t v4addr = htonl(ip_addr->ip.v4addr);
    len = strnlen(inet_ntop(AF_INET, &v4addr, ipv4_str, INET_ADDRSTRLEN),
                  INET_ADDRSTRLEN);
    snprintf(ipv4_str + len, 10, "/%d", ip_addr->prefix_len);
    return ipv4_str;
  } else {
    len = strnlen(
        inet_ntop(AF_INET6, &ip_addr->ip.v6addr, ipv6_str, INET6_ADDRSTRLEN),
        INET6_ADDRSTRLEN);
    snprintf(ipv6_str + len, 10, "/%d", ip_addr->prefix_len);
    return ipv6_str;
  }
}

#ifdef __cplusplus
}
#endif

#endif /* _switch_types_int_h_ */
