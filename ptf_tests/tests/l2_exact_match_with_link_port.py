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
DPDK L2 Exact Match (match fields, actions) with Link Port
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

# scapy related imports
from scapy.packet import *
from scapy.fields import *
from scapy.all import *

# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4


class L2_Exact_Match(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True # for verify_packets to ignore other packets received at the interface
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)
        self.capture_port = test_params['pci_bdf'][:-1] + "1"
        self.config_data = get_config_dict(config_json, test_params['pci_bdf'])
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
        # get port list and add to dataplane
        port_list = self.config_data['port_list']
        port_list[0] = test_utils.get_port_name_from_pci_bdf(self.capture_port)
        port_ids = test_utils.add_port_to_dataplane(port_list)
        
        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)

        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")


        for table in self.config_data['table']:

            print(f"Scenario : l2 exact match : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")
            
            time.sleep(5)
            # forming 1st packet and sending to validate if rule-1 hits or not
            print("sending packet to check if rule 1  hits")
            if table['description'] == "table_for_dst_mac":
                pkt = simple_tcp_packet(eth_dst=self.config_data['traffic']['in_pkt_header']['eth_mac_1'])
            else:
                pkt = simple_tcp_packet(eth_src=self.config_data['traffic']['in_pkt_header']['eth_mac_1'])

            # Verify whether packet is received as per rule 1 
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][0]], pkt)
            try:
                verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][0]][1])
                print(f"PASS: Verification of packets passed, packet received as per rule 1")
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: Verification of rule 1 packets sent failed with exception {err}")
        
           
            # forming 2nd packet and sending to validate if rule-2 hits or not
            print("sending packet to check if rule 2  hits")
            if table['description'] == "table_for_dst_mac":
                pkt = simple_tcp_packet(eth_dst=self.config_data['traffic']['in_pkt_header']['eth_mac_2'])
            else:
                pkt = simple_tcp_packet(eth_src=self.config_data['traffic']['in_pkt_header']['eth_mac_2'])

            # Verify whether packet is received as per rule 2 
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][1]], pkt)
            try:
                verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][1]][1])
                print(f"PASS: Verification of packets passed, packet received as per rule 2")
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: Verification of rule 2 packets sent failed with exception {err}")


            # forming 2th packet and sending to validate if rule-3 hits or not
            print("sending packet to check if rule 3 hits")
            if table['description'] == "table_for_dst_mac":
                pkt = simple_tcp_packet(eth_dst=self.config_data['traffic']['in_pkt_header']['eth_mac_3'])
            else:
                pkt = simple_tcp_packet(eth_src=self.config_data['traffic']['in_pkt_header']['eth_mac_3'])
            # Verify whether packet is dropped as per rule 3
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][0]], pkt)
            try:
                verify_no_packet_any(self, pkt, device_number=0, ports=[port_ids[self.config_data['traffic']['receive_port'][1]][1]])
                print(f"PASS: Verification of packets passed, packet dropped as per rule 3")
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: Verification of rule 3 packets sent failed with exception {err}")

            # forming 2th packet and sending to validate if rule-4 hits or not
            print("sending packet to check if rule 4 hits")
            if table['description'] == "table_for_dst_mac":
                pkt = simple_tcp_packet(eth_dst=self.config_data['traffic']['in_pkt_header']['eth_mac_4'])
            else:
                pkt = simple_tcp_packet(eth_src=self.config_data['traffic']['in_pkt_header']['eth_mac_4'])
            
            # Verify whether packet is dropped as per rule 4 
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][1]], pkt)
            try:
                verify_no_packet_any(self, pkt, device_number=0, ports=[port_ids[self.config_data['traffic']['receive_port'][1]][0]])
                print(f"PASS: Verification of packets passed, packet dropped as per rule 4")
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: Verification of rule 4 packets sent failed with exception {err}")


        self.dataplane.kill()


    def tearDown(self):
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
        

 

