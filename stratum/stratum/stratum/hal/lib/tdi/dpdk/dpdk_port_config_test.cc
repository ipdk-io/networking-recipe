// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/dpdk/dpdk_port_config.h"

#include "gtest/gtest.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_constants.h"
#include "stratum/hal/lib/tdi/tdi_sde_interface.h"

namespace stratum {
namespace hal {
namespace tdi {

namespace {

TEST(DpdkConfigParamTest, Create) {
  DpdkPortConfig config;
  ASSERT_EQ(config.HasAnyOf(0xFFFFFFFF), false);
}

TEST(DpdkConfigParamTest, AddOne) {
  DpdkPortConfig config;
  config.Add(GNMI_CONFIG_PIPELINE_NAME);
  // Verify that bit was set
  ASSERT_EQ(config.HasAllOf(GNMI_CONFIG_PIPELINE_NAME), true);
  // Verify that no other bit was set
  ASSERT_EQ(config.HasAnyOf(~GNMI_CONFIG_PIPELINE_NAME), false);
}

TEST(DpdkConfigParamTest, HasAnyOf) {
  DpdkPortConfig config;
  config.Add(GNMI_CONFIG_VHOST_REQUIRED);
  ASSERT_EQ(config.HasAnyOf(GNMI_CONFIG_VHOST_REQUIRED), true);
}

TEST(DpdkConfigParamTest, HasAllOf) {
  DpdkPortConfig config;
  config.Add(GNMI_CONFIG_VHOST_REQUIRED);
  ASSERT_EQ(config.HasAllOf(GNMI_CONFIG_VHOST_REQUIRED), true);
}

TEST(DpdkConfigParamTest, Remove) {
  DpdkPortConfig config;
  config.Add(GNMI_CONFIG_VHOST_REQUIRED);
  config.Remove(GNMI_CONFIG_QUEUE_COUNT);
  EXPECT_EQ(config.HasAnyOf(GNMI_CONFIG_VHOST_REQUIRED), true);
  ASSERT_EQ(config.HasAnyOf(GNMI_CONFIG_QUEUE_COUNT), false);
}

TEST(DpdkConfigParamTest, Reset) {
  DpdkPortConfig config;
  config.Add(GNMI_CONFIG_VHOST_REQUIRED);
  EXPECT_EQ(config.HasAnyOf(GNMI_CONFIG_VHOST_REQUIRED), true);
  config.Reset();
  ASSERT_EQ(config.HasAnyOf(0xFFFFFFFF), false);
}

TEST(DpdkConfigParamTest, IsParamSet) {
  DpdkPortConfig config;
  config.Add(GNMI_CONFIG_MTU_VALUE);
  ASSERT_TRUE(config.IsParamSet(ValueCase::kMtuValue));
  ASSERT_FALSE(config.IsParamSet(ValueCase::kMempoolName));
}

TEST(DpdkConfigParamTest, SetPortParam) {
  DpdkPortConfig config;
  SingletonPort sport;
  PortConfigParams* port_params = sport.mutable_config_params();
  port_params->set_port_type(PORT_TYPE_VHOST);
  auto status = config.SetParam(ValueCase::kPortType, sport);
  EXPECT_TRUE(status.ok());
  ASSERT_EQ(config.HasAnyOf(GNMI_CONFIG_PORT_TYPE), true);
  ASSERT_EQ(config.cfg.port_type, PORT_TYPE_VHOST);
}

TEST(DpdkConfigParamTest, VhostConfig) {
  DpdkPortConfig config;
  SingletonPort sport;
  PortConfigParams* port_params = sport.mutable_config_params();
  // Set parameter values
  port_params->set_port_type(PORT_TYPE_VHOST);
  port_params->set_device_type(DEVICE_TYPE_VIRTIO_BLK);
  port_params->set_queues(4);
  port_params->set_socket_path("Wrench");
  port_params->set_host_name("Gaston");
  // Verify that no parameters are defined
  ASSERT_FALSE(config.HasAnyOf(GNMI_CONFIG_VHOST_REQUIRED));
  // Set four of the parameters
  ASSERT_TRUE(config.SetParam(ValueCase::kQueueCount, sport).ok());
  ASSERT_TRUE(config.SetParam(ValueCase::kSockPath, sport).ok());
  ASSERT_TRUE(config.SetParam(ValueCase::kPortType, sport).ok());
  ASSERT_TRUE(config.SetParam(ValueCase::kDeviceType, sport).ok());
  // Verify that the set is incomplete
  ASSERT_FALSE(config.HasAllOf(GNMI_CONFIG_VHOST_REQUIRED));
  // Set the fifth parameter
  ASSERT_TRUE(config.SetParam(ValueCase::kHostConfig, sport).ok());
  // Verify that the set is complete
  ASSERT_TRUE(config.HasAllOf(GNMI_CONFIG_VHOST_REQUIRED));
}

TEST(DpdkConfigParamTest, MtuValue) {
  DpdkPortConfig config;
  SingletonPort sport;
  sport.mutable_config_params()->set_mtu(1500);
  ASSERT_TRUE(config.SetParam(ValueCase::kMtuValue, sport).ok());
  ASSERT_TRUE(config.HasAnyOf(GNMI_CONFIG_MTU_VALUE));
  ASSERT_EQ(config.cfg.mtu, 1500);
}

TEST(DpskConfigParamTest, BadMtuValue) {
  DpdkPortConfig config;
  SingletonPort sport;
  PortConfigParams* port_params = sport.mutable_config_params();
  port_params->set_mtu(0x10001);
  config.cfg.mtu = 0;
  ASSERT_FALSE(config.SetParam(ValueCase::kMtuValue, sport).ok());
  ASSERT_EQ(config.cfg.mtu, 0);
}

TEST(DpdkConfigParamTest, HotplugSocketPort) {
  DpdkPortConfig config;
  SingletonPort sport;
  sport.mutable_config_params()->mutable_hotplug_config()->set_qemu_socket_port(
      1984);
  ASSERT_TRUE(config.SetHotplugParam(PARAM_SOCK_PORT, sport).ok());
  ASSERT_TRUE(config.HasAllOf(GNMI_CONFIG_HOTPLUG_SOCKET_PORT));
  ASSERT_EQ(config.hotplug.qemu_socket_port, 1984);
}

TEST(DpdkConfigParamTest, HotplugConfig) {
  DpdkPortConfig config;
  SingletonPort sport;
  // Set parameter values
  PortConfigParams* port_params = sport.mutable_config_params();
  HotplugConfig* hotplug_config = port_params->mutable_hotplug_config();
  hotplug_config->set_qemu_socket_ip("192.168.17.5");
  hotplug_config->set_qemu_socket_port(1776);
  hotplug_config->set_qemu_hotplug_mode(HOTPLUG_MODE_ADD);
  hotplug_config->set_qemu_vm_mac_address(0x112233445566ULL);
  hotplug_config->set_qemu_vm_netdev_id("netdev");
  hotplug_config->set_qemu_vm_chardev_id("chardev");
  hotplug_config->set_native_socket_path("/into/the/woods");
  hotplug_config->set_qemu_vm_device_id("infernal");
  // Verify that set is empty
  ASSERT_FALSE(config.HasAnyOf(GNMI_CONFIG_HOTPLUG_REQUIRED));
  // Set all but one parameter
  ASSERT_TRUE(config.SetHotplugParam(PARAM_CHARDEV_ID, sport).ok());
  ASSERT_TRUE(config.SetHotplugParam(PARAM_DEVICE_ID, sport).ok());
  ASSERT_TRUE(config.SetHotplugParam(PARAM_HOTPLUG_MODE, sport).ok());
  ASSERT_TRUE(config.SetHotplugParam(PARAM_NATIVE_SOCK_PATH, sport).ok());
  ASSERT_TRUE(config.SetHotplugParam(PARAM_NETDEV_ID, sport).ok());
  ASSERT_TRUE(config.SetHotplugParam(PARAM_SOCK_IP, sport).ok());
  ASSERT_TRUE(config.SetHotplugParam(PARAM_SOCK_PORT, sport).ok());
  // Verify that set is incomplete
  ASSERT_FALSE(config.HasAllOf(GNMI_CONFIG_HOTPLUG_REQUIRED));
  // Set final parameter
  ASSERT_TRUE(config.SetHotplugParam(PARAM_VM_MAC, sport).ok());
  // Verify that set is complete
  ASSERT_TRUE(config.HasAllOf(GNMI_CONFIG_HOTPLUG_REQUIRED));
}

}  // namespace

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
