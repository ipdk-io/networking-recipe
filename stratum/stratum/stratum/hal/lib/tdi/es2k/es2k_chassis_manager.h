// Copyright 2018-present Barefoot Networks, Inc.
// Copyright 2022-2023,2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_ES2K_CHASSIS_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_ES2K_CHASSIS_MANAGER_H_

#include <map>
#include <memory>

#include "stratum/glue/thread_annotations_compat.h"
#include "absl/memory/memory.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "stratum/glue/integral_types.h"
#include "stratum/hal/lib/common/gnmi_events.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/hal/lib/common/writer_interface.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/hal/lib/tdi/tdi_port_manager.h"
#include "stratum/lib/channel/channel.h"

namespace stratum {
namespace hal {
namespace tdi {

class Es2kPortManager;
class Es2kVirtualPortManager;

// Lock which protects chassis state across the entire switch.
extern absl::Mutex chassis_lock;

class Es2kChassisManager {
 public:
  virtual ~Es2kChassisManager();

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

  virtual ::util::StatusOr<DataResponse> GetVirtualPortData(
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
  static std::unique_ptr<Es2kChassisManager> CreateInstance(
      OperationMode mode, Es2kPortManager* es2k_port_manager,
      Es2kVirtualPortManager* es2k_virtual_port_manager);

  // Es2kChassisManager is neither copyable nor movable.
  Es2kChassisManager(const Es2kChassisManager&) = delete;
  Es2kChassisManager& operator=(const Es2kChassisManager&) = delete;
  Es2kChassisManager(Es2kChassisManager&&) = delete;
  Es2kChassisManager& operator=(Es2kChassisManager&&) = delete;

 protected:
  // Default constructor. To be called by the Mock class instance only.
  Es2kChassisManager();

 private:
  // ReaderArgs encapsulates the arguments for a Channel reader thread.
  template <typename T>
  struct ReaderArgs {
    Es2kChassisManager* manager;
    std::unique_ptr<ChannelReader<T>> reader;
  };

  struct PortConfig {
    // ADMIN_STATE_UNKNOWN indicate that something went wrong during the port
    // configuration, and the port add wasn't event attempted or failed.
    AdminState admin_state;
    absl::optional<uint64> speed_bps;  // empty if port add failed
    absl::optional<int32> mtu;         // empty if MTU configuration failed
    absl::optional<TriState> autoneg;  // empty if Autoneg configuration failed
    absl::optional<FecMode> fec_mode;  // empty if port add failed
    // empty if loopback mode configuration failed
    absl::optional<LoopbackState> loopback_mode;

    PortConfig() : admin_state(ADMIN_STATE_UNKNOWN) {}
  };

  // Maximum depth of port status change event channel.
  static constexpr int kMaxPortStatusEventDepth = 1024;
  static constexpr int kMaxXcvrEventDepth = 1024;

  // Private constructor. Use CreateInstance() to create an instance of this
  // class.
  Es2kChassisManager(OperationMode mode, Es2kPortManager* es2k_port_manager,
                     Es2kVirtualPortManager* es2k_virtual_port_manager);

  ::util::StatusOr<const PortConfig*> GetPortConfig(uint64 node_id,
                                                    uint32 port_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Returns the state of a port given its ID and the ID of its node.
  ::util::StatusOr<PortState> GetPortState(uint64 node_id, uint32 port_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Returns the SDK port number for the given port. Also called SDN or data
  // plane port.
  ::util::StatusOr<uint32> GetSdkPortId(uint64 node_id, uint32 port_id) const
      SHARED_LOCKS_REQUIRED(chassis_lock);

  // Cleans up the internal state. Resets all the internal port maps and
  // deletes the pointers.
  void CleanupInternalState() EXCLUSIVE_LOCKS_REQUIRED(chassis_lock);

  // Forward PortStatus changed events through the appropriate node's registered
  // ChannelWriter<GnmiEventPtr> object.
  void SendPortOperStateGnmiEvent(uint64 node_id, uint32 port_id,
                                  PortState new_state,
                                  absl::Time time_last_changed)
      LOCKS_EXCLUDED(gnmi_event_lock_);

  // Transceiver module insert/removal event handler. This method is executed by
  // a ChannelReader thread which processes transceiver module insert/removal
  // events. Port is the 1-based frontpanel port number.
  // NOTE: This method should never be executed directly from a context which
  // first accesses the internal structures of a class below Es2kChassisManager
  // as this may result in deadlock.
  void TransceiverEventHandler(int slot, int port, HwState new_state)
      LOCKS_EXCLUDED(chassis_lock);

  // Thread function for reading transceiver events from xcvr_event_channel_.
  // Invoked with "this" as the argument in pthread_create.
  static void* TransceiverEventHandlerThreadFunc(void* arg)
      LOCKS_EXCLUDED(chassis_lock, gnmi_event_lock_);

  // Port status event handler. This method is executed by a ChannelReader
  // thread which processes SDE port status events. Port is the sdk port number
  // used by the SDE. NOTE: This method should never be executed directly from a
  // context which first accesses the internal structures of a class below
  // Es2kChassisManager as this may result in deadlock.
  void PortStatusEventHandler(int device, int port, PortState new_state,
                              absl::Time time_last_changed)
      LOCKS_EXCLUDED(chassis_lock);

  // Thread function for reading port status events from
  // port_status_event_channel_.
  static void* PortStatusEventHandlerThreadFunc(void* arg)
      LOCKS_EXCLUDED(chassis_lock);

  // Reads and processes port state events using the given ChannelReader. Called
  // by PortStatusEventHandlerThreadFunc.
  void ReadPortStatusEvents(
      const std::unique_ptr<ChannelReader<TdiPortManager::PortStatusEvent>>&
          reader) LOCKS_EXCLUDED(chassis_lock);

  // helper to add / configure / enable a port with Es2kPortManager
  ::util::Status AddPortHelper(uint64 node_id, int device, uint32 port_id,
                               const SingletonPort& singleton_port,
                               PortConfig* config);

  // helper to update port configuration with Es2kPortManager
  ::util::Status UpdatePortHelper(uint64 node_id, int device, uint32 port_id,
                                  const SingletonPort& singleton_port,
                                  const PortConfig& config_old,
                                  PortConfig* config);

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

  // Channel for receiving port status events from the Es2kPortManager.
  std::shared_ptr<Channel<TdiPortManager::PortStatusEvent>>
      port_status_event_channel_ GUARDED_BY(chassis_lock);

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
  std::map<uint64, std::map<uint32, PortConfig>>
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

  // Map from PortKey representing (slot, port) of a transceiver port to the
  // state of the transceiver module plugged into that (slot, port).
  std::map<PortKey, HwState> xcvr_port_key_to_xcvr_state_
      GUARDED_BY(chassis_lock);

  // Pointer to an Es2kPortManager implementation that wraps the SDE calls.
  Es2kPortManager* es2k_port_manager_;  // not owned by this class.

  // Pointer to an Es2kVirtualPortManager implementation that wraps the SDE
  // calls.
  Es2kVirtualPortManager*
      es2k_virtual_port_manager_;  // not owned by this class.

  friend class Es2kChassisManagerTest;
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_ES2K_CHASSIS_MANAGER_H_
