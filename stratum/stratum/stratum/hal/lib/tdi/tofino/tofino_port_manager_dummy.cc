// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Dummy implementation of Tofino Port Manager.

#include "absl/synchronization/mutex.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/tdi/tofino/tofino_port_manager.h"
#include "stratum/lib/channel/channel.h"
#include "stratum/lib/constants.h"

namespace stratum {
namespace hal {
namespace tdi {

TofinoPortManager* TofinoPortManager::singleton_ = nullptr;

TofinoPortManager* TofinoPortManager::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new TofinoPortManager();
  }
  return singleton_;
}

TofinoPortManager* TofinoPortManager::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

::util::StatusOr<PortState> TofinoPortManager::GetPortState(int device,
                                                            int port) {
  return PORT_STATE_UP;
}

::util::Status TofinoPortManager::GetPortCounters(int device, int port,
                                                  PortCounters* counters) {
  counters->set_in_octets(0);
  counters->set_out_octets(1);
  counters->set_in_unicast_pkts(2);
  counters->set_out_unicast_pkts(3);
  counters->set_in_broadcast_pkts(5);
  counters->set_out_broadcast_pkts(8);
  counters->set_in_multicast_pkts(13);
  counters->set_out_multicast_pkts(21);
  counters->set_in_discards(34);
  counters->set_out_discards(0);       // stat not available
  counters->set_in_unknown_protos(0);  // stat not meaningful
  counters->set_in_errors(55);
  counters->set_out_errors(89);
  counters->set_in_fcs_errors(144);

  return ::util::OkStatus();
}

::util::Status TofinoPortManager::OnPortStatusEvent(int device, int port,
                                                    bool up,
                                                    absl::Time timestamp) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::GetPortInfo(int device, int port,
                                              TargetDatapathId* target_dp_id) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::AddPort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::AddPort(int device, int port,
                                          uint64 speed_bps, FecMode fec_mode) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::DeletePort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::EnablePort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::DisablePort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetPortShapingRate(int device, int port,
                                                     bool is_in_pps,
                                                     uint32 burst_size,
                                                     uint64 rate_per_second) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::EnablePortShaping(int device, int port,
                                                    TriState enable) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetPortAutonegPolicy(int device, int port,
                                                       TriState autoneg) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetPortMtu(int device, int port, int32 mtu) {
  return ::util::OkStatus();
}

bool TofinoPortManager::IsValidPort(int device, int port) { return true; }

::util::Status TofinoPortManager::SetPortLoopbackMode(
    int device, int port, LoopbackState loopback_mode) {
  return ::util::OkStatus();
}

::util::StatusOr<uint32> TofinoPortManager::GetPortIdFromPortKey(
    int device, const PortKey& port_key) {
  return 43;
}

::util::StatusOr<int> TofinoPortManager::GetPcieCpuPort(int device) {
  return 1776;
}

::util::Status TofinoPortManager::SetTmCpuPort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status TofinoPortManager::SetDeflectOnDropDestination(int device,
                                                              int port,
                                                              int queue) {
  return ::util::OkStatus();
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
