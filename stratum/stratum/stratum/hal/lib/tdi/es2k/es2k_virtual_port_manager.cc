// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ES2K-specific virtual port methods.

#include "stratum/hal/lib/tdi/es2k/es2k_virtual_port_manager.h"

#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/lib/channel/channel.h"

#define VPORT_STATE_TABLE_NAME \
  "openconfig-virtual-ports.virtual-ports.virtual-port.state"
#define VPORT_CONFIG_TABLE_NAME \
  "openconfig-virtual-ports.virtual-ports.virtual-port.config"
//  "openconfig-virtual-ports"

namespace stratum {
namespace hal {
namespace tdi {

Es2kVirtualPortManager* Es2kVirtualPortManager::singleton_ = nullptr;

ABSL_CONST_INIT absl::Mutex Es2kVirtualPortManager::init_lock_(
    absl::kConstInit);

Es2kVirtualPortManager* Es2kVirtualPortManager::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new Es2kVirtualPortManager();
  }

  return singleton_;
}

Es2kVirtualPortManager* Es2kVirtualPortManager::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

void Es2kVirtualPortManager::SetTdiSdeInterface(TdiSdeInterface* tdi_sde_intf) {
  tdi_sde_interface_ = tdi_sde_intf;
}

void Es2kVirtualPortManager::SetTdiFixedFunctionManager(
    TdiFixedFunctionManager* tdi_fixed_func_mgr) {
  tdi_fixed_function_manager_ = tdi_fixed_func_mgr;
}

::util::StatusOr<uint32> Es2kVirtualPortManager::GetVSI(
    uint32 global_resource_id) {
  uint64 data;
  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());
  auto status = tdi_fixed_function_manager_->FetchVportTableData(
      session, VPORT_STATE_TABLE_NAME, global_resource_id, kVsi, &data);
  if (!status.ok()) {
    return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED)
           << "One or more read operations failed.";
  }
  return static_cast<uint32>(data);
}

::util::StatusOr<PortState> Es2kVirtualPortManager::GetPortState(
    uint32 global_resource_id) {
  uint64 data;
  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());
  auto status = tdi_fixed_function_manager_->FetchVportTableData(
      session, VPORT_STATE_TABLE_NAME, global_resource_id, kOperStatus, &data);
  if (!status.ok()) {
    return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED)
           << "One or more read operations failed.";
  }
  return static_cast<PortState>(data);
}

// Stratum's common.proto uses uint64 for MacAddress
::util::StatusOr<uint64> Es2kVirtualPortManager::GetMacAddress(
    uint32 global_resource_id) {
  uint64 data;
  ASSIGN_OR_RETURN(auto session, tdi_sde_interface_->CreateSession());
  auto status = tdi_fixed_function_manager_->FetchVportTableData(
      session, VPORT_STATE_TABLE_NAME, global_resource_id, kMacAddress, &data);
  if (!status.ok()) {
    return MAKE_ERROR(ERR_AT_LEAST_ONE_OPER_FAILED)
           << "One or more read operations failed.";
  }

  // Return mac-address with correct byte order
  uint64 swapped_mac = ((data & 0xFF) << 40) | ((data & 0xFF00) << 24) |
                       ((data & 0xFF0000) << 8) | ((data & 0xFF000000) >> 8) |
                       ((data & 0xFF00000000) >> 24) |
                       ((data & 0xFF0000000000) >> 40);
  return swapped_mac;
}

/* #### C function callback #### */
static void vport_state_notification_callback(uint32_t dev_id,
                                              uint32_t glort_id, uint8_t state,
                                              void* cookie) {
  auto vport_mgr = reinterpret_cast<Es2kVirtualPortManager*>(cookie);
  vport_mgr->SendVportStateNotificationEvent(dev_id, glort_id, state);
}

void Es2kVirtualPortManager::SendVportStateNotificationEvent(uint32_t dev_id,
                                                             uint32_t glort_id,
                                                             uint32_t state) {
  absl::ReaderMutexLock l(&gnmi_event_lock_);
  if (!gnmi_event_writer_) return;
  // Allocate and initialize an VportStateNotificationEvent event and pass it to
  // the gNMI publisher using the gNMI event notification channel.
  // The GnmiEventPtr is a smart pointer (shared_ptr<>) and it takes care of
  // the memory allocated to this event object once the event is handled by
  // the GnmiPublisher.
  if (!gnmi_event_writer_->Write(GnmiEventPtr(
          new VportStateNotificationEvent(dev_id, glort_id, state)))) {
    // Remove WriterInterface if it is no longer operational.
    gnmi_event_writer_.reset();
  }
}

::util::Status Es2kVirtualPortManager::InitializeNotificationCallback() {
  if (notif_initialized_) {
    return ::util::OkStatus();
  }

  auto status = tdi_fixed_function_manager_->InitNotificationTableWithCallback(
      VPORT_CONFIG_TABLE_NAME, &vport_state_notification_callback, this);

  if (!status.ok()) {
    LOG(ERROR)
        << "Failed to register virtual port state change notification callback";
  } else {
    notif_initialized_ = true;
  }
  return status;
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
