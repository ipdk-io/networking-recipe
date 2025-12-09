// Copyright 2023,2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_FIXED_FUNCTION_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_TDI_FIXED_FUNCTION_MANAGER_H_

#include "absl/synchronization/mutex.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_sde_interface.h"

namespace stratum {
namespace hal {
namespace tdi {

class TdiFixedFunctionManager {
 public:
  // Initializes Notification table callback
  ::util::Status InitNotificationTableWithCallback(
      std::string table_name,
      void (*ipsec_notif_cb)(uint32_t, uint32_t, bool, uint8_t, char*, bool,
                             void*),
      void* cookie) LOCKS_EXCLUDED(lock_);

  ::util::Status InitNotificationTableWithCallback(
      std::string table_name,
      void (*vport_state_notif_cb)(uint32_t, uint32_t, uint8_t, void*),
      void* cookie) LOCKS_EXCLUDED(lock_);

  // Writes IPsec SADB  table entry.
  ::util::Status WriteSadbEntry(
      std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      std::string table_name, const IPsecSadbConfigOp op_type,
      IPsecSADBConfig& sadb_config) LOCKS_EXCLUDED(lock_);

  // Fetch the SPI value from the TDI table
  ::util::Status FetchSpi(
      std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      std::string table_name, uint32* spi) LOCKS_EXCLUDED(lock_);

  // Fetch value from the TDI table
  ::util::Status FetchVportTableData(
      std::shared_ptr<TdiSdeInterface::SessionInterface> session,
      std::string table_name, uint32 global_resource_id, const char* param_name,
      uint64* data) LOCKS_EXCLUDED(lock_);

  // Creates a Fixed function table manager instance.
  static std::unique_ptr<TdiFixedFunctionManager> CreateInstance(
      OperationMode mode, TdiSdeInterface* tdi_sde_interface, int device);

 private:
  // Private constructor, we can create the instance by using `CreateInstance`
  // function only.
  explicit TdiFixedFunctionManager(OperationMode mode,
                                   TdiSdeInterface* tdi_sde_interface,
                                   int device);

  // Builds the IPSEC SADB table key information.
  ::util::Status BuildSadbTableKey(
      TdiSdeInterface::TableKeyInterface* table_key,
      IPsecSADBConfig& sadb_config) SHARED_LOCKS_REQUIRED(lock_);

  // Builds the IPSEC SADB table data information.
  ::util::Status BuildSadbTableData(
      TdiSdeInterface::TableDataInterface* table_data,
      IPsecSADBConfig& sadb_config);
  SHARED_LOCKS_REQUIRED(lock_);

  // Determines the mode of operation:
  // - OPERATION_MODE_STANDALONE: when Stratum stack runs independently and
  // therefore needs to do all the SDK initialization itself.
  // - OPERATION_MODE_COUPLED: when Stratum stack runs as part of Sandcastle
  // stack, coupled with the rest of stack processes.
  // - OPERATION_MODE_SIM: when Stratum stack runs in simulation mode.
  // Note that this variable is set upon initialization and is never changed
  // afterwards.
  OperationMode mode_;

  // Reader-writer lock used to protect access to pipeline state.
  mutable absl::Mutex lock_;

  // Pointer to a TdiSdeInterface implementation that wraps all the SDE calls.
  TdiSdeInterface* tdi_sde_interface_ = nullptr;  // not owned by this class.

  // Fixed zero-based Tofino device number corresponding to the node/ASIC
  // managed by this class instance. Assigned in the class constructor.
  const int device_;

  // TODO(5abeel): Add unit test
  // friend class TdiTableManagerTest;
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_FIXED_FUNCTION_MANAGER_H_
