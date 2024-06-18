// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Logger utility functions.
//

#include "ovsp4rt_logutils.h"

#include <cstdio>
#include <string>

#include "ovsp4rt_logging.h"

namespace {
const char* MessagePrefix(bool inserting) {
  return (inserting) ? "Error adding entry to" : "Error deleting entry from";
}
}  // namespace

namespace ovs_p4rt {

const std::string FormatMacAddr(const uint8_t* mac_addr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%x:%x:%x:%x:%x:%x", mac_addr[0], mac_addr[1],
           mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  return buf;
}

void LogTableError(bool inserting, const char* table) {
  ovsp4rt_log_error("%s %s", MessagePrefix(inserting), table);
}

void LogTableErrorWithMacAddr(bool inserting, const char* table,
                              const uint8_t* mac_addr) {
  ovsp4rt_log_error("%s %s for %s", MessagePrefix(inserting), table,
                    FormatMacAddr(mac_addr).c_str());
}

}  // namespace ovs_p4rt
