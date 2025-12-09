// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_MANAGER_MOCK_H_
#define STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_MANAGER_MOCK_H_

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

class DpdkPortManagerMock : public DpdkPortManager {
 public:
  // ---------- Common public methods ----------

  MOCK_METHOD(::util::Status, RegisterPortStatusEventWriter,
              (std::unique_ptr<ChannelWriter<PortStatusEvent>> writer),
              (override));

  MOCK_METHOD(::util::Status, UnregisterPortStatusEventWriter, (), (override));

  MOCK_METHOD(::util::Status, GetPortInfo,
              (int device, int port, TargetDatapathId* target_dp_id),
              (override));

  MOCK_METHOD(::util::StatusOr<PortState>, GetPortState, (int device, int port),
              (override));

  MOCK_METHOD(::util::Status, GetPortCounters,
              (int device, int port, PortCounters* counters), (override));

  MOCK_METHOD(::util::StatusOr<uint32>, GetPortIdFromPortKey,
              (int device, const PortKey& port_key), (override));

  MOCK_METHOD(bool, IsValidPort, (int device, int port), (override));

  MOCK_METHOD(::util::Status, AddPort, (int device, int port), (override));

  MOCK_METHOD(::util::Status, DeletePort, (int device, int port), (override));

  MOCK_METHOD(::util::Status, EnablePort, (int device, int port), (override));

  MOCK_METHOD(::util::Status, DisablePort, (int device, int port), (override));

  // ---------- Dpdk-specific public methods ----------

  MOCK_METHOD(::util::Status, AddPort,
              (int device, int port, uint64 speed_bps, FecMode fec_mode),
              (override));

  MOCK_METHOD(::util::Status, AddPort,
              (int device, int port, const PortConfigParams& config),
              (override));

  MOCK_METHOD(::util::Status, HotplugPort,
              (int device, int port, HotplugConfigParams& hotplug_config),
              (override));
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_MANAGER_MOCK_H_
