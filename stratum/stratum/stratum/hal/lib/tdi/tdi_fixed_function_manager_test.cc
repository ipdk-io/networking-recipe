// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/hal/lib/tdi/tdi_fixed_function_manager.h"

#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest-message.h"
#include "gtest/gtest.h"
#include "stratum/glue/status/status_test_util.h"
#include "stratum/hal/lib/tdi/tdi_constants.h"
#include "stratum/hal/lib/tdi/tdi_sde_mock.h"
#include "stratum/lib/test_utils/matchers.h"

namespace stratum {
namespace hal {
namespace tdi {

using ::testing::_;
using ::testing::ByMove;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using TableKeyInterface = TdiSdeInterface::TableKeyInterface;
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

constexpr char kUnsupOpTypeMsg[] =
    "Unsupported update type: 2147483647 for IPSEC SADB table.";
constexpr int kTdiRtTableId = 1;
constexpr char ipsecConfigSadbTableName[] =
    "ipsec-offload.ipsec-offload.sad.sad-entry.ipsec-sa-config";
constexpr char ipsecFetchSpiTableName[] =
    "ipsec-offload.ipsec-offload.ipsec-spi";
constexpr char vportStateTableName[] =
    "openconfig-virtual-ports.virtual-ports.virtual-port.state";

class TdiFixedFunctionManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    sde_wrapper_mock_ = absl::make_unique<TdiSdeMock>();
    fixed_function_manager_ = TdiFixedFunctionManager::CreateInstance(
        OPERATION_MODE_STANDALONE, sde_wrapper_mock_.get(), kDevice1);
  }

  ::util::Status OpTypeNotSuppError() {
    return MAKE_ERROR(ERR_INTERNAL) << kUnsupOpTypeMsg;
  }

  ::util::Status DefaultError() {
    return MAKE_ERROR(ERR_UNKNOWN) << "Some error";
  }

  void InitSadbConfig(IPsecSADBConfig& sadb_config) {
    sadb_config.set_offload_id(1);
    sadb_config.set_direction(true);
    sadb_config.set_req_id(1);
    sadb_config.set_spi(1);
    sadb_config.set_ext_seq_num(true);
    sadb_config.set_anti_replay_window_size(1);
    sadb_config.set_protocol_parameters(IPSEC_PROTOCOL_PARAMS_ESP);
    sadb_config.set_mode(IPSEC_MODE_TRANSPORT);
  }

