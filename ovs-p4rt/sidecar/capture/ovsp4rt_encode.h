// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_ENCODE_H
#define OVSP4RT_ENCODE_H

#include <nlohmann/json.hpp>

#include "ovsp4rt/ovs-p4rt.h"

namespace ovs_p4rt {

nlohmann::json EncodeMacLearningInfo(const char* func_name,
                                     const struct mac_learning_info& learn_info,
                                     bool insert_entry);

}  // namespace ovs_p4rt

#endif  // OVSP4RT_ENCODE_H
