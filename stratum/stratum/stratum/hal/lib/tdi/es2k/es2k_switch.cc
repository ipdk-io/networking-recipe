// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/es2k/es2k_switch.h"

#include <algorithm>
#include <map>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "stratum/glue/gtl/map_util.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/hal/lib/tdi/es2k/es2k_chassis_manager.h"
#include "stratum/hal/lib/tdi/es2k/es2k_node.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/hal/lib/tdi/utils.h"
#include "stratum/lib/constants.h"
#include "stratum/lib/macros.h"

namespace stratum {
namespace hal {
namespace tdi {

Es2kSwitch::Es2kSwitch(Es2kChassisManager* chassis_manager,
                       TdiIpsecManager* ipsec_manager,
                       Es2kVirtualPortManager* vport_manager,
                       const std::map<int, Es2kNode*>& device_id_to_es2k_node)
    : chassis_manager_(ABSL_DIE_IF_NULL(chassis_manager)),
      ipsec_manager_(ABSL_DIE_IF_NULL(ipsec_manager)),
      vport_manager_(ABSL_DIE_IF_NULL(vport_manager)),
      device_id_to_es2k_node_(device_id_to_es2k_node),
      node_id_to_tdi_node_() {
  for (const auto& entry : device_id_to_es2k_node_) {
    CHECK_GE(entry.first, 0)
        << "Invalid device_id number " << entry.first << ".";
    CHECK_NE(entry.second, nullptr)
        << "Detected null Es2kNode for device_id " << entry.first << ".";
  }
}

Es2kSwitch::~Es2kSwitch() {}

::util::Status Es2kSwitch::PushChassisConfig(const ChassisConfig& config) {
  absl::WriterMutexLock l(&chassis_lock);
  RETURN_IF_ERROR(chassis_manager_->PushChassisConfig(config));
  ASSIGN_OR_RETURN(const auto& node_id_to_device_id,
                   chassis_manager_->GetNodeIdToDeviceMap());
  node_id_to_tdi_node_.clear();
  for (const auto& entry : node_id_to_device_id) {
    uint64 node_id = entry.first;
    int device_id = entry.second;
    ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromDeviceId(device_id));
    RETURN_IF_ERROR(es2k_node->PushChassisConfig(config, node_id));
    node_id_to_tdi_node_[node_id] = es2k_node;
  }

  LOG(INFO) << "Chassis config pushed successfully.";

  return ::util::OkStatus();
}

::util::Status Es2kSwitch::VerifyChassisConfig(const ChassisConfig& config) {
  (void)config;
  return ::util::OkStatus();
}

::util::Status Es2kSwitch::PushForwardingPipelineConfig(
    uint64 node_id, const ::p4::v1::ForwardingPipelineConfig& config) {
  absl::WriterMutexLock l(&chassis_lock);
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  RETURN_IF_ERROR(es2k_node->PushForwardingPipelineConfig(config));
  RETURN_IF_ERROR(chassis_manager_->ReplayChassisConfig(node_id));

  LOG(INFO) << "P4-based forwarding pipeline config pushed successfully to "
            << "node with ID " << node_id << ".";

  return ::util::OkStatus();
}

::util::Status Es2kSwitch::SaveForwardingPipelineConfig(
    uint64 node_id, const ::p4::v1::ForwardingPipelineConfig& config) {
  absl::WriterMutexLock l(&chassis_lock);
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  RETURN_IF_ERROR(es2k_node->SaveForwardingPipelineConfig(config));
  RETURN_IF_ERROR(chassis_manager_->ReplayChassisConfig(node_id));

  LOG(INFO) << "P4-based forwarding pipeline config saved successfully to "
            << "node with ID " << node_id << ".";

  return ::util::OkStatus();
}

::util::Status Es2kSwitch::CommitForwardingPipelineConfig(uint64 node_id) {
  absl::WriterMutexLock l(&chassis_lock);
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  RETURN_IF_ERROR(es2k_node->CommitForwardingPipelineConfig());

  LOG(INFO) << "P4-based forwarding pipeline config committed successfully to "
            << "node with ID " << node_id << ".";

  return ::util::OkStatus();
}

::util::Status Es2kSwitch::VerifyForwardingPipelineConfig(
    uint64 node_id, const ::p4::v1::ForwardingPipelineConfig& config) {
  absl::WriterMutexLock l(&chassis_lock);
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  return es2k_node->VerifyForwardingPipelineConfig(config);
}

::util::Status Es2kSwitch::Shutdown() {
  ::util::Status status = ::util::OkStatus();
  for (const auto& entry : device_id_to_es2k_node_) {
    Es2kNode* node = entry.second;
    APPEND_STATUS_IF_ERROR(status, node->Shutdown());
  }
  APPEND_STATUS_IF_ERROR(status, chassis_manager_->Shutdown());

  return status;
}

::util::Status Es2kSwitch::Freeze() { return ::util::OkStatus(); }

::util::Status Es2kSwitch::Unfreeze() { return ::util::OkStatus(); }

::util::Status Es2kSwitch::WriteForwardingEntries(
    const ::p4::v1::WriteRequest& req, std::vector<::util::Status>* results) {
  if (!req.updates_size()) return ::util::OkStatus();  // nothing to do.
  RET_CHECK(req.device_id()) << "No device_id in WriteRequest.";
  RET_CHECK(results != nullptr)
      << "Need to provide non-null results pointer for non-empty updates.";

  absl::ReaderMutexLock l(&chassis_lock);
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(req.device_id()));
  return es2k_node->WriteForwardingEntries(req, results);
}

::util::Status Es2kSwitch::ReadForwardingEntries(
    const ::p4::v1::ReadRequest& req,
    WriterInterface<::p4::v1::ReadResponse>* writer,
    std::vector<::util::Status>* details) {
  RET_CHECK(req.device_id()) << "No device_id in ReadRequest.";
  RET_CHECK(writer) << "Channel writer must be non-null.";
  RET_CHECK(details) << "Details pointer must be non-null.";

  absl::ReaderMutexLock l(&chassis_lock);
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(req.device_id()));
  return es2k_node->ReadForwardingEntries(req, writer, details);
}

