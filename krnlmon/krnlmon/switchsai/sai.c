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

#include "krnlmon_options.h"
#include "saiinternal.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_handle.h"
#include "switchapi/switch_nhop.h"

static sai_api_service_t sai_api_service;

static const char* module[SAI_API_MAX] = {"SAI_API_UNSPECIFIED",
                                          "SAI_API_SWITCH",
                                          "SAI_API_PORT",
                                          "SAI_API_FDB",
                                          "SAI_API_VLAN",
                                          "SAI_API_VIRTUAL_ROUTER",
                                          "SAI_API_ROUTE",
                                          "SAI_API_NEXT_HOP",
                                          "SAI_API_NEXT_HOP_GROUP",
                                          "SAI_API_ROUTER_INTERFACE",
                                          "SAI_API_NEIGHBOR",
                                          "SAI_API_ACL",
                                          "SAI_API_HOSTIF",
                                          "SAI_API_MIRROR",
                                          "SAI_API_SAMPLEPACKET",
                                          "SAI_API_STP",
                                          "SAI_API_LAG",
                                          "SAI_API_POLICER",
                                          "SAI_API_WRED",
                                          "SAI_API_QOS_MAP",
                                          "SAI_API_QUEUE",
                                          "SAI_API_SCHEDULER",
                                          "SAI_API_SCHEDULER_GROUP",
                                          "SAI_API_BUFFER",
                                          "SAI_API_HASH",
                                          "SAI_API_UDF",
                                          "SAI_API_TUNNEL",
                                          "SAI_API_L2MC",
                                          "SAI_API_IPMC",
                                          "SAI_API_RPF_GROUP",
                                          "SAI_API_L2MC_GROUP",
                                          "SAI_API_IPMC_GROUP",
                                          "SAI_API_MCAST_FDB",
                                          "SAI_API_BRIDGE",
                                          "SAI_API_TAM",
                                          "SAI_API_SEGMENTROUTE",
                                          "SAI_API_MPLS",
                                          "SAI_API_DTEL",
                                          "SAI_API_BFD",
                                          "SAI_API_ISOLATION_GROUP"};

