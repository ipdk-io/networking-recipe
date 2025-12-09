// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_GLOBAL_VARS_H_
#define STRATUM_HAL_LIB_TDI_TDI_GLOBAL_VARS_H_

#include "absl/synchronization/mutex.h"

namespace stratum {
namespace hal {
namespace tdi {

// Lock that protects chassis state (ports, etc.) across the entire switch.
extern absl::Mutex chassis_lock;

// Flag that indicates the switch has been shut down. Initialized to false.
extern bool shutdown;

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_GLOBAL_VARS_H_
