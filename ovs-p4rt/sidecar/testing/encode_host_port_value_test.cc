// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <arpa/inet.h>
#include <stdarg.h>

#include <string>

#include "gtest/gtest.h"
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovs_p4rt {

//----------------------------------------------------------------------
// Proposed new function (UUT)
//----------------------------------------------------------------------
inline std::string EncodeHostPortValue(uint16_t port_value) {
  port_value = htons(port_value);
  return EncodeByteValue(2, (((port_value * 2) >> 8) & 0xff),
                         ((port_value * 2) & 0xff));
}

//----------------------------------------------------------------------
// Test case: port_encodings_are_identical
//
// Verify that EncodeHostPortValue() returns the same value as the
// manual code it is intended to replace.
//----------------------------------------------------------------------
class EncodeHostPortTestValue : public ::testing::Test {};

TEST(EncodeHostPortTestValue, port_encodings_are_identical) {
  constexpr uint16_t DST_PORT_VALUE = 42;

  // Arrange
  struct tunnel_info tunnel_info = {0};
  tunnel_info.dst_port = DST_PORT_VALUE;

  // Act
  // 1. control value: logic extracted from ovsp4rt.cc
  uint16_t dst_port = htons(tunnel_info.dst_port);

  const std::string control_value = EncodeByteValue(
      2, (((dst_port * 2) >> 8) & 0xff), ((dst_port * 2) & 0xff));

  // 2. experimental value: proposed replacement logic
  const std::string experimental_value =
      EncodeHostPortValue(tunnel_info.dst_port);

  // Assert
  ASSERT_STREQ(experimental_value.c_str(), control_value.c_str());
}

}  // namespace ovs_p4rt
