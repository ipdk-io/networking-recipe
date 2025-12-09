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
DPDK MAX vHOST PORT 

"""

# in-built module imports
import time
import sys, unittest

# ptf related imports
import ptf
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config
import common.utils.log as log

# framework related imports
from common.utils.config_file_utils import get_gnmi_params_simple, get_config_dict
from common.utils.gnmi_ctl_utils import (
    gnmi_ctl_set_and_verify,
    gnmi_set_params,
    gnmi_set_params_max_plus1_ports,
)
from common.utils.test_utils import check_and_clear_vhost, check_vhost_socket_count


class Max_vHosts_Port_Mtu(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        test_params = test_params_get()
        config_json = test_params["config_json"]
        self.config_data = get_config_dict(config_json)

        self.gnmictl_params = get_gnmi_params_simple(self.config_data)

    def runTest(self):
        # check and clear vhost ports if available 
        socket_path_dir = self.config_data["socket_path_dir"]
        max_port_count = int(self.config_data["max_port_count"])
        check_vhost = check_and_clear_vhost(socket_path_dir)
        if not check_vhost:
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to check/clear vhost-user socket")

        # Create max number of vhost-rusers port
        log.info(f"Creating vhost ports count: {max_port_count}")
        if (
            gnmi_set_params_max_plus1_ports(self.gnmictl_params)
            and len(self.gnmictl_params) > 16
        ):
            log.passed(
                f"Verification of {len(self.gnmictl_params)}th port creation failure successful | Expected count of max no of ports: less then or equal to 16"
            )
        else:
            self.result.addFailure(self, sys.exc_info())
            self.fail("MAX+1 port created")

    def tearDown(self):
        if self.result.wasSuccessful():
            log.passed("Test has PASSED")
        else:
            log.failed("Test has FAILED")
