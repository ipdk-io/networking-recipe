// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/common/target_options.h"

namespace stratum {
namespace hal {

// Note: default values MUST maintain backward compatibility with
// existing code.
TargetOptions::TargetOptions()
    : pushUpdatedChassisConfig(true),
      secureChassisConfig(false),
      allowPipelineOverwrite(true) {}

const TargetOptions TargetOptions::default_target_options = {};

}  // namespace hal
}  // namespace stratum
