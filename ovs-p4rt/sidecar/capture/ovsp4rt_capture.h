// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_CAPTURE_H
#define OVSP4RT_CAPTURE_H

#include <stdbool.h>

#include "ovsp4rt/ovs-p4rt.h"

namespace ovs_p4rt {

extern void CaptureMacLearningInfo(const char* func_name,
                                   const struct mac_learning_info& learn_info,
                                   bool insert_entry);

extern void CaptureSrcPortInfo(const char* func_name,
                               const struct src_port_info& sp_info,
                               bool insert_entry);

}  // namespace ovs_p4rt

#endif  // OVSP4RT_CAPTURE_H
