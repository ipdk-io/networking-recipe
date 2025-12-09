// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// adapted from ipdk_switch_test, which was
// adapted from bcm_switch_test

#include "stratum/hal/lib/tdi/es2k/es2k_switch.h"

#include <utility>

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "stratum/glue/status/canonical_errors.h"
#include "stratum/glue/status/status_test_util.h"
#include "stratum/hal/lib/common/gnmi_events.h"
#include "stratum/hal/lib/common/writer_mock.h"
#include "stratum/hal/lib/p4/p4_table_mapper_mock.h"
#include "stratum/hal/lib/tdi/es2k/es2k_chassis_manager_mock.h"
#include "stratum/hal/lib/tdi/es2k/es2k_node_mock.h"
#include "stratum/hal/lib/tdi/tdi_global_vars.h"
#include "stratum/hal/lib/tdi/tdi_ipsec_manager_mock.h"
#include "stratum/lib/channel/channel_mock.h"
#include "stratum/lib/utils.h"

namespace stratum {
namespace hal {
namespace tdi {

using ::testing::_;
using ::testing::DoAll;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::WithArg;
using ::testing::WithArgs;

namespace {

MATCHER_P(EqualsProto, proto, "") { return ProtoEqual(arg, proto); }

MATCHER_P(EqualsStatus, status, "") {
  return arg.error_code() == status.error_code() &&
         arg.error_message() == status.error_message();
}

MATCHER_P(DerivedFromStatus, status, "") {
  if (arg.error_code() != status.error_code()) {
    return false;
  }
  if (arg.error_message().find(status.error_message()) == std::string::npos) {
    *result_listener << "\nOriginal error string: \"" << status.error_message()
                     << "\" is missing from the actual status.";
    return false;
  }
  return true;
}

constexpr uint64 kNodeId = 13579;
constexpr int kDevice = 2;
constexpr char kErrorMsg[] = "Test error message";
constexpr uint32 kPortId = 2468;

const std::map<uint64, int>& NodeIdToDeviceMap() {
  static auto* map = new std::map<uint64, int>({{kNodeId, kDevice}});
  return *map;
}

class Es2kSwitchTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Use NiceMock to suppress "uninteresting mock function call" warnings
    chassis_manager_mock_ =
        absl::make_unique<NiceMock<Es2kChassisManagerMock>>();
    ipsec_manager_mock_ = absl::make_unique<NiceMock<IPsecManagerMock>>();
    node_mock_ = absl::make_unique<NiceMock<Es2kNodeMock>>();
    device_to_node_mock_[kDevice] = node_mock_.get();
    switch_ = Es2kSwitch::CreateInstance(chassis_manager_mock_.get(),
                                         ipsec_manager_mock_.get(),
                                         device_to_node_mock_);
    shutdown = false;  // global variable initialization

    ON_CALL(*chassis_manager_mock_, GetNodeIdToDeviceMap())
        .WillByDefault(Return(NodeIdToDeviceMap()));
  }

  void TearDown() override { device_to_node_mock_.clear(); }

  // This operation should always succeed.
  // We use it to set up a number of test cases.
  void PushChassisConfigSuccessfully() {
    ChassisConfig config;
    config.add_nodes()->set_id(kNodeId);
    EXPECT_CALL(*node_mock_, PushChassisConfig(EqualsProto(config), kNodeId))
        .WillOnce(Return(::util::OkStatus()));
    EXPECT_OK(switch_->PushChassisConfig(config));
  }

  ::util::Status DefaultError() {
    return ::util::Status(StratumErrorSpace(), ERR_UNKNOWN, kErrorMsg);
  }

