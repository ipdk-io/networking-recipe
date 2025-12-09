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

DPDK Action Selector CLI Test

"""

# in-built module imports
from itertools import count
import time
import sys

# Unittest related imports
import unittest

# ptf related imports
import ptf

# import ptf.dataplane as dataplane
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
from common.utils.gnmi_ctl_utils import gnmi_ctl_set_and_verify, ip_set_ipv4


class DPDK_Action_Selector_CLI(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()

        test_params = test_params_get()
        config_json = test_params["config_json"]

        self.config_data = get_config_dict(config_json)
        self.gnmictl_params = get_gnmi_params_simple(self.config_data)

    def runTest(self):
        # Compile p4 file using p4c compiler and generate binary using tdi pipeline builder
        if not test_utils.gen_dep_files_p4c_tdi_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # Create Ports using gnmi-ctl
        if not gnmi_ctl_set_and_verify(self.gnmictl_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        port_list = self.config_data["port_list"]

        # Run Set-pipe command for set pipeline
        if not p4rt_ctl.p4rt_ctl_set_pipe(
            self.config_data["switch"],
            self.config_data["pb_bin"],
            self.config_data["p4_info"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        table = self.config_data["table"][0]

        # Creating action_selector members
        log.info("Add action profile members")
        member_count = 0
        for member in table["member_details"]:
            if not p4rt_ctl.p4rt_ctl_add_member_and_verify(
                table["switch"], table["name"], member
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add member {member}")
            member_count += 1

        # Creating action_selector groups
        log.info("Creating action selector groups")
        group_count = 0
        for group in table["group_details"]:
            if not p4rt_ctl.p4rt_ctl_add_group_and_verify(
                table["switch"], table["name"], group
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add group {group}")
            group_count += 1

        # Getting action_selector members
        log.info("Getting action selector members")
        for mem_id in table["del_member"]:
            if not p4rt_ctl.p4rt_ctl_get_member(
                table["switch"], table["name"], member_id=mem_id
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to get member {mem_id}")

        log.info("Getting action selector groups")
        for grp_id in table["del_group"]:
            if not p4rt_ctl.p4rt_ctl_get_group(
                table["switch"], table["name"], group_id=grp_id
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add group {grp_id}")

        # Deleting groups
        log.info("Deleting groups")
        for del_grp in table["del_group"]:
            p4rt_ctl.p4rt_ctl_del_group(table["switch"], table["name"], del_grp)

        # Deleting members
        log.info("Deleting members")
        for index, del_mem in enumerate(table["del_member"]):
            if index == 0 or index == 7:
                p4rt_ctl.p4rt_ctl_del_member(table["switch"], table["name"], del_mem)

        # Checking for deleted groups. If group still exists, testcase fails
        log.info("Checking deleted groups")
        for grp_id in table["del_group"]:
            if p4rt_ctl.p4rt_ctl_get_group(
                table["switch"], table["name"], group_id=grp_id
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Deleted group check failed for {grp_id}")
            else:
                log.passed("PASS: Expected failure message for deleted group")

        # Checking for deleted members. If member still exists, testcase fails
        log.info("Checking deleted members")
        for index, mem_id in enumerate(table["del_member"]):
            if index == 0 or index == 7:
                if p4rt_ctl.p4rt_ctl_get_member(
                    table["switch"], table["name"], member_id=mem_id
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Deleted member check failed for {mem_id}")
                else:
                    log.passed("PASS: Expected failure message for deleted member")

        # checking for undeleted members. members should exist
        log.info("Checking for undeleted member")
        for index, mem_id in enumerate(table["del_member"]):
            if index in range(1, 7):
                if not p4rt_ctl.p4rt_ctl_get_member(
                    table["switch"], table["name"], member_id=mem_id
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to get member {mem_id}")

    def tearDown(self):
        if self.result.wasSuccessful():
            log.passed("Test has PASSED")
        else:
            self.fail("Test has FAILED")
