// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CLIENT_H_
#define OVSP4RT_CLIENT_H_

#include <stdint.h>

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "p4/v1/p4runtime.pb.h"
#include "session/ovsp4rt_session.h"

namespace ovsp4rt {

class Client {
 public:
  Client() {}
  virtual ~Client() = default;

  // Connects to the P4Runtime server.
  virtual absl::Status connect(const char* grpc_addr);

  // Returns a pointer to the ovsp4rt session object.
  virtual OvsP4rtSession* session() const { return session_.get(); }

  // Gets the pipeline configuration from the P4Runtime server.
  virtual absl::Status getPipelineConfig(::p4::config::v1::P4Info* p4info);

  //--------------------------------------------------------------------

  // Initializes a Read Table Entry request message.
  virtual ::p4::v1::TableEntry* initReadRequest(::p4::v1::ReadRequest* request);

  // Sends a Read Table Entry request to the P4Runtime server.
  virtual absl::StatusOr<p4::v1::ReadResponse> sendReadRequest(
      const p4::v1::ReadRequest& request);

  //--------------------------------------------------------------------

  // Initializes an Insert Table Entry request message.
  virtual ::p4::v1::TableEntry* initInsertRequest(
      ::p4::v1::WriteRequest* request);

  // Initializes a Modify Table Entry request message.
  virtual ::p4::v1::TableEntry* initModifyRequest(
      ::p4::v1::WriteRequest* request);

  // Initializes a Delete Table Entry request message.
  virtual ::p4::v1::TableEntry* initDeleteRequest(
      ::p4::v1::WriteRequest* request);

  // Initializes an Insert Table Entry or Delete Table Entry request
  // message, depending on the value of the `insert_entry` parameter.
  virtual ::p4::v1::TableEntry* initWriteRequest(
      ::p4::v1::WriteRequest* request, bool insert_entry) {
    if (insert_entry) {
      return initInsertRequest(request);
    } else {
      return initDeleteRequest(request);
    }
  }

  // Sends a Write Table Entry request to the P4Runtime server.
  virtual absl::Status sendWriteRequest(const p4::v1::WriteRequest& request);

 private:
  // Pointer to a P4Runtime session object.
  std::unique_ptr<ovsp4rt::OvsP4rtSession> session_;
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_CLIENT_H_
