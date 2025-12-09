// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_DPDK_PORT_MANAGER_H_
#define STRATUM_HAL_LIB_TDI_DPDK_PORT_MANAGER_H_

#include <algorithm>
#include <memory>
#include <string>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/tdi/tdi_port_manager.h"
#include "stratum/lib/channel/channel.h"

// Suppress clang errors
#undef LOCKS_EXCLUDED
#define LOCKS_EXCLUDED(...)

namespace stratum {
namespace hal {
namespace tdi {

class DpdkPortManager : public TdiPortManager {
 public:
  struct PortConfigParams {
    DpdkPortType port_type;
    DpdkDeviceType device_type;
    PacketDirection packet_dir;
    int queues;
    int mtu;
    std::string socket_path;
    std::string host_name;
    std::string port_name;
    std::string pipeline_name;
    std::string mempool_name;
    std::string pci_bdf;
  };

  struct HotplugConfigParams {
    uint32 qemu_socket_port;
    uint64 qemu_vm_mac_address;
    std::string qemu_socket_ip;
    std::string qemu_vm_netdev_id;
    std::string qemu_vm_chardev_id;
    std::string qemu_vm_device_id;
    std::string native_socket_path;
    QemuHotplugMode qemu_hotplug_mode;
  };

  DpdkPortManager() {}
  virtual ~DpdkPortManager() {}

  // ---------- Common public methods ----------

  ::util::Status GetPortInfo(int device, int port,
                             TargetDatapathId* target_dp_id);

  ::util::StatusOr<PortState> GetPortState(int device, int port);

  ::util::Status GetPortCounters(int device, int port, PortCounters* counters);

  ::util::StatusOr<uint32> GetPortIdFromPortKey(int device,
                                                const PortKey& port_key);

  bool IsValidPort(int device, int port);

  ::util::Status AddPort(int device, int port);

  ::util::Status DeletePort(int device, int port);

  ::util::Status EnablePort(int device, int port);

  ::util::Status DisablePort(int device, int port);

  // ---------- DPDK-specific public methods ----------

  // Legacy method (returns NOT_SUPPORTED).
  virtual ::util::Status AddPort(int device, int port, uint64 speed_bps,
                                 FecMode fec_mode);

  // Adds a new port with the given parameters.
  virtual ::util::Status AddPort(int device, int port,
                                 const PortConfigParams& config);

  // Adds/deletes a hotplug port.
  virtual ::util::Status HotplugPort(int device, int port,
                                     HotplugConfigParams& hotplug_config);

  // ---------- Factory methods ----------

  // Creates the singleton instance. Expected to be called once to initialize
  // the instance.
  static DpdkPortManager* CreateSingleton() LOCKS_EXCLUDED(init_lock_);

  // ---------- Callback methods ---------

  // The following public functions are specific to this class. They are to be
  // called by SDE callbacks only.

  // Return the singleton instance to be used in the SDE callbacks.
  static DpdkPortManager* GetSingleton() LOCKS_EXCLUDED(init_lock_);

  // Called whenever a port status event is received from SDK. It forwards the
  // port status event to the module who registered a callback by calling
  // RegisterPortStatusEventWriter().
  ::util::Status OnPortStatusEvent(int device, int dev_port, bool up,
                                   absl::Time timestamp)
      LOCKS_EXCLUDED(port_status_event_writer_lock_);

 protected:
  // The singleton instance.
  static DpdkPortManager* singleton_ GUARDED_BY(init_lock_);
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_DPDK_PORT_MANAGER_H_
