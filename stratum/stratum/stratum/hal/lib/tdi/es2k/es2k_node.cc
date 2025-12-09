// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/es2k/es2k_node.h"

#include <unistd.h>

#include <memory>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "gflags/gflags.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/hal/lib/common/proto_oneof_writer_wrapper.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_node.h"
#include "stratum/hal/lib/tdi/tdi_pipeline_utils.h"
#include "stratum/hal/lib/tdi/tdi_sde_interface.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/utils.h"
#include "stratum/public/proto/error.pb.h"

DEFINE_bool(enable_sticky_tdi_session, false,
            "Use persistent TDI session to write forwarding entries");

namespace stratum {
namespace hal {
namespace tdi {

// Initialize base class TdiNode members as well
Es2kNode::Es2kNode(TdiTableManager* tdi_table_manager,
                   TdiActionProfileManager* tdi_action_profile_manager,
                   TdiPacketioManager* tdi_packetio_manager,
                   TdiPreManager* tdi_pre_manager,
                   TdiCounterManager* tdi_counter_manager,
                   TdiSdeInterface* tdi_sde_interface, int device_id,
                   bool initialized, uint64 node_id)
    : TdiNode(tdi_table_manager, tdi_action_profile_manager,
              tdi_packetio_manager, tdi_pre_manager, tdi_counter_manager,
              tdi_sde_interface, device_id, initialized, node_id),
      pipeline_initialized_(false),
      initialized_(initialized),
      tdi_config_(),
      tdi_sde_interface_(ABSL_DIE_IF_NULL(tdi_sde_interface)),
      tdi_table_manager_(ABSL_DIE_IF_NULL(tdi_table_manager)),
      tdi_action_profile_manager_(ABSL_DIE_IF_NULL(tdi_action_profile_manager)),
      tdi_packetio_manager_(tdi_packetio_manager),
      tdi_pre_manager_(ABSL_DIE_IF_NULL(tdi_pre_manager)),
      tdi_counter_manager_(ABSL_DIE_IF_NULL(tdi_counter_manager)),
      node_id_(node_id),
      device_id_(device_id) {}

Es2kNode::Es2kNode()
    : pipeline_initialized_(false),
      initialized_(false),
      tdi_config_(),
      tdi_sde_interface_(nullptr),
      tdi_table_manager_(nullptr),
      tdi_action_profile_manager_(nullptr),
      tdi_packetio_manager_(nullptr),
      tdi_pre_manager_(nullptr),
      tdi_counter_manager_(nullptr),
      node_id_(0),
      device_id_(-1) {}

Es2kNode::~Es2kNode() = default;

// Factory function for creating the instance of the class.
std::unique_ptr<Es2kNode> Es2kNode::CreateInstance(
    TdiTableManager* tdi_table_manager,
    TdiActionProfileManager* tdi_action_profile_manager,
    TdiPacketioManager* tdi_packetio_manager, TdiPreManager* tdi_pre_manager,
    TdiCounterManager* tdi_counter_manager, TdiSdeInterface* tdi_sde_interface,
    int device_id, bool initialized, uint64 node_id) {
  return absl::WrapUnique(
      new Es2kNode(tdi_table_manager, tdi_action_profile_manager,
                   tdi_packetio_manager, tdi_pre_manager, tdi_counter_manager,
                   tdi_sde_interface, device_id, initialized, node_id));
}

::util::Status Es2kNode::WriteForwardingEntries(
    const ::p4::v1::WriteRequest& req, std::vector<::util::Status>* results) {
  absl::WriterMutexLock l(&lock_);
  RET_CHECK(req.device_id() == TdiNode::getNodeId())
      << "Request device id must be same as id of this Es2kNode.";
  RET_CHECK(req.atomicity() == ::p4::v1::WriteRequest::CONTINUE_ON_ERROR)
      << "Request atomicity "
      << ::p4::v1::WriteRequest::Atomicity_Name(req.atomicity())
      << " is not supported.";
  if (!TdiNode::getInitialized() || !TdiNode::getPipelineInitialized()) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }

