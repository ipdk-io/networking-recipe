// Copyright 2018-present Barefoot Networks, Inc.
// Copyright 2021-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_DPDK_DPDK_CHASSIS_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_DPDK_DPDK_CHASSIS_MANAGER_H_

#include <map>
#include <memory>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/gnmi_events.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/lib/channel/channel.h"

namespace stratum {
namespace hal {
namespace tdi {

class DpdkPortConfig;
class DpdkPortManager;

// Lock which protects chassis state across the entire switch.
extern absl::Mutex chassis_lock;

class DpdkChassisManager {
 public:
  virtual ~DpdkChassisManager();

  virtual ::util::Status PushChassisConfig(const ChassisConfig& config)
      EXCLUSIVE_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::Status VerifyChassisConfig(const ChassisConfig& config)
      SHARED_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::Status Shutdown() LOCKS_EXCLUDED(chassis_lock);

  virtual ::util::Status RegisterEventNotifyWriter(
      const std::shared_ptr<WriterInterface<GnmiEventPtr>>& writer)
      LOCKS_EXCLUDED(gnmi_event_lock_);

  virtual ::util::Status UnregisterEventNotifyWriter()
      LOCKS_EXCLUDED(gnmi_event_lock_);

  virtual ::util::StatusOr<DataResponse> GetPortData(
      const DataRequest::Request& request) SHARED_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::StatusOr<absl::Time> GetPortTimeLastChanged(uint64 node_id,
                                                              uint32 port_id)
      SHARED_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::Status GetPortCounters(uint64 node_id, uint32 port_id,
                                         PortCounters* counters)
      SHARED_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::Status ReplayChassisConfig(uint64 node_id)
      EXCLUSIVE_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::StatusOr<std::map<uint64, int>> GetNodeIdToDeviceMap() const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  virtual ::util::StatusOr<int> GetDeviceFromNodeId(uint64 node_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Factory function for creating the instance of the class.
  static std::unique_ptr<DpdkChassisManager> CreateInstance(
      OperationMode mode, DpdkPortManager* port_manager);

  // Determines whether the specified port configuration parameter has
  // already been set. Once set, it may not be set again.
  bool IsPortParamSet(uint64 node_id, uint32 port_id,
                      SetRequest::Request::Port::ValueCase value_case);

  // Sets the value of a port configuration parameter.
  // Once set, it may not be set again.
  ::util::Status SetPortParam(uint64 node_id, uint32 port_id,
                              const SingletonPort& singleton_port,
                              SetRequest::Request::Port::ValueCase value_case);

  // Sets the value of a hotplug configuration parameter.
  ::util::Status SetHotplugParam(uint64 node_id, uint32 port_id,
                                 const SingletonPort& singleton_port,
                                 DpdkHotplugParam param_type);

  // DpdkChassisManager is neither copyable nor movable.
  DpdkChassisManager(const DpdkChassisManager&) = delete;
  DpdkChassisManager& operator=(const DpdkChassisManager&) = delete;
  DpdkChassisManager(DpdkChassisManager&&) = delete;
  DpdkChassisManager& operator=(DpdkChassisManager&&) = delete;

 protected:
  // Default constructor. To be called by the Mock class instance only.
  DpdkChassisManager();

 private:
  // ReaderArgs encapsulates the arguments for a Channel reader thread.
  template <typename T>
  struct ReaderArgs {
    DpdkChassisManager* manager;
    std::unique_ptr<ChannelReader<T>> reader;
  };

  // Maximum depth of port status change event channel.
  static constexpr int kMaxPortStatusEventDepth = 1024;
  static constexpr int kMaxXcvrEventDepth = 1024;

  // Private constructor. Use CreateInstance() to create an instance of this
  // class.
  DpdkChassisManager(OperationMode mode, DpdkPortManager* port_manager);

  ::util::StatusOr<const DpdkPortConfig*> GetPortConfig(uint64 node_id,
                                                        uint32 port_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Returns the state of a port given its ID and the ID of its node.
  ::util::StatusOr<PortState> GetPortState(uint64 node_id, uint32 port_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Returns the SDK port number for the given port. Also called SDN or data
  // plane port.
  ::util::StatusOr<uint32> GetSdkPortId(uint64 node_id, uint32 port_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Returns the port in id and port out id required to configure pipeline
  ::util::Status GetTargetDatapathId(uint64 node_id, uint32 port_id,
                                     TargetDatapathId* target_dp_id)
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Cleans up the internal state. Resets all the internal port maps and
  // deletes the pointers.
  void CleanupInternalState() EXCLUSIVE_LOCKS_REQUIRED(chassis_lock);

  // helper to add / configure / enable a port with DpdkPortManager
  ::util::Status AddPortHelper(uint64 node_id, int device, uint32 port_id,
                               const SingletonPort& singleton_port,
                               DpdkPortConfig* config);

  // helper to hotplug add / delete a port with DpdkPortManager
  ::util::Status HotplugPortHelper(uint64 node_id, int device, uint32 port_id,
                                   const SingletonPort& singleton_port,
                                   DpdkPortConfig* config);

  // helper to update port configuration with DpdkPortManager
  ::util::Status UpdatePortHelper(uint64 node_id, int device, uint32 port_id,
                                  const SingletonPort& singleton_port,
                                  const DpdkPortConfig& config_old,
                                  DpdkPortConfig* config);

  // Determines the mode of operation:
  // - OPERATION_MODE_STANDALONE: when Stratum stack runs independently and
  // therefore needs to do all the SDK initialization itself.
  // - OPERATION_MODE_COUPLED: when Stratum stack runs as part of Sandcastle
  // stack, coupled with the rest of stack processes.
  // - OPERATION_MODE_SIM: when Stratum stack runs in simulation mode.
  // Note that this variable is set upon initialization and is never changed
  // afterwards.
  OperationMode mode_;

  bool initialized_ GUARDED_BY(chassis_lock);

  // WriterInterface<GnmiEventPtr> object for sending event notifications.
  mutable absl::Mutex gnmi_event_lock_;
  std::shared_ptr<WriterInterface<GnmiEventPtr>> gnmi_event_writer_
      GUARDED_BY(gnmi_event_lock_);

  // Map from device number to the node ID as specified by the config.
  std::map<int, uint64> device_to_node_id_ GUARDED_BY(chassis_lock);

  // Map from node ID to device number.
  std::map<uint64, int> node_id_to_device_ GUARDED_BY(chassis_lock);

  // Map from node ID to another map from port ID to PortState representing
  // the state of the singleton port uniquely identified by (node ID, port ID).
  std::map<uint64, std::map<uint32, PortState>>
      node_id_to_port_id_to_port_state_ GUARDED_BY(chassis_lock);

  // Map from node ID to another map from port ID to timestamp when the port
  // last changed state.
  std::map<uint64, std::map<uint32, absl::Time>>
      node_id_to_port_id_to_time_last_changed_ GUARDED_BY(chassis_lock);

  // Map from node ID to another map from port ID to port configuration.
  // We may change this once missing "get" methods get added to TdiSdeInterface,
  // as we would be able to rely on TdiSdeInterface to query config parameters,
  // instead of maintaining a "consistent" view in this map.
  std::map<uint64, std::map<uint32, DpdkPortConfig>>
      node_id_to_port_id_to_port_config_ GUARDED_BY(chassis_lock);

  // Map from node ID to another map from port ID to PortKey corresponding
  // to the singleton port uniquely identified by (node ID, port ID). This map
  // is updated as part of each config push.
  std::map<uint64, std::map<uint32, PortKey>>
      node_id_to_port_id_to_singleton_port_key_ GUARDED_BY(chassis_lock);

  // Map from node ID to another map from (SDN) port ID to SDK port ID.
  // SDN port IDs are used in Stratum and by callers to P4Runtime and gNMI,
  // and SDK port IDs are used in calls to the BF SDK. This map is updated
  // as part of each config push.
  std::map<uint64, std::map<uint32, uint32>> node_id_to_port_id_to_sdk_port_id_
      GUARDED_BY(chassis_lock);

  // Map from node ID to another map from SDK port ID to (SDN) port ID.
  // This contains the inverse mapping of: node_id_to_port_id_to_sdk_port_id_
  // This map is updated as part of each config push.
  std::map<uint64, std::map<uint32, uint32>> node_id_to_sdk_port_id_to_port_id_
      GUARDED_BY(chassis_lock);

  // Pointer to the DpdkPortManager implementation.
  DpdkPortManager* port_manager_;  // not owned by this class.

  friend class DpdkChassisManagerTest;
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_DPDK_DPDK_CHASSIS_MANAGER_H_
