// infrap4d_main without krnlmon

// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "gflags/gflags.h"
#include "stratum/glue/status/status.h"
#include "stratum/hal/bin/tdi/main.h"

extern "C" {
#include "daemon/daemon.h"
}

DEFINE_bool(detach, true, "Run infrap4d in detached mode");

int main(int argc, char* argv[]) {
  // Parse infrap4d command line
  stratum::hal::tdi::ParseCommandLine(argc, argv, true);

  if (FLAGS_detach) {
    daemonize_start(false);
    daemonize_complete();
  }

  auto status = stratum::hal::tdi::Main();
  if (!status.ok()) {
    // TODO: Figure out logging for infrap4d
    return status.error_code();
  }

  return 0;
}
