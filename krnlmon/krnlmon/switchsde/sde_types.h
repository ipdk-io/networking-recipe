/*
 * Copyright 2021-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SDE_TYPES_H__
#define __SDE_TYPES_H__

#include <stdbool.h>
#include <stdint.h>

/*! Specifies an ASIC in the system. */
typedef int sde_dev_id_t;

/** Specifies a port on an ASIC. This is a 9-bit value where the upper two
 *  bits specify the pipeline and the lower 7 bits specify the port number
 *  local to that pipeline. The valid range for the lower 7 bits is 0-71. */
typedef int sde_dev_port_t;

#ifndef SDE_STATUS_DEFINED
/** Specifies an error status code. */
typedef int sde_status_t;
#define SDE_STATUS_DEFINED
#endif

#endif /* __SDE_TYPES_H__ */
