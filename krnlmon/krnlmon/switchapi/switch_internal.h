/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
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

#ifndef _switch_internal_h_
#define _switch_internal_h_

#include <stdio.h>

#include "switchapi/switch_device_int.h"
#include "switchapi/switch_handle_int.h"
#include "switchapi/switch_status.h"
#include "switchutils/switch_log.h"
#include "switchutils/switch_utils.h"
#include "tdi/common/tdi_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWITCH_API_BUFFER_SIZE 512

switch_status_t SWITCH_ARRAY_INIT(switch_array_t* array);

switch_uint32_t SWITCH_ARRAY_COUNT(switch_array_t* array);

switch_status_t SWITCH_ARRAY_INSERT(switch_array_t* array,
                                    switch_uint64_t index, void* ptr);

switch_status_t SWITCH_ARRAY_GET(switch_array_t* array, switch_uint64_t index,
                                 void** ptr);
switch_status_t SWITCH_ARRAY_NEXT(switch_array_t* array, switch_uint64_t* index,
                                  void** ptr);

switch_status_t SWITCH_ARRAY_DELETE(switch_array_t* array,
                                    switch_uint64_t index);

switch_status_t SWITCH_LIST_INIT(switch_list_t* list);

switch_size_t SWITCH_LIST_COUNT(switch_list_t* list);

bool SWITCH_LIST_EMPTY(switch_list_t* list);

switch_status_t SWITCH_LIST_SORT(switch_list_t* list,
                                 switch_list_compare_func_t compare_func);

switch_status_t SWITCH_LIST_INSERT(switch_list_t* list, switch_node_t* node,
                                   void* ptr);

switch_status_t SWITCH_LIST_DELETE(switch_list_t* list, switch_node_t* node);

switch_status_t SWITCH_HASHTABLE_INIT(switch_hashtable_t* hashtable);

switch_size_t SWITCH_HASHTABLE_COUNT(switch_hashtable_t* hashtable);

switch_status_t SWITCH_HASHTABLE_INSERT(switch_hashtable_t* hashtable,
                                        switch_hashnode_t* node, void* key,
                                        void* data);

switch_status_t SWITCH_HASHTABLE_DELETE(switch_hashtable_t* hashtable,
                                        void* key, void** data);

switch_status_t SWITCH_HASHTABLE_DELETE_NODE(switch_hashtable_t* hashtable,
                                             switch_hashnode_t* node);

switch_status_t SWITCH_HASHTABLE_SEARCH(switch_hashtable_t* hashtable,
                                        void* key, void** data);

switch_status_t SWITCH_HASHTABLE_FOREACH_ARG(switch_hashtable_t* hashtable,
                                             void* func, void* arg);

switch_status_t SWITCH_HASHTABLE_DONE(switch_hashtable_t* hashtable);

const char* switch_error_to_string(switch_status_t status);

// char *switch_table_id_to_string(switch_table_id_t table_id);

switch_direction_t switch_table_id_to_direction(switch_table_id_t table_id);

const char* switch_api_type_to_string(switch_api_type_t api_type);

switch_int32_t switch_convert_string_to_ipv4(const char* v4_string,
                                             switch_ip4_t* ip);

switch_int32_t switch_convert_string_to_ipv6(const char* v6_string,
                                             switch_ip6_t* ip);

switch_status_t switch_mac_to_string(switch_mac_addr_t* mac, char* buffer,
                                     switch_int32_t buffer_size,
                                     switch_int32_t* length);

switch_status_t switch_ipv6_to_string(switch_ip6_t ip6, char* buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t* length);

switch_status_t switch_ipv4_to_string(switch_ip4_t ip4, char* buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t* length);

char* switch_handle_type_to_string(switch_handle_type_t handle_type);

#define SWITCH_LIST_GET_HEAD(__list, __node) \
  node = tommy_list_head(&__list.list);

#define SWITCH_ARRAY_FIRST_GET(__array, __index, __type, __entry) \
  Word_t __index_tmp = 0;                                         \
  Word_t* __pvalue = NULL;                                        \
  __entry = NULL;                                                 \
  if (__array.array) {                                            \
    JLF(__pvalue, __array.array, __index_tmp);                    \
    status = SWITCH_STATUS_ITEM_NOT_FOUND;                        \
    if (__pvalue) {                                               \
      __index = __index_tmp;                                      \
      __entry = (__type*)__pvalue;                                \
      status = SWITCH_STATUS_SUCCESS;                             \
    }                                                             \
  } else {                                                        \
    status = SWITCH_STATUS_SUCCESS;                               \
  }

