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
DPDK Flow Dump With Adding/Deleting rules
"""

# in-built module imports
import time
import sys
import socket
from binascii import hexlify

# Unittest related imports
import unittest

# ptf related imports
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


class DPDK_Flow_Dump_with_del_add(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config[
            "relax"
        ] = True  # for verify_packets to ignore other packets received at the interface

        test_params = test_params_get()
        config_json = test_params["config_json"]
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

        # Run Set-pipe command for set pipeline
        if not p4rt_ctl.p4rt_ctl_set_pipe(
            self.config_data["switch"],
            self.config_data["pb_bin"],
            self.config_data["p4_info"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Add rules for l3_exact match
        for table in self.config_data["table"]:
            log.info(f"Scenario : l3 exact match : {table['description']}")
            log.info(f"Adding {table['description']} rules")
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
            log.failed(
                f"The table {dump_table[1:-1]} has different entry than definition"
            )
            self.result.addFailure(self, sys.exc_info())
            self.fail(f'Failed to verify flow dump entries"')
        else:
            for each in dump_table[1:-1]:
                entry = each.strip()
                if entry in self.config_data["flow_dump_table"]:
                    log.passed(f'The entry "{entry}" in flow dump is verified')
                else:
                    log.failed(f'The entry "{entry}" in flow dump is not verified')
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f'Failed to verify entry "{entry}"')

        # Delete entry
        log.info("Delete table entry and then verify flow dump")
        for table in self.config_data["table"]:
            log.info(f"Deleting {table['description']} rules")
            for del_action in table["del_action"]:
                p4rt_ctl.p4rt_ctl_del_entry(table["switch"], table["name"], del_action)

        log.info("Verify flow dump table after deleting rules")
        dump_table = p4rt_ctl.p4rt_ctl_dump_entities(self.config_data["switch"])
        if (len(dump_table[1:-1])) != 0:
            log.info("The table is {dump_table[1:-1])} after deletion")
            log.failed(f"The table it not emty after entry deletion")
            self.result.addFailure(self, sys.exc_info())
            self.fail(f'Failed to verify "{dump_table[1:-1]}" after deletion')
        else:
            log.passed("No entry found in flow table after deletion")

        # Add back rules for l3_exact match
        log.info("Add back rules for l3_exact match")
        for table in self.config_data["table"]:
            log.info(f"Scenario : l3 exact match : {table['description']}")
            log.info(f"Adding {table['description']} rules")
            for match_action in table["match_action"]:
                if not p4rt_ctl.p4rt_ctl_add_entry(
                    table["switch"], table["name"], match_action
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        log.info("Verify flow dump table after adding back rules")
        # Verify each entry
        dump_table = p4rt_ctl.p4rt_ctl_dump_entities(self.config_data["switch"])
        if len(dump_table[1:-1]) != len(self.config_data["flow_dump_table"]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"The table {dump_table[1:-1]} has different number entry than definition"
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
        for table in self.config_data["table"]:
            log.info(f"Deleting {table['description']} rules")
            for del_action in table["del_action"]:
                p4rt_ctl.p4rt_ctl_del_entry(table["switch"], table["name"], del_action)

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
