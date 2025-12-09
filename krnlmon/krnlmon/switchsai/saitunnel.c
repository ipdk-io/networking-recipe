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

#include "saitunnel.h"

#include "saiinternal.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_status.h"
#include "switchapi/switch_tunnel.h"

char* sai_status_to_string(_In_ const sai_status_t status) {
  switch (status) {
    case SAI_STATUS_INVALID_PARAMETER:
      return "invalid parameter";
    case SAI_STATUS_NO_MEMORY:
      return "no memory";
    case SAI_STATUS_FAILURE:
    default:
      return "unknown failure";
  }
}

sai_status_t sai_ip_addr_to_switch_ip_addr(
    const _In_ sai_ip_address_t* sai_ip_addr, _Out_ switch_ip_addr_t* ip_addr) {
  if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
    ip_addr->type = SWITCH_API_IP_ADDR_V4;
    ip_addr->ip.v4addr = ntohl(sai_ip_addr->addr.ip4);
    ip_addr->prefix_len = 32;
  } else if (sai_ip_addr->addr_family == SAI_IP_ADDR_FAMILY_IPV6) {
    ip_addr->type = SWITCH_API_IP_ADDR_V6;
    memcpy(&ip_addr->ip.v6addr.u.addr8, &sai_ip_addr->addr.ip6,
           sizeof(sai_ip6_t));
    ip_addr->prefix_len = 128;
  }
  return SAI_STATUS_SUCCESS;
}

sai_status_t sai_switch_status_to_sai_status(
    _In_ const switch_status_t status) {
  switch (status) {
    case SWITCH_STATUS_SUCCESS:
      return SAI_STATUS_SUCCESS;
    case SWITCH_STATUS_FAILURE:
      return SWITCH_STATUS_FAILURE;
    case SWITCH_STATUS_INVALID_PARAMETER:
      return SAI_STATUS_INVALID_PARAMETER;
    case SWITCH_STATUS_NO_MEMORY:
      return SAI_STATUS_NO_MEMORY;
    default:
      return SAI_STATUS_FAILURE;
  }
}

static sai_status_t sai_tunnel_to_switch_tunnel_type(
    sai_tunnel_type_t sai_tunnel, switch_tunnel_type_t* switch_tunnel) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  switch (sai_tunnel) {
    case SAI_TUNNEL_TYPE_IPINIP:
      *switch_tunnel = SWITCH_TUNNEL_TYPE_IPIP;
      break;
    case SAI_TUNNEL_TYPE_IPINIP_GRE:
      *switch_tunnel = SWITCH_TUNNEL_TYPE_GRE;
      break;
    case SAI_TUNNEL_TYPE_VXLAN:
      *switch_tunnel = SWITCH_TUNNEL_TYPE_VXLAN;
      break;
    case SAI_TUNNEL_TYPE_MPLS:
    case SAI_TUNNEL_TYPE_SRV6:
    case SAI_TUNNEL_TYPE_NVGRE:
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
  }
  return status;
}

static sai_status_t sai_tunnel_attribute_parse(
    uint32_t attr_count, const sai_attribute_t* attr_list,
    switch_api_tunnel_info_t* tunnel_info) {
  const sai_attribute_t* attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t index = 0;
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_TUNNEL_ATTR_TYPE:
        status = sai_tunnel_to_switch_tunnel_type(attribute->value.s32,
                                                  &tunnel_info->tunnel_type);
        SAI_ASSERT(status == SAI_STATUS_SUCCESS);
        break;
      case SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE:
        tunnel_info->underlay_rif_handle = attribute->value.oid;
        break;
      case SAI_TUNNEL_ATTR_OVERLAY_INTERFACE:
        tunnel_info->overlay_rif_handle = attribute->value.oid;
        break;
      case SAI_TUNNEL_ATTR_ENCAP_SRC_IP:
        sai_ip_addr_to_switch_ip_addr(&attribute->value.ipaddr,
                                      &tunnel_info->src_ip);
        if (attribute->value.ipaddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
          tunnel_info->ip_type = SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4;
        } else {
          tunnel_info->ip_type = SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6;
        }
        break;
      case SAI_TUNNEL_ATTR_ENCAP_DST_IP:
        sai_ip_addr_to_switch_ip_addr(&attribute->value.ipaddr,
                                      &tunnel_info->dst_ip);
        if (attribute->value.ipaddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4) {
          tunnel_info->ip_type = SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4;
        } else {
          tunnel_info->ip_type = SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6;
        }
        break;

      case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT:
        tunnel_info->udp_port = attribute->value.u16;
        break;

      case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY:
        tunnel_info->gre_key = attribute->value.u32;
        break;
      case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY_VALID:
        break;
      case SAI_TUNNEL_ATTR_ENCAP_MAPPERS:
        SAI_ASSERT(attribute->value.objlist.count == 1);
        tunnel_info->encap_mapper_handle = attribute->value.objlist.list[0];
        break;
      case SAI_TUNNEL_ATTR_DECAP_MAPPERS:
        SAI_ASSERT(attribute->value.objlist.count == 1);
        tunnel_info->decap_mapper_handle = attribute->value.objlist.list[0];
        break;
      case SAI_TUNNEL_ATTR_DECAP_TTL_MODE:
      case SAI_TUNNEL_ATTR_DECAP_DSCP_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_TTL_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_TTL_VAL:
      case SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL:
      case SAI_TUNNEL_ATTR_DECAP_ECN_MODE:
      case SAI_TUNNEL_ATTR_ENCAP_ECN_MODE:
      default:
        break;
    }
  }
  return status;
}

