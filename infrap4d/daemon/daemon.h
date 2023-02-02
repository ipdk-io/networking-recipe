/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INFRAP4D_DAEMON_H
#define INFRAP4D_DAEMON_H 1

#include <limits.h>
#include <stdbool.h>
#include <sys/types.h>

void daemon_save_fd(int fd);
void daemonize_start(bool access_datapath);
void daemonize_complete(void);
void daemon_close_standard_fds(void);

#endif /* daemon.h */