#define SWITCH_ARRAY_NEXT_GET(__array, __o_index, __n_index, __type, __entry) \
  Word_t __index_tmp = (Word_t)__o_index;                                     \
  Word_t* __pvalue = NULL;                                                    \
  __entry = NULL;                                                             \
  JLN(__pvalue, __array.array, __index_tmp);                                  \
  if (__pvalue) {                                                             \
    __entry = (__type*)__pvalue;                                              \
    __n_index = __index_tmp;                                                  \
  } else {                                                                    \
    __n_index = SWITCH_API_INVALID_HANDLE;                                    \
    __entry = NULL;                                                           \
  }

#define FOR_EACH_IN_ARRAY(__index, __array, __type, __entry)            \
  {                                                                     \
    Word_t* __pvalue = NULL;                                            \
    Word_t* __pvalue_next = NULL;                                       \
    Word_t __index_tmp = __index;                                       \
    JLF(__pvalue, __array.array, __index_tmp);                          \
    __index = __index_tmp;                                              \
    for (; __pvalue; __pvalue = __pvalue_next, __index = __index_tmp) { \
      JLN(__pvalue_next, __array.array, __index_tmp);                   \
      __entry = (__type*)(*__pvalue);

#define FOR_EACH_IN_ARRAY_END() \
  }                             \
  }

#define FOR_EACH_IN_ARRAY_VALUE(__index, __array, __type, __entry)      \
  {                                                                     \
    Word_t* __pvalue = NULL;                                            \
    Word_t* __pvalue_next = NULL;                                       \
    Word_t __index_tmp = __index;                                       \
    JLF(__pvalue, __array.array, __index_tmp);                          \
    __index = __index_tmp;                                              \
    for (; __pvalue; __pvalue = __pvalue_next, __index = __index_tmp) { \
      JLN(__pvalue_next, __array.array, __index_tmp);                   \
      __entry = (__type)(*__pvalue);

#define FOR_EACH_IN_ARRAY_VALUE_END() \
  }                                   \
  }

#define FOR_EACH_IN_LIST(__list, __node)   \
  {                                        \
    node = tommy_list_head(&__list.list);  \
    switch_node_t* __next_node = NULL;     \
    for (; __node; __node = __next_node) { \
      __next_node = node->next;

#define FOR_EACH_IN_LIST_END() \
  }                            \
  }

#define SWITCH_HW_FLAG_ISSET(_info, _pd_entry) _info->hw_flags& _pd_entry

#define SWITCH_HW_FLAG_SET(_info, _pd_entry) _info->hw_flags |= _pd_entry

#define SWITCH_HW_FLAG_CLEAR(_info, _pd_entry) _info->hw_flags &= ~(_pd_entry)

#define SWITCH_HASHTABLE_ITERATOR(_hashtable, _func, _arg) \
  tommy_hashtable_foreach_arg(_hashtable, _func, _arg)

#define CHECK_RET(x, ret)                                            \
  do {                                                               \
    if (x) {                                                         \
      printf("ERROR %s at (%s)\n", switch_error_to_string(ret), #x); \
      return ret;                                                    \
    }                                                                \
  } while (0)

#define SWITCH_ASSERT(x) CHECK_RET(!(x), SWITCH_STATUS_INVALID_PARAMETER);

// TODO, get this info from p4 table
#define TUNNEL_TABLE_SIZE 128
#define TUNNEL_TERM_TABLE_SIZE TUNNEL_TABLE_SIZE
#define NEIGHBOR_MOD_TABLE_SIZE 1024
#define NEXTHOP_TABLE_SIZE 1024
#define NHOP_GROUP_HASH_TABLE_SIZE 1024
#define NHOP_MEMBER_HASH_TABLE_SIZE 1024
#define L2_FWD_RX_TABLE_SIZE 1024
#define L2_FWD_TX_TABLE_SIZE 1024
#define IPV4_TABLE_SIZE 1024

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* _switch_internal_h_ */
