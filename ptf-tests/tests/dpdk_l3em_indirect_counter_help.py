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
DPDK L3EM Indirect counter Help

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
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict


class IPDK_L3EM_INDIRECT_COUNTER_HELP(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()

        test_params = test_params_get()
        config_json = test_params["config_json"]

        self.config_data = get_config_dict(config_json)

    def runTest(self):
        # List the p4rt_ctl Help menu
        for option in self.config_data["help_option"]:
            helpcmd = f"p4rt-ctl {option}"
            log.info(f'To verify command "{helpcmd}"')
            result = test_utils.get_p4rt_ctl_help(option)
            if result:
                for each in self.config_data["arguments"]:
                    if each in result:
                        log.passed(
                            f'PASS: the argument "{each}" is verifed in "{helpcmd}"'
                        )
                    else:
                        self.result.addFailure(self, sys.exc_info())
                        log.failed(
                            'FAIL: unable to get argument "{each}" in "{helpcmd}"'
                        )
            else:
                self.result.addFailure(self, sys.exc_info())
                log.failed('unable to get "{helpcmd}"')

    def tearDown(self):
        if self.result.wasSuccessful():
            log.passed("Test has PASSED")
        else:
            log.failed("Test has FAILED")
