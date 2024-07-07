// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "capture_api_input.h"

#include "ovsp4rt_encode.h"

namespace ovs_p4rt {

void CaptureApiInput::encode(const char* func_name,
                             const mac_learning_info& info, bool insert_entry) {
  json_ = EncodeMacLearningInfo(func_name, info, insert_entry);
}

}  // namespace ovs_p4rt
