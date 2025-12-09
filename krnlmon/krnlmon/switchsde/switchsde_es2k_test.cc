// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// clang-format off
// clang-format -n reports the following error:
//   error: code should be clang-formatted [-Wclang-format-violations]
// This appears to be a bug. The file has already been been clang-formatted,
// and clang-format only reports the error if -n is specified.
#include "ipu_types/ipu_types.h"
#include "gtest/gtest.h"
#include "sde_status.h"
#include "sde_types.h"

//----------------------------------------------------------------------
// Test fixture
//----------------------------------------------------------------------
class Es2kSdeTest : public ::testing::Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {}

  // Tears down the test fixture.
  void TearDown() override {}
};

TEST_F(Es2kSdeTest, status_codes_match) {
  ASSERT_EQ(SDE_SUCCESS, IPU_SUCCESS);
  ASSERT_EQ(SDE_NO_SYS_RESOURCES, IPU_NO_SYS_RESOURCES);
  ASSERT_EQ(SDE_INVALID_ARG, IPU_INVALID_ARG);
  ASSERT_EQ(SDE_ALREADY_EXISTS, IPU_ALREADY_EXISTS);
  ASSERT_EQ(SDE_HW_COMM_FAIL, IPU_HW_COMM_FAIL);
  ASSERT_EQ(SDE_OBJECT_NOT_FOUND, IPU_OBJECT_NOT_FOUND);
  ASSERT_EQ(SDE_MAX_SESSIONS_EXCEEDED, IPU_MAX_SESSIONS_EXCEEDED);
  ASSERT_EQ(SDE_SESSION_NOT_FOUND, IPU_SESSION_NOT_FOUND);
  ASSERT_EQ(SDE_NO_SPACE, IPU_NO_SPACE);
  ASSERT_EQ(SDE_EAGAIN, IPU_EAGAIN);
  ASSERT_EQ(SDE_INIT_ERROR, IPU_INIT_ERROR);
  ASSERT_EQ(SDE_TXN_NOT_SUPPORTED, IPU_TXN_NOT_SUPPORTED);
  ASSERT_EQ(SDE_TABLE_LOCKED, IPU_TABLE_LOCKED);
  ASSERT_EQ(SDE_IO, IPU_IO);
  ASSERT_EQ(SDE_UNEXPECTED, IPU_UNEXPECTED);
  ASSERT_EQ(SDE_ENTRY_REFERENCES_EXIST, IPU_ENTRY_REFERENCES_EXIST);
  ASSERT_EQ(SDE_NOT_SUPPORTED, IPU_NOT_SUPPORTED);
  ASSERT_EQ(SDE_HW_UPDATE_FAILED, IPU_HW_UPDATE_FAILED);
  ASSERT_EQ(SDE_NO_LEARN_CLIENTS, IPU_NO_LEARN_CLIENTS);
  ASSERT_EQ(SDE_IDLE_UPDATE_IN_PROGRESS, IPU_IDLE_UPDATE_IN_PROGRESS);
  ASSERT_EQ(SDE_DEVICE_LOCKED, IPU_DEVICE_LOCKED);
  ASSERT_EQ(SDE_INTERNAL_ERROR, IPU_INTERNAL_ERROR);
  ASSERT_EQ(SDE_TABLE_NOT_FOUND, IPU_TABLE_NOT_FOUND);
  ASSERT_EQ(SDE_IN_USE, IPU_IN_USE);
  ASSERT_EQ(SDE_NOT_IMPLEMENTED, IPU_NOT_IMPLEMENTED);
  ASSERT_EQ(SDE_STS_MAX, IPU_STS_MAX);
}

TEST_F(Es2kSdeTest, typedef_sizes_match) {
  ASSERT_EQ(sizeof(sde_dev_id_t), sizeof(ipu_dev_id_t));
  ASSERT_EQ(sizeof(sde_dev_port_t), sizeof(ipu_dev_port_t));
  ASSERT_EQ(sizeof(sde_status_t), sizeof(ipu_status_t));
}
