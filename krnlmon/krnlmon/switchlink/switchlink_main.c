/*
 * Copyright 2013-present Barefoot Networks, Inc.
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

// Enable assert()
#undef NDEBUG

#ifndef _GNU_SOURCE
// Enable assert_perror()
#define _GNU_SOURCE
#endif

#include "switchlink_main.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/select.h>
#include <unistd.h>

#include "switchlink.h"
#include "switchlink_int.h"
#include "switchutils/switch_log.h"

static struct nl_sock* g_nlsk = NULL;
static pthread_t switchlink_thread;

enum {
  SWITCHLINK_MSG_LINK,
  SWITCHLINK_MSG_ADDR,
  SWITCHLINK_MSG_NETCONF,
  SWITCHLINK_MSG_NETCONF6,
  SWITCHLINK_MSG_NEIGH_MAC,
  SWITCHLINK_MSG_NEIGH_IP,
  SWITCHLINK_MSG_NEIGH_IP6,
  SWITCHLINK_MSG_MDB,
  SWITCHLINK_MSG_UNICAST_ROUTE,
  SWITCHLINK_MSG_UNICAST_ROUTE6,
  SWITCHLINK_MSG_MULTICAST_ROUTE,
  SWITCHLINK_MSG_MULTICAST_ROUTE6,
  SWITCHLINK_MSG_MAX,
} switchlink_msg_t;

static void sync_state(void) {
  static uint8_t msg_idx = SWITCHLINK_MSG_LINK;
  if (msg_idx == SWITCHLINK_MSG_MAX) {
    return;
  }

  struct rtgenmsg rt_hdr = {
      .rtgen_family = AF_UNSPEC,
  };

  int msg_type = -1;
  switch (msg_idx) {
    case SWITCHLINK_MSG_LINK:
      msg_type = RTM_GETLINK;
      break;

    case SWITCHLINK_MSG_ADDR:
      msg_type = RTM_GETADDR;
      break;

    case SWITCHLINK_MSG_NETCONF:
      msg_type = RTM_GETNETCONF;
      rt_hdr.rtgen_family = AF_INET;
      break;

    case SWITCHLINK_MSG_NETCONF6:
      msg_type = RTM_GETNETCONF;
      rt_hdr.rtgen_family = AF_INET6;
      break;

    case SWITCHLINK_MSG_NEIGH_MAC:
      msg_type = RTM_GETNEIGH;
      rt_hdr.rtgen_family = AF_BRIDGE;
      break;

    case SWITCHLINK_MSG_NEIGH_IP:
      msg_type = RTM_GETNEIGH;
      rt_hdr.rtgen_family = AF_INET;
      break;

    case SWITCHLINK_MSG_NEIGH_IP6:
      msg_type = RTM_GETNEIGH;
      rt_hdr.rtgen_family = AF_INET6;
      break;

    case SWITCHLINK_MSG_MDB:
      msg_type = RTM_GETMDB;
      rt_hdr.rtgen_family = AF_BRIDGE;
      break;

    case SWITCHLINK_MSG_UNICAST_ROUTE:
      msg_type = RTM_GETROUTE;
      rt_hdr.rtgen_family = AF_INET;
      break;

    case SWITCHLINK_MSG_UNICAST_ROUTE6:
      msg_type = RTM_GETROUTE;
      rt_hdr.rtgen_family = AF_INET6;
      break;

    case SWITCHLINK_MSG_MULTICAST_ROUTE:
      msg_type = RTM_GETROUTE;
      rt_hdr.rtgen_family = RTNL_FAMILY_IPMR;
      break;

    case SWITCHLINK_MSG_MULTICAST_ROUTE6:
      msg_type = RTM_GETROUTE;
      rt_hdr.rtgen_family = RTNL_FAMILY_IP6MR;
      break;
  }

  if (msg_type != -1) {
    nl_send_simple(g_nlsk, msg_type, NLM_F_DUMP, &rt_hdr, sizeof(rt_hdr));
    msg_idx++;
  }
}

/*
 * Routine Description:
 *    Process netlink messages
 *
 * Arguments:
 *    [in] nlmsg - netlink msg header
 *
 * Return Values:
 *    void
 */

static void process_message(struct nlmsghdr* nlmsg) {
  /* TODO: P4OVS: Enabling callback for link msg type only and prints for
     few protocol families to avoid flood of messages. Enable, as needed.
  */
  switch (nlmsg->nlmsg_type) {
    case RTM_NEWLINK:
      krnlmon_log_debug("Switchlink Notification RTM_NEWLINK\n");
      switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_DELLINK:
      krnlmon_log_debug("Switchlink Notification RTM_DELLINK\n");
      switchlink_process_link_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_NEWADDR:
      krnlmon_log_debug("Switchlink Notification RTM_NEWADDR\n");
      switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_DELADDR:
      krnlmon_log_debug("Switchlink Notification RTM_DELADDR\n");
      switchlink_process_address_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_NEWROUTE:
      krnlmon_log_debug("Switchlink Notification RTM_NEWROUTE\n");
      switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_DELROUTE:
      krnlmon_log_debug("Switchlink Notification RTM_DELROUTE\n");
      switchlink_process_route_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_NEWNEIGH:
      krnlmon_log_debug("Switchlink Notification RTM_NEWNEIGH\n");
      switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_DELNEIGH:
      krnlmon_log_debug("Switchlink Notification RTM_DELNEIGH\n");
      switchlink_process_neigh_msg(nlmsg, nlmsg->nlmsg_type);
      break;
    case RTM_NEWNETCONF:
    case RTM_GETMDB:
    case RTM_NEWMDB:
    case RTM_DELMDB:
      break;
    case NLMSG_DONE:
      break;
    default:
      krnlmon_log_debug("Unknown netlink message(%d). Ignoring\n",
                        nlmsg->nlmsg_type);
      break;
  }
}

