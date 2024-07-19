// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "logging/ovsp4rt_logging.h"

#define _POSIX_SOURCE
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "logging/ovsp4rt_diag_detail.h"
#include "logging/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"

using namespace ovsp4rt;

void adding_test() {
  DiagDetail detail(LOG_L2_FWD_SMAC_TABLE);
  uint8_t mac_addr[6] = {0xd, 0xe, 0xa, 0xd, 0, 0};
  LogFailureWithMacAddr(true, detail.getLogTableName(), mac_addr);
}

void removing_test() {
  DiagDetail detail(LOG_L2_FWD_RX_WITH_TUNNEL_TABLE);
  uint8_t mac_addr[6] = {0xb, 0xe, 0xe, 0xf, 0, 0};
  LogFailureWithMacAddr(false, detail.getLogTableName(), mac_addr);
}

void failure_test() {
  DiagDetail detail(LOG_SRC_IP_MAC_MAP_TABLE);
  LogFailure(true, detail.getLogTableName());
}

void log_messages() {
  constexpr char MESSAGE_TEXT[] = "Error adding to %s: entry already exists";

  ovsp4rt_log_debug(MESSAGE_TEXT, "DEBUG_TABLE");
  ovsp4rt_log_error(MESSAGE_TEXT, "ERROR_TABLE");
  ovsp4rt_log_info(MESSAGE_TEXT, "INFO_TABLE");
  ovsp4rt_log_warn(MESSAGE_TEXT, "WARN_TABLE");

  adding_test();
  removing_test();
  failure_test();
}

#if 0
constexpr char cfg_file_path[] = {"/.local/etc/ovsp4rt/ovsp4rt-zlog.cfg"};
constexpr char log_level[] = {"INFO"};
#endif

void init_logging() {
#if 0
  const char* HOME = getenv("HOME");
  std::cout << "HOME = \"" << HOME << "\"\n";

  std::string cfg_path = absl::StrCat(HOME, cfg_file_path);
  std::cout << "cfg_path = \"" << cfg_path << "\"\n";

  bf_sys_log_init((void*)cfg_path.c_str(), (void*)log_level, NULL);
#endif
}

int main() {
  init_logging();
  log_messages();
  return 0;
}
