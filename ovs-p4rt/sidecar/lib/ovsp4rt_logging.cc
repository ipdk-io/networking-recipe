// Copyright 2024 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_logging.h"

#include <stdarg.h>
#include <stdio.h>

namespace {

const char* get_level_name(int level) {
  switch (level) {
    case OVSP4RT_LEVEL_ERROR:
      return "ERROR";
    case OVSP4RT_LEVEL_WARN:
      return "WARN";
    case OVSP4RT_LEVEL_INFO:
      return "INFO";
    case OVSP4RT_LEVEL_DEBUG:
      return "DEBUG";
    default:
      return "UNKNOWN";
  }
}

}  // namespace

void ovsp4rt_log_message(int level, const char* format, ...) {
  printf("OVSP4RT %s - ", get_level_name(level));
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
}
