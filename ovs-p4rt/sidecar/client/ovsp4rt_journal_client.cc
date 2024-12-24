// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_journal_client.h"

namespace ovsp4rt {

absl::StatusOr<::p4::v1::ReadResponse> JournalClient::sendReadRequest(
    const p4::v1::ReadRequest& request) {
#if 0
  _journal.recordReadRequest(request);
  auto response = Client::sendReadRequest(request);
  _journal.recordReadResponse(response);
#else
  return Client::sendReadRequest(request);
#endif
}

absl::Status JournalClient::sendWriteRequest(
    const p4::v1::WriteRequest& request) {
#if 0
  _journal.recordWriteRequest(request);
  auto response = Client::sendWriteRequest(request);
  _journal.recordWriteResponse(response);
#else
  return Client::sendWriteRequest(request);
#endif
}

}  // namespace ovsp4rt
