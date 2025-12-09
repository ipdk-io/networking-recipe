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
DPDK K8S: stress add/remove services
"""

# in-built module imports
import time
import sys

# Unittest related imports
import unittest

# ptf related imports
import ptf
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config

# framework related imports
import common.utils.log as log
import common.utils.k8_utils as k8_utils
from common.utils.test_utils import git_clone_remote_repo


class K8_DPDK_services(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config[
            "relax"
        ] = True  # for verify_packets to ignore other packets received at the interface
        test_params = test_params_get()
        self.kube_iperf_path = test_params["kube_iperf_path"]
        self.no_of_times = int(test_params["no_of_times"])

    def runTest(self):
        # git clone kube iperf repo: https://github.com/Pharb/kubernetes-iperf3.git
        repo = "github.com/Pharb/kubernetes-iperf3.git"
        if not git_clone_remote_repo(repo, self.kube_iperf_path):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to clone git repo")

        for i in range(self.no_of_times):
            log.info(f"Add/Remove services: Iteration {i+1}")

            # Execute ./steps/setup.sh from https://github.com/Pharb/kubernetes-iperf3.git
            if not k8_utils.execute_iperf_setup(self.kube_iperf_path):
                self.result.addFailure(self, sys.exc_info())
                self.fail("Failed to run setup.sh")

            # Check if iperf3-server is running
            if not k8_utils.check_service_status("iperf3-server"):
                self.result.addFailure(self, sys.exc_info())
                self.fail("iperf3-server not running")

            # Execute ./steps/cleanup.sh from https://github.com/Pharb/kubernetes-iperf3.git
            if not k8_utils.execute_iperf_cleanup(self.kube_iperf_path):
                self.result.addFailure(self, sys.exc_info())
                self.fail("Failed to run cleanup.sh")

            # Check if iperf3-server is not running
            if not k8_utils.check_service_status(
                "iperf3-server", expected_status=False
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail("iperf3-server is still running")

    def tearDown(self):
        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