::util::Status Es2kSwitch::RegisterStreamMessageResponseWriter(
    uint64 node_id,
    std::shared_ptr<WriterInterface<::p4::v1::StreamMessageResponse>> writer) {
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  return es2k_node->RegisterStreamMessageResponseWriter(writer);
}

::util::Status Es2kSwitch::UnregisterStreamMessageResponseWriter(
    uint64 node_id) {
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  return es2k_node->UnregisterStreamMessageResponseWriter();
}

::util::Status Es2kSwitch::HandleStreamMessageRequest(
    uint64 node_id, const ::p4::v1::StreamMessageRequest& request) {
  ASSIGN_OR_RETURN(auto* es2k_node, GetEs2kNodeFromNodeId(node_id));
  return es2k_node->HandleStreamMessageRequest(request);
}

::util::Status Es2kSwitch::RegisterEventNotifyWriter(
    std::shared_ptr<WriterInterface<GnmiEventPtr>> writer) {
  auto status = chassis_manager_->RegisterEventNotifyWriter(writer);
  if (!status.ok()) return status;
  status = ipsec_manager_->RegisterEventNotifyWriter(writer);
  if (!status.ok()) return status;
  return vport_manager_->RegisterEventNotifyWriter(writer);
  // TODO(dgf): Unregister other managers event writer if one
  //            registration fails?
  //   auto rc2 = ipsec_manager_->RegisterEventNotifyWriter(writer);
  //   if (!rc2.ok()) chassis_manager_->UnregisterEventNotifyWriter();
  //   return rc2;
}

::util::Status Es2kSwitch::UnregisterEventNotifyWriter() {
  return chassis_manager_->UnregisterEventNotifyWriter();
  // TODO(dgf): Unregister ipsec_manager event writer as well?
  //            Unregister both and then return status?
  //   auto rc1 = chassis_manager_->UnregisterEventNotifyWriter();
  //   auto rc2 = ipsec_manager_->UnregisterEventNotifyWriter();
  //   return !rc1.ok() ? rc1 : rc2;
}

