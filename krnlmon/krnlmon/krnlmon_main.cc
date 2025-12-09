// Copyright 2022-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "krnlmon_main.h"

// Enable GNU extensions: pthread_setname_np()
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "absl/synchronization/notification.h"
#include "switchlink/switchlink_main.h"

static pthread_t main_tid;
static pthread_t stop_tid;

extern "C" {
// Ensure that the functions passed to pthread_create() have C interfaces.
static void* krnlmon_main_wrapper(void* arg);
static void* krnlmon_stop_wrapper(void* arg);
}

static void* krnlmon_main_wrapper(void* arg) {
  // Wait for stratum server to signal that it is ready.
  auto ready_sync = static_cast<absl::Notification*>(arg);
  ready_sync->WaitForNotification();

  // Start switchlink.
  switchlink_main();

  return nullptr;
}

static void* krnlmon_stop_wrapper(void* arg) {
  // Wait for stratum server to signal that it is done.
  auto done_sync = static_cast<absl::Notification*>(arg);
  done_sync->WaitForNotification();

  // Stop switchlink.
  switchlink_stop();

  return nullptr;
}

static void print_strerror(const char* msg, int err) {
  char errbuf[64];
  printf("%s: %s\n", msg, strerror_r(err, errbuf, sizeof(errbuf)));
}

int krnlmon_create_main_thread(absl::Notification* ready_sync) {
  int rc = pthread_create(&main_tid, NULL, &krnlmon_main_wrapper, ready_sync);
  if (rc) {
    print_strerror("Error creating switchlink_main thread", rc);
    return -1;
  }

  rc = pthread_setname_np(main_tid, "switchlink_main");
  if (rc) {
    print_strerror("Error naming switchlink_main thread", rc);
  }

  return 0;
}

int krnlmon_create_shutdown_thread(absl::Notification* done_sync) {
  int rc = pthread_create(&stop_tid, NULL, &krnlmon_stop_wrapper, done_sync);
  if (rc) {
    print_strerror("Error creating switchlink_stop thread", rc);
    return -1;
  }

  rc = pthread_setname_np(stop_tid, "switchlink_stop");
  if (rc) {
    print_strerror("Error naming switchlink_stop thread", rc);
  }

  return 0;
}
