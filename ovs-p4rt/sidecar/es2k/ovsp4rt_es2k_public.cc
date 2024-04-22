// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// ES2K-specific functions declared in ovs-p4rt.h.
//

#include "absl/flags/flag.h"
#include "openvswitch/ovs-p4rt.h"
#include "ovsp4rt_credentials.h"
#include "ovsp4rt_es2k_private.h"
#include "ovsp4rt_logging.h"
#include "ovsp4rt_private.h"
#include "ovsp4rt_session.h"
#include "ovsp4rt_utils.h"
#include "p4_name_mapping.h"

namespace {

const std::string TableErrorMessage(bool inserting, const char* table) {
  if (inserting) {
    return absl::StrCat("Error adding entry to ", table);
  } else {
    return absl::StrCat("Error deleting entry from ", table);
  }
}

const std::string FormatMac(const uint8_t* mac_addr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%x:%x:%x:%x:%x:%x", mac_addr[0], mac_addr[1],
           mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  return buf;
}

}  // namespace

//----------------------------------------------------------------------
// Functions with C interfaces
//----------------------------------------------------------------------

void ConfigFdbTableEntry(struct mac_learning_info learn_info, bool insert_entry,
                         const char* p4rt_grpc_addr) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = ovs_p4rt::OvsP4rtSession::Create(
      p4rt_grpc_addr, GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id), absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<ovs_p4rt::OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status =
      ovs_p4rt::GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  /* Hack: When we delete an FDB entry based on current logic  we will not know
   * we will not know if its an Tunnel learn FDB or regular VSI learn FDB.
   * This hack, during delete case check if entry is present in l2_to_tunnel_v4
   * and l2_to_tunnel_v6. if any of these 2 tables is true then go ahead and
   * delete the entry.
   */

  if (!insert_entry) {
    auto status_or_read_response =
        GetL2ToTunnelV4TableEntry(session.get(), learn_info, p4info);
    if (status_or_read_response.ok()) {
      learn_info.is_tunnel = true;
    }

    /* If learn_info.is_tunnel is not true, then we need to check for v6 table
     * entry as the entry can be either in V4 or V6 tunnel table.
     */
    if (!learn_info.is_tunnel) {
      status_or_read_response =
          GetL2ToTunnelV6TableEntry(session.get(), learn_info, p4info);
      if (status_or_read_response.ok()) {
        learn_info.is_tunnel = true;
      }
    }
  }

  if (learn_info.is_tunnel) {
    if (insert_entry) {
      auto status_or_read_response =
          GetFdbTunnelTableEntry(session.get(), learn_info, p4info);
      if (status_or_read_response.ok()) {
        LOG(ERROR) << "Error adding to FDB Tunnel Table: entry already exists";
        return;
      }
    }

    status = ConfigFdbTunnelTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "FDB Tunnel Table");
      // TODO(derek): Most of the error cases don't return. Should they?
    }

    status = ConfigL2TunnelTableEntry(session.get(), learn_info, p4info,
                                      insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "L2 Tunnel Table");
    }

    status = ConfigFdbSmacTableEntry(session.get(), learn_info, p4info,
                                     insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "FDB Source MAC Table");
    }
  } else {
    if (insert_entry) {
      auto status_or_read_response =
          GetFdbVlanTableEntry(session.get(), learn_info, p4info);
      if (status_or_read_response.ok()) {
        LOG(WARNING) << "Error adding to FDB Vlan Table: entry already exists";
        return;
      }

      status = ConfigFdbRxVlanTableEntry(session.get(), learn_info, p4info,
                                         insert_entry);
      if (!status.ok()) {
        LOG(ERROR) << TableErrorMessage(insert_entry, "FDB Rx Vlan Table");
      }

      status_or_read_response =
          GetTxAccVsiTableEntry(session.get(), learn_info.src_port, p4info);
      if (!status_or_read_response.ok()) {
        return;
      }

      ::p4::v1::ReadResponse read_response =
          std::move(status_or_read_response).value();
      std::vector<::p4::v1::TableEntry> table_entries;

      table_entries.reserve(read_response.entities().size());

      int param_id =
          GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                     ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

      uint32_t host_sp = 0;
      for (const auto& entity : read_response.entities()) {
        p4::v1::TableEntry table_entry_1 = entity.table_entry();
        auto* table_action = table_entry_1.mutable_action();
        auto* action = table_action->mutable_action();
        for (const auto& param : action->params()) {
          if (param_id == param.param_id()) {
            const std::string& s1 = param.value();
            std::string s2 = s1;
            for (int param_bytes = 0; param_bytes < 4; param_bytes++) {
              host_sp = host_sp << 8 | int(s2[param_bytes]);
            }
            break;
          }
        }
      }

      learn_info.src_port = host_sp;
    }

    status = ConfigFdbTxVlanTableEntry(session.get(), learn_info, p4info,
                                       insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "FDB Tx Vlan Table");
    }

    status = ConfigFdbSmacTableEntry(session.get(), learn_info, p4info,
                                     insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "FDB Source MAC Table")
                 << " for " << FormatMac(learn_info.mac_addr);
    }
  }
  if (!status.ok()) return;
  return;
}

void ConfigRxTunnelSrcTableEntry(struct tunnel_info tunnel_info,
                                 bool insert_entry,
                                 const char* p4rt_grpc_addr) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = ovs_p4rt::OvsP4rtSession::Create(
      p4rt_grpc_addr, GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id), absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  status = ConfigRxTunnelSrcPortTableEntry(session.get(), tunnel_info, p4info,
                                           insert_entry);
  if (!status.ok()) return;

  return;
}

