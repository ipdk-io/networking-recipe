# Copyright (c) 2022 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the Licens.
"""
DPDK Tap and Link Port counter feature
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
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4,gnmi_get_params_elemt_value,gnmi_get_params_counter

class Tap_Link_PortCounter(BaseTest):

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
            print(f"Scenario : wcm link port : {table['description']}")
            print(f"Adding {table['description']} rules")
            ##for match_action in table['match_action']:
            # The last match action is not addedd here and skip it for next time 
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # There would have many traffic noise when bring up port initally. Waiting for 
        # backgroud traffic pypass.Then it's more clean to count expected traffic
        time.sleep(10)

        ###########################
        #  Unicast Counter Case
        ###########################
        num = self.config_data['traffic']['in_pkt_header']['number_pkts'][3]
        pktlen = self.config_data['traffic']['in_pkt_header']['payload_size'][1]
        total_octets_send = pktlen*num
        # in case of background traffic noise, a small buffer is considered
        num_buffer = num + self.config_data['traffic']['in_pkt_header']['count_buffer'][0] + 1
        octets_buffer = pktlen * num_buffer

        print (f"Test {num} Unitcast packet from link to TAP")
        send_port_id = self.config_data['traffic']['send_port'][0]
        receive_port_id= self.config_data['traffic']['receive_port'][2]

        print (f"Record counter before sending traffic")
        send_cont_1 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
        receive_cont_1 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
        if not receive_cont_1:  
            self.result.addFailure(self, sys.exc_info())
            print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
        if not send_cont_1:
            self.result.addFailure(self, sys.exc_info())
            print (f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")
    
        pkt = simple_tcp_packet(ip_dst=self.config_data['traffic']['in_pkt_header']['ip_dst'][0], pktlen=pktlen)
        send_packet(self, port_ids[send_port_id], pkt, count=num)
        try:
            verify_packet(self, pkt, port_ids[receive_port_id][1])
            print(f"PASS: Verification of {num} packets passed per rule 2")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: Verification of packets sent failed with exception {err}")
        
        print (f"Record counter after sending traffic")
        send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
        if not send_cont_2:
            self.result.addFailure(self, sys.exc_info())
            print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

        receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
        if not receive_cont_2:
            self.result.addFailure(self, sys.exc_info())
            print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")

        #Idealy we expect counter update is equal to expected num but sometimes the port
        #also receive other unpredicatable brackgroud traffic noise such IPv6 which cause more count.
        #Thus we have to implement the counter update within a buffer range
        #This note apply to all other counter verification
        #checking counter update
        for each in self.config_data['traffic']['in_pkt_header']['pkts_counter']:
            if each == 'in-unicast-pkts':
                update = test_utils.compare_counter(send_cont_2,send_cont_1)
                port = self.config_data['port'][send_port_id]['name']
            if each == 'out-unicast-pkts':
                update = test_utils.compare_counter(receive_cont_2,receive_cont_1)
                port = self.config_data['port'][receive_port_id]['name']
    
            if update[each] in range(num, num_buffer):
                print(f"PASS: {num} packets expected and {update[each]} verified on {port} {each} counter")
            else:
                print(f"FAIL: {num} packets expected but {update[each]} verified on {port} {each} counter")
                self.result.addFailure(self, sys.exc_info())

        for each in self.config_data['traffic']['in_pkt_header']["octets_counter"]:
            if each == 'in-octets':
                update = test_utils.compare_counter(send_cont_2,send_cont_1)
                port = self.config_data['port'][send_port_id]['name']
            if each == 'out-octets':
                update = test_utils.compare_counter(receive_cont_2,receive_cont_1)
                port = self.config_data['port'][receive_port_id]['name']
        
            if update[each] in range(total_octets_send, octets_buffer):
                print(f"PASS: {total_octets_send:} octets expected and {update[each]} verified on {port} {each} counter")
            else:
                print(f"FAIL: {total_octets_send} octets expected but {update[each]} verified on {port} {each} counter")
                self.result.addFailure(self, sys.exc_info())

        ##another direction
        num = self.config_data['traffic']['in_pkt_header']['number_pkts'][1]
        pktlen = self.config_data['traffic']['in_pkt_header']['payload_size'][0]
        total_octets_send = pktlen*num  
        # in case of background traffic noise, a small buffer is considered
        num_buffer = num + self.config_data['traffic']['in_pkt_header']['count_buffer'][0] + 1
        octets_buffer = pktlen * num_buffer

        print (f"Test {num} Unitcast packet from TAP to Link")
        print (f"Record in-octets and in-unicast-pkts counter of TAP0 before sending traffic")
        send_port_id = self.config_data['traffic']['send_port'][1]
        receive_port_id = self.config_data['traffic']['receive_port'][0]

        print (f"Record counter before sending traffic")
        send_cont_1 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
        receive_cont_1 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
        if not receive_cont_1:  
            self.result.addFailure(self, sys.exc_info())
            print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
        if not send_cont_1:
            self.result.addFailure(self, sys.exc_info())
            print (f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")
       
        pkt = simple_tcp_packet(ip_dst=self.config_data['traffic']['in_pkt_header']['ip_dst'][1], pktlen=pktlen)
        send_packet(self, port_ids[send_port_id], pkt, count=num)
        try:
            verify_packet(self, pkt, port_ids[receive_port_id][1])
            print(f"PASS: Verification of {num} packets passed per rule 4")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: Verification of packets sent failed with exception {err}")

        print (f"Record counter after sending traffic")
        send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
        if not send_cont_2:
            self.result.addFailure(self, sys.exc_info())
            print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

        receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
        if not receive_cont_2:
            self.result.addFailure(self, sys.exc_info())
            print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")

        #checking counter update
        for each in self.config_data['traffic']['in_pkt_header']['pkts_counter']:
            if each == 'in-unicast-pkts':
                update = test_utils.compare_counter(send_cont_2,send_cont_1)
                port = self.config_data['port'][send_port_id]['name']
            if each == 'out-unicast-pkts':
                update = test_utils.compare_counter(receive_cont_2,receive_cont_1)
                port = self.config_data['port'][receive_port_id]['name']
        
            if update[each] in range(num, num_buffer):
                print(f"PASS: {num} packets expected and {update[each]} verified on {port} {each} counter")
            else:
                print(f"FAIL: {num} packets expected but {update[each]} verified on {port} {each} counter")
                self.result.addFailure(self, sys.exc_info())

        for each in self.config_data['traffic']['in_pkt_header']["octets_counter"]:
            if each == 'in-octets':
                update = test_utils.compare_counter(send_cont_2,send_cont_1)
                port = self.config_data['port'][send_port_id]['name']
            if each == 'out-octets':
                update = test_utils.compare_counter(receive_cont_2,receive_cont_1)
                port = self.config_data['port'][receive_port_id]['name']
        
            if update[each] in range(total_octets_send, octets_buffer):
                print(f"PASS: {total_octets_send:} octets expected and {update[each]} verified on {port} {each} counter")
            else:
                print(f"FAIL: {total_octets_send} octets expected but {update[each]} verified on {port} {each} counter")
                self.result.addFailure(self, sys.exc_info())
        
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
