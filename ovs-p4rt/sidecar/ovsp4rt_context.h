// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CONTEXT_H_
#define OVSP4RT_CONTEXT_H_

#include <stdint.h>

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "p4/v1/p4runtime.pb.h"
#include "session/ovsp4rt_session.h"

namespace ovsp4rt {

class Context {
 public:
  Context() {}

  absl::Status connect(const char* grpc_addr);

  OvsP4rtSession* session() const { return session_.get(); }

  absl::Status getPipelineConfig(::p4::config::v1::P4Info* p4info);

  // Read requests
  ::p4::v1::TableEntry* initReadRequest(::p4::v1::ReadRequest* request);

  absl::StatusOr<p4::v1::ReadResponse> sendReadRequest(
      const p4::v1::ReadRequest& request);

  // Write requests
  ::p4::v1::TableEntry* initInsertRequest(::p4::v1::WriteRequest* request);
  ::p4::v1::TableEntry* initModifyRequest(::p4::v1::WriteRequest* request);
  ::p4::v1::TableEntry* initDeleteRequest(::p4::v1::WriteRequest* request);

  ::p4::v1::TableEntry* initWriteRequest(::p4::v1::WriteRequest* request,
                                         bool insert_entry) {
    if (insert_entry) {
      return initInsertRequest(request);
    } else {
      return initDeleteRequest(request);
    }
  }

  absl::Status sendWriteRequest(const p4::v1::WriteRequest& request);

 private:
  // P4Runtime session.
  std::unique_ptr<ovsp4rt::OvsP4rtSession> session_;
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_CONTEXT_H_
