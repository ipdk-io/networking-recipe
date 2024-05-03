#define _POSIX_SOURCE
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "absl/strings/str_cat.h"
#include "lib/ovsp4rt_logging.h"
#include "openvswitch/ovs-p4rt.h"

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

void log_messages() {
  uint8_t mac_addr[6] = {0xb, 0xe, 0xe, 0xb, 0xe, 0xe};
  bool insert_entry = false;

  ovsp4rt_log_info("Error adding to FDB Tunnel Table: entry already exists");

  LogTableError(insert_entry, "FDB Tunnel Table");

  insert_entry = !insert_entry;
  LogTableError(insert_entry, "L2 Tunnel Table");

  insert_entry = !insert_entry;
  LogTableError(insert_entry, "FDB Source MAC Table");

  ovsp4rt_log_warn("Error adding to FDB Vlan Table: entry already exists");

  insert_entry = !insert_entry;
  LogTableError(insert_entry, "FDB Rx Vlan Table");

  insert_entry = !insert_entry;
  LogTableError(insert_entry, "FDB Tx Vlan Table");

  insert_entry = !insert_entry;
  ovsp4rt_log_debug(
      "%s for %s",
      TableErrorMessage(insert_entry, "FDB Source MAC Table").c_str(),
      FormatMac(mac_addr).c_str());

  insert_entry = !insert_entry;
  LogTableError(insert_entry, "SRC_IP_MAC_MAP_TABLE");

  insert_entry = !insert_entry;
  LogTableError(insert_entry, "DST_IP_MAC_MAP_TABLE");
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
