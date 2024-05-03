// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Logger utility function prototypes.
//

#ifndef OVSP4RT_LOGUTILS_H_
#define OVSP4RT_LOGUTILS_H_

#include <cstdint>
#include <string>

namespace ovs_p4rt {

const std::string FormatMac(const uint8_t* mac_addr);

void LogTableError(bool inserting, const char* table);

const std::string TableErrorMessage(bool inserting, const char* table);

}  // namespace ovs_p4rt

#endif  // OVSP4RT_LOGUTILS_H_
