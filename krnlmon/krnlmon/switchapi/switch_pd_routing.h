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

// Proxy for the target-specific switch_pd_routing.h files.
//
// This header file encapsulates the knowledge that there are different
// versions of the header file for DPDK and ES2K.
//
// Note that it must use a different include guard than the files it
// includes (which both use the same name).

#ifndef __SWITCH_PD_ROUTING_WRAPPER_H__
#define __SWITCH_PD_ROUTING_WRAPPER_H__

#if defined(DPDK_TARGET)
#include "switchapi/dpdk/switch_pd_routing.h"
#elif defined(ES2K_TARGET)
#include "switchapi/es2k/switch_pd_routing.h"
#else
#error "ASSERT: Unknown TARGET type!"
#endif

#endif  // __SWITCH_PD_ROUTING_WRAPPER_H__
