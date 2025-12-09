// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_DPDK_SDE_WRAPPER_H_
#define STRATUM_HAL_LIB_TDI_DPDK_SDE_WRAPPER_H_

#include <string>

#include "absl/synchronization/mutex.h"
#include "stratum/hal/lib/tdi/tdi.pb.h"
#include "stratum/hal/lib/tdi/tdi_sde_wrapper.h"

namespace stratum {
namespace hal {
namespace tdi {

class DpdkSdeWrapper : public TdiSdeWrapper {
 public:
  ::util::StatusOr<bool> IsSoftwareModel(int device) override;
  std::string GetChipType(int device) const override;
  std::string GetSdeVersion() const override;

  ::util::Status InitializeSde(const std::string& sde_install_path,
                               const std::string& sde_config_file,
                               bool run_in_background) override;
  ::util::Status AddDevice(int device,
                           const TdiDeviceConfig& device_config) override;

  // Creates the singleton instance. Expected to be called once to initialize
  // the instance.
  static DpdkSdeWrapper* CreateSingleton() LOCKS_EXCLUDED(init_lock_);

  // Return the singleton instance to be used in the SDE callbacks.
  static DpdkSdeWrapper* GetSingleton() LOCKS_EXCLUDED(init_lock_);

  // DpdkSdeWrapper is neither copyable nor movable.
  DpdkSdeWrapper(const DpdkSdeWrapper&) = delete;
  DpdkSdeWrapper& operator=(const DpdkSdeWrapper&) = delete;
  DpdkSdeWrapper(DpdkSdeWrapper&&) = delete;
  DpdkSdeWrapper& operator=(DpdkSdeWrapper&&) = delete;

 protected:
  // RW mutex lock for protecting the singleton instance initialization and
  // reading it back from other threads. Unlike other singleton classes, we
  // use RW lock as we need the pointer to class to be returned.
  static absl::Mutex init_lock_;

  // The singleton instance.
  static DpdkSdeWrapper* singleton_ GUARDED_BY(init_lock_);

 private:
  // Private constructor; use CreateSingleton and GetSingleton().
  DpdkSdeWrapper();
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_DPDK_SDE_WRAPPER_H_
