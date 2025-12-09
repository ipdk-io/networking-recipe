// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/tdi_global_vars.h"

#include "absl/synchronization/mutex.h"

namespace stratum {
namespace hal {
namespace tdi {

ABSL_CONST_INIT absl::Mutex chassis_lock(absl::kConstInit);
bool shutdown = false;

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
