// Copyright 2024 Intel Corporation.
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_LOGGING_H_
#define OVSP4RT_LOGGING_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
  OVSP4RT_LEVEL_DEBUG,
  OVSP4RT_LEVEL_INFO,
  OVSP4RT_LEVEL_WARN,
  OVSP4RT_LEVEL_ERROR,
};

#define ovsp4rt_log_debug(...) \
  ovsp4rt_log_message(OVSP4RT_LEVEL_DEBUG, __VA_ARGS__)

#define ovsp4rt_log_error(...) \
  ovsp4rt_log_message(OVSP4RT_LEVEL_ERROR, __VA_ARGS__)

#define ovsp4rt_log_info(...) \
  ovsp4rt_log_message(OVSP4RT_LEVEL_INFO, __VA_ARGS__)

#define ovsp4rt_log_warn(...) \
  ovsp4rt_log_message(OVSP4RT_LEVEL_WARN, __VA_ARGS__)

extern void ovsp4rt_log_message(int level, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif  // OVSP4RT_LOGGING_H_
