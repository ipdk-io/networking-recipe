// Copyright 2020 Google LLC
// Copyright 2021-present Open Networking Foundation
// Copyright (c) 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <string>

#include "grpcpp/channel.h"
#include "grpcpp/create_channel.h"
#include "p4/v1/p4runtime.grpc.pb.h"
#include "p4/v1/p4runtime.pb.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "ovs_p4rt_session.h"

namespace ovs_p4rt_cpp {
using ::p4::config::v1::P4Info;
using ::p4::v1::GetForwardingPipelineConfigRequest;
using ::p4::v1::GetForwardingPipelineConfigResponse;
using ::p4::v1::P4Runtime;
using ::p4::v1::ReadRequest;
using ::p4::v1::ReadResponse;
using ::p4::v1::TableEntry;
using ::p4::v1::Update;
using ::p4::v1::WriteRequest;
using ::p4::v1::WriteResponse;

// Helper functions
grpc::Status AbslStatusToGrpcStatus(const absl::Status& status) {
  return grpc::Status(static_cast<grpc::StatusCode>(status.code()),
                      std::string(status.message()));
}

absl::Status GrpcStatusToAbslStatus(const grpc::Status& status) {
  return absl::Status(static_cast<absl::StatusCode>(status.error_code()),
                      status.error_message());
}

// Create P4Runtime Stub.
std::unique_ptr<P4Runtime::Stub> CreateP4RuntimeStub(
    const std::string& address,
    const std::shared_ptr<grpc::ChannelCredentials>& credentials) {
  grpc::ChannelArguments args;
  return P4Runtime::NewStub(
      grpc::CreateCustomChannel(address, credentials, args));
}

// Creates a session with the switch, which lasts until the session object is
// destructed.
absl::StatusOr<std::unique_ptr<OvsP4rtSession>> OvsP4rtSession::Create(
    std::unique_ptr<P4Runtime::Stub> stub, uint32_t device_id,
    absl::uint128 election_id) {
  std::unique_ptr<OvsP4rtSession> session = absl::WrapUnique(
      new OvsP4rtSession(device_id, std::move(stub), election_id));

  // Send arbitration request.
  p4::v1::StreamMessageRequest arbt_request;
  auto arbitration = arbt_request.mutable_arbitration();
  arbitration->set_device_id(device_id);
  *arbitration->mutable_election_id() = session->election_id_;
  if (!session->stream_channel_->Write(arbt_request)) {
      session->stream_channel_->Finish();	  
      return ::absl::UnavailableError("Unable to initiate P4RT connection "
          "for Write request, closing stream channel");
  }

  // Wait for arbitration response from P4RT server.
  p4::v1::StreamMessageResponse response;
  if (!session->stream_channel_->Read(&response)) {
      session->stream_channel_->Finish();
      return ::absl::InternalError("No arbitration response received");
  }
  if (response.update_case() != p4::v1::StreamMessageResponse::kArbitration) {
     return ::absl::InternalError("No arbitration update received");
  }
  if (response.arbitration().device_id() != session->device_id_) {
      return ::absl::InternalError("Received device id doesn't match");
  }

  return std::move(session);
}

// Creates a session with the switch, which lasts until the session object is
// destructed.
absl::StatusOr<std::unique_ptr<OvsP4rtSession>> OvsP4rtSession::Create(
    const std::string& address,
    const std::shared_ptr<grpc::ChannelCredentials>& credentials,
    uint32_t device_id, absl::uint128 election_id) {
  return Create(CreateP4RuntimeStub(address, credentials), device_id,
                election_id);
}

absl::Status GetForwardingPipelineConfig(OvsP4rtSession* session,
                                         p4::config::v1::P4Info* p4info) {
  GetForwardingPipelineConfigRequest request;
  request.set_device_id(session->DeviceId());
  request.set_response_type(GetForwardingPipelineConfigRequest::P4INFO_AND_COOKIE);

  GetForwardingPipelineConfigResponse response;
  grpc::ClientContext context;
  absl::Status status = GrpcStatusToAbslStatus(session->Stub().GetForwardingPipelineConfig(
          &context, request, &response));

  *p4info = response.config().p4info();

  return absl::OkStatus();
}

absl::StatusOr<ReadResponse> SendReadRequest(OvsP4rtSession* session,
                                             const ReadRequest& read_request) {
  grpc::ClientContext context;
  auto reader = session->Stub().Read(&context, read_request);

  ReadResponse response;
  ReadResponse partial_response;
  while (reader->Read(&partial_response)) {
    response.MergeFrom(partial_response);
  }

  grpc::Status reader_status = reader->Finish();
  if (!reader_status.ok()) {
    return GrpcStatusToAbslStatus(reader_status);
  }

  return std::move(response);
}

absl::Status SendWriteRequest(OvsP4rtSession* session,
                              const WriteRequest& write_request) {
  grpc::ClientContext context;
  WriteResponse response;

  ::grpc::Status status =
      session->Stub().Write(&context, write_request, &response);

  return GrpcStatusToAbslStatus(status);
}

::p4::v1::TableEntry* SetupTableEntryToInsert(OvsP4rtSession* session,
		             ::p4::v1::WriteRequest* req, ::p4::v1::Entity *entity) {
  req->set_device_id(session->DeviceId());
  *req->mutable_election_id() = session->ElectionId();
  auto* update = req->add_updates();
  update->set_type(::p4::v1::Update::INSERT);
  return update->mutable_entity()->mutable_table_entry();
}

::p4::v1::TableEntry* SetupTableEntryToModify(OvsP4rtSession* session,
                             ::p4::v1::WriteRequest* req, ::p4::v1::Entity *entity) {
  req->set_device_id(session->DeviceId());
  *req->mutable_election_id() = session->ElectionId();
  auto* update = req->add_updates();
  update->set_type(::p4::v1::Update::MODIFY);
  return update->mutable_entity()->mutable_table_entry();
}

::p4::v1::TableEntry* SetupTableEntryToDelete(OvsP4rtSession* session,
                             ::p4::v1::WriteRequest* req, ::p4::v1::Entity *entity) {
  req->set_device_id(session->DeviceId());
  *req->mutable_election_id() = session->ElectionId();
  auto* update = req->add_updates();
  update->set_type(::p4::v1::Update::DELETE);
  return update->mutable_entity()->mutable_table_entry();
}

}  // namespace ovs_p4rt_cpp
