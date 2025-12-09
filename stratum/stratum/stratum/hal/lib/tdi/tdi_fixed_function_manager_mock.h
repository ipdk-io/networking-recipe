// Copyright (c) 2025 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_FIXED_FUNCTION_MANAGER_MOCK_H_
#define STRATUM_HAL_LIB_TDI_TDI_FIXED_FUNCTION_MANAGER_MOCK_H_

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/tdi_fixed_function_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

class TdiFixedFunctionManagerMock : public TdiFixedFunctionManager {
 public:
  MOCK_METHOD(::util::Status, InitNotificationTableWithCallback,
              (std::string table_name,
               void (*ipsec_notif_cb)(uint32_t, uint32_t, bool, uint8_t, char*,
                                      bool, void*),
               void* cookie),
              (override));

  MOCK_METHOD(::util::Status, WriteSadbEntry,
              (std::shared_ptr<TdiSdeInterface::SessionInterface> session,
               std::string table_name, const IPsecSadbConfigOp op_type,
               IPsecSADBConfig& sadb_config),
              (override));

  MOCK_METHOD(::util::Status, FetchSpi,
              (std::shared_ptr<TdiSdeInterface::SessionInterface> session,
               std::string table_name, uint32* spi),
              (override));

  MOCK_METHOD(::util::Status, FetchVportTableData,
              (std::shared_ptr<TdiSdeInterface::SessionInterface> session,
               std::string table_name, uint32 global_resource_id,
               const char* param_name, uint64* data),
              (override));

  static std::unique_ptr<TdiFixedFunctionManagerMock> CreateInstance(
      OperationMode mode, TdiSdeInterface* tdi_sde_interface, int device) {
    return absl::WrapUnique(
        new TdiFixedFunctionManagerMock(mode, tdi_sde_interface, device));
  }

 protected:
  TdiFixedFunctionManagerMock(OperationMode mode,
                              TdiSdeInterface* tdi_sde_interface, int device)
      : TdiFixedFunctionManager(mode, tdi_sde_interface, device) {}
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_FIXED_FUNCTION_MANAGER_MOCK_H_
