// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_PORT_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_TDI_PORT_MANAGER_H_

#include <memory>

#include "absl/time/time.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/lib/channel/channel.h"

namespace stratum {
namespace hal {
namespace tdi {

// TdiPortManager is a proxy class for per target port management
class TdiPortManager {
 public:
  // PortStatusEvent encapsulates the information received on a port status
  // event. Port refers to the SDE internal device port ID.
  struct PortStatusEvent {
    int device;
    int port;
    PortState state;
    absl::Time time_last_changed;
  };

  TdiPortManager();
  virtual ~TdiPortManager() {}

  // Registers a writer through which to send any port status events. The
  // message contains a tuple (device, port, state), where port refers to the
  // SDE internal device port ID. There can only be one writer.
  virtual ::util::Status RegisterPortStatusEventWriter(
      std::unique_ptr<ChannelWriter<PortStatusEvent>> writer);

  // Unregisters the port status writer.
  virtual ::util::Status UnregisterPortStatusEventWriter();

  // Gets Port Info
  virtual ::util::Status GetPortInfo(int device, int port,
                                     TargetDatapathId* target_dp_id) = 0;

  // Gets the operational state of a port.
  virtual ::util::StatusOr<PortState> GetPortState(int device, int port) = 0;

  // Gets the port counters of a port.
  virtual ::util::Status GetPortCounters(int device, int port,
                                         PortCounters* counters) = 0;

  // Returns the SDE device port ID for the given PortKey.
  virtual ::util::StatusOr<uint32> GetPortIdFromPortKey(
      int device, const PortKey& port_key) = 0;

  // Checks if a port is valid.
  virtual bool IsValidPort(int device, int port) = 0;

  // Adds a new port.
  virtual ::util::Status AddPort(int device, int port) = 0;

  // Deletes a port.
  virtual ::util::Status DeletePort(int device, int port) = 0;

  // Enables a port.
  virtual ::util::Status EnablePort(int device, int port) = 0;

  // Disables a port.
  virtual ::util::Status DisablePort(int device, int port) = 0;

 protected:
  // RW mutex lock for protecting the singleton instance initialization and
  // reading it back from other threads. Unlike other singleton classes, we
  // use RW lock as we need the pointer to class to be returned.
  static absl::Mutex init_lock_;

  // Timeout for Write() operations on port status events.
  static constexpr absl::Duration kWriteTimeout = absl::InfiniteDuration();

  // RW Mutex to protect the port status writer.
  mutable absl::Mutex port_status_event_writer_lock_;

  // Writer to forward the port status change message to. It is registered
  // by chassis manager to receive port status change events.
  std::unique_ptr<ChannelWriter<PortStatusEvent>> port_status_event_writer_
      GUARDED_BY(port_status_event_writer_lock_);
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_PORT_MANAGER_H_
