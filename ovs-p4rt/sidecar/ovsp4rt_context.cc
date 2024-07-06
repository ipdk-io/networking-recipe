// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_context.h"

#include "absl/flags/flag.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ovsp4rt_credentials.h"
#include "ovsp4rt_session.h"

#define DEFAULT_OVS_P4RT_ROLE_NAME "ovs-p4rt"

ABSL_FLAG(uint64_t, device_id, 1, "P4Runtime device ID.");

ABSL_FLAG(std::string, role_name, DEFAULT_OVS_P4RT_ROLE_NAME,
          "P4 config role name.");

namespace ovs_p4rt {

absl::Status Ovsp4rtContext::connect(const char* grpc_addr) {
  // Start a new client session.
  auto result = ovs_p4rt::OvsP4rtSession::Create(
      grpc_addr, GenerateClientCredentials(), absl::GetFlag(FLAGS_device_id),
      absl::GetFlag(FLAGS_role_name));
  if (!result.ok()) {
    return result.status();
  }

  // Unwrap the session from the StatusOr object.
  session_ = std::move(result).value();
  return absl::OkStatus();
}

absl::Status Ovsp4rtContext::getPipelineConfig(
    ::p4::config::v1::P4Info* p4info) {
  return GetForwardingPipelineConfig(session_.get(), p4info);
}

::p4::v1::TableEntry* Ovsp4rtContext::initReadRequest(
    ::p4::v1::ReadRequest* request) {
  return SetupTableEntryToRead(session_.get(), request);
}

absl::StatusOr<::p4::v1::ReadResponse> Ovsp4rtContext::sendReadRequest(
    const p4::v1::ReadRequest& request) {
  return SendReadRequest(session_.get(), request);
}

::p4::v1::TableEntry* Ovsp4rtContext::initInsertRequest(
    ::p4::v1::WriteRequest* request) {
  return SetupTableEntryToInsert(session_.get(), request);
}

::p4::v1::TableEntry* Ovsp4rtContext::initModifyRequest(
    ::p4::v1::WriteRequest* request) {
  return SetupTableEntryToModify(session_.get(), request);
}

::p4::v1::TableEntry* Ovsp4rtContext::initDeleteRequest(
    ::p4::v1::WriteRequest* request) {
  return SetupTableEntryToDelete(session_.get(), request);
}

absl::Status Ovsp4rtContext::sendWriteRequest(
    const p4::v1::WriteRequest& request) {
  return SendWriteRequest(session_.get(), request);
}

}  // namespace ovs_p4rt
