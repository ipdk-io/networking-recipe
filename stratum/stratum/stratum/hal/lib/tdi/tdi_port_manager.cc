// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/tdi_port_manager.h"

#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"
#include "stratum/lib/channel/channel.h"

namespace stratum {
namespace hal {
namespace tdi {

constexpr absl::Duration TdiPortManager::kWriteTimeout;

ABSL_CONST_INIT absl::Mutex TdiPortManager::init_lock_(absl::kConstInit);

TdiPortManager::TdiPortManager() : port_status_event_writer_(nullptr) {}

::util::Status TdiPortManager::RegisterPortStatusEventWriter(
    std::unique_ptr<ChannelWriter<PortStatusEvent>> writer) {
  absl::WriterMutexLock l(&port_status_event_writer_lock_);
  port_status_event_writer_ = std::move(writer);
  return ::util::OkStatus();
}

::util::Status TdiPortManager::UnregisterPortStatusEventWriter() {
  absl::WriterMutexLock l(&port_status_event_writer_lock_);
  port_status_event_writer_ = nullptr;
  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
