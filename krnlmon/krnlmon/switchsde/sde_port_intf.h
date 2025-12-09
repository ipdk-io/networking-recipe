/*
 * Copyright 2024 Intel Corporation.
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

#ifndef __SDE_PORT_INTF_H__
#define __SDE_PORT_INTF_H__

#include <stdint.h>

#include "sde_types.h"

struct port_info_t;

sde_status_t sde_pal_get_port_id_from_mac(sde_dev_id_t dev_id, char* mac,
                                          uint32_t* port_id);

sde_status_t sde_pal_port_info_get(sde_dev_id_t dev_id, sde_dev_port_t dev_port,
                                   struct port_info_t** port_info);

#endif  // __SDE_PORT_INTF_H__
