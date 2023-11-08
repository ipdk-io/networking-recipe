// Copyright 2020 Google LLC
// Copyright 2021-present Open Networking Foundation
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_SESSION_H_
#define OVSP4RT_SESSION_H_

#include <grpcpp/grpcpp.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "p4/v1/p4runtime.grpc.pb.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

// Generates an election id that increases monotonically over time.
// Specifically, the upper 64 bits are the unix timestamp in seconds, and the
// lower 64 bits are 0. This is compatible with election systems that use the
// same epoch-based election IDs, and in that case, this election ID will be
// guaranteed to be higher than any previous election ID, allowing the user
// to switch between ovs-p4rt and p4rt-ctl as master controller.
inline ::absl::uint128 TimeBasedElectionId() {
  return ::absl::MakeUint128(::absl::ToUnixSeconds(::absl::Now()), 0);
}

class OvsP4rtSession {
 public:
  // Create the session with given P4runtime stub and device id
  static ::absl::StatusOr<std::unique_ptr<OvsP4rtSession>> Create(
      std::unique_ptr<p4::v1::P4Runtime::Stub> stub, uint32_t device_id,
      ::absl::uint128 election_id = TimeBasedElectionId());

  // Create the session with given grpc address, channel credentials
  // and device id
  static ::absl::StatusOr<std::unique_ptr<OvsP4rtSession>> Create(
      const std::string& address,
      const std::shared_ptr<grpc::ChannelCredentials>& credentials,
      uint32_t device_id, ::absl::uint128 election_id = TimeBasedElectionId());

  // Disable copy semantics.
  OvsP4rtSession(const OvsP4rtSession&) = delete;
  OvsP4rtSession& operator=(const OvsP4rtSession&) = delete;

  // Allow move semantics.
  OvsP4rtSession(OvsP4rtSession&&) = default;
  OvsP4rtSession& operator=(OvsP4rtSession&&) = default;

  uint32_t DeviceId() const { return device_id_; }

  p4::v1::Uint128 ElectionId() const { return election_id_; }

  p4::v1::P4Runtime::Stub& Stub() { return *stub_; }

 private:
  OvsP4rtSession(uint32_t device_id,
                 std::unique_ptr<p4::v1::P4Runtime::Stub> stub,
                 ::absl::uint128 election_id)
      : device_id_(device_id),
        stub_(std::move(stub)),
        stream_channel_context_(::absl::make_unique<grpc::ClientContext>()),
        stream_channel_(stub_->StreamChannel(stream_channel_context_.get())) {
    election_id_.set_high(::absl::Uint128High64(election_id));
    election_id_.set_low(::absl::Uint128Low64(election_id));
  }

  uint32_t device_id_;

  p4::v1::Uint128 election_id_;

  std::unique_ptr<p4::v1::P4Runtime::Stub> stub_;

  std::unique_ptr<grpc::ClientContext> stream_channel_context_;

  std::unique_ptr<grpc::ClientReaderWriter<p4::v1::StreamMessageRequest,
                                           p4::v1::StreamMessageResponse>>
      stream_channel_;
};

std::unique_ptr<p4::v1::P4Runtime::Stub> CreateP4RuntimeStub(
    const std::string& address,
    const std::shared_ptr<grpc::ChannelCredentials>& credentials);

// Functions that operate on a OvsP4rtSession.

::absl::StatusOr<p4::v1::ReadResponse> SendReadRequest(
    OvsP4rtSession* session, const p4::v1::ReadRequest& read_request);

::absl::Status SendWriteRequest(OvsP4rtSession* session,
                                const p4::v1::WriteRequest& write_request);

::absl::Status GetForwardingPipelineConfig(OvsP4rtSession* session,
                                           p4::config::v1::P4Info* p4info);

::p4::v1::TableEntry* SetupTableEntryToInsert(OvsP4rtSession* session,
                                              ::p4::v1::WriteRequest* req);

::p4::v1::TableEntry* SetupTableEntryToModify(OvsP4rtSession* session,
                                              ::p4::v1::WriteRequest* req);

::p4::v1::TableEntry* SetupTableEntryToDelete(OvsP4rtSession* session,
                                              ::p4::v1::WriteRequest* req);

}  // namespace ovs_p4rt

#endif  // OVS_P4RT_SESSION_H_
