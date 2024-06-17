// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#define _POSIX_SOURCE
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "lib/ovsp4rt_logging.h"
#include "lib/ovsp4rt_logging_names.h"
#include "lib/ovsp4rt_logutils.h"
#include "ovsp4rt/ovs-p4rt.h"

using namespace ovs_p4rt;

void log_messages() {
  uint8_t mac_addr[6] = {0xb, 0xe, 0xe, 0xb, 0xe, 0xe};
  bool insert_entry = false;

  ovsp4rt_log_info("Error adding to %s: entry already exists",
                   LOG_FDB_TUNNEL_TABLE);

  LogTableErrorWithMacAddr(insert_entry, LOG_FDB_TUNNEL_TABLE, mac_addr);

  insert_entry = !insert_entry;
  LogTableErrorWithMacAddr(insert_entry, LOG_L2_TUNNEL_TABLE, mac_addr);

  insert_entry = !insert_entry;
  LogTableErrorWithMacAddr(insert_entry, LOG_FDB_SMAC_TABLE, mac_addr);

  ovsp4rt_log_warn("Error adding to %s: entry already exists",
                   LOG_FDB_TX_VLAN_TABLE);

  insert_entry = !insert_entry;
  LogTableErrorWithMacAddr(insert_entry, LOG_FDB_TX_VLAN_TABLE, mac_addr);

  insert_entry = !insert_entry;
  LogTableError(insert_entry, LOG_FDB_TX_VLAN_TABLE);

  insert_entry = !insert_entry;
  LogTableErrorWithMacAddr(insert_entry, LOG_FDB_SMAC_TABLE, mac_addr);

  insert_entry = !insert_entry;
  LogTableError(insert_entry, LOG_FDB_SRC_IP_MAC_MAP_TABLE);

  insert_entry = !insert_entry;
  LogTableError(insert_entry, LOG_FDB_DST_IP_MAC_MAP_TABLE);
}

constexpr char cfg_file_path[] = {"/.local/etc/ovsp4rt/ovsp4rt-zlog.cfg"};
constexpr char log_level[] = {"INFO"};

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
