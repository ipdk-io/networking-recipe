/*
 * Copyright 2013-present Barefoot Networks, Inc.
 * Copyright 2022-2024 Intel Corporation.
 *
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

#ifndef __SWITCHLINK_INT_H__
#define __SWITCHLINK_INT_H__

#include <stdbool.h>

struct nlmsghdr;

extern void switchlink_init_db(void);
extern void switchlink_init_api(void);
extern void switchlink_init_link(void);

extern void switchlink_process_link_msg(const struct nlmsghdr* nlmsg,
                                        int msgtype);
extern void switchlink_process_neigh_msg(const struct nlmsghdr* nlmsg,
                                         int msgtype);
extern void switchlink_process_address_msg(const struct nlmsghdr* nlmsg,
                                           int msgtype);
extern void switchlink_process_route_msg(const struct nlmsghdr* nlmsg,
                                         int msgtype);

extern bool switchlink_validate_driver(const char* ifname);

#endif /* __SWITCHLINK_INT_H__ */
