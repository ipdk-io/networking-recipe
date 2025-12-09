// Copyright 2018-present Barefoot Networks, Inc.
// Copyright 2021-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/dpdk/dpdk_chassis_manager.h"

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/base/const_init.h"
#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "stratum/glue/gtl/map_util.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status_macros.h"
#include "stratum/hal/lib/common/constants.h"
#include "stratum/hal/lib/common/gnmi_events.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_config.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_constants.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_manager.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/hal/lib/tdi/tdi_port_manager.h"
#include "stratum/lib/macros.h"
#include "stratum/public/proto/error.pb.h"

namespace stratum {
namespace hal {
namespace tdi {

using PortStatusEvent = TdiPortManager::PortStatusEvent;

/* static */
constexpr int DpdkChassisManager::kMaxPortStatusEventDepth;
/* static */
constexpr int DpdkChassisManager::kMaxXcvrEventDepth;

constexpr uint64 SPEED_BPS_UNKNOWN = 0;

DpdkChassisManager::DpdkChassisManager(OperationMode mode,
                                       DpdkPortManager* port_manager)
    : mode_(mode),
      initialized_(false),
      gnmi_event_writer_(nullptr),
      device_to_node_id_(),
      node_id_to_device_(),
      node_id_to_port_id_to_port_state_(),
      node_id_to_port_id_to_time_last_changed_(),
      node_id_to_port_id_to_port_config_(),
      node_id_to_port_id_to_singleton_port_key_(),
      node_id_to_port_id_to_sdk_port_id_(),
      node_id_to_sdk_port_id_to_port_id_(),
      port_manager_(ABSL_DIE_IF_NULL(port_manager)) {}

DpdkChassisManager::DpdkChassisManager()
    : mode_(OPERATION_MODE_STANDALONE),
      initialized_(false),
      gnmi_event_writer_(nullptr),
      device_to_node_id_(),
      node_id_to_device_(),
      node_id_to_port_id_to_port_state_(),
      node_id_to_port_id_to_time_last_changed_(),
      node_id_to_port_id_to_port_config_(),
      node_id_to_port_id_to_singleton_port_key_(),
      node_id_to_port_id_to_sdk_port_id_(),
      node_id_to_sdk_port_id_to_port_id_(),
      port_manager_(nullptr) {}

DpdkChassisManager::~DpdkChassisManager() = default;

namespace {

bool IsConfigComplete(const DpdkPortConfig& config) {
  switch (config.cfg.port_type) {
    case PORT_TYPE_VHOST:
      return config.HasAllOf(GNMI_CONFIG_VHOST_REQUIRED);
    case PORT_TYPE_LINK:
      return config.HasAllOf(GNMI_CONFIG_LINK_REQUIRED);
    case PORT_TYPE_TAP:
      return config.HasAllOf(GNMI_CONFIG_TAP_REQUIRED);
    default:
      break;
  }
  return false;
}

void SupplyDefaultParams(DpdkPortConfig& config) {
  if (!config.HasAnyOf(GNMI_CONFIG_PIPELINE_NAME)) {
    config.cfg.pipeline_name = DEFAULT_PIPELINE;
    config.Add(GNMI_CONFIG_PIPELINE_NAME);
  }
  if (!config.HasAnyOf(GNMI_CONFIG_MEMPOOL_NAME)) {
    config.cfg.mempool_name = DEFAULT_MEMPOOL;
    config.Add(GNMI_CONFIG_MEMPOOL_NAME);
  }
  if (!config.HasAnyOf(GNMI_CONFIG_MTU_VALUE)) {
    config.cfg.mtu = DEFAULT_MTU;
    config.Add(GNMI_CONFIG_MTU_VALUE);
  }
  if (!config.HasAnyOf(GNMI_CONFIG_PACKET_DIR)) {
    config.cfg.packet_dir = DEFAULT_PACKET_DIR;
    config.Add(GNMI_CONFIG_PACKET_DIR);
  }
}

bool HasUnsupportedParams(const DpdkPortConfig& config) {
  switch (config.cfg.port_type) {
    case PORT_TYPE_VHOST:
      return config.HasAnyOf(GNMI_CONFIG_VHOST_UNSUPPORTED);
    case PORT_TYPE_LINK:
      return config.HasAnyOf(GNMI_CONFIG_LINK_UNSUPPORTED);
    case PORT_TYPE_TAP:
      return config.HasAnyOf(GNMI_CONFIG_TAP_UNSUPPORTED);
    default:
      break;
  }
  return false;
}

}  // namespace

// Determines whether a port parameter has been configured.
bool DpdkChassisManager::IsPortParamSet(
    uint64 node_id, uint32 port_id,
    SetRequest::Request::Port::ValueCase value_case) {
  auto& config = node_id_to_port_id_to_port_config_[node_id][port_id];
  return config.IsParamSet(value_case);
}

// Sets the value of a port configuration parameter.
::util::Status DpdkChassisManager::SetPortParam(
    uint64 node_id, uint32 port_id, const SingletonPort& singleton_port,
    SetRequest::Request::Port::ValueCase value_case) {
  auto& config = node_id_to_port_id_to_port_config_[node_id][port_id];
  RETURN_IF_ERROR(config.SetParam(value_case, singleton_port));

  if (config.HasAnyOf(GNMI_CONFIG_PORT_TYPE) && !config.port_done) {
    if (IsConfigComplete(config)) {
      auto device = node_id_to_device_[node_id];
      auto sdk_port_id = node_id_to_port_id_to_sdk_port_id_[node_id][port_id];

      LOG(INFO) << "Required parameters are configured, configure port via TDI";
      LOG(INFO) << "SDK_PORT ID while validating = " << sdk_port_id;

      SupplyDefaultParams(config);

      if (HasUnsupportedParams(config)) {
        // Unsupported list of Params, clear the validate field.
        config.Reset();
        return MAKE_ERROR(ERR_INVALID_PARAM)
               << "Unsupported parameter list for given Port Type";
      }
      ::util::Status status =
          AddPortHelper(node_id, device, sdk_port_id, singleton_port, &config);
      if (status.ok()) {
        config.port_done = true;
      } else {
        config.Reset();
        return status;
      }
    }
  }
  google::FlushLogFiles(google::INFO);
  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::SetHotplugParam(
    uint64 node_id, uint32 port_id, const SingletonPort& singleton_port,
    DpdkHotplugParam param_type) {
  auto& config = node_id_to_port_id_to_port_config_[node_id][port_id];
  RETURN_IF_ERROR(config.SetHotplugParam(param_type, singleton_port));

  auto hotplug_params =
      static_cast<DpdkPortManager::HotplugConfigParams*>(&config.hotplug);
  auto device = node_id_to_device_[node_id];
  auto sdk_port_id = node_id_to_port_id_to_sdk_port_id_[node_id][port_id];

  if (config.HasAllOf(GNMI_CONFIG_HOTPLUG_REQUIRED) &&
      hotplug_params->qemu_hotplug_mode == HOTPLUG_MODE_ADD) {
    if (!config.port_done) {
      config.Remove(GNMI_CONFIG_HOTPLUG_REQUIRED);
      return MAKE_ERROR(ERR_INTERNAL)
             << "Unsupported operation: requested port does not exist";
    }
    if (config.hotplug_done) {
      config.Remove(GNMI_CONFIG_HOTPLUG_REQUIRED);
      return MAKE_ERROR(ERR_INTERNAL)
             << "Unsupported operation: requested port is already hotplugged";
    }

    RETURN_IF_ERROR(HotplugPortHelper(node_id, device, sdk_port_id,
                                      singleton_port, &config));
    config.hotplug_done = true;
    LOG(INFO) << "Port was successfully hotplugged";

    // Unset this entry to allow future entries
    config.Remove(GNMI_CONFIG_HOTPLUG_MODE);
    hotplug_params->qemu_hotplug_mode = HOTPLUG_MODE_NONE;
  } else if (config.HasAnyOf(GNMI_CONFIG_HOTPLUG_MODE) &&
             hotplug_params->qemu_hotplug_mode == HOTPLUG_MODE_DEL) {
    if (!config.hotplug_done) {
      config.Remove(GNMI_CONFIG_HOTPLUG_MODE);
      return MAKE_ERROR(ERR_INTERNAL)
             << "Unsupported operation: no hotplugged device to delete";
    }
    RETURN_IF_ERROR(HotplugPortHelper(node_id, device, sdk_port_id,
                                      singleton_port, &config));
    config.Remove(GNMI_CONFIG_HOTPLUG_REQUIRED);
    config.hotplug_done = false;
    hotplug_params->qemu_hotplug_mode = HOTPLUG_MODE_NONE;
    LOG(INFO) << "Port was successfully removed from QEMU VM";
  }
  google::FlushLogFiles(google::INFO);
  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::AddPortHelper(
    uint64 node_id, int device, uint32 sdk_port_id,
    const SingletonPort& singleton_port /* desired config */,
    /* out */ DpdkPortConfig* config /* new config */) {
  config->admin_state = ADMIN_STATE_UNKNOWN;
  // SingletonPort ID is the SDN/Stratum port ID
  uint32 port_id = singleton_port.id();

  const auto& config_params = singleton_port.config_params();

  if (config_params.admin_state() == ADMIN_STATE_UNKNOWN) {
    return MAKE_ERROR(ERR_INVALID_PARAM)
           << "Invalid admin state for port " << port_id << " in node "
           << node_id << " (SDK Port " << sdk_port_id << ").";
  }
  if (config_params.admin_state() == ADMIN_STATE_DIAG) {
    return MAKE_ERROR(ERR_UNIMPLEMENTED)
           << "Unsupported 'diags' admin state for port " << port_id
           << " in node " << node_id << " (SDK Port " << sdk_port_id << ").";
  }

  config->admin_state = ADMIN_STATE_DISABLED;

  DpdkPortManager::PortConfigParams port_params = config->cfg;
  port_params.port_name = singleton_port.name();

  LOG(INFO) << "Adding port " << port_id << " in node " << node_id
            << " (SDK Port " << sdk_port_id << ").";

  RETURN_IF_ERROR(port_manager_->AddPort(device, sdk_port_id, port_params));

  // Check if Control Port Creation is opted.
  if (config->control_port.length()) {
    LOG(INFO) << "Autocreating Control TAP port";
    // Packet direction for control port will always be host type
    port_params.port_type = PORT_TYPE_TAP;
    port_params.packet_dir = DEFAULT_PACKET_DIR;
    port_params.port_name = config->control_port;

    /* sdk_ctl_port_id is uniquely derived from the SDK_PORT_CONTROL_BASE range
     * and maps 1:1 to parent port's sdk_port_id.
     */
    uint32 ctl_port_id = SDK_PORT_CONTROL_BASE + sdk_port_id;
    RETURN_IF_ERROR(port_manager_->AddPort(device, ctl_port_id, port_params));
  }

  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::HotplugPortHelper(
    uint64 node_id, int device, uint32 sdk_port_id,
    const SingletonPort& singleton_port /* desired config */,
    /* out */ DpdkPortConfig* config /* new config */) {
  // SingletonPort ID is the SDN/Stratum port ID
  uint32 port_id = singleton_port.id();

  LOG(INFO) << "Hotplugging port " << port_id << " in node " << node_id
            << " (SDK Port " << sdk_port_id << ").";

  RETURN_IF_ERROR(
      port_manager_->HotplugPort(device, sdk_port_id, config->hotplug));

  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::UpdatePortHelper(
    uint64 node_id, int device, uint32 sdk_port_id,
    const SingletonPort& singleton_port /* desired config */,
    const DpdkPortConfig& config_old /* current config */,
    /* out */ DpdkPortConfig* config /* new config */) {
  /* TODO : None of port updates are supported by DPDK SDE,
   * maybe we can remove this functionality or return unsupported
   * for DPDK
   */
  *config = config_old;
  // SingletonPort ID is the SDN/Stratum port ID
  uint32 port_id = singleton_port.id();

  if (!port_manager_->IsValidPort(device, sdk_port_id)) {
    config->admin_state = ADMIN_STATE_UNKNOWN;
    return MAKE_ERROR(ERR_INTERNAL)
           << "Port " << port_id << " in node " << node_id << " is not valid"
           << " (SDK Port " << sdk_port_id << ").";
  }

  const auto& config_params = singleton_port.config_params();
  if (config_params.admin_state() == ADMIN_STATE_UNKNOWN) {
    return MAKE_ERROR(ERR_INVALID_PARAM)
           << "Invalid admin state for port " << port_id << " in node "
           << node_id << " (SDK Port " << sdk_port_id << ").";
  }
  if (config_params.admin_state() == ADMIN_STATE_DIAG) {
    return MAKE_ERROR(ERR_UNIMPLEMENTED)
           << "Unsupported 'diags' admin state for port " << port_id
           << " in node " << node_id << " (SDK Port " << sdk_port_id << ").";
  }

  bool config_changed = false;

  bool need_disable = false, need_enable = false;
  if (config_params.admin_state() == ADMIN_STATE_DISABLED) {
    // if the new admin state is disabled, we need to disable the port if it was
    // previously enabled.
    need_disable = (config_old.admin_state != ADMIN_STATE_DISABLED);
  } else if (config_params.admin_state() == ADMIN_STATE_ENABLED) {
    // if the new admin state is enabled, we need to:
    //  * disable the port if there is a config chaned and the port was
    //    previously enabled
    //  * enable the port if it needs to be disabled first because of a config
    //    change if it is currently disabled
    need_disable =
        config_changed && (config_old.admin_state != ADMIN_STATE_DISABLED);
    need_enable =
        need_disable || (config_old.admin_state == ADMIN_STATE_DISABLED);
  }

  if (need_disable) {
    LOG(INFO) << "Disabling port " << port_id << " in node " << node_id
              << " (SDK Port " << sdk_port_id << ").";
    config->admin_state = ADMIN_STATE_DISABLED;
  }
  if (need_enable) {
    LOG(INFO) << "Enabling port " << port_id << " in node " << node_id
              << " (SDK Port " << sdk_port_id << ").";
    config->admin_state = ADMIN_STATE_ENABLED;
  }

  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::PushChassisConfig(
    const ChassisConfig& config) {
  // new maps
  std::map<int, uint64> device_to_node_id;
  std::map<uint64, int> node_id_to_device;
  std::map<uint64, std::map<uint32, PortState>>
      node_id_to_port_id_to_port_state;
  std::map<uint64, std::map<uint32, absl::Time>>
      node_id_to_port_id_to_time_last_changed;
  std::map<uint64, std::map<uint32, DpdkPortConfig>>
      node_id_to_port_id_to_port_config;
  std::map<uint64, std::map<uint32, PortKey>>
      node_id_to_port_id_to_singleton_port_key;
  std::map<uint64, std::map<uint32, uint32>> node_id_to_port_id_to_sdk_port_id;
  std::map<uint64, std::map<uint32, uint32>> node_id_to_sdk_port_id_to_port_id;

  {
    int device = 0;
    for (const auto& node : config.nodes()) {
      device_to_node_id[device] = node.id();
      node_id_to_device[node.id()] = device;
      device++;
    }
  }

  for (const auto& singleton_port : config.singleton_ports()) {
    uint32 port_id = singleton_port.id();
    uint64 node_id = singleton_port.node();

    auto* device = gtl::FindOrNull(node_id_to_device, node_id);
    if (device == nullptr) {
      return MAKE_ERROR(ERR_INVALID_PARAM)
             << "Invalid ChassisConfig, unknown node id " << node_id
             << " for port " << port_id << ".";
    }
    node_id_to_port_id_to_port_state[node_id][port_id] = PORT_STATE_UNKNOWN;
    node_id_to_port_id_to_time_last_changed[node_id][port_id] =
        absl::UnixEpoch();
    node_id_to_port_id_to_port_config[node_id][port_id] = DpdkPortConfig();
    PortKey singleton_port_key(singleton_port.slot(), singleton_port.port(),
                               singleton_port.channel());
    node_id_to_port_id_to_singleton_port_key[node_id][port_id] =
        singleton_port_key;

    // Translate the logical SDN port to SDK port (BF device port ID)
    ASSIGN_OR_RETURN(uint32 sdk_port, port_manager_->GetPortIdFromPortKey(
                                          *device, singleton_port_key));
    node_id_to_port_id_to_sdk_port_id[node_id][port_id] = sdk_port;
    LOG(INFO) << "SDK_PORT = " << sdk_port << " for port_id = " << port_id;
    node_id_to_sdk_port_id_to_port_id[node_id][sdk_port] = port_id;

    PortKey port_group_key(singleton_port.slot(), singleton_port.port());
  }

  for (const auto& singleton_port : config.singleton_ports()) {
    uint32 port_id = singleton_port.id();
    uint64 node_id = singleton_port.node();
    // we checked that node_id was valid in the previous loop
    auto device = node_id_to_device[node_id];

    // TODO(antonin): we currently ignore slot
    // Stratum requires slot and port to be set. We use port and channel to
    // get Tofino device port (called SDK port ID).

    const DpdkPortConfig* config_old = nullptr;
    const auto* port_id_to_port_config_old =
        gtl::FindOrNull(node_id_to_port_id_to_port_config_, node_id);
    if (port_id_to_port_config_old != nullptr) {
      config_old = gtl::FindOrNull(*port_id_to_port_config_old, port_id);
    }

    auto& config = node_id_to_port_id_to_port_config[node_id][port_id];
    uint32 sdk_port_id = node_id_to_port_id_to_sdk_port_id[node_id][port_id];
    if (config_old == nullptr) {
      // new port
      // if anything fails, config.admin_state will be set to
      // ADMIN_STATE_UNKNOWN (invalid)
      // RETURN_IF_ERROR(
      //    AddPortHelper(node_id, device, sdk_port_id, singleton_port,
      //    &config));
      continue;
    } else {
      // port already exists, config may have changed
      if (config_old->admin_state == ADMIN_STATE_UNKNOWN) {
        // something is wrong with the port. We make sure the port is deleted
        // first (and ignore the error status if there is one), then add the
        // port again.
        if (port_manager_->IsValidPort(device, sdk_port_id)) {
          port_manager_->DeletePort(device, sdk_port_id);
        }
        RETURN_IF_ERROR(AddPortHelper(node_id, device, sdk_port_id,
                                      singleton_port, &config));
        continue;
      }

      // if anything fails, config.admin_state will be set to
      // ADMIN_STATE_UNKNOWN (invalid)
      RETURN_IF_ERROR(UpdatePortHelper(node_id, device, sdk_port_id,
                                       singleton_port, *config_old, &config));
    }
  }

  // Clean up from old config.
  for (const auto& node_ports_old : node_id_to_port_id_to_port_config_) {
    auto node_id = node_ports_old.first;
    for (const auto& port_old : node_ports_old.second) {
      auto port_id = port_old.first;
      if (node_id_to_port_id_to_port_config.count(node_id) > 0 &&
          node_id_to_port_id_to_port_config[node_id].count(port_id) > 0) {
        continue;
      }
      auto device = node_id_to_device_[node_id];
      uint32 sdk_port_id = node_id_to_port_id_to_sdk_port_id_[node_id][port_id];
      // remove ports which are no longer present in the ChassisConfig
      // TODO(bocon): Collect these errors and keep trying to remove old ports
      LOG(INFO) << "Deleting port " << port_id << " in node " << node_id
                << " (SDK port " << sdk_port_id << ").";
      RETURN_IF_ERROR(port_manager_->DeletePort(device, sdk_port_id));
    }
  }

  device_to_node_id_ = device_to_node_id;
  node_id_to_device_ = node_id_to_device;
  node_id_to_port_id_to_port_state_ = node_id_to_port_id_to_port_state;
  node_id_to_port_id_to_time_last_changed_ =
      node_id_to_port_id_to_time_last_changed;
  node_id_to_port_id_to_port_config_ = node_id_to_port_id_to_port_config;
  node_id_to_port_id_to_singleton_port_key_ =
      node_id_to_port_id_to_singleton_port_key;
  node_id_to_port_id_to_sdk_port_id_ = node_id_to_port_id_to_sdk_port_id;
  node_id_to_sdk_port_id_to_port_id_ = node_id_to_sdk_port_id_to_port_id;
  initialized_ = true;

  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::VerifyChassisConfig(
    const ChassisConfig& config) {
  RET_CHECK(config.trunk_ports_size() == 0)
      << "Trunk ports are not supported on Tofino.";
  RET_CHECK(config.port_groups_size() == 0)
      << "Port groups are not supported on Tofino.";
  RET_CHECK(config.nodes_size() > 0)
      << "The config must contain at least one node.";

  // Find the supported Tofino chip types based on the given platform.
  RET_CHECK(config.has_chassis() && config.chassis().platform())
      << "Config needs a Chassis message with correct platform.";
  switch (config.chassis().platform()) {
    case PLT_GENERIC_BAREFOOT_TOFINO:
    case PLT_GENERIC_BAREFOOT_TOFINO2:
    case PLT_P4_SOFT_SWITCH:
      break;
    default:
      return MAKE_ERROR(ERR_INVALID_PARAM)
             << "Unsupported platform: "
             << Platform_Name(config.chassis().platform());
  }

  // Validate Node messages. Make sure there is no two nodes with the same id.
  std::map<uint64, int> node_id_to_device;
  std::map<int, uint64> device_to_node_id;
  for (const auto& node : config.nodes()) {
    RET_CHECK(node.slot() > 0)
        << "No positive slot in " << node.ShortDebugString();
    RET_CHECK(node.id() > 0) << "No positive ID in " << node.ShortDebugString();
    RET_CHECK(gtl::InsertIfNotPresent(&node_id_to_device, node.id(), -1))
        << "The id for Node " << PrintNode(node) << " was already recorded "
        << "for another Node in the config.";
  }
  {
    int device = 0;
    for (const auto& node : config.nodes()) {
      device_to_node_id[device] = node.id();
      node_id_to_device[node.id()] = device;
      ++device;
    }
  }

  // Go over all the singleton ports in the config:
  // 1- Validate the basic singleton port properties.
  // 2- Make sure there is no two ports with the same (slot, port, channel).
  // 3- Make sure for each (slot, port) pair, the channels of all the ports
  //    are valid. This depends on the port speed.
  // 4- Make sure no singleton port has the reserved CPU port ID. CPU port is
  //    a special port and is not in the list of singleton ports. It is
  //    configured separately.
  // 5- Make sure IDs of the singleton ports are unique per node.
  std::map<uint64, std::set<uint32>> node_id_to_port_ids;
  std::set<PortKey> singleton_port_keys;
  for (const auto& singleton_port : config.singleton_ports()) {
    // TODO(max): enable once we decoupled port ids from sdk ports.
    // RET_CHECK(singleton_port.id() > 0)
    //     << "No positive ID in " << PrintSingletonPort(singleton_port) << ".";
    RET_CHECK(singleton_port.id() != kCpuPortId)
        << "SingletonPort " << PrintSingletonPort(singleton_port)
        << " has the reserved CPU port ID (" << kCpuPortId << ").";
    RET_CHECK(singleton_port.slot() > 0)
        << "No valid slot in " << singleton_port.ShortDebugString() << ".";
    RET_CHECK(singleton_port.port() > 0)
        << "No valid port in " << singleton_port.ShortDebugString() << ".";
    PortKey singleton_port_key(singleton_port.slot(), singleton_port.port(),
                               singleton_port.channel());
    RET_CHECK(!singleton_port_keys.count(singleton_port_key))
        << "The (slot, port, channel) tuple for SingletonPort "
        << PrintSingletonPort(singleton_port)
        << " was already recorded for another SingletonPort in the config.";
    singleton_port_keys.insert(singleton_port_key);
    RET_CHECK(singleton_port.node() > 0)
        << "No valid node ID in " << singleton_port.ShortDebugString() << ".";
    RET_CHECK(node_id_to_device.count(singleton_port.node()))
        << "Node ID " << singleton_port.node() << " given for SingletonPort "
        << PrintSingletonPort(singleton_port)
        << " has not been given to any Node in the config.";
    RET_CHECK(
        !node_id_to_port_ids[singleton_port.node()].count(singleton_port.id()))
        << "The id for SingletonPort " << PrintSingletonPort(singleton_port)
        << " was already recorded for another SingletonPort for node with ID "
        << singleton_port.node() << ".";
    node_id_to_port_ids[singleton_port.node()].insert(singleton_port.id());
  }

  std::map<uint64, std::map<uint32, PortKey>>
      node_id_to_port_id_to_singleton_port_key;

  for (const auto& singleton_port : config.singleton_ports()) {
    uint32 port_id = singleton_port.id();
    uint64 node_id = singleton_port.node();

    PortKey singleton_port_key(singleton_port.slot(), singleton_port.port(),
                               singleton_port.channel());
    node_id_to_port_id_to_singleton_port_key[node_id][port_id] =
        singleton_port_key;

    // Make sure that the port exists by getting the SDK port ID.
    const int* device = gtl::FindOrNull(node_id_to_device, node_id);
    RET_CHECK(device != nullptr)
        << "Node " << node_id << " not found for port " << port_id << ".";
    RETURN_IF_ERROR(
        port_manager_->GetPortIdFromPortKey(*device, singleton_port_key)
            .status());
  }

  // If the class is initialized, we also need to check if the new config will
  // require a change in the port layout. If so, report reboot required.
  if (initialized_) {
    if (node_id_to_port_id_to_singleton_port_key !=
        node_id_to_port_id_to_singleton_port_key_) {
      return MAKE_ERROR(ERR_REBOOT_REQUIRED)
             << "The switch is already initialized, but we detected the newly "
             << "pushed config requires a change in the port layout. The stack "
             << "needs to be rebooted to finish config push.";
    }

    if (node_id_to_device != node_id_to_device_) {
      return MAKE_ERROR(ERR_REBOOT_REQUIRED)
             << "The switch is already initialized, but we detected the newly "
             << "pushed config requires a change in node_id_to_device. The "
                "stack "
             << "needs to be rebooted to finish config push.";
    }
  }

  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::RegisterEventNotifyWriter(
    const std::shared_ptr<WriterInterface<GnmiEventPtr>>& writer) {
  absl::WriterMutexLock l(&gnmi_event_lock_);
  gnmi_event_writer_ = writer;
  return ::util::OkStatus();
}

::util::Status DpdkChassisManager::UnregisterEventNotifyWriter() {
  absl::WriterMutexLock l(&gnmi_event_lock_);
  gnmi_event_writer_ = nullptr;
  return ::util::OkStatus();
}

::util::StatusOr<const DpdkPortConfig*> DpdkChassisManager::GetPortConfig(
    uint64 node_id, uint32 port_id) const {
  auto* port_id_to_config =
      gtl::FindOrNull(node_id_to_port_id_to_port_config_, node_id);
  RET_CHECK(port_id_to_config != nullptr)
      << "Node " << node_id << " is not configured or not known.";
  const DpdkPortConfig* config = gtl::FindOrNull(*port_id_to_config, port_id);
  RET_CHECK(config != nullptr)
      << "Port " << port_id << " is not configured or not known for node "
      << node_id << ".";
  return config;
}

::util::StatusOr<uint32> DpdkChassisManager::GetSdkPortId(
    uint64 node_id, uint32 port_id) const {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }

  const auto* port_map =
      gtl::FindOrNull(node_id_to_port_id_to_sdk_port_id_, node_id);
  RET_CHECK(port_map != nullptr)
      << "Node " << node_id << " is not configured or not known.";

  const uint32* sdk_port_id = gtl::FindOrNull(*port_map, port_id);
  RET_CHECK(sdk_port_id != nullptr)
      << "Port " << port_id << " for node " << node_id
      << " is not configured or not known.";

  return *sdk_port_id;
}

::util::Status DpdkChassisManager::GetTargetDatapathId(
    uint64 node_id, uint32 port_id, TargetDatapathId* target_dp_id) {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }

  ASSIGN_OR_RETURN(auto sdk_port_id, GetSdkPortId(node_id, port_id));
  ASSIGN_OR_RETURN(auto device, GetDeviceFromNodeId(node_id));
  return port_manager_->GetPortInfo(device, sdk_port_id, target_dp_id);
}

::util::StatusOr<DataResponse> DpdkChassisManager::GetPortData(
    const DataRequest::Request& request) {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  DataResponse resp;
  using Request = DataRequest::Request;
  switch (request.request_case()) {
    case Request::kOperStatus: {
      ASSIGN_OR_RETURN(auto port_state,
                       GetPortState(request.oper_status().node_id(),
                                    request.oper_status().port_id()));
      resp.mutable_oper_status()->set_state(port_state);
      ASSIGN_OR_RETURN(absl::Time last_changed,
                       GetPortTimeLastChanged(request.oper_status().node_id(),
                                              request.oper_status().port_id()));
      resp.mutable_oper_status()->set_time_last_changed(
          absl::ToUnixNanos(last_changed));
      break;
    }
    case Request::kAdminStatus: {
      ASSIGN_OR_RETURN(auto* config,
                       GetPortConfig(request.admin_status().node_id(),
                                     request.admin_status().port_id()));
      resp.mutable_admin_status()->set_state(config->admin_state);
      break;
    }
    case Request::kMacAddress: {
      // TODO(unknown) Find out why the controller needs it.
      // Find MAC address of port located at:
      // - node_id: req.mac_address().node_id()
      // - port_id: req.mac_address().port_id()
      // and then write it into the response.
      resp.mutable_mac_address()->set_mac_address(kDummyMacAddress);
      break;
    }
    case DataRequest::Request::kLacpRouterMac: {
      // Find LACP System ID MAC address of port located at:
      // - node_id: req.lacp_router_mac().node_id()
      // - port_id: req.lacp_router_mac().port_id()
      // and then write it into the response.
      resp.mutable_lacp_router_mac()->set_mac_address(kDummyMacAddress);
      break;
    }
    case Request::kPortCounters: {
      RETURN_IF_ERROR(GetPortCounters(request.port_counters().node_id(),
                                      request.port_counters().port_id(),
                                      resp.mutable_port_counters()));
      break;
    }
    case Request::kPortSpeed:
    case Request::kNegotiatedPortSpeed:
    case Request::kAutonegStatus:
    case Request::kFrontPanelPortInfo:
    case Request::kFecStatus:
    case Request::kLoopbackStatus: {
      return MAKE_ERROR(ERR_INTERNAL) << "Attribute not supported by DPDK";
    }
    case Request::kSdnPortId: {
      ASSIGN_OR_RETURN(auto sdk_port_id,
                       GetSdkPortId(request.sdn_port_id().node_id(),
                                    request.sdn_port_id().port_id()));
      resp.mutable_sdn_port_id()->set_port_id(sdk_port_id);
      break;
    }
    case Request::kTargetDpId: {
      RETURN_IF_ERROR(GetTargetDatapathId(request.target_dp_id().node_id(),
                                          request.target_dp_id().port_id(),
                                          resp.mutable_target_dp_id()));
      break;
    }
    case Request::kForwardingViability: {
      // Find current port forwarding viable state for port located at:
      // - node_id: req.forwarding_viable().node_id()
      // - port_id: req.forwarding_viable().port_id()
      // and then write it into the response.
      resp.mutable_forwarding_viability()->set_state(
          TRUNK_MEMBER_BLOCK_STATE_UNKNOWN);
      break;
    }
    case DataRequest::Request::kHealthIndicator: {
      // Find current port health indicator (LED) for port located at:
      // - node_id: req.health_indicator().node_id()
      // - port_id: req.health_indicator().port_id()
      // and then write it into the response.
      resp.mutable_health_indicator()->set_state(HEALTH_STATE_UNKNOWN);
      break;
    }
    default:
      return MAKE_ERROR(ERR_INTERNAL) << "Not supported yet";
  }
  return resp;
}

::util::StatusOr<PortState> DpdkChassisManager::GetPortState(
    uint64 node_id, uint32 port_id) const {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  ASSIGN_OR_RETURN(auto device, GetDeviceFromNodeId(node_id));

  auto* port_id_to_port_state =
      gtl::FindOrNull(node_id_to_port_id_to_port_state_, node_id);
  RET_CHECK(port_id_to_port_state != nullptr)
      << "Node " << node_id << " is not configured or not known.";
  const PortState* port_state_ptr =
      gtl::FindOrNull(*port_id_to_port_state, port_id);
  // TODO(antonin): Once we implement PushChassisConfig, port_state_ptr should
  // never be NULL
  if (port_state_ptr != nullptr && *port_state_ptr != PORT_STATE_UNKNOWN) {
    return *port_state_ptr;
  }

  // If state is unknown, query the state
  LOG(INFO) << "Querying state of port " << port_id << " in node " << node_id
            << ".";
  ASSIGN_OR_RETURN(auto sdk_port_id, GetSdkPortId(node_id, port_id));
  ASSIGN_OR_RETURN(auto port_state,
                   port_manager_->GetPortState(device, sdk_port_id));
  LOG(INFO) << "State of port " << port_id << " in node " << node_id
            << " (SDK port " << sdk_port_id
            << "): " << PrintPortState(port_state);
  return port_state;
}

::util::StatusOr<absl::Time> DpdkChassisManager::GetPortTimeLastChanged(
    uint64 node_id, uint32 port_id) {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }

  RET_CHECK(node_id_to_port_id_to_time_last_changed_.count(node_id));
  RET_CHECK(node_id_to_port_id_to_time_last_changed_[node_id].count(port_id));
  return node_id_to_port_id_to_time_last_changed_[node_id][port_id];
}

::util::Status DpdkChassisManager::GetPortCounters(uint64 node_id,
                                                   uint32 port_id,
                                                   PortCounters* counters) {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  ASSIGN_OR_RETURN(auto device, GetDeviceFromNodeId(node_id));
  ASSIGN_OR_RETURN(auto sdk_port_id, GetSdkPortId(node_id, port_id));
  return port_manager_->GetPortCounters(device, sdk_port_id, counters);
}

::util::StatusOr<std::map<uint64, int>>
DpdkChassisManager::GetNodeIdToDeviceMap() const {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  return node_id_to_device_;
}

::util::Status DpdkChassisManager::ReplayChassisConfig(uint64 node_id) {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  ASSIGN_OR_RETURN(auto device, GetDeviceFromNodeId(node_id));

  for (auto& p : node_id_to_port_id_to_port_state_[node_id])
    p.second = PORT_STATE_UNKNOWN;

  for (auto& p : node_id_to_port_id_to_time_last_changed_[node_id]) {
    p.second = absl::UnixEpoch();
  }

  LOG(INFO) << "Replaying ports for node " << node_id << ".";

  auto replay_one_port = [node_id, device, this](
                             uint32 port_id, const DpdkPortConfig& config,
                             DpdkPortConfig* config_new) -> ::util::Status {
    VLOG(1) << "Replaying port " << port_id << " in node " << node_id << ".";

    if (config.admin_state == ADMIN_STATE_UNKNOWN) {
      LOG(WARNING) << "Port " << port_id << " in node " << node_id
                   << " was not configured properly, so skipping replay.";
      return ::util::OkStatus();
    }

    ASSIGN_OR_RETURN(auto sdk_port_id, GetSdkPortId(node_id, port_id));
    // NOTE: This is the legacy AddPort method. DPDK does not support it.
    RETURN_IF_ERROR(port_manager_->AddPort(
        device, sdk_port_id, SPEED_BPS_UNKNOWN, FEC_MODE_UNKNOWN));
    config_new->admin_state = ADMIN_STATE_DISABLED;

    if (config.admin_state == ADMIN_STATE_ENABLED) {
      VLOG(1) << "Enabling port " << port_id << " in node " << node_id
              << " (SDK port " << sdk_port_id << ").";
      config_new->admin_state = ADMIN_STATE_ENABLED;
    }

    return ::util::OkStatus();
  };

  ::util::Status status = ::util::OkStatus();  // errors to keep track of.

  for (auto& p : node_id_to_port_id_to_port_config_[node_id]) {
    uint32 port_id = p.first;
    DpdkPortConfig config_new;
    APPEND_STATUS_IF_ERROR(status,
                           replay_one_port(port_id, p.second, &config_new));
    p.second = config_new;
  }

  return status;
}

std::unique_ptr<DpdkChassisManager> DpdkChassisManager::CreateInstance(
    OperationMode mode, DpdkPortManager* port_manager) {
  return absl::WrapUnique(new DpdkChassisManager(mode, port_manager));
}

::util::StatusOr<int> DpdkChassisManager::GetDeviceFromNodeId(
    uint64 node_id) const {
  if (!initialized_) {
    return MAKE_ERROR(ERR_NOT_INITIALIZED) << "Not initialized!";
  }
  const int* device = gtl::FindOrNull(node_id_to_device_, node_id);
  RET_CHECK(device != nullptr)
      << "Node " << node_id << " is not configured or not known.";

  return *device;
}

void DpdkChassisManager::CleanupInternalState() {
  device_to_node_id_.clear();
  node_id_to_device_.clear();
  node_id_to_port_id_to_port_state_.clear();
  node_id_to_port_id_to_time_last_changed_.clear();
  node_id_to_port_id_to_port_config_.clear();
  node_id_to_port_id_to_singleton_port_key_.clear();
  node_id_to_port_id_to_sdk_port_id_.clear();
  node_id_to_sdk_port_id_to_port_id_.clear();
}

::util::Status DpdkChassisManager::Shutdown() {
  ::util::Status status = ::util::OkStatus();
  {
    absl::ReaderMutexLock l(&chassis_lock);
    if (!initialized_) return status;
  }
  // It is fine to release the chassis lock here (it is actually needed to call
  // UnregisterEventWriters or there would be a deadlock). Because initialized_
  // is set to true, RegisterEventWriters cannot be called.
  absl::WriterMutexLock l(&chassis_lock);
  initialized_ = false;
  CleanupInternalState();
  return status;
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
