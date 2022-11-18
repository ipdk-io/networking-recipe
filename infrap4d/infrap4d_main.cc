// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#if defined(DPDK_TARGET)
#include "stratum/hal/bin/tdi/dpdk/dpdk_main.h"
#elif defined (TOFINO_TARGET)
#include "stratum/hal/bin/tdi/tofino/tofino_main.h"
#endif

#include "krnlmon_main.h"
#include <pthread.h>

int main(int argc, char* argv[]) {
#if defined(TOFINO_TARGET)
  return stratum::hal::tdi::TofinoMain(argc, argv.error_code());
#elif defined(DPDK_TARGET)
  auto status = stratum::hal::tdi::DpdkMain(argc, argv);
  if (!status.ok()) {
     //TODO: Figure out logging for infrap4d
     return status.error_code();
   }
  return 0;
#endif  
}

