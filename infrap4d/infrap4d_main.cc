// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#if defined(DPDK_TARGET)
  #include "stratum/hal/bin/tdi/dpdk/dpdk_main.h"
#elif defined(ES2K_TARGET)
  #include "stratum/hal/bin/tdi/es2k/es2k_main.h"
#else
  #error "TDI target type not defined!"
#endif

extern "C"  {
#include "daemon/daemon.h"
}

#include "absl/synchronization/notification.h"
#include "gflags/gflags.h"
#include "krnlmon_main.h"
#include "stratum/glue/status/status.h"

DEFINE_bool(detach, true, "Run infrap4d in attached mode");

// Invokes the main function for the TDI target.
static inline ::util::Status target_main(absl::Notification* ready_sync,
                                         absl::Notification* done_sync) {
#if defined(DPDK_TARGET)
  return stratum::hal::tdi::DpdkMain(ready_sync, done_sync);
#elif defined(ES2K_TARGET)
  return stratum::hal::tdi::Es2kMain(ready_sync, done_sync);
#else
  #error "TDI target type not defined!"
#endif
}

int main(int argc, char* argv[]) {
  // Parse infrap4d command line
  stratum::hal::tdi::ParseCommandLine(argc, argv, true);

  if (FLAGS_detach) {
      daemonize_start(false);
      daemonize_complete();
  }

  absl::Notification ready_sync;
  absl::Notification done_sync;

  krnlmon_create_main_thread(&ready_sync);
  krnlmon_create_shutdown_thread(&done_sync);

  auto status = target_main(&ready_sync, &done_sync);
  if (!status.ok()) {
     // TODO: Figure out logging for infrap4d
     return status.error_code();
   }

  return 0;
}
