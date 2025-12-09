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

#ifndef __SWITCH_FDB_H__
#define __SWITCH_FDB_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_L2_HASH_SEED 0x12345678

#define SWITCH_L2_HASH_KEY_SIZE sizeof(switch_mac_addr_t)

#define SWITCH_L2_KEY_GET(_api_l2_info, _l2_key)                  \
  {                                                               \
    do {                                                          \
      SWITCH_MEMSET(&_l2_key, 0x0, sizeof(switch_l2_key_t));      \
      SWITCH_ASSERT(SWITCH_RIF_HANDLE(_api_l2_info->rif_handle)); \
      _l2_key.rif_handle = _api_l2_info->rif_handle;              \
      SWITCH_MEMCPY(&_l2_key.mac, &_api_l2_info->dst_mac,         \
                    sizeof(switch_mac_addr_t));                   \
    } while (0);                                                  \
  }

/** l2 info handle wrappers */
#define switch_l2_tx_handle_create(_device)                   \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_L2_FWD_TX, \
                       sizeof(switch_l2_info_t))

#define switch_l2_tx_handle_delete(_device, _handle, _count) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_L2_FWD_TX, _handle)

#define switch_l2_rx_handle_delete(_device, _handle, _count) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_L2_FWD_RX, _handle)

#define switch_l2_rx_handle_create(_device)                   \
  switch_handle_create(_device, SWITCH_HANDLE_TYPE_L2_FWD_RX, \
                       sizeof(switch_l2_info_t))

#define switch_l2_tx_get(_device, _handle, _info)                     \
  ({                                                                  \
    switch_l2_info_t* _tmp_l2_info = NULL;                            \
    (void)(_tmp_l2_info == *_info);                                   \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_L2_FWD_TX, _handle, \
                      (void**)_info);                                 \
  })

#define switch_l2_rx_get(_device, _handle, _info)                     \
  ({                                                                  \
    switch_l2_info_t* _tmp_l2_info = NULL;                            \
    (void)(_tmp_l2_info == *_info);                                   \
    switch_handle_get(_device, SWITCH_HANDLE_TYPE_L2_FWD_RX, _handle, \
                      (void**)_info);                                 \
  })

#define SWITCH_L2_TX_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_L2_FWD_TX)

#define SWITCH_L2_RX_HANDLE(handle) \
  SWITCH_HANDLE_VALID(handle, SWITCH_HANDLE_TYPE_L2_FWD_RX)

typedef enum switch_l2_info_type_s {
  SWITCH_L2_FWD_NONE,
  SWITCH_L2_FWD_TX,
  SWITCH_L2_FWD_RX,
  SWITCH_L2_FWD_MAX
} switch_l2_info_type_t;

typedef enum switch_l2_learn_from_t {
  SWITCH_L2_FWD_LEARN_NONE,
  SWITCH_L2_FWD_LEARN_TUNNEL_INTERFACE,
  SWITCH_L2_FWD_LEARN_VLAN_INTERFACE,
  SWITCH_L2_FWD_LEARN_PHYSICAL_INTERFACE,
  SWITCH_L2_FWD_LEARN_MAX
} switch_l2_learn_from_t;

typedef struct switch_api_l2_info_s {
  switch_mac_addr_t dst_mac;

  switch_handle_t rif_handle;

  switch_l2_info_type_t type;

  uint8_t port_id;

  uint8_t bridge_id;

  switch_l2_learn_from_t learn_from;

  switch_handle_t l2_handle;

} switch_api_l2_info_t;

typedef struct switch_l2_info_s {
  /** hashtable node */
  switch_hashnode_t node;

  switch_api_l2_info_t api_l2_info;

} switch_l2_info_t;

typedef struct switch_l2_context_s {
  switch_hashtable_t l2_hashtable;

} switch_l2_context_t;

switch_status_t switch_l2_init(switch_device_t device);

switch_status_t switch_l2_free(switch_device_t device);

switch_status_t switch_api_l2_forward_create(const switch_device_t device,
                                             switch_api_l2_info_t* api_l2_info,
                                             switch_handle_t* l2_handle);

switch_status_t switch_api_l2_forward_delete(
    const switch_device_t device, const switch_api_l2_info_t* api_l2_info);

switch_status_t switch_l2_hash_key_init(void* args, switch_uint8_t* key,
                                        switch_uint32_t* len);

switch_int32_t switch_l2_hash_compare(const void* key1, const void* key2);

switch_status_t switch_api_l2_handle_get(const switch_device_t device,
                                         const switch_mac_addr_t* l2_key,
                                         switch_handle_t* l2_handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_FDB_H__ */
