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
DPDK Flow Dump with Action Selector
"""
# in-built module imports
from itertools import count
import sys

# Unittest related imports
import unittest
import common.utils.log as log

# ptf related imports
import ptf
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


class DPDK_Flow_Dump_with_action_selector(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True

        test_params = test_params_get()
        config_json = test_params["config_json"]
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

        # Get dump entries table
        dump_table = p4rt_ctl.p4rt_ctl_dump_entities(self.config_data["switch"])
        # Verify each entry
        if len(dump_table[1:-1]) != len(self.config_data["flow_dump_table"]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                "The table {dump_table[1:-1]} has different number entry than definition"
            )
        else:
            for each in dump_table[1:-1]:
                entry = each.strip()
                if entry in self.config_data["flow_dump_table"]:
                    log.passed(f'The entry "{entry}" in flow dump is verified')
                else:
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f'Failed to verify entry "{entry}"')

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
