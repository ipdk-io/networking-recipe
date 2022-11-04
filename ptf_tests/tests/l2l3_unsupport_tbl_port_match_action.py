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
DPDK L2 L3 Configure Unsupport table, ports, match fields and actions
"""

# in-built module imports
import time
import sys

# Unittest related imports
import unittest

# ptf related imports
import ptf
import ptf.dataplane as dataplane
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config

# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4, gnmi_get_params_counter


class L2L3_Unpsupport_Tbl_Port_Match_Action(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True # for verify_packets to ignore other packets received at the interface
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)

        self.config_data = get_config_dict(config_json)

        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)


    def runTest(self):
        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        ip_set_ipv4(self.interface_ip_list)

        port_list = self.config_data['port_list']
        port_ids = test_utils.add_port_to_dataplane(port_list)

        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)

        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
        
      
        for table in self.config_data['table']:
            print(f"Scenario : {table['description']}")
            print(f"Adding {table['description']} rules")
            if table['name'] == "pipe.ipv4_host":
                #add invalid match action
                for match_action in table['match_action']:
                    if ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"Failed: invalid entry {match_action} should not be added")
                    print (f"PASS: expected failure to add Invalid match action {match_action}")
            elif table['name'] == "ingress.ipv4_host_dst":
                #add valid match action
                for match_action in table['match_action']:
                    if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"Failed to add table entry {match_action}")

        print (f"Sending Traffic")
        # Verify support table, ports and match actions
        for i in range(len(self.config_data['table'][1]['match_action'])):
            print (f"Verifing match action {table['match_action'][i]}") 
            pkt = simple_tcp_packet(ip_src = self.config_data['traffic']['ip_src'][0],
                                        ip_dst=self.config_data['traffic']['ip_dst'][i])
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][i]],
                                        pkt, count= self.config_data['traffic']['pkt_num'])
            try:
                verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][i]][1])
                print(f"PASS: Verification of packets passed, packet received as per rule {i+1}")
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: Verification of packets sent failed with exception {err}")
                
        self.dataplane.kill()

    def tearDown(self):
    
        for table in self.config_data['table']:
            if table['name'] == "ingress.ipv4_host_dst":
                print(f"Deleting {table['description']} rules")
                for del_action in table['del_action']:
                    ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)
         
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
