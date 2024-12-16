// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CLIENT_MOCK_H_
#define OVSP4RT_CLIENT_MOCK_H_

#include "gmock/gmock.h"
#include "ovsp4rt_client_interface.h"

namespace ovsp4rt {

class ClientMock : public ClientInterface {
 public:
  MOCK_METHOD(absl::Status, connect, (const char*));

  MOCK_METHOD(absl::Status, getPipelineConfig, (::p4::config::v1::P4Info*));

  MOCK_METHOD(::p4::v1::TableEntry*, initReadRequest, (::p4::v1::ReadRequest*));

  MOCK_METHOD(absl::StatusOr<p4::v1::ReadResponse>, sendReadRequest,
              (const p4::v1::ReadRequest&));

  MOCK_METHOD(::p4::v1::TableEntry*, initInsertRequest,
              (::p4::v1::WriteRequest*));

  MOCK_METHOD(::p4::v1::TableEntry*, initModifyRequest,
              (::p4::v1::WriteRequest*));

  MOCK_METHOD(::p4::v1::TableEntry*, initDeleteRequest,
              (::p4::v1::WriteRequest*));

  MOCK_METHOD(::p4::v1::TableEntry*, initWriteRequest,
              (::p4::v1::WriteRequest*, bool));

  MOCK_METHOD(absl::Status, sendWriteRequest,
              (const p4::v1::WriteRequest& request));
};

}  // namespace ovsp4rt

#endif  // OVSP4RT_CLIENT_MOCK_H_