/*
 * Routine Description:
 *    Create tunnel
 *
 * Arguments:
 *    [out] tunnel_id - Tunnel id
 *    [in] switch_id - Switch Id
 *    [in] attr_count - Number of attributes
 *    [in] attr_list - Array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 */
static sai_status_t sai_create_tunnel(_Out_ sai_object_id_t* tunnel_id,
                                      _In_ sai_object_id_t switch_id,
                                      _In_ uint32_t attr_count,
                                      _In_ const sai_attribute_t* attr_list) {
  switch_api_tunnel_info_t tunnel_info = {0};
  switch_handle_t tunnel_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;

  status = sai_tunnel_attribute_parse(attr_count, attr_list, &tunnel_info);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to parse atributes when creating tunnel, error: %s \n",
        sai_status_to_string(status));
    return status;
  }

  switch_status =
      switch_api_tunnel_create(switch_id, &tunnel_info, &tunnel_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to create tunnel, error: %s \n",
                      sai_status_to_string(status));
    return status;
  }

  *tunnel_id = tunnel_handle;
  krnlmon_log_debug(
      "tunnel created for dest port: %d and handle"
      "is: 0x%lx",
      tunnel_info.udp_port, tunnel_handle);

  return status;
}

/*
 * Routine Description:
 *    Remove tunnel
 *
 * Arguments:
 *    [in] tunnel_handle - tunnel handle
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 */
static sai_status_t sai_remove_tunnel(_In_ sai_object_id_t tunnel_handle) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_tunnel_delete(tunnel_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to delete tunnel, error: %s\n",
                      sai_status_to_string(status));
    return status;
  }

  krnlmon_log_debug("Tunnel entry delete success, handle 0x%lx", tunnel_handle);

  return status;
}

static char* sai_tunnel_string(sai_tunnel_type_t sai_tunnel_type) {
  switch (sai_tunnel_type) {
    case SAI_TUNNEL_TYPE_IPINIP:
      return "IPinIP";
    case SAI_TUNNEL_TYPE_IPINIP_GRE:
      return "GRE";
    case SAI_TUNNEL_TYPE_VXLAN:
      return "Vxlan";
    case SAI_TUNNEL_TYPE_MPLS:
      return "Mpls";
    case SAI_TUNNEL_TYPE_SRV6:
    case SAI_TUNNEL_TYPE_NVGRE:
    default:
      return "Unsupported";
  }
}

static sai_status_t sai_tunnel_term_to_switch_type(
    sai_tunnel_term_table_entry_type_t sai_type,
    switch_tunnel_term_entry_type_t* switch_type) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (sai_type) {
    case SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P:
      *switch_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P;
      break;
    case SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP:
      *switch_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2MP;
      break;
    case SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_MP2P:
    case SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_MP2MP:
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
  }
  return status;
}

static char* switch_tunnel_term_type_to_string(
    switch_tunnel_term_entry_type_t switch_type) {
  switch (switch_type) {
    case SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P:
      return "term_type_p2p";
    case SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2MP:
      return "term_type_p2mp";
    default:
      return "unsupported";
  }
}

