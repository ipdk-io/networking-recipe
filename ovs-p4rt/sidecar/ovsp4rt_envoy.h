// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_ENVOY_H_
#define OVSP4RT_ENVOY_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ovsp4rt_session.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovsp4rt {

// Abstracts the interface to the P4Runtime session.
// SIMPLIFIED VERSION FOR EARLY TESTING.
class Envoy {
 public:
  absl::StatusOr<p4::v1::ReadResponse> sendReadRequest(
      const p4::v1::ReadRequest& read_request) {
    return absl::OkStatus();
  }

  absl::Status sendWriteRequest(const p4::v1::WriteRequest& write_request) {
    return absl::OkStatus();
  }
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_ENVOY_H_
