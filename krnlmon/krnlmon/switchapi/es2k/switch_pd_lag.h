/*
 * Copyright 2024 Intel Corporation.
 * SPDX-License_Identifier: Apache-2.0
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

// Proxy for the version-specific switch_pd_lag.h files.
//
// This header file encapsulates the knowledge that there are different
// versions of the header file.
//
// Note that it must use a different include guard than the files it
// includes.

#ifndef __SWITCH_PD_LAG_WRAPPER_H__
#define __SWITCH_PD_LAG_WRAPPER_H__

#include "switchapi/es2k/lnw_v3/switch_pd_lag.h"

#endif  // __SWITCH_PD_LAG_WRAPPER_H__
