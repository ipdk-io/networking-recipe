/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013 Nicira, Inc.
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

#include "fatal-signal.h"
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifndef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX TYPE_MAXIMUM(sig_atomic_t)
#endif

/* Signals to catch. */
static const int fatal_signals[] = { SIGTERM, SIGINT, SIGHUP, SIGALRM,
                                     SIGSEGV };

/* Hooks to call upon catching a signal */
struct infrap4d_hook {
    void (*hook_cb)(void *aux);
    void (*cancel_cb)(void *aux);
    void *aux;
    bool run_at_exit;
};
#define MAX_HOOKS 32
static struct infrap4d_hook infrap4d_hooks[MAX_HOOKS];
static size_t infrap4d_n_hooks;

static int infrap4d_signal_fds[2];
static volatile sig_atomic_t infrap4d_stored_sig_nr = SIG_ATOMIC_MAX;

/* Clears all of the fatal signal hooks without executing them.  If any of the
 * hooks passed a 'cancel_cb' function to fatal_signal_add_hook(), then those
 * functions will be called, allowing them to free resources, etc.
 *
 * Following a fork, one of the resulting processes can call this function to
 * allow it to terminate without calling the hooks registered before calling
 * this function.  New hooks registered after calling this function will take
 * effect normally. */
void
fatal_signal_fork(void)
{
    size_t i;

    for (i = 0; i < infrap4d_n_hooks; i++) {
        struct infrap4d_hook *h = &infrap4d_hooks[i];
        if (h->cancel_cb) {
            h->cancel_cb(h->aux);
        }
    }
    infrap4d_n_hooks = 0;

    /* Raise any signals that we have already received with the default
     * handler. */
    if (infrap4d_stored_sig_nr != SIG_ATOMIC_MAX) {
        raise(infrap4d_stored_sig_nr);
    }
}
