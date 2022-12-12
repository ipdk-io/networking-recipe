// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#if defined(DPDK_TARGET)
#include "stratum/hal/bin/tdi/dpdk/dpdk_main.h"
#elif defined (TOFINO_TARGET)
#include "stratum/hal/bin/tdi/tofino/tofino_main.h"
#endif

extern "C"  {
#include "daemon/daemon.h"
}

#include "krnlmon_main.h"
#include "gflags/gflags.h"

DEFINE_bool(detach, true, "Run infrap4d in attached mode");

pthread_cond_t rpc_start_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t rpc_start_lock = PTHREAD_MUTEX_INITIALIZER;
int rpc_start_cookie = 0;

pthread_cond_t rpc_stop_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t rpc_stop_lock = PTHREAD_MUTEX_INITIALIZER;
int rpc_stop_cookie = 0;

int main(int argc, char* argv[]) {
#if defined(TOFINO_TARGET)
  return stratum::hal::tdi::TofinoMain(argc, argv).error_code();
#elif defined(DPDK_TARGET)
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  if (FLAGS_detach) {
      daemonize_start(false);
      daemonize_complete();
  }

  krnlmon_init();

  krnlmon_shutdown();

  auto status = stratum::hal::tdi::DpdkMain(argc, argv);
  if (!status.ok()) {
     //TODO: Figure out logging for infrap4d
     return status.error_code();
   }

  return 0;
#endif
}
