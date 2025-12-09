// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TOFINO_TOFINO_PORT_MANAGER_MOCK_H_
#define STRATUM_HAL_LIB_TDI_TOFINO_TOFINO_PORT_MANAGER_MOCK_H_

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/tofino/tofino_port_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

class TofinoPortManagerMock : public TofinoPortManager {
 public:
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

  // ---------- Tofino-specific public methods ----------

  MOCK_METHOD(::util::Status, AddPort,
              (int device, int port, uint64 speed_bps, FecMode fec_mode),
              (override));

  MOCK_METHOD(::util::Status, SetPortShapingRate,
              (int device, int port, bool is_in_pps, uint32 burst_size,
               uint64 rate_per_second),
              (override));

  MOCK_METHOD(::util::Status, EnablePortShaping,
              (int device, int port, TriState enable), (override));

  MOCK_METHOD(::util::Status, SetPortAutonegPolicy,
              (int device, int port, TriState autoneg), (override));

  MOCK_METHOD(::util::Status, SetPortMtu, (int device, int port, int32 mtu),
              (override));

  MOCK_METHOD(::util::Status, SetPortLoopbackMode,
              (int uint, int port, LoopbackState loopback_mode), (override));

  MOCK_METHOD(::util::StatusOr<int>, GetPcieCpuPort, (int device), (override));

  MOCK_METHOD(::util::Status, SetTmCpuPort, (int device, int port), (override));

  MOCK_METHOD(::util::Status, SetDeflectOnDropDestination,
              (int device, int port, int queue), (override));
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TOFINO_TOFINO_PORT_MANAGER_MOCK_H_
