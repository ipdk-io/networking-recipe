// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// adapted from bf_chassis_manager_test.cc

#include "stratum/hal/lib/tdi/dpdk/dpdk_chassis_manager.h"

#include <string>
#include <utility>

#include "absl/strings/match.h"
#include "absl/synchronization/notification.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "stratum/glue/integral_types.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/status_test_util.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/hal/lib/common/common.pb.h"
#include "stratum/hal/lib/common/writer_mock.h"
#include "stratum/hal/lib/tdi/dpdk/dpdk_port_manager_mock.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/lib/constants.h"
#include "stratum/lib/test_utils/matchers.h"
#include "stratum/lib/utils.h"

#undef IPU_ADD_NEW_PORT

#if !defined(IPU_ADD_NEW_PORT)
#define EXPECT_ADD_PORT_CALL(device, port, config)
#define EXPECT_ENABLE_PORT_CALL(device, port)
#else
#define EXPECT_ADD_PORT_CALL(device, port, config) \
  EXPECT_CALL(*port_manager_, AddPort((device), (port), _))
#define EXPECT_ENABLE_PORT_CALL(device, port) \
  EXPECT_CALL(*port_manager_, EnablePort(device, port))
#endif

namespace stratum {
namespace hal {
namespace tdi {

using PortStatusEvent = TdiPortManager::PortStatusEvent;
using ValueCase = SetRequest::Request::Port::ValueCase;

using test_utils::EqualsProto;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::DoAll;
using ::testing::HasSubstr;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::WithArg;

namespace {

constexpr uint64 kNodeId = 7654321ULL;
// For Tofino, device is the 0-based index of the node in the ChassisConfig.
constexpr int kDevice = 0;
constexpr int kSlot = 1;
constexpr int kPort = 1;
constexpr uint32 kPortId = 12345;
constexpr uint32 kSdkPortOffset = 900000;
constexpr uint32 kDefaultPortId = kSdkPortOffset + kPortId;

MATCHER_P(GnmiEventEq, event, "") {
  if (absl::StrContains(typeid(*event).name(), "PortOperStateChangedEvent")) {
    const auto& cast_event =
        static_cast<const PortOperStateChangedEvent&>(*event);
    const auto& cast_arg = static_cast<const PortOperStateChangedEvent&>(*arg);
    return cast_event.GetPortId() == cast_arg.GetPortId() &&
           cast_event.GetNodeId() == cast_arg.GetNodeId() &&
           cast_event.GetNewState() == cast_arg.GetNewState() &&
           cast_event.GetTimeLastChanged() == cast_arg.GetTimeLastChanged();
  }
  return false;
}

// A helper class to build a single-node ChassisConfig message.
class ChassisConfigBuilder {
 public:
  explicit ChassisConfigBuilder(uint64 node_id = kNodeId) : node_id(node_id) {
    config_.set_description("Test config for DpdkChassisManager");
    auto* chassis = config_.mutable_chassis();
    chassis->set_platform(PLT_P4_SOFT_SWITCH);
    chassis->set_name("tdi-dpdk");

    auto* node = config_.add_nodes();
    node->set_id(node_id);
    node->set_slot(kSlot);
  }

  // Adds a port to the ChassisConfig message.
  SingletonPort* AddPort(uint32 port_id, int32 port, AdminState admin_state) {
    auto* sport = config_.add_singleton_ports();
    sport->set_id(port_id);
    sport->set_node(node_id);
    sport->set_port(port);
    sport->set_slot(kSlot);
    sport->set_channel(0);

    auto* params = sport->mutable_config_params();
    params->set_admin_state(admin_state);

    return sport;
  }

  // Gets a mutable singleton port from the ChassisConfig message.
  SingletonPort* GetPort(uint32 port_id) {
    for (auto& sport : *config_.mutable_singleton_ports()) {
      if (sport.id() == port_id) return &sport;
    }
    return nullptr;
  }

  void SetVendorConfig(const VendorConfig& vendor_config) {
    *config_.mutable_vendor_config() = vendor_config;
  }

  void RemoveLastPort() { config_.mutable_singleton_ports()->RemoveLast(); }

  const ChassisConfig& Get() const { return config_; }

 private:
  uint64 node_id;
  ChassisConfig config_;
};

}  // namespace

class DpdkChassisManagerTest : public ::testing::Test {
 protected:
  DpdkChassisManagerTest() {}

