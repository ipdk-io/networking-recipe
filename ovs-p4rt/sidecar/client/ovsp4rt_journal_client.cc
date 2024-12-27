// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_journal_client.h"

namespace ovsp4rt {

// TODO(derek): func_name parameter?
absl::StatusOr<::p4::v1::ReadResponse> JournalClient::sendReadRequest(
    const p4::v1::ReadRequest& request) {
  journal_.recordReadRequest(request);
  auto response = Client::sendReadRequest(request);
  journal_.recordReadResponse(response);
  return response;
}

// TODO(derek): func_name parameter?
absl::Status JournalClient::sendWriteRequest(
    const p4::v1::WriteRequest& request) {
  journal_.recordWriteRequest(request);
  auto status = Client::sendWriteRequest(request);
  journal_.recordWriteStatus(status);
  return status;
}

}  // namespace ovsp4rt
