// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// DPDK-specific port manager methods.

#include "stratum/hal/lib/tdi/dpdk/dpdk_port_manager.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "absl/synchronization/mutex.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/tdi/tdi_port_manager.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/lib/channel/channel.h"

extern "C" {
#include "bf_pal/bf_pal_port_intf.h"
}

namespace stratum {
namespace hal {
namespace tdi {

DpdkPortManager* DpdkPortManager::singleton_ = nullptr;

DpdkPortManager* DpdkPortManager::CreateSingleton() {
  absl::WriterMutexLock l(&init_lock_);
  if (!singleton_) {
    singleton_ = new DpdkPortManager();
  }

  return singleton_;
}

DpdkPortManager* DpdkPortManager::GetSingleton() {
  absl::ReaderMutexLock l(&init_lock_);
  return singleton_;
}

::util::StatusOr<PortState> DpdkPortManager::GetPortState(int device,
                                                          int port) {
  return PORT_STATE_UP;
}

::util::Status DpdkPortManager::GetPortCounters(int device, int port,
                                                PortCounters* counters) {
  uint64_t stats[BF_PORT_NUM_COUNTERS] = {0};
  RETURN_IF_TDI_ERROR(
      bf_pal_port_all_stats_get(static_cast<bf_dev_id_t>(device),
                                static_cast<bf_dev_port_t>(port), stats));
  counters->set_in_octets(stats[RX_BYTES]);
  counters->set_out_octets(stats[TX_BYTES]);
  counters->set_in_unicast_pkts(stats[RX_PACKETS]);
  counters->set_out_unicast_pkts(stats[TX_PACKETS]);
  counters->set_in_broadcast_pkts(stats[RX_BROADCAST]);
  counters->set_out_broadcast_pkts(stats[TX_BROADCAST]);
  counters->set_in_multicast_pkts(stats[RX_MULTICAST]);
  counters->set_out_multicast_pkts(stats[TX_MULTICAST]);
  counters->set_in_discards(stats[RX_DISCARDS]);
  counters->set_out_discards(stats[TX_DISCARDS]);
  counters->set_in_unknown_protos(0);  // stat not meaningful
  counters->set_in_errors(stats[RX_ERRORS]);
  counters->set_out_errors(stats[TX_ERRORS]);
  counters->set_in_fcs_errors(0);

  return ::util::OkStatus();
}

namespace {
dpdk_port_type_t get_target_port_type(DpdkPortType type) {
  switch (type) {
    case PORT_TYPE_VHOST:
      return BF_DPDK_LINK;
    case PORT_TYPE_TAP:
      return BF_DPDK_TAP;
    case PORT_TYPE_LINK:
      return BF_DPDK_LINK;
    case PORT_TYPE_SOURCE:
      return BF_DPDK_SOURCE;
    case PORT_TYPE_SINK:
      return BF_DPDK_SINK;
    default:
      break;
  }
  return BF_DPDK_PORT_MAX;
}
}  // namespace

::util::Status DpdkPortManager::GetPortInfo(int device, int port,
                                            TargetDatapathId* target_dp_id) {
  struct port_info_t* port_info = NULL;
  RETURN_IF_TDI_ERROR(bf_pal_port_info_get(static_cast<bf_dev_id_t>(device),
                                           static_cast<bf_dev_port_t>(port),
                                           &port_info));
  target_dp_id->set_tdi_portin_id((port_info)->port_attrib.port_in_id);
  target_dp_id->set_tdi_portout_id((port_info)->port_attrib.port_out_id);

  return ::util::OkStatus();
}

::util::Status DpdkPortManager::HotplugPort(
    int device, int port, HotplugConfigParams& hotplug_config) {
  auto hotplug_attrs = absl::make_unique<hotplug_attributes_t>();
  strncpy(hotplug_attrs->qemu_socket_ip, hotplug_config.qemu_socket_ip.c_str(),
          sizeof(hotplug_attrs->qemu_socket_ip));
  strncpy(hotplug_attrs->qemu_vm_netdev_id,
          hotplug_config.qemu_vm_netdev_id.c_str(),
          sizeof(hotplug_attrs->qemu_vm_netdev_id));
  strncpy(hotplug_attrs->qemu_vm_chardev_id,
          hotplug_config.qemu_vm_chardev_id.c_str(),
          sizeof(hotplug_attrs->qemu_vm_chardev_id));
  strncpy(hotplug_attrs->qemu_vm_device_id,
          hotplug_config.qemu_vm_device_id.c_str(),
          sizeof(hotplug_attrs->qemu_vm_device_id));
  strncpy(hotplug_attrs->native_socket_path,
          hotplug_config.native_socket_path.c_str(),
          sizeof(hotplug_attrs->native_socket_path));
  // Convert enum to Boolean (NONE == false, ADD or DEL == true)
  hotplug_attrs->qemu_hotplug = (hotplug_config.qemu_hotplug_mode != 0);
  hotplug_attrs->qemu_socket_port = hotplug_config.qemu_socket_port;
  uint64 mac_address = hotplug_config.qemu_vm_mac_address;

  std::string string_mac =
      (absl::StrFormat("%02x:%02x:%02x:%02x:%02x:%02x",
                       (mac_address >> 40) & 0xFF, (mac_address >> 32) & 0xFF,
                       (mac_address >> 24) & 0xFF, (mac_address >> 16) & 0xFF,
                       (mac_address >> 8) & 0xFF, mac_address & 0xFF));
  strcpy(hotplug_attrs->qemu_vm_mac_address, string_mac.c_str());

  LOG(INFO) << "Parameters for hotplug are:"
            << " qemu_socket_port=" << hotplug_attrs->qemu_socket_port
            << " qemu_vm_mac_address=" << hotplug_attrs->qemu_vm_mac_address
            << " qemu_socket_ip=" << hotplug_attrs->qemu_socket_ip
            << " qemu_vm_netdev_id=" << hotplug_attrs->qemu_vm_netdev_id
            << " qemu_vm_chardev_id=" << hotplug_attrs->qemu_vm_chardev_id
            << " qemu_vm_device_id=" << hotplug_attrs->qemu_vm_device_id
            << " native_socket_path=" << hotplug_attrs->native_socket_path
            << " qemu_hotplug=" << hotplug_attrs->qemu_hotplug;

  if (hotplug_config.qemu_hotplug_mode == HOTPLUG_MODE_ADD) {
    RETURN_IF_TDI_ERROR(bf_pal_hotplug_add(static_cast<bf_dev_id_t>(device),
                                           static_cast<bf_dev_port_t>(port),
                                           hotplug_attrs.get()));
  } else if (hotplug_config.qemu_hotplug_mode == HOTPLUG_MODE_DEL) {
    RETURN_IF_TDI_ERROR(bf_pal_hotplug_del(static_cast<bf_dev_id_t>(device),
                                           static_cast<bf_dev_port_t>(port),
                                           hotplug_attrs.get()));
  }

  return ::util::OkStatus();
}

::util::Status DpdkPortManager::AddPort(int device, int port) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "AddPort(device, port) not supported";
}

::util::Status DpdkPortManager::AddPort(int device, int port, uint64 speed_bps,
                                        FecMode fec_mode) {
  return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED)
         << "AddPort(device, port, speed, fec_mode) not supported";
}

::util::Status DpdkPortManager::AddPort(int device, int port,
                                        const PortConfigParams& config) {
  static int port_in;
  static int port_out;

  auto port_attrs = absl::make_unique<port_attributes_t>();
  strncpy(port_attrs->port_name, config.port_name.c_str(),
          sizeof(port_attrs->port_name));
  strncpy(port_attrs->pipe_in, config.pipeline_name.c_str(),
          sizeof(port_attrs->pipe_in));
  strncpy(port_attrs->pipe_out, config.pipeline_name.c_str(),
          sizeof(port_attrs->pipe_out));
  strncpy(port_attrs->mempool_name, config.mempool_name.c_str(),
          sizeof(port_attrs->mempool_name));
  port_attrs->port_type = get_target_port_type(config.port_type);
  port_attrs->port_dir = PM_PORT_DIR_DEFAULT;
  port_attrs->port_in_id = port_in++;
  port_attrs->port_out_id = port_out++;
  port_attrs->net_port = config.packet_dir;

  LOG(INFO) << "Parameters for DPDK are:"
            << " port_name=" << port_attrs->port_name
            << " port_type=" << port_attrs->port_type
            << " port_in_id=" << port_attrs->port_in_id
            << " port_out_id=" << port_attrs->port_out_id
            << " pipeline_in_name=" << port_attrs->pipe_in
            << " pipeline_out_name=" << port_attrs->pipe_out
            << " mempool_name=" << port_attrs->mempool_name
            << " net_port=" << port_attrs->net_port << " sdk_port_id= " << port;

  if (port_attrs->port_type == BF_DPDK_LINK) {
    // Update LINK parameters
    if (config.port_type == PORT_TYPE_VHOST) {
      port_attrs->link.dev_hotplug_enabled = 1;
      strncpy(port_attrs->link.pcie_domain_bdf, config.port_name.c_str(),
              sizeof(port_attrs->link.pcie_domain_bdf));
      snprintf(port_attrs->link.dev_args, DEV_ARGS_LEN, "iface=%s,queues=%d",
               config.socket_path.c_str(), config.queues);
    } else {
      strncpy(port_attrs->link.pcie_domain_bdf, config.pci_bdf.c_str(),
              sizeof(port_attrs->link.pcie_domain_bdf));
    }
    LOG(INFO) << "LINK Parameters of the port are "
              << " pcie_domain_bdf=" << port_attrs->link.pcie_domain_bdf
              << " dev_args=" << port_attrs->link.dev_args;
  } else if (port_attrs->port_type == BF_DPDK_TAP) {
    port_attrs->tap.mtu = config.mtu;
    LOG(INFO) << "TAP Parameters of the port are "
              << "mtu = " << port_attrs->tap.mtu;
  }

  auto bf_status =
      bf_pal_port_add(static_cast<bf_dev_id_t>(device),
                      static_cast<bf_dev_port_t>(port), port_attrs.get());
  if (bf_status != BF_SUCCESS) {
    // Revert the port_in and port_out values
    port_in--;
    port_out--;
    RETURN_IF_TDI_ERROR(bf_status);
  }

  return ::util::OkStatus();
}

::util::Status DpdkPortManager::DeletePort(int device, int port) {
  RETURN_IF_TDI_ERROR(bf_pal_port_del(static_cast<bf_dev_id_t>(device),
                                      static_cast<bf_dev_port_t>(port)));
  return ::util::OkStatus();
}

::util::Status DpdkPortManager::EnablePort(int device, int port) {
  return MAKE_ERROR(ERR_UNIMPLEMENTED) << "EnablePort not implemented";
}

::util::Status DpdkPortManager::DisablePort(int device, int port) {
  return MAKE_ERROR(ERR_UNIMPLEMENTED) << "DisablePort not implemented";
}

// Should this return ::util::StatusOr<bool>?
bool DpdkPortManager::IsValidPort(int device, int port) {
  // NOTE: Method returns bool. What is BF_SUCCESS (an enum) doing here?
  // Is the method supposed to succeed or fail? The name suggests
  // that it is supposed to succeed, but BF_SUCCESS == 0, which when
  // converted to a Boolean is FALSE, so it is actually failure.
  return BF_SUCCESS;
}

::util::StatusOr<uint32> DpdkPortManager::GetPortIdFromPortKey(
    int device, const PortKey& port_key) {
  const int port = port_key.port;
  RET_CHECK(port >= 0) << "Port ID must be non-negative. Attempted to get port "
                       << port << " on dev " << device << ".";

  // PortKey uses three possible values for channel:
  //     > 0: port is channelized (first channel is 1)
  //     0: port is not channelized
  //     < 0: port channel is not important (e.g. for port groups)
  // BF SDK expects the first channel to be 0
  //     Convert base-1 channel to base-0 channel if port is channelized
  //     Otherwise, port is already 0 in the non-channelized case
  const int channel =
      (port_key.channel > 0) ? port_key.channel - 1 : port_key.channel;
  RET_CHECK(channel >= 0) << "Channel must be set for port " << port
                          << " on dev " << device << ".";

  char port_string[MAX_PORT_HDL_STRING_LEN];
  int r = snprintf(port_string, sizeof(port_string), "%d/%d", port, channel);
  RET_CHECK(r > 0 && r < sizeof(port_string))
      << "Failed to build port string for port " << port << " channel "
      << channel << " on dev " << device << ".";

  bf_dev_port_t dev_port;
  RETURN_IF_TDI_ERROR(bf_pal_port_str_to_dev_port_map(
      static_cast<bf_dev_id_t>(device), port_string, &dev_port));
  return static_cast<uint32>(dev_port);
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
