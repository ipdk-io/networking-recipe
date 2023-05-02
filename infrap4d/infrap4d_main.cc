// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "absl/synchronization/notification.h"
#include "gflags/gflags.h"
#include "krnlmon_main.h"
#include "stratum/glue/status/status.h"
#include "stratum/hal/bin/tdi/main.h"

extern "C"  {
#include "daemon/daemon.h"
}

DEFINE_bool(detach, true, "Run infrap4d in attached mode");
DEFINE_bool(disable_krnlmon, false, "Run infrap4d without krnlmon support");

int main(int argc, char* argv[]) {
  // Parse infrap4d command line
  stratum::hal::tdi::ParseCommandLine(argc, argv, true);

  if (FLAGS_detach) {
      daemonize_start(false);
      daemonize_complete();
  }

  absl::Notification ready_sync;
  absl::Notification done_sync;

  /* ABSL notification logic is used to synchronize  between stratum thread and
   * switchlink thread. Once stratum initialization is complete, stratum thread
   * notifies other threads who are waiting for the notification.
   *
   * By Providing an option to user to disable krnlmon via disable_krnlmon
   * flag, we will not have a krnlmon listener who is waiting for this
   * notification. This will not have an effect in stratum initialization
   * sequence, just disables krnlmon logic.
   */
  if (!FLAGS_disable_krnlmon) {
      krnlmon_create_main_thread(&ready_sync);
      krnlmon_create_shutdown_thread(&done_sync);
  }

  auto status = stratum::hal::tdi::Main(&ready_sync, &done_sync);
  if (!status.ok()) {
     // TODO: Figure out logging for infrap4d
     return status.error_code();
   }

  return 0;
}