::util::Status Es2kSwitch::RetrieveValue(uint64 node_id,
                                         const DataRequest& request,
                                         WriterInterface<DataResponse>* writer,
                                         std::vector<::util::Status>* details) {
  absl::ReaderMutexLock l(&chassis_lock);
  for (const auto& req : request.requests()) {
    DataResponse resp;
    ::util::Status status = ::util::OkStatus();
    switch (req.request_case()) {
      // Port data request
      case DataRequest::Request::kOperStatus:
      case DataRequest::Request::kAdminStatus:
      case DataRequest::Request::kMacAddress:
      case DataRequest::Request::kPortSpeed:
      case DataRequest::Request::kNegotiatedPortSpeed:
      case DataRequest::Request::kLacpRouterMac:
      case DataRequest::Request::kPortCounters:
      case DataRequest::Request::kForwardingViability:
      case DataRequest::Request::kHealthIndicator:
      case DataRequest::Request::kAutonegStatus:
      case DataRequest::Request::kFrontPanelPortInfo:
      case DataRequest::Request::kLoopbackStatus:
      case DataRequest::Request::kSdnPortId: {
        auto port_data = chassis_manager_->GetPortData(req);
        if (!port_data.ok()) {
          status.Update(port_data.status());
        } else {
          resp = port_data.ConsumeValueOrDie();
        }
        break;
      }
      // Node information request
      case DataRequest::Request::kNodeInfo: {
        auto device_id =
            chassis_manager_->GetDeviceFromNodeId(req.node_info().node_id());
        if (!device_id.ok()) {
          status.Update(device_id.status());
        } else {
          auto* node_info = resp.mutable_node_info();
        }
        break;
      }
      // IPsecOffload request
      case DataRequest::Request::kIpsecOffloadInfo: {
        uint32 fetched_spi = 0;
        auto fetch_status = ipsec_manager_->GetSpiData(fetched_spi);
        if (!fetch_status.ok()) {
          status.Update(fetch_status);
        } else {
          auto* info = resp.mutable_ipsec_offload_info();
          info->set_spi(fetched_spi);
        }
        break;
      }
      // VirtualPort data request
      case DataRequest::Request::kVportVsi:
      case DataRequest::Request::kVportOperStatus:
      case DataRequest::Request::kVportMacAddress: {
        auto vport_data = chassis_manager_->GetVirtualPortData(req);
        if (!vport_data.ok()) {
          status.Update(vport_data.status());
        } else {
          resp = vport_data.ConsumeValueOrDie();
        }
        break;
      }
      default:
        status =
            MAKE_ERROR(ERR_UNIMPLEMENTED)
            << "DataRequest field "
            << req.descriptor()->FindFieldByNumber(req.request_case())->name()
            << " is not supported yet!";
        break;
    }
    if (status.ok()) {
      // If everything is OK send it to the caller.
      writer->Write(resp);
    }
    if (details) details->push_back(status);
  }
  return ::util::OkStatus();
}

::util::Status Es2kSwitch::SetValue(uint64 node_id, const SetRequest& request,
                                    std::vector<::util::Status>* details) {
  for (const auto& req : request.requests()) {
    ::util::Status status = ::util::OkStatus();
    switch (req.request_case()) {
      case SetRequest::Request::RequestCase::kIpsecOffloadConfig: {
        absl::WriterMutexLock l(&chassis_lock);
        auto op_type = req.ipsec_offload_config().ipsec_sadb_config_op();
        auto payload = const_cast<IPsecSADBConfig&>(
            req.ipsec_offload_config().ipsec_sadb_config_info());
        status.Update(ipsec_manager_->WriteConfigSADBEntry(op_type, payload));
        break;
      }
      case SetRequest::Request::RequestCase::kNode: {
        absl::WriterMutexLock l(&chassis_lock);
        auto vport_notif_enable = req.node().enable_vport_status_notif();
        if (vport_notif_enable) {
          status.Update(vport_manager_->InitializeNotificationCallback());
        }
        break;
      }
      default:
        status = MAKE_ERROR(ERR_INTERNAL)
                 << req.ShortDebugString() << " Not supported yet!";
    }
    if (details) details->push_back(status);
  }

  return ::util::OkStatus();
}

::util::StatusOr<std::vector<std::string>> Es2kSwitch::VerifyState() {
  return std::vector<std::string>();
}

std::unique_ptr<Es2kSwitch> Es2kSwitch::CreateInstance(
    Es2kChassisManager* chassis_manager, TdiIpsecManager* ipsec_manager,
    Es2kVirtualPortManager* vport_manager,
    const std::map<int, Es2kNode*>& device_id_to_es2k_node) {
  return absl::WrapUnique(new Es2kSwitch(
      chassis_manager, ipsec_manager, vport_manager, device_id_to_es2k_node));
}

::util::StatusOr<Es2kNode*> Es2kSwitch::GetEs2kNodeFromDeviceId(
    int device_id) const {
  Es2kNode* es2k_node = gtl::FindPtrOrNull(device_id_to_es2k_node_, device_id);
  if (es2k_node == nullptr) {
    return MAKE_ERROR(ERR_INVALID_PARAM)
           << "Device " << device_id << " is unknown.";
  }
  return es2k_node;
}

::util::StatusOr<Es2kNode*> Es2kSwitch::GetEs2kNodeFromNodeId(
    uint64 node_id) const {
  Es2kNode* es2k_node = gtl::FindPtrOrNull(node_id_to_tdi_node_, node_id);
  if (es2k_node == nullptr) {
    return MAKE_ERROR(ERR_INVALID_PARAM)
           << "Node with ID " << node_id
           << " is unknown or no config has been pushed to it yet.";
  }
  return es2k_node;
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