  void SetUp() override {
    // Use NiceMock to suppress default action/value warnings.
    port_manager_ = absl::make_unique<NiceMock<DpdkPortManagerMock>>();
    // TODO(max): create parameterized test suite over mode.
    chassis_manager_ = DpdkChassisManager::CreateInstance(
        OPERATION_MODE_STANDALONE, port_manager_.get());

    // Expectations
    ON_CALL(*port_manager_, IsValidPort(_, _))
        .WillByDefault(
            WithArg<1>(Invoke([](uint32 id) { return id > kSdkPortOffset; })));
  }

  void RegisterSdkPortId(uint32 port_id, int slot, int port, int channel,
                         int device) {
    PortKey port_key(slot, port, channel);
    EXPECT_CALL(*port_manager_, GetPortIdFromPortKey(device, port_key))
        .WillRepeatedly(Return(port_id + kSdkPortOffset));
  }

  void RegisterSdkPortId(const SingletonPort* singleton_port) {
    RegisterSdkPortId(singleton_port->id(), singleton_port->slot(),
                      singleton_port->port(), singleton_port->channel(),
                      kDevice);  // TODO(bocon): look up device from node
  }

  ::util::Status CheckCleanInternalState() {
    RET_CHECK(chassis_manager_->device_to_node_id_.empty());
    RET_CHECK(chassis_manager_->node_id_to_device_.empty());
    RET_CHECK(chassis_manager_->node_id_to_port_id_to_port_state_.empty());
    RET_CHECK(chassis_manager_->node_id_to_port_id_to_port_config_.empty());
    RET_CHECK(
        chassis_manager_->node_id_to_port_id_to_singleton_port_key_.empty());
    RET_CHECK(chassis_manager_->node_id_to_port_id_to_sdk_port_id_.empty());
    RET_CHECK(chassis_manager_->node_id_to_sdk_port_id_to_port_id_.empty());
    return ::util::OkStatus();
  }

  bool Initialized() { return chassis_manager_->initialized_; }

  ::util::Status VerifyChassisConfig(const ChassisConfig& config) {
    absl::ReaderMutexLock l(&chassis_lock);
    return chassis_manager_->VerifyChassisConfig(config);
  }

  ::util::Status PushChassisConfig(const ChassisConfig& config) {
    absl::WriterMutexLock l(&chassis_lock);
    return chassis_manager_->PushChassisConfig(config);
  }

  ::util::Status PushChassisConfig(const ChassisConfigBuilder& builder) {
    absl::WriterMutexLock l(&chassis_lock);
    return chassis_manager_->PushChassisConfig(builder.Get());
  }

  ::util::Status PushBaseChassisConfig(ChassisConfigBuilder* builder) {
    RET_CHECK(!Initialized())
        << "Can only call PushBaseChassisConfig() for first ChassisConfig!";
    RegisterSdkPortId(builder->AddPort(kPortId, kPort, ADMIN_STATE_ENABLED));

    EXPECT_ADD_PORT_CALL(kDevice, kDefaultPortId, _);
    EXPECT_ENABLE_PORT_CALL(kDevice, kDefaultPortId);

    RETURN_IF_ERROR(PushChassisConfig(builder->Get()));

    auto device = GetDeviceFromNodeId(kNodeId);
    RET_CHECK(device.ok());
    RET_CHECK(device.ValueOrDie() == kDevice) << "Invalid device number!";
    RET_CHECK(Initialized()) << "Class is not initialized after push!";
    return ::util::OkStatus();
  }

  ::util::Status ReplayChassisConfig(uint64 node_id) {
    absl::WriterMutexLock l(&chassis_lock);
    return chassis_manager_->ReplayChassisConfig(node_id);
  }

  ::util::Status PushBaseChassisConfig() {
    ChassisConfigBuilder builder;
    return PushBaseChassisConfig(&builder);
  }

  ::util::StatusOr<int> GetDeviceFromNodeId(uint64 node_id) const {
    absl::ReaderMutexLock l(&chassis_lock);
    return chassis_manager_->GetDeviceFromNodeId(node_id);
  }

  ::util::Status Shutdown() { return chassis_manager_->Shutdown(); }

