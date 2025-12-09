// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_COMMON_TARGET_OPTIONS_H_
#define STRATUM_HAL_LIB_COMMON_TARGET_OPTIONS_H_

namespace stratum {
namespace hal {

class TargetOptions {
 public:
  TargetOptions();

  // Whether ConfigMonitoringService should push the updated chassis
  // configuration. Default: true.
  bool pushUpdatedChassisConfig;

  // Whether ConfigMonitoringService should require that the config file
  // be a regular file (not a symlink). Default: false.
  bool secureChassisConfig;

  // Whether P4Service supports overwrite of an already-configured pipeline
  // for a single device. Default: true.
  bool allowPipelineOverwrite;

  // Default target options.
  static const TargetOptions default_target_options;
};

}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_COMMON_TARGET_OPTIONS_H_
