// Copyright 2021-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_CONFIG_H_
#define STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_CONFIG_H_

#include <cstdint>
#include <string>

#include "stratum/glue/integral_types.h"
#include "stratum/glue/status/status.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_manager.h"

namespace stratum {
namespace hal {
namespace tdi {

using ValueCase = SetRequest::Request::Port::ValueCase;

class DpdkPortConfig {
 public:
  // ADMIN_STATE_UNKNOWN indicates that something went wrong during port
  // configuration, and the port add failed or was not attempted.
  AdminState admin_state;

  // The configuration objects we pass to the SDK.
  DpdkPortManager::PortConfigParams cfg;
  DpdkPortManager::HotplugConfigParams hotplug;

  // TAP control port.
  std::string control_port;

  // Whether the port has been added.
  bool port_done;

  // Whether the HOTPLUG config has been applied.
  bool hotplug_done;

  DpdkPortConfig() { Reset(); }

  // Determines whether the specified parameter has been configured.
  bool IsParamSet(ValueCase value_case) const {
    auto param_mask = ParamMaskForCase(value_case);
    return (params_set & param_mask) != 0;
  }

  // Stores the specified parameter.
  ::util::Status SetParam(ValueCase value_case,
                          const SingletonPort& singleton_port);

  // Stores the specified hotplug parameter.
  ::util::Status SetHotplugParam(DpdkHotplugParam param_type,
                                 const SingletonPort& singleton_port);

  // Checks whether any the specified parameters have been configured.
  bool HasAnyOf(uint32_t param_mask) const {
    return (params_set & param_mask) != 0;
  }

  // Checks whether all the specified parameters have been configured.
  bool HasAllOf(uint32_t param_mask) const {
    return (params_set & param_mask) == param_mask;
  }

  // Adds the specified parameters to the configuration set.
  void Add(uint32 param_mask) { params_set |= param_mask; }

  // Removes the specified parameters from the configuration set.
  void Remove(uint32_t param_mask) { params_set &= ~param_mask; }

  // Resets parameters to initial state.
  void Reset() {
    admin_state = ADMIN_STATE_UNKNOWN;
    params_set = 0;
    port_done = false;
    hotplug_done = false;
    cfg = {};
    hotplug = {};
  }

 private:
  // Returns the parameter mask for a SetRequest::Request::Port::ValueCase.
  static uint32_t ParamMaskForCase(ValueCase value_case);

  // Bitmask indicating which parameters have been configured.
  uint32_t params_set;
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_CONFIG_H_
