// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
// Logger utility functions.
//

#include "common/ovsp4rt_logutils.h"

#include <cstdio>
#include <string>

#include "absl/strings/str_cat.h"
#include "lib/ovsp4rt_logging.h"

namespace ovs_p4rt {

const std::string TableErrorMessage(bool inserting, const char* table) {
  if (inserting) {
    return absl::StrCat("Error adding entry to ", table);
  } else {
    return absl::StrCat("Error deleting entry from ", table);
  }
}

const std::string FormatMac(const uint8_t* mac_addr) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%x:%x:%x:%x:%x:%x", mac_addr[0], mac_addr[1],
           mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  return buf;
}

void LogTableError(bool inserting, const char* table) {
  ovsp4rt_log_error("%s", TableErrorMessage(inserting, table).c_str());
}

}  // namespace ovs_p4rt
