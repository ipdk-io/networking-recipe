// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_PORT_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_ES2K_PORT_MANAGER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "stratum/hal/lib/tdi/tdi_port_manager.h"

// Suppress clang errors
#undef LOCKS_EXCLUDED
#define LOCKS_EXCLUDED(...)

namespace stratum {
namespace hal {
namespace tdi {

class Es2kPortManager : public TdiPortManager {
 public:
  Es2kPortManager() {}
  virtual ~Es2kPortManager() {}

  // ---------- Common public methods ----------

  ::util::Status GetPortInfo(int device, int port,
                             TargetDatapathId* target_dp_id);
  ::util::StatusOr<PortState> GetPortState(int device, int port);
  ::util::Status GetPortCounters(int device, int port, PortCounters* counters);
  ::util::StatusOr<uint32> GetPortIdFromPortKey(int device,
                                                const PortKey& port_key);
  bool IsValidPort(int device, int port);
  ::util::Status AddPort(int device, int port);
  ::util::Status DeletePort(int device, int port);
  ::util::Status EnablePort(int device, int port);
  ::util::Status DisablePort(int device, int port);

  // ---------- ES2K-specific public methods ----------

  virtual ::util::Status AddPort(int device, int port, uint64 speed_bps,
                                 FecMode fec_mode);
  virtual ::util::Status EnablePortShaping(int device, int port,
                                           TriState enable);
  virtual ::util::Status SetPortAutonegPolicy(int device, int port,
                                              TriState autoneg);
  virtual ::util::Status SetPortMtu(int device, int port, int32 mtu);
  virtual ::util::Status SetPortLoopbackMode(int uint, int port,
                                             LoopbackState loopback_mode);

  // Creates the singleton instance. Expected to be called once to initialize
  // the instance.
  static Es2kPortManager* CreateSingleton() LOCKS_EXCLUDED(init_lock_);

  // The following public functions are specific to this class. They are to be
  // called by SDE callbacks only.

  // Return the singleton instance to be used in the SDE callbacks.
  static Es2kPortManager* GetSingleton() LOCKS_EXCLUDED(init_lock_);

  // Called whenever a port status event is received from SDK. It forwards the
  // port status event to the module who registered a callback by calling
  // RegisterPortStatusEventWriter().
  ::util::Status OnPortStatusEvent(int device, int dev_port, bool up,
                                   absl::Time timestamp)
      LOCKS_EXCLUDED(port_status_event_writer_lock_);

 protected:
  // The singleton instance.
  static Es2kPortManager* singleton_ GUARDED_BY(init_lock_);
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_PORT_MANAGER_H_
