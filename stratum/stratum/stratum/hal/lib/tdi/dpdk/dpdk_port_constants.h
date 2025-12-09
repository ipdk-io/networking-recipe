// Copyright 2021-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_CONSTANTS_H_
#define STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_CONSTANTS_H_

#include <stdint.h>

namespace stratum {
namespace hal {
namespace tdi {

static inline constexpr uint32_t CONFIG_BIT(int n) { return 1U << n; }

constexpr uint32_t GNMI_CONFIG_PORT_TYPE = CONFIG_BIT(0);
constexpr uint32_t GNMI_CONFIG_DEVICE_TYPE = CONFIG_BIT(1);
constexpr uint32_t GNMI_CONFIG_QUEUE_COUNT = CONFIG_BIT(2);
constexpr uint32_t GNMI_CONFIG_SOCKET_PATH = CONFIG_BIT(3);
constexpr uint32_t GNMI_CONFIG_HOST_NAME = CONFIG_BIT(4);
constexpr uint32_t GNMI_CONFIG_PIPELINE_NAME = CONFIG_BIT(5);
constexpr uint32_t GNMI_CONFIG_MEMPOOL_NAME = CONFIG_BIT(6);
constexpr uint32_t GNMI_CONFIG_MTU_VALUE = CONFIG_BIT(7);

constexpr uint32_t GNMI_CONFIG_PCI_BDF_VALUE = CONFIG_BIT(8);
constexpr uint32_t GNMI_CONFIG_HOTPLUG_SOCKET_IP = CONFIG_BIT(9);
constexpr uint32_t GNMI_CONFIG_HOTPLUG_SOCKET_PORT = CONFIG_BIT(10);
constexpr uint32_t GNMI_CONFIG_HOTPLUG_MODE = CONFIG_BIT(11);
constexpr uint32_t GNMI_CONFIG_HOTPLUG_VM_MAC_ADDRESS = CONFIG_BIT(12);
constexpr uint32_t GNMI_CONFIG_HOTPLUG_VM_NETDEV_ID = CONFIG_BIT(13);
constexpr uint32_t GNMI_CONFIG_HOTPLUG_VM_CHARDEV_ID = CONFIG_BIT(14);
constexpr uint32_t GNMI_CONFIG_NATIVE_SOCKET_PATH = CONFIG_BIT(15);

constexpr uint32_t GNMI_CONFIG_HOTPLUG_VM_DEVICE_ID = CONFIG_BIT(16);
constexpr uint32_t GNMI_CONFIG_PACKET_DIR = CONFIG_BIT(17);

// Parameters that must be set for a VHOST port.
constexpr uint32_t GNMI_CONFIG_VHOST_REQUIRED =
    GNMI_CONFIG_PORT_TYPE | GNMI_CONFIG_DEVICE_TYPE | GNMI_CONFIG_QUEUE_COUNT |
    GNMI_CONFIG_SOCKET_PATH | GNMI_CONFIG_HOST_NAME;

// Parameters that must not be set for a VHOST port.
constexpr uint32_t GNMI_CONFIG_VHOST_UNSUPPORTED = GNMI_CONFIG_PCI_BDF_VALUE;

// Parameters that must be set for a LINK port.
constexpr uint32_t GNMI_CONFIG_LINK_REQUIRED =
    GNMI_CONFIG_PORT_TYPE | GNMI_CONFIG_PCI_BDF_VALUE;

// Parameters that must not be set for a LINK port.
constexpr uint32_t GNMI_CONFIG_LINK_UNSUPPORTED =
    GNMI_CONFIG_DEVICE_TYPE | GNMI_CONFIG_QUEUE_COUNT |
    GNMI_CONFIG_SOCKET_PATH | GNMI_CONFIG_HOST_NAME;

// Parameters that must be set for a TAP port.
constexpr uint32_t GNMI_CONFIG_TAP_REQUIRED = GNMI_CONFIG_PORT_TYPE;

// Parameters that must not be set for a TAP port.
constexpr uint32_t GNMI_CONFIG_TAP_UNSUPPORTED =
    GNMI_CONFIG_DEVICE_TYPE | GNMI_CONFIG_QUEUE_COUNT |
    GNMI_CONFIG_SOCKET_PATH | GNMI_CONFIG_HOST_NAME | GNMI_CONFIG_PCI_BDF_VALUE;

// Parameters that must be set for a HOTPLUG port.
constexpr uint32_t GNMI_CONFIG_HOTPLUG_REQUIRED =
    GNMI_CONFIG_HOTPLUG_SOCKET_IP | GNMI_CONFIG_HOTPLUG_SOCKET_PORT |
    GNMI_CONFIG_HOTPLUG_MODE | GNMI_CONFIG_HOTPLUG_VM_MAC_ADDRESS |
    GNMI_CONFIG_HOTPLUG_VM_NETDEV_ID | GNMI_CONFIG_HOTPLUG_VM_CHARDEV_ID |
    GNMI_CONFIG_NATIVE_SOCKET_PATH | GNMI_CONFIG_HOTPLUG_VM_DEVICE_ID;

constexpr char DEFAULT_PIPELINE[] = "pipe";
constexpr char DEFAULT_MEMPOOL[] = "MEMPOOL0";
constexpr int32 DEFAULT_MTU = 1500;
constexpr auto DEFAULT_PACKET_DIR = DIRECTION_HOST;
constexpr int32 MAX_MTU = 65535;

// SDK_PORT_CONTROL_BASE is used as an offset to define a reserved
// port range for the control ports.
constexpr uint32_t SDK_PORT_CONTROL_BASE = 256;

enum qemu_cmd_type {
  CHARDEV_ADD,
  NETDEV_ADD,
  DEVICE_ADD,
  CHARDEV_DEL,
  NETDEV_DEL,
  DEVICE_DEL
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_DPDK_DPDK_PORT_CONSTANTS_H_
