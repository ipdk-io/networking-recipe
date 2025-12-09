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

DPDK Action Selector Traffic Test with TAP Port
TC1 : 2 members with action send (to 2 different ports) associated to 1 group with match field dst IP
TC2 : 5 members with action send (to 5 different ports) associated to 3 groups with match field dst IP 

"""

# in-built module imports
from itertools import count
import time
import sys

# Unittest related imports
import unittest
import common.utils.log as log

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
    gnmi_get_params_counter,
)


class L3_Action_Selector(BaseTest):
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
        # Compile p4 file using p4c compiler and generate binary using ovs pipeline builder
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

        table = self.config_data["table"][0]

        # Add action profile members
        log.info("Add action profile members")
        for member in table["member_details"]:
            if not p4rt_ctl.p4rt_ctl_add_member_and_verify(
                table["switch"], table["name"], member
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add member {member}")

        # Adding action selector groups
        log.info("Adding action selector groups")
        group_count = 0
        for group in table["group_details"]:
            if not p4rt_ctl.p4rt_ctl_add_group_and_verify(
                table["switch"], table["name"], group
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add group {group}")
            group_count += 1

        # Setting rule for l3 action selector
        log.info(f"Setting up rule for : {table['description']}")
        table = self.config_data["table"][1]
        for match_action in table["match_action"]:
            if not p4rt_ctl.p4rt_ctl_add_entry(
                table["switch"], table["name"], match_action
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add table entry {match_action}")

        pkt = simple_udp_packet(
            eth_src=self.config_data["traffic"]["in_pkt_header"]["eth_mac"][0],
            eth_dst=self.config_data["traffic"]["in_pkt_header"]["eth_mac"][1],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_address"][0],
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_address"][2],
            udp_sport=self.config_data["traffic"]["in_pkt_header"]["udp_port"][0],
            udp_dport=self.config_data["traffic"]["in_pkt_header"]["udp_port"][1],
        )
        send_packet(self, port_ids[self.config_data["traffic"]["send_port"][3]], pkt)
        try:
            verify_no_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][0]][1]
            )
            log.passed(
                " Verification of udp packet check passed : No traffic from TAP3 to TAP0 port"
            )
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(
                f" Verification of udp packet sent failed with exception {err}: TAP3->TAP0"
            )

        pkt = simple_udp_packet(
            eth_src=self.config_data["traffic"]["in_pkt_header"]["eth_mac"][0],
            eth_dst=self.config_data["traffic"]["in_pkt_header"]["eth_mac"][1],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_address"][1],
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_address"][2],
            udp_sport=self.config_data["traffic"]["in_pkt_header"]["udp_port"][0],
            udp_dport=self.config_data["traffic"]["in_pkt_header"]["udp_port"][1],
        )
        send_packet(self, port_ids[self.config_data["traffic"]["send_port"][3]], pkt)
        try:
            verify_no_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][1]][1]
            )
            log.passed(
                " Verification of udp packet check passed : No traffic from TAP3 to TAP1 port"
            )
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(
                f" Verification of udp packet sent failed with exception {err}: TAP3->TAP1"
            )

        pkt = simple_udp_packet(
            eth_src=self.config_data["traffic"]["in_pkt_header"]["eth_mac"][0],
            eth_dst=self.config_data["traffic"]["in_pkt_header"]["eth_mac"][1],
            ip_src=self.config_data["traffic"]["in_pkt_header"]["ip_address"][1],
            ip_dst=self.config_data["traffic"]["in_pkt_header"]["ip_address"][2],
            udp_sport=self.config_data["traffic"]["in_pkt_header"]["udp_port"][0],
            udp_dport=self.config_data["traffic"]["in_pkt_header"]["udp_port"][1],
        )
        send_packet(self, port_ids[self.config_data["traffic"]["send_port"][3]], pkt)
        try:
            verify_no_packet(
                self, pkt, port_ids[self.config_data["traffic"]["receive_port"][2]][1]
            )
            log.passed(
                " Verification of udp packet check passed : No traffic from TAP3 to TAP2 port"
            )
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            log.failed(
                f" Verification of udp packet sent failed with exception {err}: TAP3->TAP2"
            )

        self.dataplane.kill()

    def tearDown(self):
        # Deleting rules
        log.info("Deleting rules")
        table = self.config_data["table"][1]
        for del_action in table["del_action"]:
            p4rt_ctl.p4rt_ctl_del_entry(
                table["switch"], table["name"], del_action.split(",")[0]
            )

        # Deleting group
        log.info("Deleting groups")
        table = self.config_data["table"][0]
        for del_group in table["del_group"]:
            p4rt_ctl.p4rt_ctl_del_group(table["switch"], table["name"], del_group)

        # Delting member
        log.info("Deleting members")
        for del_member in table["del_member"]:
            p4rt_ctl.p4rt_ctl_del_member(table["switch"], table["name"], del_member)

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
