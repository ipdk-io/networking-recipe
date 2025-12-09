// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/tdi_action_profile_manager.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest.h"
#include "stratum/glue/status/status_test_util.h"
#include "stratum/hal/lib/common/writer_mock.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_sde_mock.h"
#include "stratum/lib/test_utils/matchers.h"
#include "stratum/lib/utils.h"

namespace stratum {
namespace hal {
namespace tdi {

using test_utils::EqualsProto;
using ::testing::_;
using ::testing::ByMove;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using TableDataInterface = TdiSdeInterface::TableDataInterface;

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

constexpr char kUnspExternMsg[] = "Unsupported extern type 1";
constexpr int kTdiRtTableId = 1;

class TdiActionProfileManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    sde_wrapper_mock_ = absl::make_unique<TdiSdeMock>();
    action_profile_manager_ = TdiActionProfileManager::CreateInstance(
        sde_wrapper_mock_.get(), kDevice1);
  }

  ::util::Status InvalidParamError() {
    return MAKE_ERROR(ERR_INVALID_PARAM) << kUnspExternMsg;
  }

  ::util::Status OperNotSuppError() {
    return MAKE_ERROR(ERR_OPER_NOT_SUPPORTED) << kUnspExternMsg;
  }
  static constexpr int kDevice1 = 0;

  std::unique_ptr<TdiSdeMock> sde_wrapper_mock_;
  std::unique_ptr<TdiActionProfileManager> action_profile_manager_;
};

/*
 * Validates WriteActionProfileEntry for update type: INSERT
 * Extern entry is wrapped with action profile member and the function
 * is expected to return OK after invoking it with correct set of params.
 */

TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestActionProfileIdInsert) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionProfileId);
  entry.set_extern_type_id(kTnaExternActionProfileId);

  // set update type to INSERT
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::INSERT;

  // create action profile member
  auto* act_prof_member = new ::p4::v1::ActionProfileMember();
  act_prof_member->set_member_id(1);
  act_prof_member->set_action_profile_id(1);
  act_prof_member->mutable_action()->set_action_id(1);
  act_prof_member->mutable_action()->add_params()->set_param_id(1);
  act_prof_member->mutable_action()->add_params()->set_value("000001");

  // pack action profile member to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_member);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_,
              InsertActionProfileMember(kDevice1, _, kTdiRtTableId, 1, _))
      .WillOnce(Return(::util::OkStatus()));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->WriteActionProfileEntry(
      session_mock, update_type, entry));
}

/*
 * Validates WriteActionProfileEntry for update type: MODIFY
 * Extern entry is wrapped with action profile member and the function
 * is expected to return OK after invoking it with correct set of params.
 */
TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestActionProfileIdModify) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionProfileId);
  entry.set_extern_type_id(kTnaExternActionProfileId);

  // set update type to MODIFY
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::MODIFY;

  // create action profile member
  auto* act_prof_member = new ::p4::v1::ActionProfileMember();
  act_prof_member->set_member_id(1);
  act_prof_member->set_action_profile_id(1);
  act_prof_member->mutable_action()->set_action_id(1);
  act_prof_member->mutable_action()->add_params()->set_param_id(1);
  act_prof_member->mutable_action()->add_params()->set_value("000001");

  // pack action profile member to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_member);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_,
              ModifyActionProfileMember(kDevice1, _, kTdiRtTableId, 1, _))
      .WillOnce(Return(::util::OkStatus()));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->WriteActionProfileEntry(
      session_mock, update_type, entry));
}

/*
 * Validates WriteActionProfileEntry for update type: DELETE
 * Extern entry is wrapped with action profile member and the function
 * is expected to return OK after invoking it with correct set of params.
 */
TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestActionProfileIdDelete) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionProfileId);
  entry.set_extern_type_id(kTnaExternActionProfileId);

  // set update type to DELETE
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::DELETE;

  // create action profile member
  auto* act_prof_member = new ::p4::v1::ActionProfileMember();
  act_prof_member->set_member_id(1);
  act_prof_member->set_action_profile_id(1);
  act_prof_member->mutable_action()->set_action_id(1);
  act_prof_member->mutable_action()->add_params()->set_param_id(1);
  act_prof_member->mutable_action()->add_params()->set_value("000001");

  // pack action profile member to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_member);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_,
              DeleteActionProfileMember(kDevice1, _, kTdiRtTableId, 1))
      .WillOnce(Return(::util::OkStatus()));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->WriteActionProfileEntry(
      session_mock, update_type, entry));
}

