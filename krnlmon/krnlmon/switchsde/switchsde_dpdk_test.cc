// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "bf_types/bf_types.h"
#include "gtest/gtest.h"
#include "sde_status.h"
#include "sde_types.h"

//----------------------------------------------------------------------
// Test fixture
//----------------------------------------------------------------------
class DpdkSdeTest : public ::testing::Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {}

  // Tears down the test fixture.
  void TearDown() override {}
};

TEST_F(DpdkSdeTest, status_codes_match) {
  ASSERT_EQ(SDE_SUCCESS, BF_SUCCESS);
  ASSERT_EQ(SDE_NO_SYS_RESOURCES, BF_NO_SYS_RESOURCES);
  ASSERT_EQ(SDE_INVALID_ARG, BF_INVALID_ARG);
  ASSERT_EQ(SDE_ALREADY_EXISTS, BF_ALREADY_EXISTS);
  ASSERT_EQ(SDE_HW_COMM_FAIL, BF_HW_COMM_FAIL);
  ASSERT_EQ(SDE_OBJECT_NOT_FOUND, BF_OBJECT_NOT_FOUND);
  ASSERT_EQ(SDE_MAX_SESSIONS_EXCEEDED, BF_MAX_SESSIONS_EXCEEDED);
  ASSERT_EQ(SDE_SESSION_NOT_FOUND, BF_SESSION_NOT_FOUND);
  ASSERT_EQ(SDE_NO_SPACE, BF_NO_SPACE);
  ASSERT_EQ(SDE_EAGAIN, BF_EAGAIN);
  ASSERT_EQ(SDE_INIT_ERROR, BF_INIT_ERROR);
  ASSERT_EQ(SDE_TXN_NOT_SUPPORTED, BF_TXN_NOT_SUPPORTED);
  ASSERT_EQ(SDE_TABLE_LOCKED, BF_TABLE_LOCKED);
  ASSERT_EQ(SDE_IO, BF_IO);
  ASSERT_EQ(SDE_UNEXPECTED, BF_UNEXPECTED);
  ASSERT_EQ(SDE_ENTRY_REFERENCES_EXIST, BF_ENTRY_REFERENCES_EXIST);
  ASSERT_EQ(SDE_NOT_SUPPORTED, BF_NOT_SUPPORTED);
  ASSERT_EQ(SDE_HW_UPDATE_FAILED, BF_HW_UPDATE_FAILED);
  ASSERT_EQ(SDE_NO_LEARN_CLIENTS, BF_NO_LEARN_CLIENTS);
  ASSERT_EQ(SDE_IDLE_UPDATE_IN_PROGRESS, BF_IDLE_UPDATE_IN_PROGRESS);
  ASSERT_EQ(SDE_DEVICE_LOCKED, BF_DEVICE_LOCKED);
  ASSERT_EQ(SDE_INTERNAL_ERROR, BF_INTERNAL_ERROR);
  ASSERT_EQ(SDE_TABLE_NOT_FOUND, BF_TABLE_NOT_FOUND);
  ASSERT_EQ(SDE_IN_USE, BF_IN_USE);
  ASSERT_EQ(SDE_NOT_IMPLEMENTED, BF_NOT_IMPLEMENTED);
  ASSERT_EQ(SDE_STS_MAX, BF_STS_MAX);
}

TEST_F(DpdkSdeTest, typedef_sizes_match) {
  ASSERT_EQ(sizeof(sde_dev_id_t), sizeof(bf_dev_id_t));
  ASSERT_EQ(sizeof(sde_dev_port_t), sizeof(bf_dev_port_t));
  ASSERT_EQ(sizeof(sde_status_t), sizeof(bf_status_t));
}
