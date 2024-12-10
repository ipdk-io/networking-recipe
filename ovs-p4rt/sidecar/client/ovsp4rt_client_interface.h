// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CLIENT_INTERFACE_H_
#define OVSP4RT_CLIENT_INTERFACE_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "p4/v1/p4runtime.pb.h"
#include "session/ovsp4rt_session.h"

namespace ovsp4rt {

class ClientInterface {
 public:
  virtual ~ClientInterface() = default;

  // Connects to the P4Runtime server.
  virtual absl::Status connect(const char* grpc_addr) = 0;

  // Returns a pointer to the ovsp4rt session object.
  virtual OvsP4rtSession* session() const = 0;

  // Gets the pipeline configuration from the P4Runtime server.
  virtual absl::Status getPipelineConfig(::p4::config::v1::P4Info* p4info) = 0;

  //--------------------------------------------------------------------

  // Initializes a Read Table Entry request message.
  virtual ::p4::v1::TableEntry* initReadRequest(
      ::p4::v1::ReadRequest* request) = 0;

  // Sends a Read Table Entry request to the P4Runtime server.
  virtual absl::StatusOr<p4::v1::ReadResponse> sendReadRequest(
      const p4::v1::ReadRequest& request) = 0;

  //--------------------------------------------------------------------

  // Initializes an Insert Table Entry request message.
  virtual ::p4::v1::TableEntry* initInsertRequest(
      ::p4::v1::WriteRequest* request) = 0;

  // Initializes a Modify Table Entry request message.
  virtual ::p4::v1::TableEntry* initModifyRequest(
      ::p4::v1::WriteRequest* request) = 0;

  // Initializes a Delete Table Entry request message.
  virtual ::p4::v1::TableEntry* initDeleteRequest(
      ::p4::v1::WriteRequest* request) = 0;

  // Initializes an Insert Table Entry or Delete Table Entry request
  // message, depending on the value of the `insert_entry` parameter.
  virtual ::p4::v1::TableEntry* initWriteRequest(
      ::p4::v1::WriteRequest* request, bool insert_entry) = 0;

  // Sends a Write Table Entry request to the P4Runtime server.
  virtual absl::Status sendWriteRequest(
      const p4::v1::WriteRequest& request) = 0;

 protected:
  // Default constructor. To be called by the Mock class instance only.
  ClientInterface() {}
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_CLIENT_INTERFACE_H_