  static constexpr int kDevice1 = 0;
  std::unique_ptr<TdiSdeMock> sde_wrapper_mock_;
  std::unique_ptr<TdiFixedFunctionManager> fixed_function_manager_;
};

/*
 * Validates WriteSadbEntry for op_type: ADD_ENTRY
 * IPsecSADBConfig object is populated with accepted param
 * values and we check if WriteSadbEntry is invoked properly
 */
TEST_F(TdiFixedFunctionManagerTest, WriteSadbEntryTestAddEntry) {
  std::string table_name = ipsecConfigSadbTableName;
  const IPsecSadbConfigOp op_type = IPSEC_SADB_CONFIG_OP_ADD_ENTRY;

  // initialize IPsecSADBConfig
  IPsecSADBConfig sadb_config;
  InitSadbConfig(sadb_config);

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();
  auto table_key_mock = absl::make_unique<TableKeyMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteSadbEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTableId(table_name))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*table_key_mock,
              SetExact(kIpsecSadbOffloadId, sadb_config.offload_id()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_key_mock, SetExact(kIpsecSadbDir, sadb_config.direction()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_data_mock, SetParam(kIpsecSadbReqId, sadb_config.req_id()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_data_mock, SetParam(kIpsecSadbSpi, sadb_config.spi()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_data_mock,
              SetParam(kIpsecSadbSeqNum, sadb_config.ext_seq_num()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_data_mock, SetParam(kIpsecSadbReplayWindow,
                                         sadb_config.anti_replay_window_size()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(
      *table_data_mock,
      SetParam(kIpsecSadbProtoParams, (uint8)sadb_config.protocol_parameters()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_data_mock, SetParam(kIpsecSadbMode, sadb_config.mode()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*sde_wrapper_mock_,
              InsertTableEntry(kDevice1, _, kTdiRtTableId, table_key_mock.get(),
                               table_data_mock.get()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableKey(kTdiRtTableId))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableKeyInterface>>(
              std::move(table_key_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  // perform test and check results (act + assert)
  EXPECT_OK(fixed_function_manager_->WriteSadbEntry(session_mock, table_name,
                                                    op_type, sadb_config));
}

/*
 * Validates WriteSadbEntry for op_type: DEL_ENTRY
 * IPsecSADBConfig object is populated with accepted param
 * values and we check if WriteSadbEntry is invoked properly
 */
TEST_F(TdiFixedFunctionManagerTest, WriteSadbEntryTestDelEntry) {
  std::string table_name = ipsecConfigSadbTableName;
  const IPsecSadbConfigOp op_type = IPSEC_SADB_CONFIG_OP_DEL_ENTRY;

  // initialize IPsecSADBConfig
  IPsecSADBConfig sadb_config;
  InitSadbConfig(sadb_config);

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();
  auto table_key_mock = absl::make_unique<TableKeyMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteSadbEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTableId(table_name))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*table_key_mock,
              SetExact(kIpsecSadbOffloadId, sadb_config.offload_id()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_key_mock, SetExact(kIpsecSadbDir, sadb_config.direction()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*sde_wrapper_mock_, DeleteTableEntry(kDevice1, _, kTdiRtTableId,
                                                   table_key_mock.get()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableKey(kTdiRtTableId))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableKeyInterface>>(
              std::move(table_key_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  // perform test and check results (act + assert)
  EXPECT_OK(fixed_function_manager_->WriteSadbEntry(session_mock, table_name,
                                                    op_type, sadb_config));
}

/*
 * Validates WriteSadbEntry for unsupported op_type
 * The function is supposed to return error with
 * Unsupported op_type.
 */
TEST_F(TdiFixedFunctionManagerTest, WriteSadbEntryTestUnsupportedType) {
  std::string table_name = ipsecConfigSadbTableName;
  const IPsecSadbConfigOp op_type =
      IPsecSadbConfigOp_INT_MAX_SENTINEL_DO_NOT_USE_;

  // initialize IPsecSADBConfig
  IPsecSADBConfig sadb_config;
  InitSadbConfig(sadb_config);

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();
  auto table_key_mock = absl::make_unique<TableKeyMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteSadbEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTableId(table_name))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*table_key_mock,
              SetExact(kIpsecSadbOffloadId, sadb_config.offload_id()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*table_key_mock, SetExact(kIpsecSadbDir, sadb_config.direction()))
      .WillOnce(Return(::util::OkStatus()));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableKey(kTdiRtTableId))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableKeyInterface>>(
              std::move(table_key_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  // perform test and check results (act + assert)
  EXPECT_THAT(fixed_function_manager_->WriteSadbEntry(session_mock, table_name,
                                                      op_type, sadb_config),
              DerivedFromStatus(OpTypeNotSuppError()));
}

/*
 * Validates FetchSpi method.
 */
TEST_F(TdiFixedFunctionManagerTest, FetchSpiTest) {
  std::string table_name = ipsecFetchSpiTableName;
  uint32 fetched_spi;

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteSadbEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTableId(table_name))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*table_data_mock, GetParam(kIpsecFetchSpi, _))
      .WillOnce(DoAll(SetArgPointee<1>(8), Return(::util::OkStatus())));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_OK(fixed_function_manager_->FetchSpi(session_mock, table_name,
                                              &fetched_spi));
  EXPECT_EQ(fetched_spi, 8);
}

/*
 * Verifies FetchSpi() error handling if the underlying call to GetParam()
 * fails. It should propagate the status code and return an spi_value of zero
 */
TEST_F(TdiFixedFunctionManagerTest, FetchSpiTestFailure) {
  std::string table_name = ipsecFetchSpiTableName;
  uint32 fetched_spi;

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteSadbEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTableId(table_name))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*table_data_mock, GetParam(kIpsecFetchSpi, _))
      .WillOnce(DoAll(SetArgPointee<1>(0), Return(DefaultError())));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, _))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_THAT(
      fixed_function_manager_->FetchSpi(session_mock, table_name, &fetched_spi),
      DerivedFromStatus(DefaultError()));
  EXPECT_EQ(fetched_spi, 0);
}

/*
 * Verifies WriteSadbEntry() error handling if the underlying call to
 * SetExact() fails. It should propagate the status code.
 */
TEST_F(TdiFixedFunctionManagerTest, WriteSadbEntryTestFailure) {
  std::string table_name = ipsecConfigSadbTableName;
  const IPsecSadbConfigOp op_type = IPSEC_SADB_CONFIG_OP_ADD_ENTRY;

  // initialize IPsecSADBConfig
  IPsecSADBConfig sadb_config;
  InitSadbConfig(sadb_config);

  // mocked session object
  auto session_mock = std::make_shared<SessionMock>();
  auto table_key_mock = absl::make_unique<TableKeyMock>();

  // Specify the behavior of certain methods of the SDE Wrapper mock
  // when they are invoked by WriteSadbEntry().
  EXPECT_CALL(
      // mock object to monitor
      *sde_wrapper_mock_,
      // method signature to match
      GetTableId(table_name))
      // action to take on first and only call
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*table_key_mock,
              SetExact(kIpsecSadbOffloadId, sadb_config.offload_id()))
      .WillOnce(Return(DefaultError()));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableKey(kTdiRtTableId))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableKeyInterface>>(
              std::move(table_key_mock)))));

  // perform test and check results (act + assert)
  EXPECT_THAT(fixed_function_manager_->WriteSadbEntry(session_mock, table_name,
                                                      op_type, sadb_config),
              DerivedFromStatus(DefaultError()));
}

/*
 * Validates FetchVportTableData method.
 */
TEST_F(TdiFixedFunctionManagerTest, FetchVportTableDataTest) {
  std::string table_name = vportStateTableName;
  uint64 fetched_data;
  uint32 glort_id = 5678;
  uint32 expected_vsi_val = 12;

  auto session_mock = std::make_shared<SessionMock>();
  auto table_key_mock = absl::make_unique<TableKeyMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Set expectations BEFORE moving mocks
  EXPECT_CALL(*table_key_mock, SetExact(kGlobalResourceId, glort_id))
      .WillOnce(Return(::util::OkStatus()));
  EXPECT_CALL(*table_data_mock, GetParam(kVsi, _))
      .WillOnce(DoAll(SetArgPointee<1>(expected_vsi_val),
                      Return(::util::OkStatus())));

  // Configure SDE wrapper AFTER setting mock expectations
  EXPECT_CALL(*sde_wrapper_mock_, GetTableId(table_name))
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableKey(kTdiRtTableId))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableKeyInterface>>(
              std::move(table_key_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, 0))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_,
              GetTableEntry(kDevice1, _, kTdiRtTableId, _, _))
      .WillOnce(Return(::util::OkStatus()));

  // Execute test
  EXPECT_OK(fixed_function_manager_->FetchVportTableData(
      session_mock, table_name, glort_id, kVsi, &fetched_data));
  EXPECT_EQ(fetched_data, expected_vsi_val);
}

/*
 * Validates FetchVportTableData method error handling.
 */
TEST_F(TdiFixedFunctionManagerTest, FetchVportTableDataTestFailure) {
  std::string table_name = vportStateTableName;
  uint64 fetched_data = 0;
  uint32 glort_id = 5678;

  auto session_mock = std::make_shared<SessionMock>();
  auto table_key_mock = absl::make_unique<TableKeyMock>();
  auto table_data_mock = absl::make_unique<TableDataMock>();

  // Set expectations BEFORE moving mocks
  EXPECT_CALL(*table_key_mock, SetExact(kGlobalResourceId, glort_id))
      .WillOnce(Return(::util::OkStatus()));
  EXPECT_CALL(*table_data_mock, GetParam(kVsi, _))
      .WillOnce(Return(DefaultError()));

  // Configure SDE wrapper AFTER setting mock expectations
  EXPECT_CALL(*sde_wrapper_mock_, GetTableId(table_name))
      .WillOnce(Return(kTdiRtTableId));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableKey(kTdiRtTableId))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableKeyInterface>>(
              std::move(table_key_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_, CreateTableData(kTdiRtTableId, 0))
      .WillOnce(
          Return(ByMove(::util::StatusOr<std::unique_ptr<TableDataInterface>>(
              std::move(table_data_mock)))));

  EXPECT_CALL(*sde_wrapper_mock_,
              GetTableEntry(kDevice1, _, kTdiRtTableId, _, _))
      .WillOnce(Return(::util::OkStatus()));

  // Execute test
  EXPECT_THAT(fixed_function_manager_->FetchVportTableData(
                  session_mock, table_name, glort_id, kVsi, &fetched_data),
              DerivedFromStatus(DefaultError()));
  EXPECT_EQ(fetched_data, 0);
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
