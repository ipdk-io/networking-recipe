// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_PORT_MANAGER_MOCK_H_
#define STRATUM_HAL_LIB_TDI_ES2K_PORT_MANAGER_MOCK_H_

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/es2k/es2k_port_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

class Es2kPortManagerMock : public Es2kPortManager {
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

  // ---------- ES2K-specific public methods ----------

  MOCK_METHOD(::util::Status, AddPort,
              (int device, int port, uint64 speed_bps, FecMode fec_mode),
              (override));

  MOCK_METHOD(::util::Status, EnablePortShaping,
              (int device, int port, TriState enable), (override));

  MOCK_METHOD(::util::Status, SetPortAutonegPolicy,
              (int device, int port, TriState autoneg), (override));

  MOCK_METHOD(::util::Status, SetPortMtu, (int device, int port, int32 mtu),
              (override));

  MOCK_METHOD(::util::Status, SetPortLoopbackMode,
              (int uint, int port, LoopbackState loopback_mode), (override));
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_PORT_MANAGER_MOCK_H_
