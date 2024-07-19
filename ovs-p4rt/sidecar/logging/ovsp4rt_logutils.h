// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Logger utility function prototypes.
//

#ifndef OVSP4RT_LOGUTILS_H_
#define OVSP4RT_LOGUTILS_H_

#include <cstdint>
#include <string>

namespace ovsp4rt {

const std::string FormatMacAddr(const uint8_t* mac_addr);

void LogFailure(bool inserting, const char* table);

void LogFailureWithMacAddr(bool inserting, const char* table,
                           const uint8_t* mac_addr);

}  // namespace ovsp4rt

#endif  // OVSP4RT_LOGUTILS_H_
