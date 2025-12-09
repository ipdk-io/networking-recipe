// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_ES2K_ES2K_NODE_MOCK_H_
#define STRATUM_HAL_LIB_ES2K_ES2K_NODE_MOCK_H_

#include <map>
#include <memory>
#include <vector>

#include "gmock/gmock.h"
#include "stratum/hal/lib/tdi/es2k/es2k_node.h"

namespace stratum {
namespace hal {
namespace tdi {

class Es2kNodeMock : public Es2kNode {
 public:
  MOCK_METHOD(::util::Status, PushChassisConfig,
              (const ChassisConfig& config, uint64 node_id));

  MOCK_METHOD(::util::Status, VerifyChassisConfig,
              (const ChassisConfig& config, uint64 node_id));

  MOCK_METHOD(::util::Status, PushForwardingPipelineConfig,
              (const ::p4::v1::ForwardingPipelineConfig& config));

  MOCK_METHOD(::util::Status, VerifyForwardingPipelineConfig,
              (const ::p4::v1::ForwardingPipelineConfig& config), (const));

  MOCK_METHOD(::util::Status, Shutdown, ());
  MOCK_METHOD(::util::Status, Freeze, ());
  MOCK_METHOD(::util::Status, Unfreeze, ());

  MOCK_METHOD(::util::Status, WriteForwardingEntries,
              (const ::p4::v1::WriteRequest& req,
               std::vector<::util::Status>* results));

  MOCK_METHOD(::util::Status, ReadForwardingEntries,
              (const ::p4::v1::ReadRequest& req,
               WriterInterface<::p4::v1::ReadResponse>* writer,
               std::vector<::util::Status>* details));

  MOCK_METHOD(::util::Status, RegisterStreamMessageResponseWriter,
              (const std::shared_ptr<
                  WriterInterface<::p4::v1::StreamMessageResponse>>& writer));

  MOCK_METHOD(::util::Status, HandleStreamMessageRequest,
              (const ::p4::v1::StreamMessageRequest& req));
};

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_ES2K_ES2K_NODE_MOCK_H_