  ::util::Status ShutdownAndTestCleanState() {
    RETURN_IF_ERROR(Shutdown());
    RETURN_IF_ERROR(CheckCleanInternalState());
    return ::util::OkStatus();
  }

  ::util::Status RegisterEventNotifyWriter(
      const std::shared_ptr<WriterInterface<GnmiEventPtr>>& writer) {
    return chassis_manager_->RegisterEventNotifyWriter(writer);
  }

  ::util::Status UnregisterEventNotifyWriter() {
    return chassis_manager_->UnregisterEventNotifyWriter();
  }

  std::unique_ptr<DpdkPortManagerMock> port_manager_;
  std::unique_ptr<DpdkChassisManager> chassis_manager_;

  static constexpr int kTestTransceiverWriterId = 20;
};

TEST_F(DpdkChassisManagerTest, PreFirstConfigPushState) {
  ASSERT_OK(CheckCleanInternalState());
  EXPECT_FALSE(Initialized());
  // TODO(antonin): add more checks (to verify that method calls fail as
  // expected)
}

TEST_F(DpdkChassisManagerTest, FirstConfigPush) {
  ASSERT_OK(PushBaseChassisConfig());
  ASSERT_OK(ShutdownAndTestCleanState());
}

TEST_F(DpdkChassisManagerTest, RemovePort) {
  ChassisConfigBuilder builder;
  ASSERT_OK(PushBaseChassisConfig(&builder));

  builder.RemoveLastPort();
  EXPECT_CALL(*port_manager_, DeletePort(kDevice, kDefaultPortId));
  ASSERT_OK(PushChassisConfig(builder));

  ASSERT_OK(ShutdownAndTestCleanState());
}

TEST_F(DpdkChassisManagerTest, IsPortParamSet) {
  SingletonPort sport;
  sport.mutable_config_params()->set_port_type(PORT_TYPE_VHOST);
  ASSERT_FALSE(
      chassis_manager_->IsPortParamSet(kDevice, kPort, ValueCase::kPortType));
  ASSERT_TRUE(chassis_manager_
                  ->SetPortParam(kDevice, kPort, sport, ValueCase::kPortType)
                  .ok());
  ASSERT_TRUE(
      chassis_manager_->IsPortParamSet(kDevice, kPort, ValueCase::kPortType));
}

TEST_F(DpdkChassisManagerTest, SetPortParam) {
  ChassisConfigBuilder builder;
  ASSERT_OK(PushBaseChassisConfig(&builder));

  const uint32 portId = kPortId + 1;
  const int port = kPort + 1;
  const uint32 sdkPortId = portId + kSdkPortOffset;

  auto sport = builder.AddPort(portId, port, ADMIN_STATE_ENABLED);
  RegisterSdkPortId(sport);
  EXPECT_CALL(*port_manager_, AddPort(_, _, _));

  PortConfigParams* config_params = sport->mutable_config_params();
  config_params->set_admin_state(ADMIN_STATE_ENABLED);
  config_params->set_port_type(PORT_TYPE_VHOST);
  config_params->set_device_type(DEVICE_TYPE_VIRTIO_NET);
  config_params->set_queues(2);
  config_params->set_socket_path("/socket/to/me");
  config_params->set_host_name("Fawlty");

  ASSERT_TRUE(chassis_manager_
                  ->SetPortParam(kDevice, kPort, *sport, ValueCase::kPortType)
                  .ok());
  ASSERT_TRUE(chassis_manager_
                  ->SetPortParam(kDevice, kPort, *sport, ValueCase::kDeviceType)
                  .ok());
  ASSERT_TRUE(chassis_manager_
                  ->SetPortParam(kDevice, kPort, *sport, ValueCase::kQueueCount)
                  .ok());
  ASSERT_TRUE(chassis_manager_
                  ->SetPortParam(kDevice, kPort, *sport, ValueCase::kSockPath)
                  .ok());
  ASSERT_TRUE(chassis_manager_
                  ->SetPortParam(kDevice, kPort, *sport, ValueCase::kHostConfig)
                  .ok());
}

TEST_F(DpdkChassisManagerTest, SetHotplugParam) {
  SingletonPort sport;
  PortConfigParams* config_params = sport.mutable_config_params();
  HotplugConfig* hotplug_config = config_params->mutable_hotplug_config();
  hotplug_config->set_qemu_socket_ip("/qemu/socket_ip");
  ASSERT_TRUE(
      chassis_manager_->SetHotplugParam(kDevice, kPort, sport, PARAM_SOCK_IP)
          .ok());
}

TEST_F(DpdkChassisManagerTest, ReplayPorts) {
  const std::string kVendorConfigText = R"pb(
    tofino_config {
      node_id_to_deflect_on_drop_configs {
        key: 7654321
        value {
          drop_targets {
            port: 12345
            queue: 4
          }
          drop_targets {
            sdk_port: 56789
            queue: 1
          }
        }
      }
      node_id_to_port_shaping_config {
        key: 7654321
        value {
          per_port_shaping_configs {
            key: 12345
            value {
              byte_shaping {
                rate_bps: 10000000000
                burst_bytes: 16384
              }
            }
          }
        }
      }
    }
  )pb";

