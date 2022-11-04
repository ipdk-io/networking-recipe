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
SHOW TAP PORT MTU CONFIG

"""

# in-built module imports
import time
import sys,unittest
# ptf related imports
import ptf
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config
# framework related imports
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_gnmi_params_simple, get_config_dict
from common.utils.gnmi_cli_utils import gnmi_set_params, gnmi_get_element_value, get_port_mtu_linuxcli
from common.lib.port_config import PortConfig


class ShowTapPortMtu(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        test_params = test_params_get()
        config_json = test_params['config_json']

        self.config_data = get_config_dict(config_json)
        self.gnmicli_params = get_gnmi_params_simple(self.config_data)


    def runTest(self):
        print(" ")
        print("Assigning MTU to devices [ If no MTU is assigned, default value of 1500 is expected/assigned ]")
        gnmi_set_params(self.gnmicli_params)
        port_list = self.config_data['port_list']
        port_config = PortConfig()

        default_mtu = False 
        for port in port_list:
            for port_details in self.config_data['port']:
                if 'mtu' not in port_details:
                    port_details['mtu'] = '1500'
                    default_mtu = True
                if (port_details['name'] == port): 
                       assigned_mtu = port_details['mtu']
                       param='device:virtual-device,name:' + port  
                       gnmicli_mtu = gnmi_get_element_value(param, 'mtu')
                       if not gnmicli_mtu:
                           self.result.addFailure(self, sys.exc_info())
                           self.fail(f"Failed to get MTU from GNMI-CLI for " + port + " port")
                       linuxcli_mtu=get_port_mtu_linuxcli(port)
                       if not linuxcli_mtu:
                           self.result.addFailure(self, sys.exc_info())
                           self.fail(f"Failed to get MTU from LINUX-CLI for " + port + " port")
                       if assigned_mtu.strip() == gnmicli_mtu.strip() == linuxcli_mtu.strip():
                           if default_mtu:
                                print("PASS: Port " + port + " MTU Assignment Match Expected MTU | Assigned[Default]: " + assigned_mtu.strip() + " GNMICLI: " \
                                + gnmicli_mtu.strip()+ " LINUXCLI: " + linuxcli_mtu.strip())
                           else:
                                print("PASS: Port " + port + " MTU Assignment Match Expected MTU | Assigned: " + assigned_mtu.strip() + " GNMICLI: " \
                                + gnmicli_mtu.strip()+ " LINUXCLI: " + linuxcli_mtu.strip())
                           return True
                       else:
                           if default_mtu:
                                print("FAIL: Port " + port + " MTU Assignment Mismatch Expected MTU |  Assigned[Default]: " + assigned_mtu.strip() + " GNMICLI: " \
                                + gnmicli_mtu.strip() + " LINUXCLI: " + linuxcli_mtu.strip())
                                self.result.addFailure(self, sys.exc_info())
                           else:
                                print("FAIL: Port " + port + " MTU Assignment Mismatch Expected MTU |  Assigned: " + assigned_mtu.strip() + " GNMICLI: " \
                                + gnmicli_mtu.strip() + " LINUXCLI: " + linuxcli_mtu.strip())
                                self.result.addFailure(self, sys.exc_info())
                       

    def tearDown(self):

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
        

 

