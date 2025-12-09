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
DPDK L3 WCM with Link Port
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
    ip_set_dev_up,
)


class L3_WCM_Link(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True

        test_params = test_params_get()
        config_json = test_params["config_json"]
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)

        self.capture_port = test_params["pci_bdf"][:-1] + "1"
        self.config_data = get_config_dict(config_json, test_params["pci_bdf"])
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

        # Get port list and add to dataplane
        port_list = self.config_data["port_list"]
        port_list[0] = test_utils.get_port_name_from_pci_bdf(self.capture_port)
        port_ids = test_utils.add_port_to_dataplane(port_list)

        # Ensure ports are up
        for portname in port_list:
            if not ip_set_dev_up(portname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to bring up the port {portname}")

        # Add port in dataplane
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

        # Add rules for each table
        for table in self.config_data["table"]:
            log.info(f"Scenario : wcm link port : {table['description']}")
            log.info(f"Adding {table['description']} rules")
            for match_action in table["match_action"]:
                if not p4rt_ctl.p4rt_ctl_add_entry(
                    table["switch"], table["name"], match_action
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Sleep for set up ready
        time.sleep(5)
        log.info("sending packet to TAP0 to check if rule 1 drop")
        # Define tcp packet
        pkt = simple_tcp_packet(
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_dst"][1]
        )
        # Send traffic
        send_packet(
            self,
            port_ids[self.config_data["traffic"]["send_port"][1]],
            pkt,
            count=self.config_data["traffic"]["num_pkt"],
        )
        # Verify traffic
        try:
            verify_no_packet_any(
                self,
                pkt,
                device_number=0,
                ports=[port_ids[self.config_data["traffic"]["receive_port"][0]][1]],
            )
            log.passed(f"Verification of packets passed, packet dropped as per rule 1")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(f"Verification of packets sent failed with exception {err}")

        log.info("sending packet to check if rule 2 hit")
        # Define tcp packet
        pkt = simple_tcp_packet(
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_dst"][0],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_src"][0],
        )
        # Send traffic
        send_packet(
            self,
            port_ids[self.config_data["traffic"]["send_port"][1]],
            pkt,
            count=self.config_data["traffic"]["num_pkt"],
        )
        # Verify traffic
        try:
            verify_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][1]][1]
            )
            log.passed(f"Verification of packets passed, packet received as per rule 2")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(f"Verification of packets sent failed with exception {err}")

        log.info("sending packet to check if rule 3 hit")
        # Define tcp packet
        pkt = simple_tcp_packet(
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_dst"][2],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_src"][0],
        )
        # Send traffic
        send_packet(
            self,
            port_ids[self.config_data["traffic"]["send_port"][1]],
            pkt,
            count=self.config_data["traffic"]["num_pkt"],
        )
        # Verify traffic
        try:
            verify_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][0]][1]
            )
            log.passed(f"Verification of packets passed, packet received as per rule 3")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(f"Verification of packets sent failed with exception {err}")

        log.info("sending packet to check if rule 4 drop")
        # Define tcp packet
        pkt = simple_tcp_packet(
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_dst"][4],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_src"][0],
        )
        # Send traffic
        send_packet(
            self,
            port_ids[self.config_data["traffic"]["send_port"][0]],
            pkt,
            count=self.config_data["traffic"]["num_pkt"],
        )
        # Verify traffic
        try:
            verify_no_packet_any(
                self,
                pkt,
                device_number=0,
                ports=[port_ids[self.config_data["traffic"]["receive_port"][1]][1]],
            )
            log.passed(f"Verification of packets passed, packet dropped as per rule 4")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(f"Verification of packets sent failed with exception {err}")

        log.info("sending packet to check if rule 5 hit")
        # Define tcp packet
        pkt = simple_tcp_packet(
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_dst"][3],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_src"][0],
        )
        # Send traffic
        send_packet(
            self,
            port_ids[self.config_data["traffic"]["send_port"][0]],
            pkt,
            count=self.config_data["traffic"]["num_pkt"],
        )
        # Verify traffic
        try:
            verify_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][1]][1]
            )
            log.passed(f"Verification of packets passed, packet received as per rule 5")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(
                f"failedVerification of packets sent failed with exception {err}"
            )

        log.info("sending packet to check if rule 6 hit")
        # Define tcp packet
        pkt = simple_tcp_packet(
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_dst"][5],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_src"][0],
        )
        # Send traffic
        send_packet(
            self,
            port_ids[self.config_data["traffic"]["send_port"][0]],
            pkt,
            count=self.config_data["traffic"]["num_pkt"],
        )
        # Verify traffic
        try:
            verify_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][2]][1]
            )
            log.passed(f"Verification of packets passed, packet received as per rule 6")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(f"Verification of packets sent failed with exception {err}")

        self.dataplane.kill()

    def tearDown(self):
        # Delete rules
        for table in self.config_data["table"]:
            log.info(f"Deleting {table['description']} rules")
            for del_action in table["del_action"]:
                p4rt_ctl.p4rt_ctl_del_entry(table["switch"], table["name"], del_action)

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
