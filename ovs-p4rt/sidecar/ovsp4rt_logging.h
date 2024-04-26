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

#ifndef OVSP4RT_LOGGING_H_
#define OVSP4RT_LOGGING_H_

#include "target-sys/bf_sal/bf_sys_log.h"

// There's a syntax error in the definition of OVSP4RT in bf_sys_log.h,
// which is in target-sysutils. Work around the problem by replacing
// the defective definition with a valid one.
#undef OVSP4RT
#define OVSP4RT (BF_MOD_START + 23)

#define ovsp4rt_log_critical(...) \
  bf_sys_log_and_trace(OVSP4RT, BF_LOG_CRIT, __VA_ARGS__)

#define ovsp4rt_log_error(...) \
  bf_sys_log_and_trace(OVSP4RT, BF_LOG_ERR, __VA_ARGS__)

#define ovsp4rt_log_warn(...) \
  bf_sys_log_and_trace(OVSP4RT, BF_LOG_WARN, __VA_ARGS__)

#define ovsp4rt_log_info(...) \
  bf_sys_log_and_trace(OVSP4RT, BF_LOG_INFO, __VA_ARGS__)

#define ovsp4rt_log_debug(...) \
  bf_sys_log_and_trace(OVSP4RT, BF_LOG_DBG, __VA_ARGS__)

#define ovsp4rt_log ovsp4rt_log_debug

#endif  // OVSP4RT_LOGGING_H_