/*
 * Validates WriteActionProfileEntry for update type: INSERT
 * Extern entry is wrapped with action profile group and the function
 * is expected to return OK after invoking it with correct set of params.
 */
TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestActionSelectorIdInsert) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionSelectorId);
  entry.set_extern_type_id(kTnaExternActionSelectorId);

  // set update type to INSERT
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::INSERT;

  // create action profile group
  auto* act_prof_group = new ::p4::v1::ActionProfileGroup();
  act_prof_group->set_group_id(1);
  act_prof_group->set_action_profile_id(1);
  act_prof_group->set_max_size(4);
  act_prof_group->add_members()->set_member_id(1);
  act_prof_group->add_members()->set_weight(1);

  // pack action profile group to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_group);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_,
              InsertActionProfileGroup(kDevice1, _, kTdiRtTableId, 1, 4, _, _))
      .WillOnce(Return(::util::OkStatus()));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->WriteActionProfileEntry(
      session_mock, update_type, entry));
}

/*
 * Validates WriteActionProfileEntry for update type: MODIFY
 * Extern entry is wrapped with action profile group and the function
 * is expected to return OK after invoking it with correct set of params.
 */
TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestActionSelectorIdModify) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionSelectorId);
  entry.set_extern_type_id(kTnaExternActionSelectorId);

  // set update type to MODIFY
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::MODIFY;

  // create action profile group
  auto* act_prof_group = new ::p4::v1::ActionProfileGroup();
  act_prof_group->set_group_id(1);
  act_prof_group->set_action_profile_id(1);
  act_prof_group->set_max_size(4);
  act_prof_group->add_members()->set_member_id(1);
  act_prof_group->add_members()->set_weight(1);

  // pack action profile group to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_group);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_,
              ModifyActionProfileGroup(kDevice1, _, kTdiRtTableId, 1, 4, _, _))
      .WillOnce(Return(::util::OkStatus()));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->WriteActionProfileEntry(
      session_mock, update_type, entry));
}

/*
 * Validates WriteActionProfileEntry for update type: DELETE
 * Extern entry is wrapped with action profile group and the function
 * is expected to return OK after invoking it with correct set of params.
 */
TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestActionSelectorIdDelete) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionSelectorId);
  entry.set_extern_type_id(kTnaExternActionSelectorId);

  // set update type to DELETE
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::DELETE;

  // create action profile group
  auto* act_prof_group = new ::p4::v1::ActionProfileGroup();
  act_prof_group->set_group_id(1);
  act_prof_group->set_action_profile_id(1);
  act_prof_group->set_max_size(4);
  act_prof_group->add_members()->set_member_id(1);
  act_prof_group->add_members()->set_weight(1);

  // pack action profile group to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_group);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_,
              DeleteActionProfileGroup(kDevice1, _, kTdiRtTableId, 1))
      .WillOnce(Return(::util::OkStatus()));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->WriteActionProfileEntry(
      session_mock, update_type, entry));
}

/*
 * Validates WriteActionProfileEntry for update type: INSERT
 * Extern entry is wrapped with unsupported  extern type id and the function
 * is supposed to return error with Unsupported extern type id.
 */
TEST_F(TdiActionProfileManagerTest,
       WriteActionProfileEntryTestUnsupportedType) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(1);
  entry.set_extern_type_id(1);

  // set update type to INSERT
  ::p4::v1::Update::Type update_type = ::p4::v1::Update::INSERT;

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  // perform test and check results (act + assert)
  EXPECT_THAT(action_profile_manager_->WriteActionProfileEntry(
                  session_mock, update_type, entry),
              DerivedFromStatus(InvalidParamError()));
}

/*
 * Validates ReadActionProfileEntry
 * We match the read operation output with the expected response.
 */
