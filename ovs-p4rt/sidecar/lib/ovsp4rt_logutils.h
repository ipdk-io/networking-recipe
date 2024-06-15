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

const std::string FormatMacAddr(const uint8_t* mac_addr);

const std::string FormatTableError(bool inserting, const char* table);

void LogTableError(bool inserting, const char* table);

void LogTableErrorWithMacAddr(bool inserting, const char* table,
                              const uint8_t* mac_addr);

}  // namespace ovs_p4rt

#endif  // OVSP4RT_LOGUTILS_H_
