// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_VIRTUAL_PORT_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_ES2K_VIRTUAL_PORT_MANAGER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "stratum/glue/integral_types.h"
#include "stratum/hal/lib/common/gnmi_events.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/hal/lib/tdi/tdi_fixed_function_manager.h"
#include "stratum/hal/lib/tdi/tdi_sde_interface.h"

// Suppress clang errors
#undef LOCKS_EXCLUDED
#define LOCKS_EXCLUDED(...)

namespace stratum {
namespace hal {
namespace tdi {

class Es2kVirtualPortManager {
 public:
  Es2kVirtualPortManager()
      : notif_initialized_(false), gnmi_event_writer_(nullptr) {}
  virtual ~Es2kVirtualPortManager() {}

  void SetTdiSdeInterface(TdiSdeInterface* tdi_sde_intf);
  void SetTdiFixedFunctionManager(TdiFixedFunctionManager* tdi_fixed_func_mgr);
  void SendVportStateNotificationEvent(uint32 dev_id, uint32 glort_id,
                                       uint32 state);

  ::util::StatusOr<uint32> GetVSI(uint32 global_resource_id);
  ::util::StatusOr<PortState> GetPortState(uint32 global_resource_id);
  // Stratum's common.proto uses uint64 for MacAddress
  ::util::StatusOr<uint64> GetMacAddress(uint32 global_resource_id);

  // Creates the singleton instance. Expected to be called once to initialize
  // the instance.
  static Es2kVirtualPortManager* CreateSingleton() LOCKS_EXCLUDED(init_lock_);

  // The following public functions are specific to this class. They are to be
  // called by SDE callbacks only.

  // Return the singleton instance to be used in the SDE callbacks.
  static Es2kVirtualPortManager* GetSingleton() LOCKS_EXCLUDED(init_lock_);

  virtual ::util::Status RegisterEventNotifyWriter(
      const std::shared_ptr<WriterInterface<GnmiEventPtr>>& writer) {
    absl::WriterMutexLock l(&gnmi_event_lock_);
    gnmi_event_writer_ = writer;
    return ::util::OkStatus();
  }

  virtual ::util::Status UnregisterEventNotifyWriter() {
    absl::WriterMutexLock l(&gnmi_event_lock_);
    gnmi_event_writer_ = nullptr;
    return ::util::OkStatus();
  }
  ::util::Status InitializeNotificationCallback();

 protected:
  // RW mutex lock for protecting the singleton instance initialization and
  // reading it back from other threads. Unlike other singleton classes, we
  // use RW lock as we need the pointer to class to be returned.
  static absl::Mutex init_lock_;

  // The singleton instance.
  static Es2kVirtualPortManager* singleton_ GUARDED_BY(init_lock_);

 private:
  // Pointer to TdiSdeInterface implementation.
  TdiSdeInterface* tdi_sde_interface_;  // not owned by this class.

  // Pointer to FixedFunctionManager. (not owned by this class)
  TdiFixedFunctionManager* tdi_fixed_function_manager_;
  bool notif_initialized_;
  // WriterInterface<GnmiEventPtr> object for sending event notifications.
  mutable absl::Mutex gnmi_event_lock_;
  std::shared_ptr<WriterInterface<GnmiEventPtr>> gnmi_event_writer_
      GUARDED_BY(gnmi_event_lock_);
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_VIRTUAL_PORT_MANAGER_H_
