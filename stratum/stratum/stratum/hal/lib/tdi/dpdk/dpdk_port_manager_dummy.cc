// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Dummy implementation of DPDK port manager.

#include "absl/synchronization/mutex.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_manager.h"
#include "stratum/lib/channel/channel.h"

namespace stratum {
namespace hal {
namespace tdi {

DpdkPortManager* DpdkPortManager::singleton_ = nullptr;

DpdkPortManager* DpdkPortManager::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new DpdkPortManager();
  }

  return singleton_;
}

DpdkPortManager* DpdkPortManager::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

::util::StatusOr<PortState> DpdkPortManager::GetPortState(int device,
                                                          int port) {
  return PORT_STATE_UP;
}

::util::Status DpdkPortManager::GetPortCounters(int device, int port,
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
  counters->set_out_discards(55);
  counters->set_in_unknown_protos(0);  // stat not meaningful
  counters->set_in_errors(89);
  counters->set_out_errors(144);
  counters->set_in_fcs_errors(233);
  return ::util::OkStatus();
}

::util::Status DpdkPortManager::GetPortInfo(int device, int port,
                                            TargetDatapathId* target_dp_id) {
  target_dp_id->set_tdi_portin_id(1);
  target_dp_id->set_tdi_portout_id(2);

  return ::util::OkStatus();
}

::util::Status DpdkPortManager::HotplugPort(
    int device, int port, HotplugConfigParams& hotplug_config) {
  return ::util::OkStatus();
}

::util::Status DpdkPortManager::AddPort(int device, int port) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "AddPort(device, port) not supported";
}

::util::Status DpdkPortManager::AddPort(int device, int port, uint64 speed_bps,
                                        FecMode fec_mode) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "AddPort(device, port, speed, fec_mode) not supported";
}

::util::Status DpdkPortManager::AddPort(int device, int port,
                                        const PortConfigParams& config) {
  return ::util::OkStatus();
}

::util::Status DpdkPortManager::DeletePort(int device, int port) {
  return ::util::OkStatus();
}

::util::Status DpdkPortManager::EnablePort(int device, int port) {
  return MAKE_ERROR(ERR_UNIMPLEMENTED) << "EnablePort not implemented";
}

::util::Status DpdkPortManager::DisablePort(int device, int port) {
  return MAKE_ERROR(ERR_UNIMPLEMENTED) << "DisablePort not implemented";
}

// Should this return ::util::StatusOr<bool>?
bool DpdkPortManager::IsValidPort(int device, int port) { return true; }

::util::StatusOr<uint32> DpdkPortManager::GetPortIdFromPortKey(
    int device, const PortKey& port_key) {
  return 43;
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
