// Copyright (c) 2022-2023 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_IPSEC_MANAGER_MOCK_H_
#define STRATUM_HAL_LIB_TDI_IPSEC_MANAGER_MOCK_H_

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/tdi_ipsec_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

class IPsecManagerMock : public TdiIpsecManager {
 public:
  MOCK_METHOD(::util::Status, InitializeNotificationCallback, ());

  MOCK_METHOD(::util::Status, GetSpiData, (uint32 & fetched_spi));

  MOCK_METHOD(::util::Status, WriteConfigSADBEntry,
              (const IPsecSadbConfigOp op_type, IPsecSADBConfig& msg));

  MOCK_METHOD(::util::Status, RegisterEventNotifyWriter,
              (const std::shared_ptr<WriterInterface<GnmiEventPtr>>& writer));

  MOCK_METHOD(::util::Status, UnregisterEventNotifyWriter, ());

  MOCK_METHOD(void, SendSADExpireNotificationEvent,
              (uint32_t dev_id, uint32_t ipsec_sa_spi,
               bool soft_lifetime_expire, uint8_t ipsec_sa_protocol,
               char* ipsec_sa_dest_address, bool ipv4));
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_IPSEC_MANAGER_MOCK_H_
