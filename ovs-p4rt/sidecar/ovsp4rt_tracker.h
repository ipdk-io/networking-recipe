// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_TRACKER_H_
#define OVSP4RT_TRACKER_H_

#include <stdbool.h>

#include "ovsp4rt/ovs-p4rt.h"
#include "p4/v1/p4runtime.pb.h"

namespace ovs_p4rt {

// Captures the inputs and outputs to an API function.
class Tracker {
 public:
  void captureInput(const char* func, const struct mac_learning_info& info,
                    bool insert_entry) {}

  void captureOutput(const char* func, ::p4::v1::WriteRequest& request) {}

  void saveSnapshot() {}
};

}  // namespace ovs_p4rt

#endif  // OVSP4RT_TRACKER_H_
