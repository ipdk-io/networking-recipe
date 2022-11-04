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
DPDK L3 Exact Match (match fields, actions) with TAP Port
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
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4
import common.utils.tcpdump_utils as tcpdump_utils


class Conttol_Port_Link(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True # for verify_packets to ignore other packets received at the interface
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        self.capture_port = test_params['pci_bdf'][:-1] + "1"
        self.config_data = get_config_dict(config_json, test_params['pci_bdf'])
        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)
        self.control_port =  test_utils.get_control_port(self.config_data)
        
    def runTest(self):
        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        #bring up control TAP
        ip_set_ipv4(self.interface_ip_list)
    
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
        
        for table in self.config_data['table']:
            print(f"Scenario : Control TAP with Link : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")
        
        print (f"Starting tcpdump to capture any traffic through control port {self.control_port[0]}")
        tcpdump_utils.tcpdump_start_pcap(self.control_port[0])
        
        # waiting for more traffic
        time.sleep(5)
        print (f"Verify if any traffic is captured in contorl port")
        if tcpdump_utils.tcpdump_get_pcap(self.control_port[0]):
            print(f"PASS: There are some control traffic via control port{self.control_port[0]}")
        else:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: No traffic seen in control pot {self.control_port[0]}")

    def tearDown(self):
        print (f'Start to tear down')
        for table in self.config_data['table']:
            print(f"Delete {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)
                
        print (f"Check if any tcpdump is running and tear down it")
        tcpdump_utils.tcpdump_tear_down()
    
        result = tcpdump_utils.tcpdump_remove_pcap_file(self.control_port[0])
        if result: 
            print (f"Remove exiting pcap file directory {result}")
        else:
            self.fail(f"FAIL: Unable to remove exising pcap file")
            
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
      