sai_status_t sai_api_query(_In_ sai_api_t sai_api_id,
                           _Out_ void** api_method_table) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!api_method_table) {
    status = SAI_STATUS_INVALID_PARAMETER;
    krnlmon_log_error("null api method table: invalid parameter");
    return status;
  }

  switch (sai_api_id) {
    case SAI_API_SWITCH:
      *api_method_table = &sai_api_service.switch_api;
      break;

    case SAI_API_PORT:
      *api_method_table = &sai_api_service.port_api;
      break;

    case SAI_API_BRIDGE:
      *api_method_table = &sai_api_service.bridge_api;
      break;

    case SAI_API_FDB:
      *api_method_table = &sai_api_service.fdb_api;
      break;

    case SAI_API_VLAN:
      *api_method_table = &sai_api_service.vlan_api;
      break;

    case SAI_API_VIRTUAL_ROUTER:
      *api_method_table = &sai_api_service.vr_api;
      break;

    case SAI_API_ROUTER_INTERFACE:
      *api_method_table = &sai_api_service.rif_api;
      break;

    case SAI_API_ROUTE:
      *api_method_table = &sai_api_service.route_api;
      break;

    case SAI_API_NEIGHBOR:
      *api_method_table = &sai_api_service.neighbor_api;
      break;

    case SAI_API_NEXT_HOP:
      *api_method_table = &sai_api_service.nhop_api;
      break;

    case SAI_API_NEXT_HOP_GROUP:
      *api_method_table = &sai_api_service.nhop_group_api;
      break;

    case SAI_API_QOS_MAP:
      *api_method_table = &sai_api_service.qos_api;
      break;

    case SAI_API_ACL:
      *api_method_table = &sai_api_service.acl_api;
      break;

    case SAI_API_LAG:
      *api_method_table = &sai_api_service.lag_api;
      break;

    case SAI_API_STP:
      *api_method_table = &sai_api_service.stp_api;
      break;

    case SAI_API_HOSTIF:
      *api_method_table = &sai_api_service.hostif_api;
      break;

    case SAI_API_MIRROR:
      *api_method_table = &sai_api_service.mirror_api;
      break;

    case SAI_API_SAMPLEPACKET:
      *api_method_table = &sai_api_service.samplepacket_api;
      break;

    case SAI_API_HASH:
      *api_method_table = &sai_api_service.hash_api;
      break;

    case SAI_API_UDF:
      *api_method_table = &sai_api_service.udf_api;
      break;

    case SAI_API_IPMC:
      *api_method_table = &sai_api_service.ipmc_api;
      break;

    case SAI_API_L2MC:
      *api_method_table = &sai_api_service.l2mc_api;
      break;

    case SAI_API_POLICER:
      *api_method_table = &sai_api_service.policer_api;
      break;

    case SAI_API_QUEUE:
      *api_method_table = &sai_api_service.queue_api;
      break;

    case SAI_API_BUFFER:
      *api_method_table = &sai_api_service.buffer_api;
      break;

    case SAI_API_SCHEDULER:
      *api_method_table = &sai_api_service.scheduler_api;
      break;

    case SAI_API_DTEL:
      *api_method_table = &sai_api_service.dtel_api;
      break;

    case SAI_API_SCHEDULER_GROUP:
      *api_method_table = &sai_api_service.scheduler_group_api;
      break;

    case SAI_API_WRED:
      *api_method_table = &sai_api_service.wred_api;
      break;

    case SAI_API_TUNNEL:
      *api_method_table = &sai_api_service.tunnel_api;
      break;

    case SAI_API_UNSPECIFIED:
      krnlmon_log_error(" SAI API unspecified");
      break;

    case SAI_API_RPF_GROUP:
    case SAI_API_L2MC_GROUP:
    case SAI_API_IPMC_GROUP:
    case SAI_API_MCAST_FDB:
    case SAI_API_TAM:
    case SAI_API_SRV6:
    case SAI_API_MPLS:
    case SAI_API_BFD:
    case SAI_API_ISOLATION_GROUP:
    case SAI_API_NAT:
    case SAI_API_COUNTER:
    case SAI_API_DEBUG_COUNTER:
    case SAI_API_MACSEC:
    case SAI_API_SYSTEM_PORT:
    case SAI_API_MY_MAC:
    case SAI_API_MAX:
    default:
      *api_method_table = NULL;
      status = SAI_STATUS_INVALID_PARAMETER;
  }

  if (status == SAI_STATUS_SUCCESS) {
    krnlmon_log_debug("api query for module: %s", module[sai_api_id]);
  } else if (sai_api_id >= SAI_API_MAX) {
    krnlmon_log_error("api query failed, invalid api id: %d\n", sai_api_id);
  } else {
    krnlmon_log_error("api query failed, api %s not implemented\n",
                      module[sai_api_id]);
  }

  return status;
}

/*
 * Routine Description:
 *     Query sai object type.
 *
 * Arguments:
 *     [in] sai_object_id_t
 *
 * Return Values:
 *    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
 *    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
 */
