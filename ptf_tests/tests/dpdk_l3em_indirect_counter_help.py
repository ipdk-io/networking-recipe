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
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict

class IPDK_L3EM_INDIRECT_COUNTER_HELP(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        
        self.config_data = get_config_dict(config_json)
      
    def runTest(self):
        
        for option in  self.config_data['help_option']:
            helpcmd = f"ovs-p4ctl {option}"
            print (f"To verify command \"{helpcmd}\"")
            result = test_utils.get_ovs_p4ctl_help(option)
            if result:
                for each in self.config_data['arguments']:
                    if each in result:
                        print (f"PASS: the argument \"{each}\" is verifed in \"{helpcmd}\"")
                    else:
                        self.result.addFailure(self, sys.exc_info())
                        print("FAIL: unable to get argument \"{each}\" in \"{helpcmd}\"")
            else:
                self.result.addFailure(self, sys.exc_info())
                print("FAIL: unable to get \"{helpcmd}\"")
                
    def tearDown(self):
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
