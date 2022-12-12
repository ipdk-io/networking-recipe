/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015 Nicira, Inc.
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
#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fatal-signal.h"

#ifdef __linux__
#define LINUX 1
#else
#define LINUX 0
#endif

/* --detach: Should we run in the background? */
bool infrap4d_detach = true;             /* Was --detach specified? */
static bool infrap4d_detached;           /* Have we already detached? */

/* --pidfile: Name of pidfile (null if none). */
char *infrap4d_pidfile;

/* File descriptor used by daemonize_start() and daemonize_complete(). */
int infrap4d_daemonize_fd = -1;

static pid_t fork_and_clean_up(void);
static void daemonize_post_detach(void);

int
read_fully(int fd, void *p_, size_t size, size_t *bytes_read)
{
    char *p = p_;

    *bytes_read = 0;
    while (size > 0) {
        ssize_t retval = read(fd, p, size);
        if (retval > 0) {
            *bytes_read += retval;
            size -= retval;
            p += retval;
        } else if (retval == 0) {
            return EOF;
        } else if (errno != EINTR) {
            return errno;
        }
    }
    return 0;
}

int
write_fully(int fd, const void *p_, size_t size, size_t *bytes_written)
{
    const char *p = p_;

    *bytes_written = 0;
    while (size > 0) {
        ssize_t retval = write(fd, p, size);
        if (retval > 0) {
            *bytes_written += retval;
            size -= retval;
            p += retval;
        } else if (retval == 0) {
            return EPROTO;
        } else if (errno != EINTR) {
            return errno;
        }
    }
    return 0;
}

/* Calls fork() and on success returns its return value.  On failure, logs an
 * error and exits unsuccessfully.
 *
 * Post-fork, but before returning, this function calls a few other functions
 * that are generally useful if the child isn't planning to exec a new
 * process. */
static pid_t
fork_and_clean_up(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        printf("fork failed\n");
    }
    else if (pid > 0) {
        /* Running in parent process. */
        fatal_signal_fork();
    }
    return pid;
}

/* Forks, then:
 *
 *   - In the parent, waits for the child to signal that it has completed its
 *     startup sequence.  Then stores -1 in '*fdp' and returns the child's
 *     pid in '*child_pid' argument.
 *
 *   - In the child, stores a fd in '*fdp' and returns 0 through '*child_pid'
 *     argument.  The caller should pass the fd to fork_notify_startup() after
 *     it finishes its startup sequence.
 *
 * Returns 0 on success.  If something goes wrong and child process was not
 * able to signal its readiness by calling fork_notify_startup(), then this
 * function returns -1. However, even in case of failure it still sets child
 * process id in '*child_pid'. */
static int
fork_and_wait_for_startup(int *fdp, pid_t *child_pid)
{
    int fds[2];
    pid_t pid;
    int ret = 0;

    if (pipe(fds)) {
        printf("failed to create pipe \n");
    }
    pid = fork_and_clean_up();
    if (pid > 0) {
        /* Running in parent process. */
        size_t bytes_read;
        char c;

        close(fds[1]);
        if (read_fully(fds[0], &c, 1, &bytes_read) != 0) {
            int retval;
            int status;

            do {
                retval = waitpid(pid, &status, 0);
            } while (retval == -1 && errno == EINTR);

            if (retval == pid) {
                if (WIFEXITED(status) && WEXITSTATUS(status)) {
                    /* Child exited with an error.  Convey the same error
                     * to our parent process as a courtesy. */
                    exit(WEXITSTATUS(status));
                } else {
		    ret = -1;
                }
            } else {
                abort();
            }
        }
        *fdp = fds[0];
    } else if (!pid) {
        /* Running in child process. */
        close(fds[0]);
        *fdp = fds[1];
    }
    *child_pid = pid;
    return ret;
}

static void
fork_notify_startup(int fd)
{
    if (fd != -1) {
        size_t bytes_written;
        int error;

        error = write_fully(fd, "", 1, &bytes_written);
        if (error) {
            printf("pipe write failed \n");
        }
    }
}

/* If daemonization is configured, then starts daemonization, by forking and
 * returning in the child process.  The parent process hangs around until the
 * child lets it know either that it completed startup successfully (by calling
 * daemonize_complete()) or that it failed to start up (by exiting with a
 * nonzero exit code). */
void
daemonize_start(bool access_datapath)
{
    infrap4d_daemonize_fd = -1;

    if (infrap4d_detach) {
        pid_t pid;

        if (fork_and_wait_for_startup(&infrap4d_daemonize_fd, &pid)) {
            printf("could not detach from foreground session \n");
        }
        if (pid > 0) {
            /* Running in parent process. */
            exit(0);
        } else {    
            /* Running in daemon or monitor process. */
            setsid();
        }
    }
}

/* If daemonization is configured, then this function notifies the parent
 * process that the child process has completed startup successfully.  It also
 * call daemonize_post_detach().
 *
 * Calling this function more than once has no additional effect. */
void
daemonize_complete(void)
{
    if (infrap4d_pidfile) {
        free(infrap4d_pidfile);
        infrap4d_pidfile = NULL;
    }

    if (!infrap4d_detached) {
        infrap4d_detached = true;

        fork_notify_startup(infrap4d_daemonize_fd);
        daemonize_post_detach();
    }
}

/* If daemonization is configured, then this function does traditional Unix
 * daemonization behavior: join a new session, chdir to the root (if not
 * disabled), and close the standard file descriptors.
 *
 * It only makes sense to call this function as part of an implementation of a
 * special daemon subprocess.  A normal daemon should just call
 * daemonize_complete(). */
static void
daemonize_post_detach(void)
{
    if (infrap4d_detach) {
        close_standard_fds();
    }
}