sai_object_type_t sai_object_type_query(_In_ sai_object_id_t sai_object_id) {
  sai_object_type_t object_type = SAI_OBJECT_TYPE_NULL;
  switch_nhop_id_type_t nhop_type = 0;
  switch_handle_type_t handle_type = SWITCH_HANDLE_TYPE_NONE;

  handle_type = switch_handle_type_get(sai_object_id);
  switch (handle_type) {
    case SWITCH_HANDLE_TYPE_PORT:
      object_type = SAI_OBJECT_TYPE_PORT;
      break;
    case SWITCH_HANDLE_TYPE_LAG:
      object_type = SAI_OBJECT_TYPE_LAG;
      break;
    case SWITCH_HANDLE_TYPE_LAG_MEMBER:
      object_type = SAI_OBJECT_TYPE_LAG_MEMBER;
      break;
    case SWITCH_HANDLE_TYPE_RIF:
      object_type = SAI_OBJECT_TYPE_ROUTER_INTERFACE;
      break;
    case SWITCH_HANDLE_TYPE_INTERFACE:
      object_type = SAI_OBJECT_TYPE_BRIDGE_PORT;
      break;
    case SWITCH_HANDLE_TYPE_VRF:
      object_type = SAI_OBJECT_TYPE_VIRTUAL_ROUTER;
      break;
    case SWITCH_HANDLE_TYPE_NHOP:
      switch_api_nhop_id_type_get(0, sai_object_id, &nhop_type);
      if (nhop_type == SWITCH_NHOP_ID_TYPE_ONE_PATH) {
        object_type = SAI_OBJECT_TYPE_NEXT_HOP;
      } else if (nhop_type == SWITCH_NHOP_ID_TYPE_ECMP) {
        object_type = SAI_OBJECT_TYPE_NEXT_HOP_GROUP;
      } else {
        object_type = SAI_OBJECT_TYPE_NULL;
      }
      break;
    case SWITCH_HANDLE_TYPE_STP:
      object_type = SAI_OBJECT_TYPE_STP;
      break;
    case SWITCH_HANDLE_TYPE_ACL:
      object_type = SAI_OBJECT_TYPE_ACL_TABLE;
      break;
    case SWITCH_HANDLE_TYPE_ACE:
      object_type = SAI_OBJECT_TYPE_ACL_ENTRY;
      break;
    case SWITCH_HANDLE_TYPE_RANGE:
      object_type = SAI_OBJECT_TYPE_ACL_RANGE;
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF:
      object_type = SAI_OBJECT_TYPE_HOSTIF;
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF_GROUP:
      object_type = SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP;
      break;
    case SWITCH_HANDLE_TYPE_MIRROR:
      object_type = SAI_OBJECT_TYPE_MIRROR_SESSION;
      break;
    case SWITCH_HANDLE_TYPE_MGID:
      object_type = SAI_OBJECT_TYPE_NEXT_HOP_GROUP;
      break;
    case SWITCH_HANDLE_TYPE_ACL_COUNTER:
      object_type = SAI_OBJECT_TYPE_ACL_COUNTER;
      break;
    case SWITCH_HANDLE_TYPE_METER:
      object_type = SAI_OBJECT_TYPE_POLICER;
      break;
    case SWITCH_HANDLE_TYPE_SCHEDULER:
      object_type = SAI_OBJECT_TYPE_SCHEDULER;
      break;
    case SWITCH_HANDLE_TYPE_QUEUE:
      object_type = SAI_OBJECT_TYPE_QUEUE;
      break;
    case SWITCH_HANDLE_TYPE_BUFFER_POOL:
      object_type = SAI_OBJECT_TYPE_BUFFER_POOL;
      break;
    case SWITCH_HANDLE_TYPE_BUFFER_PROFILE:
      object_type = SAI_OBJECT_TYPE_BUFFER_PROFILE;
      break;
    case SWITCH_HANDLE_TYPE_QOS_MAP:
      object_type = SAI_OBJECT_TYPE_QOS_MAP;
      break;
    case SWITCH_HANDLE_TYPE_PRIORITY_GROUP:
      object_type = SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP;
      break;
    case SWITCH_HANDLE_TYPE_NHOP_MEMBER:
      object_type = SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER;
      break;
    case SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE:
      object_type = SAI_OBJECT_TYPE_HOSTIF_TRAP;
      break;
    case SWITCH_HANDLE_TYPE_STP_PORT:
      object_type = SAI_OBJECT_TYPE_STP_PORT;
      break;
    case SWITCH_HANDLE_TYPE_DEVICE:
      object_type = SAI_OBJECT_TYPE_SWITCH;
      break;
    case SWITCH_HANDLE_TYPE_LOGICAL_NETWORK:
      object_type = SAI_OBJECT_TYPE_BRIDGE;
      break;
    case SWITCH_HANDLE_TYPE_VLAN:
      object_type = SAI_OBJECT_TYPE_VLAN;
      break;
    case SWITCH_HANDLE_TYPE_BD_MEMBER:
      object_type = SAI_OBJECT_TYPE_VLAN_MEMBER;
      break;
    case SWITCH_HANDLE_TYPE_SCHEDULER_GROUP:
      object_type = SAI_OBJECT_TYPE_SCHEDULER_GROUP;
      break;
    case SWITCH_HANDLE_TYPE_ACL_GROUP:
      object_type = SAI_OBJECT_TYPE_ACL_TABLE_GROUP;
      break;
    case SWITCH_HANDLE_TYPE_ACL_GROUP_MEMBER:
      object_type = SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER;
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER:
      object_type = SAI_OBJECT_TYPE_TUNNEL_MAP;
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL:
      object_type = SAI_OBJECT_TYPE_TUNNEL;
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_TERM:
      object_type = SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY;
      break;
    case SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY:
      object_type = SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY;
      break;
    case SWITCH_HANDLE_TYPE_DTEL:
      object_type = SAI_OBJECT_TYPE_DTEL;
      break;
    case SWITCH_HANDLE_TYPE_DTEL_QUEUE_ALERT:
      object_type = SAI_OBJECT_TYPE_DTEL_QUEUE_REPORT;
      break;
    case SWITCH_HANDLE_TYPE_DTEL_INT_SESSION:
      object_type = SAI_OBJECT_TYPE_DTEL_INT_SESSION;
      break;
    case SWITCH_HANDLE_TYPE_DTEL_REPORT_SESSION:
      object_type = SAI_OBJECT_TYPE_DTEL_REPORT_SESSION;
      break;
    case SWITCH_HANDLE_TYPE_DTEL_EVENT:
      object_type = SAI_OBJECT_TYPE_DTEL_EVENT;
      break;
    case SWITCH_HANDLE_TYPE_HASH:
      object_type = SAI_OBJECT_TYPE_HASH;
      break;
    case SWITCH_HANDLE_TYPE_NHOP_GROUP:
      object_type = SAI_OBJECT_TYPE_NEXT_HOP_GROUP;
      break;
    case SWITCH_HANDLE_TYPE_BD:
    case SWITCH_HANDLE_TYPE_NEIGHBOR:
    case SWITCH_HANDLE_TYPE_RMAC:
    case SWITCH_HANDLE_TYPE_MGID_ECMP:
    case SWITCH_HANDLE_TYPE_URPF:
    case SWITCH_HANDLE_TYPE_SFLOW:
    case SWITCH_HANDLE_TYPE_SFLOW_ACE:
    case SWITCH_HANDLE_TYPE_LABEL:
    case SWITCH_HANDLE_TYPE_BFD:
    case SWITCH_HANDLE_TYPE_WRED:
    case SWITCH_HANDLE_TYPE_RPF_GROUP:
    case SWITCH_HANDLE_TYPE_MAC:
    case SWITCH_HANDLE_TYPE_ROUTE:
    case SWITCH_HANDLE_TYPE_MTU:
    case SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER:
    case SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER:
    case SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER:
    case SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER:
    case SWITCH_HANDLE_TYPE_RACL_COUNTER:
    case SWITCH_HANDLE_TYPE_EGRESS_ACL_COUNTER:
    case SWITCH_HANDLE_TYPE_WRED_COUNTER:
    case SWITCH_HANDLE_TYPE_WRED_PROFILE:
    case SWITCH_HANDLE_TYPE_TUNNEL_ENCAP:
    case SWITCH_HANDLE_TYPE_MPLS:
    case SWITCH_HANDLE_TYPE_MPLS_LABEL_STACK:
    case SWITCH_HANDLE_TYPE_SR_SIDLIST:
    case SWITCH_HANDLE_TYPE_EGRESS_METER:
    case SWITCH_HANDLE_TYPE_METER_COLOR_ACTION:
    case SWITCH_HANDLE_TYPE_L2_FWD_RX:
    case SWITCH_HANDLE_TYPE_L2_FWD_TX:
    case SWITCH_HANDLE_TYPE_MAX:
    case SWITCH_HANDLE_TYPE_NONE:
    default:
      object_type = SAI_OBJECT_TYPE_NULL;
      break;
  }

  return object_type;
}

sai_status_t sai_initialize(void) {
  // Init Switch API
  switch_api_init(0);

  sai_fdb_initialize(&sai_api_service);
  sai_router_interface_initialize(&sai_api_service);
  sai_next_hop_initialize(&sai_api_service);
  sai_next_hop_group_initialize(&sai_api_service);
  sai_route_initialize(&sai_api_service);
  sai_virtual_router_initialize(&sai_api_service);
  sai_neighbor_initialize(&sai_api_service);
  sai_tunnel_initialize(&sai_api_service);
#ifdef LAG_OPTION
  sai_lag_initialize(&sai_api_service);
#endif

  return SAI_STATUS_SUCCESS;
}
