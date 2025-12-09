// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Dummy main program to test krnlmon build.

#include "absl/synchronization/notification.h"
#include "krnlmon_main.h"

int main(int argc, char* argv[]) {
  absl::Notification ready_sync;
  absl::Notification done_sync;

  krnlmon_create_main_thread(&ready_sync);
  krnlmon_create_shutdown_thread(&done_sync);

  return 0;
}