  VendorConfig vendor_config;
  ASSERT_OK(ParseProtoFromString(kVendorConfigText, &vendor_config));

  ChassisConfigBuilder builder;
  builder.SetVendorConfig(vendor_config);
  ASSERT_OK(PushBaseChassisConfig(&builder));

  const uint32 sdkPortId = kDefaultPortId;
  EXPECT_ADD_PORT_CALL(kDevice, sdkPortId, _);
  EXPECT_ENABLE_PORT_CALL(kDevice, sdkPortId);

  EXPECT_OK(ReplayChassisConfig(kNodeId));

  ASSERT_OK(ShutdownAndTestCleanState());
}

TEST_F(DpdkChassisManagerTest, DISABLED_UpdateInvalidPort) {
  ASSERT_OK(PushBaseChassisConfig());
  ChassisConfigBuilder builder;
  const uint32 portId = kPortId + 1;
  const uint32 sdkPortId = portId + kSdkPortOffset;
  SingletonPort* new_port =
      builder.AddPort(portId, kPort + 1, ADMIN_STATE_ENABLED);
  RegisterSdkPortId(new_port);
  EXPECT_CALL(*port_manager_, AddPort(kDevice, sdkPortId, _))
      .WillOnce(Return(::util::OkStatus()));
  EXPECT_CALL(*port_manager_, EnablePort(kDevice, sdkPortId))
      .WillOnce(Return(::util::OkStatus()));
  ASSERT_OK(PushChassisConfig(builder));

  EXPECT_CALL(*port_manager_, IsValidPort(kDevice, sdkPortId))
      .WillOnce(Return(false));

  // Update port, but port is invalid.
  new_port->set_speed_bps(10000000000ULL);
  auto status = PushChassisConfig(builder);

  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.error_code(), ERR_INTERNAL);
  std::stringstream err_msg;
  err_msg << "Port " << portId << " in node " << kNodeId
          << " is not valid (SDK Port " << sdkPortId << ").";
  EXPECT_THAT(status.error_message(), HasSubstr(err_msg.str()));
  ASSERT_OK(ShutdownAndTestCleanState());
}

TEST_F(DpdkChassisManagerTest, VerifyChassisConfigSuccess) {
  const std::string kConfigText1 = R"(
      description: "Sample Generic Tofino config 2x25G ports."
      chassis {
        platform: PLT_GENERIC_BAREFOOT_TOFINO
        name: "standalone"
      }
      nodes {
        id: 7654321
        slot: 1
      }
      singleton_ports {
        id: 1
        slot: 1
        port: 1
        channel: 1
        speed_bps: 25000000000
        node: 7654321
        config_params {
          admin_state: ADMIN_STATE_ENABLED
        }
      }
      singleton_ports {
        id: 2
        slot: 1
        port: 1
        channel: 2
        speed_bps: 25000000000
        node: 7654321
        config_params {
          admin_state: ADMIN_STATE_ENABLED
        }
      }
  )";

  ChassisConfig config1;
  ASSERT_OK(ParseProtoFromString(kConfigText1, &config1));

  EXPECT_CALL(*port_manager_, GetPortIdFromPortKey(kDevice, PortKey(1, 1, 1)))
      .WillRepeatedly(Return(1 + kSdkPortOffset));
  EXPECT_CALL(*port_manager_, GetPortIdFromPortKey(kDevice, PortKey(1, 1, 2)))
      .WillRepeatedly(Return(2 + kSdkPortOffset));

  ASSERT_OK(VerifyChassisConfig(config1));
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
