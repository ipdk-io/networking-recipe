// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "bf_types/bf_types.h"
#include "gtest/gtest.h"
#include "stratum/hal/lib/tdi/tdi_status.h"

namespace stratum {
namespace hal {
namespace tdi {

class DpdkTdiStatusTest : public ::testing::Test {};

// Verify that BF status codes map directly to TDI status codes.
TEST_F(DpdkTdiStatusTest, status_codes_match) {
  ASSERT_EQ(TDI_SUCCESS, BF_SUCCESS);
  ASSERT_EQ(TDI_NO_SYS_RESOURCES, BF_NO_SYS_RESOURCES);
  ASSERT_EQ(TDI_INVALID_ARG, BF_INVALID_ARG);
  ASSERT_EQ(TDI_ALREADY_EXISTS, BF_ALREADY_EXISTS);
  ASSERT_EQ(TDI_HW_COMM_FAIL, BF_HW_COMM_FAIL);
  ASSERT_EQ(TDI_OBJECT_NOT_FOUND, BF_OBJECT_NOT_FOUND);
  ASSERT_EQ(TDI_MAX_SESSIONS_EXCEEDED, BF_MAX_SESSIONS_EXCEEDED);
  ASSERT_EQ(TDI_SESSION_NOT_FOUND, BF_SESSION_NOT_FOUND);
  ASSERT_EQ(TDI_NO_SPACE, BF_NO_SPACE);
  ASSERT_EQ(TDI_EAGAIN, BF_EAGAIN);
  ASSERT_EQ(TDI_INIT_ERROR, BF_INIT_ERROR);
  ASSERT_EQ(TDI_TXN_NOT_SUPPORTED, BF_TXN_NOT_SUPPORTED);
  ASSERT_EQ(TDI_TABLE_LOCKED, BF_TABLE_LOCKED);
  ASSERT_EQ(TDI_IO, BF_IO);
  ASSERT_EQ(TDI_UNEXPECTED, BF_UNEXPECTED);
  ASSERT_EQ(TDI_ENTRY_REFERENCES_EXIST, BF_ENTRY_REFERENCES_EXIST);
  ASSERT_EQ(TDI_NOT_SUPPORTED, BF_NOT_SUPPORTED);
  ASSERT_EQ(TDI_HW_UPDATE_FAILED, BF_HW_UPDATE_FAILED);
  ASSERT_EQ(TDI_NO_LEARN_CLIENTS, BF_NO_LEARN_CLIENTS);
  ASSERT_EQ(TDI_IDLE_UPDATE_IN_PROGRESS, BF_IDLE_UPDATE_IN_PROGRESS);
  ASSERT_EQ(TDI_DEVICE_LOCKED, BF_DEVICE_LOCKED);
  ASSERT_EQ(TDI_INTERNAL_ERROR, BF_INTERNAL_ERROR);
  ASSERT_EQ(TDI_TABLE_NOT_FOUND, BF_TABLE_NOT_FOUND);
  ASSERT_EQ(TDI_IN_USE, BF_IN_USE);
  ASSERT_EQ(TDI_NOT_IMPLEMENTED, BF_NOT_IMPLEMENTED);
  ASSERT_EQ(TDI_STS_MAX, BF_STS_MAX);
}

// Verify that status() returns the original TDI status code.
TEST_F(DpdkTdiStatusTest, status_method_returns_tdi_status) {
  TdiStatus ret(TDI_NOT_IMPLEMENTED);
  ASSERT_EQ(ret.status(), TDI_NOT_IMPLEMENTED);
}

// Verify that error_code() returns the corresponding Stratum ErrorCode.
TEST_F(DpdkTdiStatusTest, error_code_method_returns_errorcode) {
  TdiStatus ret(TDI_NO_SPACE);
  ASSERT_EQ(ret.error_code(), ERR_NO_RESOURCE);
}

// Verify that success status evaluates to Boolean True.
TEST_F(DpdkTdiStatusTest, successful_status_is_boolean_true) {
  TdiStatus okay(TDI_SUCCESS);
  ASSERT_TRUE(okay);
}

// Verify that failure status evaluates to Boolean False.
TEST_F(DpdkTdiStatusTest, failure_status_is_boolean_false) {
  TdiStatus not_okay(TDI_INTERNAL_ERROR);
  ASSERT_FALSE(not_okay);
}

}  // namespace tdi
}  // namespace hal
}  // namespace stratum