  std::shared_ptr<TdiSdeInterface::SessionInterface> session;
  if (!FLAGS_enable_sticky_tdi_session) {
    // Use transient TDI session.
    ASSIGN_OR_RETURN(session, tdi_sde_interface_->CreateSession());
  } else if (forwarding_session_) {
    // Use persistent TDI session.
    session = forwarding_session_;
  } else {
    // Create persistent TDI session.
    ASSIGN_OR_RETURN(session, tdi_sde_interface_->CreateSession());
    forwarding_session_ = session;
  }

  bool success = true;
  RETURN_IF_ERROR(session->BeginBatch());
  for (const auto& update : req.updates()) {
    ::util::Status status = ::util::OkStatus();
    switch (update.entity().entity_case()) {
      case ::p4::v1::Entity::kTableEntry:
        status = tdi_table_manager_->WriteTableEntry(
            session, update.type(), update.entity().table_entry());
        break;
      case ::p4::v1::Entity::kExternEntry:
        status = WriteExternEntry(session, update.type(),
                                  update.entity().extern_entry());
        break;
      case ::p4::v1::Entity::kActionProfileMember:
        status = tdi_action_profile_manager_->WriteActionProfileMember(
            session, update.type(), update.entity().action_profile_member());
        break;
      case ::p4::v1::Entity::kActionProfileGroup:
        status = tdi_action_profile_manager_->WriteActionProfileGroup(
            session, update.type(), update.entity().action_profile_group());
        break;
      case ::p4::v1::Entity::kPacketReplicationEngineEntry:
        status = tdi_pre_manager_->WritePreEntry(
            session, update.type(),
            update.entity().packet_replication_engine_entry());
        break;
      case ::p4::v1::Entity::kDirectCounterEntry:
        status = tdi_table_manager_->WriteDirectCounterEntry(
            session, update.type(), update.entity().direct_counter_entry());
        break;
      case ::p4::v1::Entity::kCounterEntry:
        status = tdi_counter_manager_->WriteIndirectCounterEntry(
            session, update.type(), update.entity().counter_entry());
        break;
      case ::p4::v1::Entity::kRegisterEntry: {
        status = tdi_table_manager_->WriteRegisterEntry(
            session, update.type(), update.entity().register_entry());
        break;
      }
      case ::p4::v1::Entity::kMeterEntry: {
        status = tdi_table_manager_->WriteMeterEntry(
            session, update.type(), update.entity().meter_entry());
        break;
      }
      case ::p4::v1::Entity::kDirectMeterEntry:
        status = tdi_table_manager_->WriteDirectMeterEntry(
            session, update.type(), update.entity().direct_meter_entry());
        break;
      case ::p4::v1::Entity::kValueSetEntry:
      case ::p4::v1::Entity::kDigestEntry:
      default:
        status = MAKE_ERROR(ERR_UNIMPLEMENTED)
                 << "Unsupported entity type: " << update.ShortDebugString();
        break;
    }
    success &= status.ok();
    results->push_back(status);
  }
  RETURN_IF_ERROR(session->EndBatch());

  if (!success) {
    // Tally up the number of ALREADY_EXISTS errors.
    int already_exists_errors = 0;
    for (const ::util::Status& status : *results) {
      if (status.error_code() == ::util::error::Code::ALREADY_EXISTS) {
        ++already_exists_errors;
      } else if (!status.ok()) {
        already_exists_errors = 0;
        break;
      }
    }
    if (already_exists_errors) {
      // If all the errors are ALREADY_EXISTS, downgrade severity to INFO
      // and set the description to something less likely to alarm the
      // customer.
      const char* entries = (already_exists_errors == 1) ? "entry" : "entries";
      return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED)
                 .severity(INFO)
                 .without_logging()
             << "Duplicate table " << entries << " (may not be an error)";

    } else {
      return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED).without_logging()
             << "One or more write operations failed.";
    }
  }

