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

#ifdef RTE_FLOW_SHIM
extern "C"  {
#include "rte_flow_shim/rfs_interface.h"
}
#endif

DEFINE_bool(detach, true, "Run infrap4d in attached mode");
DEFINE_bool(disable_krnlmon, false, "Run infrap4d without krnlmon support");

#ifdef RTE_FLOW_SHIM
#define RFS_CLI_PORT 9090
DEFINE_string(rfs_enable, "dummy", "Specify this option to enable RTE Flow shim commands to"
  " P4 based pipeline. Pass p4 program name as argument");
DEFINE_string(rfs_cfg, "dummy.cfg", "tdi/bf-rt.json file generated for the p4 program");

void *RFSCliFunc(void* arg) {
  LOG(INFO) << "rte_flow_shim thread is running" ;
  rfs_start_cli_server(RFS_CLI_PORT);
  return nullptr;
}

#endif

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
  
#ifdef RTE_FLOW_SHIM
  pthread_t rfs_cli_tid; // Thread ID for RTE_FLOW_SHIM CLI
#endif

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

  auto status = target_main(&ready_sync, &done_sync);
  if (!status.ok()) {
     // TODO: Figure out logging for infrap4d
     return status.error_code();
   }
  
#ifdef RTE_FLOW_SHIM
  if (FLAGS_rfs_enable.find("dummy") == std::string::npos &&
      FLAGS_rfs_cfg.find("dummy.cfg") == std::string::npos ) {
    rfs_set_p4_program_name(FLAGS_rfs_enable.c_str());
  
    if ( rte_flow_shim_lib_init(FLAGS_rfs_cfg.c_str()) ) {
      LOG(ERROR) << "Failed to init rte_flow_shim library";
    } else { 
      LOG(INFO) << "rte_flow_shim started successfully";
    }

    bf_switchd_rfs_fp_init();

    tdi_rte_shim_ops_init(&rfs_tdi_ops);
    LOG(INFO) << "bf_switchd_rfs_fp_init started successfully";

    pthread_create(&rfs_cli_tid, nullptr, RFSCliFunc, nullptr);
  }
#endif
  return 0;
}