  std::unique_ptr<Es2kChassisManagerMock> chassis_manager_mock_;
  std::unique_ptr<IPsecManagerMock> ipsec_manager_mock_;
  std::unique_ptr<Es2kNodeMock> node_mock_;
  std::map<int, Es2kNode*> device_to_node_mock_;
  std::unique_ptr<Es2kSwitch> switch_;
};

TEST_F(Es2kSwitchTest, PushChassisConfigSucceeds) {
  PushChassisConfigSuccessfully();
}

TEST_F(Es2kSwitchTest, PushChassisConfigFailsWhenNodePushFails) {
  ChassisConfig config;
  config.add_nodes()->set_id(kNodeId);
  EXPECT_CALL(*node_mock_, PushChassisConfig(EqualsProto(config), kNodeId))
      .WillOnce(Return(DefaultError()));

  EXPECT_THAT(switch_->PushChassisConfig(config),
              DerivedFromStatus(DefaultError()));
}

TEST_F(Es2kSwitchTest, VerifyChassisConfigSucceeds) {
  ChassisConfig config;
  config.add_nodes()->set_id(kNodeId);
  EXPECT_OK(switch_->VerifyChassisConfig(config));
}

TEST_F(Es2kSwitchTest, ShutdownSucceeds) {
  EXPECT_CALL(*node_mock_, Shutdown()).WillOnce(Return(::util::OkStatus()));
  EXPECT_CALL(*chassis_manager_mock_, Shutdown())
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_OK(switch_->Shutdown());
}

TEST_F(Es2kSwitchTest, ShutdownFailsWhenSomeManagerShutdownFails) {
  EXPECT_CALL(*node_mock_, Shutdown()).WillOnce(Return(::util::OkStatus()));
  EXPECT_CALL(*chassis_manager_mock_, Shutdown())
      .WillOnce(Return(DefaultError()));

  EXPECT_THAT(switch_->Shutdown(), DerivedFromStatus(DefaultError()));
}

// Chassis config pushed successfully.
// PushForwardingPipelineConfig() should propagate the config.
TEST_F(Es2kSwitchTest, DISABLED_PushForwardingPipelineConfigSucceeds) {
  PushChassisConfigSuccessfully();

  ::p4::v1::ForwardingPipelineConfig config;
  EXPECT_CALL(*node_mock_, PushForwardingPipelineConfig(EqualsProto(config)))
      .WillOnce(Return(::util::OkStatus()));
  EXPECT_OK(switch_->PushForwardingPipelineConfig(kNodeId, config));
}

// When Es2kSwitchTest fails to push a forwarding config during
// PushForwardingPipelineConfig(), it should fail immediately.
TEST_F(Es2kSwitchTest, PushForwardingPipelineConfigFailsWhenPushFails) {
  PushChassisConfigSuccessfully();

  ::p4::v1::ForwardingPipelineConfig config;
  EXPECT_CALL(*node_mock_, PushForwardingPipelineConfig(EqualsProto(config)))
      .WillOnce(Return(DefaultError()));
  EXPECT_THAT(switch_->PushForwardingPipelineConfig(kNodeId, config),
              DerivedFromStatus(DefaultError()));
}

TEST_F(Es2kSwitchTest, VerifyForwardingPipelineConfigSucceeds) {
  PushChassisConfigSuccessfully();

  ::p4::v1::ForwardingPipelineConfig config;
  // Verify should always be called before push.
  EXPECT_CALL(*node_mock_, VerifyForwardingPipelineConfig(EqualsProto(config)))
      .WillOnce(Return(::util::OkStatus()));
  EXPECT_OK(switch_->VerifyForwardingPipelineConfig(kNodeId, config));
}

// Test registration of a writer for sending gNMI events.
TEST_F(Es2kSwitchTest, RegisterEventNotifyWriterTest) {
  auto writer = std::shared_ptr<WriterInterface<GnmiEventPtr>>(
      new WriterMock<GnmiEventPtr>());

  EXPECT_CALL(*chassis_manager_mock_, RegisterEventNotifyWriter(writer))
      .WillOnce(Return(::util::OkStatus()))
      .WillOnce(Return(DefaultError()));

  // Successful Es2kChassisManager registration.
  EXPECT_OK(switch_->RegisterEventNotifyWriter(writer));

  // Failed Es2kChassisManager registration.
  EXPECT_THAT(switch_->RegisterEventNotifyWriter(writer),
              DerivedFromStatus(DefaultError()));
}

namespace {

void ExpectMockWriteDataResponse(WriterMock<DataResponse>* writer,
                                 DataResponse* resp) {
  // Mock implementation of Write() that saves the response to local variable.
  EXPECT_CALL(*writer, Write(_))
      .WillOnce(DoAll(WithArg<0>(Invoke([resp](DataResponse r) {
                        // Copy the response.
                        *resp = r;
                      })),
                      Return(true)));
}

}  // namespace

// ------------------------------------------------------------------------
// [ RUN      ] Es2kSwitchTest.GetSdnPortId
// I20230523 21:03:48.518360    12 es2k_switch.cc:59] Chassis config pushed
// successfully. stratum/hal/lib/tdi/es2k/es2k_switch_test.cc:232: Failure Value
// of: resp.has_sdn_port_id()
//   Actual: false
// Expected: true
// stratum/hal/lib/tdi/es2k/es2k_switch_test.cc:233: Failure
// Expected equality of these values:
//   kPortId
//     Which is: 2468
//   resp.sdn_port_id().port_id()
//     Which is: 0
// stratum/hal/lib/tdi/es2k/es2k_switch_test.cc:235: Failure
// Value of: details.at(0)
// Expected: is equal to OK
//   Actual: generic::unknown:  (of type util::Status)
// stratum/hal/lib/tdi/es2k/es2k_switch_test.cc:207: Failure
// Actual function call count doesn't match EXPECT_CALL(*writer, Write(_))...
//          Expected: to be called once
//            Actual: never called - unsatisfied and active
// [  FAILED  ] Es2kSwitchTest.GetSdnPortId (0 ms)
// ------------------------------------------------------------------------
TEST_F(Es2kSwitchTest, DISABLED_GetSdnPortId) {
  PushChassisConfigSuccessfully();

  WriterMock<DataResponse> writer;
  DataResponse resp;
  // Expect Write() call and store data in resp.
  ExpectMockWriteDataResponse(&writer, &resp);

  DataRequest req;
  auto* req_info = req.add_requests()->mutable_sdn_port_id();
  req_info->set_node_id(kNodeId);
  req_info->set_port_id(kPortId);
  std::vector<::util::Status> details;

  EXPECT_OK(switch_->RetrieveValue(kNodeId, req, &writer, &details));
  EXPECT_TRUE(resp.has_sdn_port_id());
  EXPECT_EQ(kPortId, resp.sdn_port_id().port_id());
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

// ------------------------------------------------------------------------
// These tests return ERR_INTERNAL: Not supported yet!
// They should be reviewed in conjunction with Es2kPortManager and either
// updated (if necessary) and enable, or removed entirely.
// ------------------------------------------------------------------------
TEST_F(Es2kSwitchTest, DISABLED_SetPortAdminStatusPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);
  request->mutable_admin_status()->set_state(AdminState::ADMIN_STATE_ENABLED);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(
      /* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

TEST_F(Es2kSwitchTest, DISABLED_SetPortMacAddressPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);
  request->mutable_mac_address()->set_mac_address(0x112233445566);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(
      /* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

TEST_F(Es2kSwitchTest, DISABLED_SetPortSpeedPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);
  request->mutable_port_speed()->set_speed_bps(40000000000);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(
      /* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

TEST_F(Es2kSwitchTest, DISABLED_SetPortLacpSystemIdMacPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);
  request->mutable_lacp_router_mac()->set_mac_address(0x112233445566);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(
      /* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

TEST_F(Es2kSwitchTest, DISABLED_SetPortLacpSystemPriorityPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);
  request->mutable_lacp_system_priority()->set_priority(10);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(
      /* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

TEST_F(Es2kSwitchTest, DISABLED_SetPortHealthIndicatorPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);
  request->mutable_health_indicator()->set_state(HealthState::HEALTH_STATE_BAD);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(
      /* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0), ::util::OkStatus());
}

TEST_F(Es2kSwitchTest, SetPortNoContentsPass) {
  SetRequest req;
  auto* request = req.add_requests()->mutable_port();
  request->set_node_id(1);
  request->set_port_id(2);

  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(/* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0).ToString(), HasSubstr("Not supported yet"));
}

TEST_F(Es2kSwitchTest, SetNoContentsPass) {
  SetRequest req;
  req.add_requests();
  std::vector<::util::Status> details;
  EXPECT_OK(switch_->SetValue(/* node_id */ 0, req, &details));
  ASSERT_EQ(details.size(), 1);
  EXPECT_THAT(details.at(0).ToString(), HasSubstr("Not supported yet"));
}

// TODO(unknown): Complete unit test coverage.

}  // namespace
}  // namespace tdi
}  // namespace hal
}  // namespace stratum
