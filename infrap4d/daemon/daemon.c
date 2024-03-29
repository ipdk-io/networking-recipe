/*
 * Copyright (c) 2014 Nicira, Inc.
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
#include "daemon.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/* For each of the standard file descriptors, whether to replace it by
 * /dev/null (if false) or keep it for the daemon to use (if true). */
static bool infrap4d_save_fds[3];

/* A daemon doesn't normally have any use for the file descriptors for stdin,
 * stdout, and stderr after it detaches.  To keep these file descriptors from
 * e.g. holding an SSH session open, by default detaching replaces each of
 * these file descriptors by /dev/null.  But a few daemons expect the user to
 * redirect stdout or stderr to a file, in which case it is desirable to keep
 * these file descriptors.  This function, therefore, disables replacing 'fd'
 * by /dev/null when the daemon detaches. */
void daemon_save_fd(int fd) {
  assert(fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO);
  infrap4d_save_fds[fd] = true;
}

/* Returns a readable and writable fd for /dev/null, if successful, otherwise
 * a negative errno value.  The caller must not close the returned fd (because
 * the same fd will be handed out to subsequent callers). */
static int get_null_fd(void) {
  static int null_fd;
  char* device = "/dev/null";

  if (!null_fd) {
    null_fd = open(device, O_RDWR);
    if (null_fd < 0) {
      int error = errno;
      null_fd = -error;
    }
  }

  return null_fd;
}

/* Close standard file descriptors (except any that the client has requested we
 * leave open by calling daemon_save_fd()).  If we're started from e.g. an SSH
 * session, then this keeps us from holding that session open artificially. */
void daemon_close_standard_fds(void) {
  int null_fd = get_null_fd();
  if (null_fd >= 0) {
    int fd;

    for (fd = 0; fd < 3; fd++) {
      if (!infrap4d_save_fds[fd]) {
        dup2(null_fd, fd);
      }
    }
  }
}