  LOG(INFO) << "P4-based forwarding entities written successfully to node with "
            << "ID " << TdiNode::getNodeId() << ".";
  return ::util::OkStatus();
}

::util::Status Es2kNode::ReadForwardingEntries(
    const ::p4::v1::ReadRequest& req,
    WriterInterface<::p4::v1::ReadResponse>* writer,
    std::vector<::util::Status>* details) {
  RET_CHECK(writer) << "Channel writer must be non-null.";
  RET_CHECK(details) << "Details pointer must be non-null.";

  absl::ReaderMutexLock l(&lock_);
  RET_CHECK(req.device_id() == TdiNode::getNodeId())
      << "Request device id must be same as id of this Es2kNode.";
  if (!TdiNode::getInitialized() || !TdiNode::getPipelineInitialized()) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  ::p4::v1::ReadResponse resp;
  bool success = true;
  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());
  for (const auto& entity : req.entities()) {
    switch (entity.entity_case()) {
      case ::p4::v1::Entity::kTableEntry: {
        auto status = tdi_table_manager_->ReadTableEntry(
            session, entity.table_entry(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kExternEntry: {
        auto status = ReadExternEntry(session, entity.extern_entry(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kActionProfileMember: {
        auto status = tdi_action_profile_manager_->ReadActionProfileMember(
            session, entity.action_profile_member(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kActionProfileGroup: {
        auto status = tdi_action_profile_manager_->ReadActionProfileGroup(
            session, entity.action_profile_group(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kPacketReplicationEngineEntry: {
        auto status = tdi_pre_manager_->ReadPreEntry(
            session, entity.packet_replication_engine_entry(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kDirectCounterEntry: {
        auto status = tdi_table_manager_->ReadDirectCounterEntry(
            session, entity.direct_counter_entry());
        if (!status.ok()) {
          success = false;
          details->push_back(status.status());
          break;
        }
        success &= status.ok();
        details->push_back(status.status());
        resp.add_entities()->mutable_direct_counter_entry()->CopyFrom(
            status.ValueOrDie());
        break;
      }
      case ::p4::v1::Entity::kCounterEntry: {
        auto status = tdi_counter_manager_->ReadIndirectCounterEntry(
            session, entity.counter_entry(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kRegisterEntry: {
        auto status = tdi_table_manager_->ReadRegisterEntry(
            session, entity.register_entry(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kMeterEntry: {
        auto status = tdi_table_manager_->ReadMeterEntry(
            session, entity.meter_entry(), writer);
        success &= status.ok();
        details->push_back(status);
        break;
      }
      case ::p4::v1::Entity::kDirectMeterEntry: {
        auto status = tdi_table_manager_->ReadDirectMeterEntry(
            session, entity.direct_meter_entry());
        if (!status.ok()) {
          success = false;
          details->push_back(status.status());
          break;
        }
        success &= status.ok();
        details->push_back(status.status());
        resp.add_entities()->mutable_direct_meter_entry()->CopyFrom(
            status.ValueOrDie());
        break;
      }
      case ::p4::v1::Entity::kValueSetEntry:
      case ::p4::v1::Entity::kDigestEntry:
      default: {
        success = false;
        details->push_back(MAKE_ERROR(ERR_UNIMPLEMENTED)
                           << "Unsupported entity type: "
                           << entity.ShortDebugString());
        break;
      }
    }
  }
  RET_CHECK(writer->Write(resp)) << "Write to stream channel failed.";
  if (!success) {
    return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED).without_logging()
           << "One or more read operations failed.";
  }
  return ::util::OkStatus();
}

::util::Status Es2kNode::WriteExternEntry(
    std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    const ::p4::v1::Update::Type type, const ::p4::v1::ExternEntry& entry) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "Unsupported extern entry: " << entry.ShortDebugString() << ".";
}

::util::Status Es2kNode::ReadExternEntry(
    std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    const ::p4::v1::ExternEntry& entry,
    WriterInterface<::p4::v1::ReadResponse>* writer) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "Unsupported extern entry: " << entry.ShortDebugString() << ".";
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
