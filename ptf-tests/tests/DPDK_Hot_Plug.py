# Copyright (c) 2022 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
DPDK Hot Plug
"""

# in-built module imports
import time
import sys
from itertools import dropwhile
import common.utils.log as log

# Unittest related imports
import unittest

# ptf related imports
import ptf
import ptf.dataplane as dataplane
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config

# scapy related imports
from scapy.packet import *
from scapy.fields import *
from scapy.all import *

# framework related imports
import common.utils.p4rtctl_utils as p4rt_ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import (
    get_config_dict,
    get_gnmi_params_simple,
    get_gnmi_params_hotplug,
    get_interface_ipv4_dict,
    get_interface_ipv4_dict_hotplug,
    get_interface_mac_dict_hotplug,
    get_interface_ipv4_route_dict_hotplug,
    create_port_vm_map,
)
from common.utils.gnmi_ctl_utils import (
    gnmi_ctl_set_and_verify,
    gnmi_set_params,
    ip_set_ipv4,
)
from common.lib.telnet_connection import connectionManager


class Dpdk_Hot_Plug(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True

        test_params = test_params_get()
        config_json = test_params["config_json"]
        try:
            self.vm_cred = test_params["vm_cred"]
        except KeyError:
            self.vm_cred = ""
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)
        self.config_data = get_config_dict(
            config_json, vm_location_list=test_params["vm_location_list"]
        )
        self.gnmictl_params = get_gnmi_params_simple(self.config_data)
        self.gnmictl_hotplug_params = get_gnmi_params_hotplug(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)

    def runTest(self):
        # Create VM
        result, vm_name = test_utils.vm_create_with_hotplug(self.config_data)
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to create {vm_name}")

        # Give sleep for VM to come up
        log.info("Sleeping for 30 seconds for the vms to come up")
        time.sleep(30)

        # Log in on VM using Telnet
        vm = self.config_data["vm"][0]
        vm["qemu-hotplug-mode"]["qemu-socket-ip"]
        conn1 = connectionManager(
            vm["qemu-hotplug-mode"]["qemu-socket-ip"],
            vm["qemu-hotplug-mode"]["serial-telnet-port"],
            vm["vm_username"],
            password=vm["vm_password"],
        )
        vm1_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result1 = list(dropwhile(lambda x: "lo\r" not in x, result))

        # Create vhost-user Ports using gnmi-ctl
        if not gnmi_ctl_set_and_verify(self.gnmictl_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi ctl ports")

        # Add vhost-users as hotplug to the VM
        if not gnmi_ctl_set_and_verify(self.gnmictl_hotplug_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure hotplug through gnmi")

        # Check and verify vhost-user hotplugged successfully to the VM
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result2 = list(dropwhile(lambda x: "lo\r" not in x, result))
        vm1interfaces = list(set(vm1result2) - set(vm1result1))
        vm1interfaces = [x.strip() for x in vm1interfaces]
        log.info(f"interfaces: {vm1interfaces}")

        # closing telnet session
        conn1.close()

        self.dataplane.kill()

    def tearDown(self):
        if self.result.wasSuccessful():
            log.passed("Test has PASSED")
        else:
            log.failed("Test has FAILED")
