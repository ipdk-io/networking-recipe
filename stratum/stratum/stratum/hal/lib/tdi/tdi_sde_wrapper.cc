// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Target-agnostic SDE wrapper methods.

#include "stratum/hal/lib/tdi/tdi_sde_wrapper.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <set>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_sde_helpers.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/lib/channel/channel.h"

DEFINE_string(tdi_sde_config_dir, "/var/run/stratum/tdi_config",
              "The dir used by the SDE to load the device configuration.");

namespace stratum {
namespace hal {
namespace tdi {

using namespace stratum::hal::tdi::helpers;

TdiSdeWrapper::TdiSdeWrapper()
    : tdi_info_(nullptr), port_status_event_writer_(nullptr) {}

// Create and start an new session.
::util::StatusOr<std::shared_ptr<TdiSdeInterface::SessionInterface>>
TdiSdeWrapper::CreateSession() {
  return Session::CreateSession();
}

::util::StatusOr<std::unique_ptr<TdiSdeInterface::TableKeyInterface>>
TdiSdeWrapper::CreateTableKey(uint32 table_id) {
  ::absl::ReaderMutexLock l(&data_lock_);
  return TableKey::CreateTableKey(tdi_info_, table_id);
}

::util::StatusOr<std::unique_ptr<TdiSdeInterface::TableDataInterface>>
TdiSdeWrapper::CreateTableData(uint32 table_id, uint32 action_id) {
  ::absl::ReaderMutexLock l(&data_lock_);
  return TableData::CreateTableData(tdi_info_, table_id, action_id);
}

::util::StatusOr<uint32> TdiSdeWrapper::GetTdiRtId(uint32 p4info_id) const {
  ::absl::ReaderMutexLock l(&data_lock_);
  return tdi_id_mapper_->GetTdiRtId(p4info_id);
}

::util::StatusOr<uint32> TdiSdeWrapper::GetP4InfoId(uint32 tdi_id) const {
  ::absl::ReaderMutexLock l(&data_lock_);
  return tdi_id_mapper_->GetP4InfoId(tdi_id);
}

::util::StatusOr<uint32> TdiSdeWrapper::GetActionSelectorTdiRtId(
    uint32 action_profile_id) const {
  ::absl::ReaderMutexLock l(&data_lock_);
  return tdi_id_mapper_->GetActionSelectorTdiRtId(action_profile_id);
}

::util::StatusOr<uint32> TdiSdeWrapper::GetActionProfileTdiRtId(
    uint32 action_selector_id) const {
  ::absl::ReaderMutexLock l(&data_lock_);
  return tdi_id_mapper_->GetActionProfileTdiRtId(action_selector_id);
}

::util::StatusOr<uint32> TdiSdeWrapper::GetTableId(
    std::string& table_name) const {
  const ::tdi::Table* table;
  if (nullptr != tdi_info_) {
    RETURN_IF_TDI_ERROR(tdi_info_->tableFromNameGet(table_name, &table));
    return (table->tableInfoGet()->idGet());
  }

  return MAKE_ERROR(ERR_INTERNAL) << "Error retrieving information from TDI";
}

//------------------------------------------------------------------------------
// Packet i/o
// Return ERR_OPER_NOT_SUPPORTED?
//------------------------------------------------------------------------------
::util::Status TdiSdeWrapper::TxPacket(int device, const std::string& buffer) {
  return ::util::OkStatus();
}

::util::Status TdiSdeWrapper::StartPacketIo(int device) {
  return ::util::OkStatus();
}

::util::Status TdiSdeWrapper::StopPacketIo(int device) {
  return ::util::OkStatus();
}

::util::Status TdiSdeWrapper::RegisterPacketReceiveWriter(
    int device, std::unique_ptr<ChannelWriter<std::string>> writer) {
  return ::util::OkStatus();
}

::util::Status TdiSdeWrapper::UnregisterPacketReceiveWriter(int device) {
  return ::util::OkStatus();
}

//------------------------------------------------------------------------------
// IPsec Notification
//------------------------------------------------------------------------------
::util::Status TdiSdeWrapper::InitNotificationTableWithCallback(
    int dev_id, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    const std::string& table_name, ipsec_notification_table_callback_t callback,
    void* cookie) const LOCKS_EXCLUDED(data_lock_) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "Notification Table not supported";
}

::util::Status TdiSdeWrapper::SetPacketIoConfig(
    const PacketIoConfig& pktio_config) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED) << "PacketIO not supported";
}

::util::Status TdiSdeWrapper::InitNotificationTableWithCallback(
    int dev_id, std::shared_ptr<TdiSdeInterface::SessionInterface> session,
    const std::string& table_name, vport_notification_table_callback_t callback,
    void* cookie) const LOCKS_EXCLUDED(data_lock_) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "Notification Table not supported";
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
