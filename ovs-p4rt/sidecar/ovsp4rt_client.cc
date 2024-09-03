// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_client.h"

#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "session/ovsp4rt_credentials.h"
#include "session/ovsp4rt_session.h"

#define DEFAULT_ROLE_NAME "ovs-p4rt"

ABSL_FLAG(uint64_t, device_id, 1, "P4Runtime device ID.");

ABSL_FLAG(std::string, role_name, DEFAULT_ROLE_NAME, "P4 config role name.");

namespace ovsp4rt {

absl::Status Client::connect(const char* grpc_addr) {
  // Start a new client session.
  auto result = ovsp4rt::OvsP4rtSession::Create(
      grpc_addr, GenerateClientCredentials(), absl::GetFlag(FLAGS_device_id),
      absl::GetFlag(FLAGS_role_name));
  if (!result.ok()) {
    return result.status();
  }

  // Unwrap the session from the StatusOr object.
  session_ = std::move(result).value();
  return absl::OkStatus();
}

absl::Status Client::getPipelineConfig(::p4::config::v1::P4Info* p4info) {
  return GetForwardingPipelineConfig(session_.get(), p4info);
}

::p4::v1::TableEntry* Client::initReadRequest(::p4::v1::ReadRequest* request) {
  return SetupTableEntryToRead(session_.get(), request);
}

absl::StatusOr<::p4::v1::ReadResponse> Client::sendReadRequest(
    const p4::v1::ReadRequest& request) {
  return SendReadRequest(session_.get(), request);
}

::p4::v1::TableEntry* Client::initInsertRequest(
    ::p4::v1::WriteRequest* request) {
  return SetupTableEntryToInsert(session_.get(), request);
}

::p4::v1::TableEntry* Client::initModifyRequest(
    ::p4::v1::WriteRequest* request) {
  return SetupTableEntryToModify(session_.get(), request);
}

::p4::v1::TableEntry* Client::initDeleteRequest(
    ::p4::v1::WriteRequest* request) {
  return SetupTableEntryToDelete(session_.get(), request);
}

absl::Status Client::sendWriteRequest(const p4::v1::WriteRequest& request) {
  return SendWriteRequest(session_.get(), request);
}

}  // namespace ovsp4rt
