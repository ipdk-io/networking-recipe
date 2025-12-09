// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"
#include "ipu_types/ipu_types.h"
#include "stratum/hal/lib/tdi/tdi_status.h"

namespace stratum {
namespace hal {
namespace tdi {

class Es2kTdiStatusTest : public ::testing::Test {};

// Verify that IPU status codes map directly to TDI status codes.
TEST_F(Es2kTdiStatusTest, status_codes_match) {
  ASSERT_EQ(TDI_SUCCESS, IPU_SUCCESS);
  ASSERT_EQ(TDI_NO_SYS_RESOURCES, IPU_NO_SYS_RESOURCES);
  ASSERT_EQ(TDI_INVALID_ARG, IPU_INVALID_ARG);
  ASSERT_EQ(TDI_ALREADY_EXISTS, IPU_ALREADY_EXISTS);
  ASSERT_EQ(TDI_HW_COMM_FAIL, IPU_HW_COMM_FAIL);
  ASSERT_EQ(TDI_OBJECT_NOT_FOUND, IPU_OBJECT_NOT_FOUND);
  ASSERT_EQ(TDI_MAX_SESSIONS_EXCEEDED, IPU_MAX_SESSIONS_EXCEEDED);
  ASSERT_EQ(TDI_SESSION_NOT_FOUND, IPU_SESSION_NOT_FOUND);
  ASSERT_EQ(TDI_NO_SPACE, IPU_NO_SPACE);
  ASSERT_EQ(TDI_EAGAIN, IPU_EAGAIN);
  ASSERT_EQ(TDI_INIT_ERROR, IPU_INIT_ERROR);
  ASSERT_EQ(TDI_TXN_NOT_SUPPORTED, IPU_TXN_NOT_SUPPORTED);
  ASSERT_EQ(TDI_TABLE_LOCKED, IPU_TABLE_LOCKED);
  ASSERT_EQ(TDI_IO, IPU_IO);
  ASSERT_EQ(TDI_UNEXPECTED, IPU_UNEXPECTED);
  ASSERT_EQ(TDI_ENTRY_REFERENCES_EXIST, IPU_ENTRY_REFERENCES_EXIST);
  ASSERT_EQ(TDI_NOT_SUPPORTED, IPU_NOT_SUPPORTED);
  ASSERT_EQ(TDI_HW_UPDATE_FAILED, IPU_HW_UPDATE_FAILED);
  ASSERT_EQ(TDI_NO_LEARN_CLIENTS, IPU_NO_LEARN_CLIENTS);
  ASSERT_EQ(TDI_IDLE_UPDATE_IN_PROGRESS, IPU_IDLE_UPDATE_IN_PROGRESS);
  ASSERT_EQ(TDI_DEVICE_LOCKED, IPU_DEVICE_LOCKED);
  ASSERT_EQ(TDI_INTERNAL_ERROR, IPU_INTERNAL_ERROR);
  ASSERT_EQ(TDI_TABLE_NOT_FOUND, IPU_TABLE_NOT_FOUND);
  ASSERT_EQ(TDI_IN_USE, IPU_IN_USE);
  ASSERT_EQ(TDI_NOT_IMPLEMENTED, IPU_NOT_IMPLEMENTED);
  ASSERT_EQ(TDI_STS_MAX, IPU_STS_MAX);
}

// Verify that status() returns the original TDI status code.
TEST_F(Es2kTdiStatusTest, status_method_returns_tdi_status) {
  TdiStatus ret(TDI_NOT_IMPLEMENTED);
  ASSERT_EQ(ret.status(), TDI_NOT_IMPLEMENTED);
}

// Verify that error_code() returns the corresponding Stratum ErrorCode.
TEST_F(Es2kTdiStatusTest, error_code_method_returns_errorcode) {
  TdiStatus ret(TDI_NO_SPACE);
  ASSERT_EQ(ret.error_code(), ERR_NO_RESOURCE);
}

// Verify that success status evaluates to Boolean True.
TEST_F(Es2kTdiStatusTest, successful_status_is_boolean_true) {
  TdiStatus okay(TDI_SUCCESS);
  ASSERT_TRUE(okay);
}

// Verify that failure status evaluates to Boolean False.
TEST_F(Es2kTdiStatusTest, failure_status_is_boolean_false) {
  TdiStatus not_okay(TDI_INTERNAL_ERROR);
  ASSERT_FALSE(not_okay);
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
