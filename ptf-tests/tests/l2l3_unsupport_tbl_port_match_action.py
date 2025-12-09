# Copyright (c) 2022 Intel Corporation.
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
DPDK L2 L3 Configure Unsupport table, ports, match fields and actions
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
from common.utils.config_file_utils import (
    get_config_dict,
    get_gnmi_params_simple,
    get_interface_ipv4_dict,
)
from common.utils.gnmi_ctl_utils import (
    gnmi_ctl_set_and_verify,
    ip_set_ipv4,
)


class L2L3_Unpsupport_Tbl_Port_Match_Action(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True

        test_params = test_params_get()
        config_json = test_params["config_json"]
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)

        self.config_data = get_config_dict(config_json)

        self.gnmictl_params = get_gnmi_params_simple(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)

    def runTest(self):
        # Compile p4 file using p4c compiler and generate binary using tdi pipeline builder
        if not test_utils.gen_dep_files_p4c_tdi_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # Create ports using gnmi-ctl
        if not gnmi_ctl_set_and_verify(self.gnmictl_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi ctl ports")

        # Set ip address for interface
        ip_set_ipv4(self.interface_ip_list)

        port_list = self.config_data["port_list"]
        port_ids = test_utils.add_port_to_dataplane(port_list)

        # Add port into dataplane
        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)

        # Run Set-pipe command for set pipeline
        if not p4rt_ctl.p4rt_ctl_set_pipe(
            self.config_data["switch"],
            self.config_data["pb_bin"],
            self.config_data["p4_info"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Configure rules
        for table in self.config_data["table"]:
            log.info(f"Scenario : {table['description']}")
            log.info(f"Adding {table['description']} rules")
            if table["name"] == "pipe.ipv4_host":
                # add invalid match action
                for match_action in table["match_action"]:
                    if p4rt_ctl.p4rt_ctl_add_entry(
                        table["switch"], table["name"], match_action
                    ):
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(
                            f"Failed: invalid entry {match_action} should not be added"
                        )
                    log.passed(
                        f"expected failure to add Invalid match action {match_action}"
                    )
            elif table["name"] == "ingress.ipv4_host_dst":
                # add valid match action
                for match_action in table["match_action"]:
                    if not p4rt_ctl.p4rt_ctl_add_entry(
                        table["switch"], table["name"], match_action
                    ):
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"Failed to add table entry {match_action}")

        log.info("Sending Traffic")
        # Verify support table, ports and match actions
        for i in range(len(self.config_data["table"][1]["match_action"])):
            log.info(f"Verifing match action {table['match_action'][i]}")
            # Define tcp packet
            pkt = simple_tcp_packet(
                ip_src=self.config_data["traffic"]["ip_src"][0],
                ip_dst=self.config_data["traffic"]["ip_dst"][i],
            )
            # Send traffic
            send_packet(
                self,
                port_ids[self.config_data["traffic"]["send_port"][i]],
                pkt,
                count=self.config_data["traffic"]["pkt_num"],
            )
            # Verify traffic reveived
            try:
                verify_packet(
                    self,
                    pkt,
                    port_ids[self.config_data["traffic"]["receive_port"][i]][1],
                )
                log.passed(
                    f"Verification of packets passed, packet received as per rule {i+1}"
                )
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                log.failed(f"Verification of packets sent failed with exception {err}")

        self.dataplane.kill()

    def tearDown(self):
        # Delete rules
        for table in self.config_data["table"]:
            if table["name"] == "ingress.ipv4_host_dst":
                log.info(f"Deleting {table['description']} rules")
                for del_action in table["del_action"]:
                    p4rt_ctl.p4rt_ctl_del_entry(
                        table["switch"], table["name"], del_action
                    )

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