static sai_status_t sai_tunnel_term_attribute_parse(
    uint32_t attr_count, const sai_attribute_t* attr_list,
    switch_api_tunnel_term_info_t* tunnel_term_info) {
  switch_tunnel_term_entry_type_t switch_tunnel_term_type = 0;
  switch_tunnel_type_t switch_tunnel_type = 0;
  sai_status_t status = SAI_STATUS_SUCCESS;
  const sai_attribute_t* attr = NULL;
  uint32_t index = 0;

  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID:
        tunnel_term_info->vrf_handle = (switch_handle_t)attr->value.oid;
        krnlmon_log_debug("VRF handle 0x%lx", attr->value.oid);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP:
        sai_ip_addr_to_switch_ip_addr(&attr->value.ipaddr,
                                      &tunnel_term_info->dst_ip);
        krnlmon_log_debug("Tunnel Dip: 0x%x", attr->value.ipaddr.addr.ip4);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP:
        sai_ip_addr_to_switch_ip_addr(&attr->value.ipaddr,
                                      &tunnel_term_info->src_ip);
        krnlmon_log_debug("Tunnel Sip: 0x%x", attr->value.ipaddr.addr.ip4);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID:
        tunnel_term_info->tunnel_id = (switch_handle_t)attr->value.u32;
        krnlmon_log_debug("Tunnel ID %d", attr->value.u32);
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE:
        status = sai_tunnel_term_to_switch_type(attr->value.s32,
                                                &switch_tunnel_term_type);
        if (status == SAI_STATUS_SUCCESS) {
          tunnel_term_info->term_entry_type = switch_tunnel_term_type;
          krnlmon_log_debug("Tunnel type %s",
                            sai_tunnel_string(attr->value.s32));
        }
        break;
      case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE:
        status = sai_tunnel_to_switch_tunnel_type(attr->value.s32,
                                                  &switch_tunnel_type);
        if (status == SAI_STATUS_SUCCESS) {
          tunnel_term_info->tunnel_type = switch_tunnel_type;
          krnlmon_log_debug("Tunnel term type %s",
                            switch_tunnel_term_type_to_string(attr->value.s32));
        }
        break;
      default:
        break;
    }
  }
  return status;
}

/*
 * Routine Description:
 *    Create tunnel termination table entry
 *
 * Arguments:
 *    [out] tunnel_term_table_entry_id - Tunnel termination table entry id
 *    [in] switch_id - Switch Id
 *    [in] attr_count - Number of attributes
 *    [in] attr_list - Array of attributes
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 */
static sai_status_t sai_create_tunnel_term_table_entry(
    _Out_ sai_object_id_t* tunnel_term_id, _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count, _In_ const sai_attribute_t* attr_list) {
  switch_handle_t tunnel_term_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_api_tunnel_term_info_t api_term_info = {0};
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_MEMSET(&api_term_info, 0, sizeof(switch_api_tunnel_term_info_t));

  status =
      sai_tunnel_term_attribute_parse(attr_count, attr_list, &api_term_info);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to parse atributes when creating tunnel term "
        "table entry, error: %s \n",
        sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_tunnel_term_create(switch_id, &api_term_info,
                                                &tunnel_term_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error(
        "Failed to create tunnel term table entry, "
        "error: %s",
        sai_status_to_string(status));
    return status;
  }

  *tunnel_term_id = tunnel_term_handle;
  krnlmon_log_debug("Tunnel term table entry add, handle 0x%lx",
                    tunnel_term_handle);
  return status;
}

/*
 * Routine Description:
 *    Remove tunnel termination table entry
 *
 * Arguments:
 *    [in] tunnel_term_handle - Tunnel termination handle
 *
 * Return Values:
 *    SAI_STATUS_SUCCESS on success
 *    Failure status code on error
 *
 */
static sai_status_t sai_remove_tunnel_term_table_entry(
    _In_ sai_object_id_t tunnel_term_handle) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_status = switch_api_tunnel_term_delete(tunnel_term_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    krnlmon_log_error("Failed to delete tunnel term table entry, error: %s",
                      sai_status_to_string(status));
    return status;
  }

  krnlmon_log_debug("Tunnel term table entry delete success, handle 0x%lx",
                    tunnel_term_handle);
  return status;
}

/*
 *  TUNNEL method table retrieved with sai_api_query()
 */
sai_tunnel_api_t tunnel_api = {
    .create_tunnel = sai_create_tunnel,
    .remove_tunnel = sai_remove_tunnel,
    .create_tunnel_term_table_entry = sai_create_tunnel_term_table_entry,
    .remove_tunnel_term_table_entry = sai_remove_tunnel_term_table_entry};

sai_status_t sai_tunnel_initialize(sai_api_service_t* sai_api_service) {
  sai_api_service->tunnel_api = tunnel_api;
  return SAI_STATUS_SUCCESS;
}