static int receive_message(struct nl_msg* msg, void* arg) {
  struct nlmsghdr* nl_msg = nlmsg_hdr(msg);
  int nl_msg_sz = nlmsg_get_max_size(msg);
  while (nlmsg_ok(nl_msg, nl_msg_sz)) {
    process_message(nl_msg);
    nl_msg = nlmsg_next(nl_msg, &nl_msg_sz);
  }

  return 0;
}

static void free_socket(void) {
  // free the socket
  nl_socket_free(g_nlsk);
  g_nlsk = NULL;
}

static void init_socket(void) {
  int nlsk_fd, sock_flags;

  // allocate a new socket
  g_nlsk = nl_socket_alloc();
  if (g_nlsk == NULL) {
    perror("nl_socket_alloc");
    return;
  }

  nl_socket_set_local_port(g_nlsk, 0);

  // disable sequence number checking
  nl_socket_disable_seq_check(g_nlsk);

  // set the callback function
  nl_socket_modify_cb(g_nlsk, NL_CB_VALID, NL_CB_CUSTOM, receive_message, NULL);
  nl_socket_modify_cb(g_nlsk, NL_CB_FINISH, NL_CB_CUSTOM, receive_message,
                      NULL);

  // connect to the netlink route socket
  if (nl_connect(g_nlsk, NETLINK_ROUTE) < 0) {
    perror("nl_connect:NETLINK_ROUTE");
    free_socket();
    return;
  }

  // register for the following messages
  nl_socket_add_memberships(g_nlsk, RTNLGRP_LINK, 0);
  nl_socket_add_memberships(g_nlsk, RTNLGRP_NOTIFY, 0);
  nl_socket_add_memberships(g_nlsk, RTNLGRP_NEIGH, 0);
  nl_socket_add_memberships(g_nlsk, RTNLGRP_IPV4_IFADDR, 0);
  nl_socket_add_memberships(g_nlsk, RTNLGRP_IPV4_ROUTE, 0);
  nl_socket_add_memberships(g_nlsk, RTNLGRP_IPV6_IFADDR, 0);
  nl_socket_add_memberships(g_nlsk, RTNLGRP_IPV6_ROUTE, 0);
  // nl_add_socket_memberships(g_nlsk, RTNLGRP_IPV4_RULE, 0);
  // nl_add_socket_memberships(g_nlsk, RTNLGRP_IPV6_RULE, 0);

  // set socket to be non-blocking
  nlsk_fd = nl_socket_get_fd(g_nlsk);
  if (nlsk_fd < 0) {
    perror("nl_socket_get_fd");
    free_socket();
    return;
  }
  sock_flags = fcntl(nlsk_fd, F_GETFL, 0);
  if (fcntl(nlsk_fd, F_SETFL, sock_flags | O_NONBLOCK) < 0) {
    perror("fcntl");
    free_socket();
    return;
  }

  // start building state from the kernel
  sync_state();
}

static void receive_event_loop(void) {
  int nlsk_fd;
  nlsk_fd = nl_socket_get_fd(g_nlsk);
  krnlmon_assert(nlsk_fd > 0);

  while (1) {
    int ret, num_fds;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(nlsk_fd, &read_fds);
    num_fds = nlsk_fd + 1;

    ret = select(num_fds, &read_fds, NULL, NULL, NULL);
    if (ret == -1) {
      char errbuf[64];

      // errno is volatile. Make a local copy of its value.
      int err = errno;

      // This is not really an error condition. Ignore it.
      if (err == EINTR) continue;

      krnlmon_log_critical("Error selecting event socket: %s",
                           strerror_r(err, errbuf, sizeof(errbuf)));
      return;
    } else {
      if (FD_ISSET(nlsk_fd, &read_fds)) {
        nl_recvmsgs_default(g_nlsk);
      }
    }
  }
}

// start_routine if running in a thread
void* switchlink_start(void* args) {
  (void)args;
  krnlmon_log_debug("switchlink main started");
  (void)switchlink_main();
  return NULL;
}

// thread body
int switchlink_main(void) {
  switchlink_init_db();
  switchlink_init_api();
  switchlink_init_link();
  init_socket();

  if (g_nlsk) {
    usleep(20000);
    receive_event_loop();
    free_socket();
  }

  return 0;
}

int switchlink_stop(void) {
  // TODO: switchlink_thread is never initialized!
  int status = pthread_cancel(switchlink_thread);
  if (status == 0) {
    return pthread_join(switchlink_thread, NULL);
  }

  return status;
}
