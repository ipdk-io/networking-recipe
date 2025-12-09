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

#ifndef __SWITCH_TDI_H__
#define __SWITCH_TDI_H__

// Disable clang-format so it doesn't alphabetize the #includes.
// tdi_defs.h must precede the other header files because one or
// more of them depend on tdi_defs.h but don't include it.
// clang-format off
#include "tdi/common/tdi_defs.h"
#include "tdi/common/c_frontend/tdi_info.h"
#include "tdi/common/c_frontend/tdi_init.h"
#include "tdi/common/c_frontend/tdi_session.h"
#include "tdi/common/c_frontend/tdi_table.h"
#include "tdi/common/c_frontend/tdi_table_info.h"
// clang-format on

#endif  // __SWITCH_TDI_H__
