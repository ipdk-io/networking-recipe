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

#include "saiinternal.h"
#include "switchapi/switch_base_types.h"

sai_status_t sai_ipv4_prefix_length(_In_ sai_ip4_t ip4,
                                    _Out_ uint32_t* prefix_length) {
  int x = 0;
  *prefix_length = 0;
  while (ip4) {
    x = ip4 & 0x1;
    if (x) (*prefix_length)++;
    ip4 = ip4 >> 1;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipv6_prefix_length(_In_ const sai_ip6_t ip6,
                                    _Out_ uint32_t* prefix_length) {
  int i = 0, x = 0;
  sai_ip6_t ip6_temp;
  memcpy(ip6_temp, ip6, 16);
  *prefix_length = 0;
  for (i = 0; i < 16; i++) {
    if (ip6_temp[i] == 0xFF) {
      *prefix_length += 8;
    } else {
      while (ip6_temp[i]) {
        x = ip6_temp[i] & 0x1;
        if (x) (*prefix_length)++;
        ip6_temp[i] = ip6_temp[i] >> 1;
      }
    }
  }
  return SAI_STATUS_SUCCESS;
}

const sai_attribute_t* get_attr_from_list(_In_ sai_attr_id_t attr_id,
                                          _In_ const sai_attribute_t* attr_list,
                                          _In_ uint32_t attr_count) {
  if (attr_list == NULL || attr_count == 0) {
    return NULL;
  }

  for (unsigned int index = 0; index < attr_count; index++) {
    if (attr_list[index].id == attr_id) {
      return &attr_list[index];
    }
  }

  return NULL;
}

sai_status_t sai_ipv4_to_string(_In_ sai_ip4_t ip4, _In_ uint32_t max_length,
                                _Out_ char* entry_string,
                                _Out_ int* entry_length) {
  inet_ntop(AF_INET, &ip4, entry_string, max_length);
  *entry_length = (int)strlen(entry_string);
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipv6_to_string(_In_ sai_ip6_t ip6, _In_ uint32_t max_length,
                                _Out_ char* entry_string,
                                _Out_ int* entry_length) {
  inet_ntop(AF_INET6, &ip6, entry_string, max_length);
  *entry_length = (int)strnlen(entry_string, max_length);
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipaddress_to_string(_In_ sai_ip_address_t ip_addr,
                                     _In_ uint32_t max_length,
                                     _Out_ char* entry_string,
                                     _Out_ int* entry_length) {
  if (ip_addr.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    sai_ipv4_to_string(ip_addr.addr.ip4, max_length, entry_string,
                       entry_length);
  } else if (ip_addr.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    sai_ipv6_to_string(ip_addr.addr.ip6, max_length, entry_string,
                       entry_length);
  } else {
    snprintf(entry_string, max_length, "Invalid addr family %d",
             ip_addr.addr_family);
    return SAI_STATUS_INVALID_PARAMETER;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ip_prefix_to_switch_ip_addr(
    const _In_ sai_ip_prefix_t* sai_ip_addr, _Out_ switch_ip_addr_t* ip_addr) {
  if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    ip_addr->type = SWITCH_API_IP_ADDR_V4;
    ip_addr->ip.v4addr = ntohl(sai_ip_addr->addr.ip4);
    sai_ipv4_prefix_length(ntohl(sai_ip_addr->mask.ip4), &ip_addr->prefix_len);
  } else if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    ip_addr->type = SWITCH_API_IP_ADDR_V6;
    memcpy((void*)&ip_addr->ip.v6addr, (void*)&sai_ip_addr->addr.ip6, 16);
    sai_ipv6_prefix_length(sai_ip_addr->mask.ip6, &ip_addr->prefix_len);
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_ipprefix_to_string(_In_ sai_ip_prefix_t ip_prefix,
                                    _In_ uint32_t max_length,
                                    _Out_ char* entry_string,
                                    _Out_ int* entry_length) {
  int len = 0;
  uint32_t pos = 0;

  if (ip_prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    sai_ipv4_to_string(ip_prefix.addr.ip4, max_length, entry_string, &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    pos += snprintf(entry_string + pos, max_length - pos, "/");
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    sai_ipv4_to_string(ip_prefix.mask.ip4, max_length - pos, entry_string + pos,
                       &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
  } else if (ip_prefix.addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    sai_ipv6_to_string(ip_prefix.addr.ip6, max_length, entry_string, &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    pos += snprintf(entry_string + pos, max_length - pos, "/");
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
    sai_ipv6_to_string(ip_prefix.mask.ip6, max_length - pos, entry_string + pos,
                       &len);
    pos += len;
    if (pos > max_length) {
      *entry_length = max_length;
      return SAI_STATUS_SUCCESS;
    }
  } else {
    snprintf(entry_string, max_length, "Invalid addr family %d",
             ip_prefix.addr_family);
    return SAI_STATUS_INVALID_PARAMETER;
  }

  *entry_length = pos;
  return SAI_STATUS_SUCCESS;
}
