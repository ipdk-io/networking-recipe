/*
 * Copyright 2022-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __KRNLMON_LOG_H__
#define __KRNLMON_LOG_H__

#include "target-sys/bf_sal/bf_sys_log.h"

#define krnlmon_log_critical(...) \
  bf_sys_log_and_trace(KRNLMON, BF_LOG_CRIT, __VA_ARGS__)

#define krnlmon_log_error(...) \
  bf_sys_log_and_trace(KRNLMON, BF_LOG_ERR, __VA_ARGS__)

#define krnlmon_log_warn(...) \
  bf_sys_log_and_trace(KRNLMON, BF_LOG_WARN, __VA_ARGS__)

#define krnlmon_log_info(...) \
  bf_sys_log_and_trace(KRNLMON, BF_LOG_INFO, __VA_ARGS__)

#define krnlmon_log_debug(...) \
  bf_sys_log_and_trace(KRNLMON, BF_LOG_DBG, __VA_ARGS__)

#define krnlmon_log krnlmon_log_debug

#endif  // __KRNLMON_LOG_H__