TEST_F(TdiActionProfileManagerTest, ReadActionProfileEntryTestActionProfileId) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionProfileId);
  entry.set_extern_type_id(kTnaExternActionProfileId);
  WriterMock<::p4::v1::ReadResponse> writer_mock;

  // create action profile member
  auto* act_prof_member = new ::p4::v1::ActionProfileMember();
  act_prof_member->set_member_id(1);
  act_prof_member->set_action_profile_id(1);
  act_prof_member->mutable_action()->set_action_id(1);
  act_prof_member->mutable_action()->add_params()->set_param_id(1);
  act_prof_member->mutable_action()->add_params()->set_value("000001");

  // pack action profile member to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_member);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by ReadActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  const std::string kActionProfMemberResponseText = R"pb(
      entities {
        action_profile_member {
          action_profile_id: 1
          member_id: 1
          action {
            action_id: 1
            params {
              param_id: 1
              value: "000001"
            }
          }
        }
      }
    )pb";
  ::p4::v1::ReadResponse resp;
  ASSERT_OK(ParseProtoFromString(kActionProfMemberResponseText, &resp));

  // TODO: needs rework, instead of mocking Write to return true, it needs
  // to return the response which can be validated against local response.
  EXPECT_CALL(writer_mock, Write(_)).WillOnce(Return(true));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->ReadActionProfileEntry(session_mock, entry,
                                                            &writer_mock));
}

/*
 * Validates ReadActionProfileEntry
 * We match the read operation output with the expected response.
 */
TEST_F(TdiActionProfileManagerTest,
       ReadActionProfileEntryTestActionSelectorId) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(kTnaExternActionSelectorId);
  entry.set_extern_type_id(kTnaExternActionSelectorId);

  // create action profile group
  auto* act_prof_group = new ::p4::v1::ActionProfileGroup();
  act_prof_group->set_group_id(1);
  act_prof_group->set_action_profile_id(1);
  act_prof_group->set_max_size(4);
  act_prof_group->add_members()->set_member_id(1);
  act_prof_group->add_members()->set_weight(1);

  // pack action profile group to extern entry
  auto* any = new google::protobuf::Any();
  any->PackFrom(*act_prof_group);
  entry.set_allocated_entry(any);

  // mocked objects
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();
  WriterMock<::p4::v1::ReadResponse> writer_mock;

  std::vector<int> group_ids = {1};
  std::vector<int> max_group_sizes = {4};
  std::vector<std::vector<uint32>> member_ids = {{1}};
  std::vector<std::vector<bool>> member_status = {{true}};

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by ReadActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_,
              GetActionProfileGroups(kDevice1, _, kTdiRtTableId, 1, _, _, _, _))
      .WillOnce(
          DoAll(SetArgPointee<4>(group_ids), SetArgPointee<5>(max_group_sizes),
                SetArgPointee<6>(member_ids), SetArgPointee<7>(member_status),
                Return(::util::OkStatus())));

  EXPECT_CALL(*sde_wrapper_mock_, GetActionProfileTdiRtId(kTdiRtTableId))
      .WillOnce(Return(1));

  EXPECT_CALL(*sde_wrapper_mock_, GetP4InfoId(1)).WillOnce(Return(1));

  // JSON representation of the protobuf we expect ReadProfileActionEntry()
  // to specify when it calls the writer mock.
  const std::string kActionProfGroupResponseText = R"pb(
      entities {
        action_profile_group {
          action_profile_id: 1
          group_id: 1
          members {
            member_id: 1
            weight: 1
          }
          max_size: 4
        }
      }
    )pb";
  ::p4::v1::ReadResponse resp;
  ASSERT_OK(ParseProtoFromString(kActionProfGroupResponseText, &resp));
  EXPECT_CALL(writer_mock, Write(EqualsProto(resp))).WillOnce(Return(true));

  // perform test and check results (act + assert)
  EXPECT_OK(action_profile_manager_->ReadActionProfileEntry(session_mock, entry,
                                                            &writer_mock));
}

/*
 * Validates ReadActionProfileEntry for update type: INSERT
 * Extern entry is wrapped with unsupported extern type id and the function
 * is supposed to return error with Unsupported extern type id.
 */
TEST_F(TdiActionProfileManagerTest, ReadActionProfileEntryTestUnsupportedType) {
  // create extern entry
  ::p4::v1::ExternEntry entry;
  entry.set_extern_id(1);
  entry.set_extern_type_id(1);

  WriterInterface<::p4::v1::ReadResponse>* writer;

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by ReadActionProfileEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTdiRtId(entry.extern_id()))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  // perform test and check results (act + assert)
  EXPECT_THAT(action_profile_manager_->ReadActionProfileEntry(session_mock,
                                                              entry, writer),
              DerivedFromStatus(OperNotSuppError()));
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