void ConfigTunnelSrcPortTableEntry(struct src_port_info tnl_sp,
                                   bool insert_entry,
                                   const char* p4rt_grpc_addr) {
  using namespace ovs_p4rt;

  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  // Start a new client session.
  auto status_or_session = OvsP4rtSession::Create(
      p4rt_grpc_addr, GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id), absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  if (insert_entry) {
    table_entry =
        ovs_p4rt::SetupTableEntryToInsert(session.get(), &write_request);
  } else {
    table_entry =
        ovs_p4rt::SetupTableEntryToDelete(session.get(), &write_request);
  }

  PrepareSrcPortTableEntry(table_entry, tnl_sp, p4info, insert_entry);

  status = ovs_p4rt::SendWriteRequest(session.get(), write_request);

  // TODO: handle error scenarios. For now return irrespective of the status.
  if (!status.ok()) return;
}

void ConfigSrcPortTableEntry(struct src_port_info vsi_sp, bool insert_entry,
                             const char* p4rt_grpc_addr) {
  using namespace ovs_p4rt;

  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  // Start a new client session.
  auto status_or_session = OvsP4rtSession::Create(
      p4rt_grpc_addr, GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id), absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  auto status_or_read_response =
      GetTxAccVsiTableEntry(session.get(), vsi_sp.src_port, p4info);
  if (!status_or_read_response.ok()) return;

  ::p4::v1::ReadResponse read_response =
      std::move(status_or_read_response).value();
  std::vector<::p4::v1::TableEntry> table_entries;

  table_entries.reserve(read_response.entities().size());

  int param_id =
      GetParamId(p4info, TX_ACC_VSI_TABLE_ACTION_L2_FWD_AND_BYPASS_BRIDGE,
                 ACTION_L2_FWD_AND_BYPASS_BRIDGE_PARAM_PORT);

  uint32_t host_sp = 0;
  for (const auto& entity : read_response.entities()) {
    p4::v1::TableEntry table_entry_1 = entity.table_entry();
    auto* table_action = table_entry_1.mutable_action();
    auto* action = table_action->mutable_action();
    for (const auto& param : action->params()) {
      if (param_id == param.param_id()) {
        const std::string& s1 = param.value();
        std::string s2 = s1;
        for (int param_bytes = 0; param_bytes < 4; param_bytes++) {
          host_sp = host_sp << 8 | int(s2[param_bytes]);
        }
        break;
      }
    }
  }

  vsi_sp.src_port = host_sp;

  status = ConfigureVsiSrcPortTableEntry(session.get(), vsi_sp, p4info,
                                         insert_entry);
  if (!status.ok()) return;

  return;
}

void ConfigVlanTableEntry(uint16_t vlan_id, bool insert_entry,
                          const char* p4rt_grpc_addr) {
  using namespace ovs_p4rt;

  p4::v1::WriteRequest write_request;
  ::p4::v1::TableEntry* table_entry;

  // Start a new client session.
  auto status_or_session = OvsP4rtSession::Create(
      p4rt_grpc_addr, GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id), absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status = GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  status =
      ConfigVlanPushTableEntry(session.get(), vlan_id, p4info, insert_entry);
  if (!status.ok()) return;

  status =
      ConfigVlanPopTableEntry(session.get(), vlan_id, p4info, insert_entry);
  if (!status.ok()) return;

  return;
}

void ConfigIpMacMapTableEntry(struct ip_mac_map_info ip_info, bool insert_entry,
                              const char* p4rt_grpc_addr) {
  using namespace ovs_p4rt;

  // Start a new client session.
  auto status_or_session = ovs_p4rt::OvsP4rtSession::Create(
      p4rt_grpc_addr, GenerateClientCredentials(),
      absl::GetFlag(FLAGS_device_id), absl::GetFlag(FLAGS_role_name));
  if (!status_or_session.ok()) return;

  // Unwrap the session from the StatusOr object.
  std::unique_ptr<ovs_p4rt::OvsP4rtSession> session =
      std::move(status_or_session).value();
  ::p4::config::v1::P4Info p4info;
  ::absl::Status status =
      ovs_p4rt::GetForwardingPipelineConfig(session.get(), &p4info);
  if (!status.ok()) return;

  if (insert_entry) {
    auto status_or_read_response =
        GetVmSrcTableEntry(session.get(), ip_info, p4info);
    if (status_or_read_response.ok()) {
      goto try_dstip;
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    status = ConfigSrcIpMacMapTableEntry(session.get(), ip_info, p4info,
                                         insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "SRC_IP_MAC_MAP_TABLE");
    }
  }

try_dstip:
  if (insert_entry) {
    auto status_or_read_response =
        GetVmDstTableEntry(session.get(), ip_info, p4info);
    if (status_or_read_response.ok()) {
      return;
    }
  }

  if (ValidIpAddr(ip_info.src_ip_addr.ip.v4addr.s_addr)) {
    status = ConfigDstIpMacMapTableEntry(session.get(), ip_info, p4info,
                                         insert_entry);
    if (!status.ok()) {
      LOG(ERROR) << TableErrorMessage(insert_entry, "DST_IP_MAC_MAP_TABLE");
    }
  }

  return;
}
