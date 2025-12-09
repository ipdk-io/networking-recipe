/*
 * Copyright 2023-2024 Intel Corporation.
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

#include <linux/errno.h>
#include <linux/ethtool.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include <linux/sockios.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "switchlink_int.h"

/*
 * Routine Description:
 *    Check if the interface driver is valid for our use case
 *
 * Arguments:
 *    [in] ifname - Interface name
 *
 * Return Values:
 *    boolean
 */
bool switchlink_validate_driver(const char* ifname) {
  struct ethtool_drvinfo drv = {0};
  char drvname[32] = {0};
  struct ifreq ifr = {0};
  int fd, r = 0;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    return false;
  }

  drv.cmd = ETHTOOL_GDRVINFO;
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
  ifr.ifr_data = (void*)&drv;

  r = ioctl(fd, SIOCETHTOOL, &ifr);
  if (r) {
    goto end;
  }

  strncpy(drvname, drv.driver, sizeof(drvname));

  if (!memcmp(drvname, "openvswitch", strlen(drvname)) ||
      !memcmp(drvname, "idpf", strlen(drvname))) {
    close(fd);
    return true;
  }
end:
  close(fd);
  return false;
}
