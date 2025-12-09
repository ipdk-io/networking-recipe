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
DPDK Flow Dump with PSA indirect counter

"""
# in-built module imports
import sys

# Unittest related imports
import unittest

# ptf related imports
from ptf.base_tests import BaseTest
from ptf.testutils import *

# framework related imports
import common.utils.log as log
import common.utils.p4rtctl_utils as p4rt_ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple
from common.utils.gnmi_ctl_utils import gnmi_ctl_set_and_verify


class DPDK_Flow_Dump_with_PSA_indirect_counter(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()

        test_params = test_params_get()
        config_json = test_params["config_json"]

        self.config_data = get_config_dict(config_json)
        self.gnmictl_params = get_gnmi_params_simple(self.config_data)

    def runTest(self):
        # Create Ports using gnmictl
        if not gnmi_ctl_set_and_verify(self.gnmictl_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi ctl ports")

        # Create binary from p4 program using pna and psa arch
        if not test_utils.gen_dep_files_p4c_tdi_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # Run Set-pipe command for set pipeline
        if not p4rt_ctl.p4rt_ctl_set_pipe(
            self.config_data["switch"],
            self.config_data["pb_bin"],
            self.config_data["p4_info"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # add  forward rules
        for table in self.config_data["table"]:
            log.info(f"Scenario : {table['description']}")
            log.info(f"Adding {table['description']} rules")
            for match_action in table["match_action"]:
                if not p4rt_ctl.p4rt_ctl_add_entry(
                    table["switch"], table["name"], match_action
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Get dump entries table
        log.info("Get flow dump table entry")
        dump_table = p4rt_ctl.p4rt_ctl_dump_entities(self.config_data["switch"])
        # Verify each entry
        log.info("Verify each table entry")
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
        log.info("delete table entries")
        for table in self.config_data["table"]:
            log.info(f"Deleting {table['description']} rules")
            for del_action in table["del_action"]:
                p4rt_ctl.p4rt_ctl_del_entry(table["switch"], table["name"], del_action)

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
