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
DPDK L3 Exact Match (match fields, actions) with vHost

"""

# in-built module imports
import time
import sys

# Unittest related imports
import unittest

# ptf related imports
import ptf
import ptf.dataplane as dataplane
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config

# framework related imports
import common.utils.log as log
import common.utils.p4rtctl_utils as p4rt_ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple
from common.utils.gnmi_ctl_utils import gnmi_ctl_set_and_verify
from common.lib.telnet_connection import connectionManager


class L3_Exact_Match(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()

        test_params = test_params_get()
        config_json = test_params["config_json"]

        try:
            self.vm_cred = test_params["vm_cred"]
        except KeyError:
            self.vm_cred = ""

        self.config_data = get_config_dict(
            config_json,
            vm_location_list=test_params["vm_location_list"],
            vm_cred=self.vm_cred,
        )
        self.gnmictl_params = get_gnmi_params_simple(self.config_data)

    def runTest(self):
        # Generate p4c artifact and create binary by using tdi pna arch
        if not test_utils.gen_dep_files_p4c_tdi_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # Create ports using gnmi-ctl
        if not gnmi_ctl_set_and_verify(self.gnmictl_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi ctl ports")

        # Run Set-pipe command for set pipeline
        if not p4rt_ctl.p4rt_ctl_set_pipe(
            self.config_data["switch"],
            self.config_data["pb_bin"],
            self.config_data["p4_info"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Create VMs
        result, vm_name = test_utils.vm_create(self.config_data["vm_location_list"])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"VM creation failed for {vm_name}")

        # Create telnet instance for VMs created
        vm_id = 0
        for vm, port in zip(self.config_data["vm"], self.config_data["port"]):
            globals()["conn" + str(vm_id + 1)] = connectionManager(
                "127.0.0.1", f"655{vm_id}", vm["vm_username"], vm["vm_password"]
            )
            globals()["vm" + str(vm_id + 1) + "_command_list"] = [
                f"ip addr add {port['ip_address']} dev {port['interface']}",
                f"ip link set dev {port['interface']} up",
                f"ip link set dev {port['interface']} address {port['mac_local']}",
                f"ip route add {vm['dst_nw']} via {vm['dst_gw']} dev {port['interface']}",
                f"ip neigh add dev {port['interface']} {vm['remote_ip']} lladdr {vm['mac_remote']}",
            ]
            vm_id += 1

        # Add l3 exact match forward rules
        for table in self.config_data["table"]:
            log.info(f"Scenario : {table['description']}")
            log.info(f"Adding {table['description']} rules")
            for match_action in table["match_action"]:
                if not p4rt_ctl.p4rt_ctl_add_entry(
                    table["switch"], table["name"], match_action
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Configuring VMs
        log.info("Configuring VM0 ....")
        test_utils.configure_vm(conn1, vm1_command_list)

        log.info("Configuring VM1 ....")
        test_utils.configure_vm(conn2, vm2_command_list)

        # ping test between VMs
        log.info("Ping test from VM0 to VM1")
        result = test_utils.vm_to_vm_ping_test(
            conn1, self.config_data["vm"][0]["remote_ip"]
        )
        if not result:
            self.result.addFailure(self, sys.exc_info())
            log.failed("Ping test failed for VM0")

        log.info("Ping test from VM1 to VM0")
        result = test_utils.vm_to_vm_ping_test(
            conn2, self.config_data["vm"][1]["remote_ip"]
        )
        if not result:
            self.result.addFailure(self, sys.exc_info())
            log.failed("Ping test failed for VM1")

        # Close telnet connections
        log.info(f"close VM telnet session")
        conn1.close()
        conn2.close()

    def tearDown(self):
        # Delete table entries
        for table in self.config_data["table"]:
            log.info(f"Deleting {table['description']} rules")
            for del_action in table["del_action"]:
                p4rt_ctl.p4rt_ctl_del_entry(table["switch"], table["name"], del_action)

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
