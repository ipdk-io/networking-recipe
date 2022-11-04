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

DPDK Action Selector Traffic Test with TAP Port
TC1 : 2 members with action send (to 2 different ports) associated to 1 group with match field dst IP
TC2 : 5 members with action send (to 5 different ports) associated to 3 groups with match field dst IP 

"""

# in-built module imports
from itertools import count
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
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4, gnmi_get_params_counter


class L3_Action_Selector(BaseTest):

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
        # set pipe line
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
        
        table = self.config_data['table'][0]
        
        print(f"##########  Scenario : {table['description']} ##########")

        print("Add action profile members")
        for member in table['member_details']:
            if not ovs_p4ctl.ovs_p4ctl_add_member_and_verify(table['switch'],table['name'],member):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add member {member}")

        print("Adding action selector groups")
        group_count = 0
        for group in table['group_details']:
            if not ovs_p4ctl.ovs_p4ctl_add_group_and_verify(table['switch'],table['name'],group):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add group {group}")
            group_count+=1
        
        print(f"Setting up rule for : {table['description']}")
        table = self.config_data['table'][1]
        for match_action in table['match_action']:
            if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add table entry {match_action}")

        num = self.config_data['traffic']['number_pkts'][0]
        pktlen = self.config_data['traffic']['payload_size'][0]
        total_octets_send = pktlen*num
        # In case of background traffic noise, a small buffer is considered
        num_buffer = num + self.config_data['traffic']['count_buffer'][0] + 1
        octets_buffer = pktlen * num_buffer

        # verify whether traffic hits group-1
        send_port_id = self.config_data['traffic']['send_port'][0]
        receive_port_id= self.config_data['traffic']['receive_port'][0]

        # There would have many traffic noise when bring up port initally. Waiting for 
        # backgroud traffic pypass.Then it's more clean to count expected traffic
        time.sleep(10)
       
        ####total_num, total_octets =  num, total_octets_send
        for src in self.config_data['traffic']['in_pkt_header']['ip_src']:
            print("sending packet to check if it hit group 1")

            # record port counter before sending traffic 
            receive_cont_1 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
            if not receive_cont_1:  
                self.result.addFailure(self, sys.exc_info())
                print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
            send_cont_1 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
            if not send_cont_1:
                self.result.addFailure(self, sys.exc_info())
                print (f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")
        
            pkt = simple_tcp_packet(ip_src=src, ip_dst=self.config_data['traffic']['in_pkt_header']['ip_dst'][0],pktlen=pktlen)
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][0]], pkt, count=num)
            try:
                verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][0]][1])
                print(f"PASS: Verification of packets passed, packets received as per group 1: member 1")
            except Exception as err:
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: Verification of packets sent failed with exception {err}")
    
            #Record port counter after sending traffic
            send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
            if not send_cont_2:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

            receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
            if not receive_cont_2:
                self.result.addFailure(self, sys.exc_info())
                print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")

            #checking counter update
            for each in self.config_data['traffic']['pkts_counter']:
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

            for each in self.config_data['traffic']["octets_counter"]:
                if each == 'in-octets':
                    update = test_utils.compare_counter(send_cont_2,send_cont_1)
                    port = self.config_data['port'][send_port_id]['name']
                if each == 'out-octets':
                    update = test_utils.compare_counter(receive_cont_2,receive_cont_1)
                    port = self.config_data['port'][receive_port_id]['name']
            
                if update[each] in range(total_octets_send, octets_buffer):
                    print(f"PASS: {total_octets_send:} packets expected and {update[each]} verified on {port} {each} counter")
                else:
                    print(f"FAIL: {total_octets_send} packets expected but {update[each]} verified on {port} {each} counter")
                    self.result.addFailure(self, sys.exc_info())

        # verify whether traffic hits group-2
        iteration = 1 
        for src in self.config_data['traffic']['in_pkt_header']['ip_src']:
         
            send_port_id = self.config_data['traffic']['send_port'][1]
            if iteration == 1:
                receive_port_id= self.config_data['traffic']['receive_port'][1]
            if iteration == 2:
                receive_port_id= self.config_data['traffic']['receive_port'][2]

            # Record port counter before sending traffic
            receive_cont_1 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
            if not receive_cont_1:  
                self.result.addFailure(self, sys.exc_info())
                print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
            send_cont_1 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
            if not send_cont_1:
                self.result.addFailure(self, sys.exc_info())
                print (f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

            print("sending packet to check if it hit group 2")
            pkt = simple_tcp_packet(ip_src=src, ip_dst=self.config_data['traffic']['in_pkt_header']['ip_dst'][1],pktlen=pktlen)
            send_packet(self, port_ids[self.config_data['traffic']['send_port'][1]], pkt, count=num)
            if iteration == 1:
                # Sending traffic
                try:
                    verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][1]][1])
                    print(f"PASS: Verification of packets passed, packets received as per group 2 : member 2")
                except Exception as err:
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"FAIL: Verification of packets sent failed with exception {err}")

                #Record port counter after sending traffic
                send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
                if not send_cont_2:
                    self.result.addFailure(self, sys.exc_info())
                    print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

                receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
                if not receive_cont_2:
                    self.result.addFailure(self, sys.exc_info())
                    print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
              
                #checking counter update
                for each in self.config_data['traffic']['pkts_counter']:
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

                for each in self.config_data['traffic']["octets_counter"]:
                    if each == 'in-octets':
                        update = test_utils.compare_counter(send_cont_2,send_cont_1)
                        port = self.config_data['port'][send_port_id]['name']
                    if each == 'out-octets':
                        update = test_utils.compare_counter(receive_cont_2,receive_cont_1)
                        port = self.config_data['port'][receive_port_id]['name']
                
                    if update[each] in range(total_octets_send, octets_buffer):
                        print(f"PASS: {total_octets_send:} packets expected and {update[each]} verified on {port} {each} counter")
                    else:
                        print(f"FAIL: {total_octets_send} packets expected but {update[each]} verified on {port} {each} counter")
                        self.result.addFailure(self, sys.exc_info())
                
            elif iteration  == 2 :
                try:
                    verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][2]][1])
                    print(f"PASS: Verification of packets passed, packets received as per group 2 : member 3")
                except Exception as err:
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"FAIL: Verification of packets sent failed with exception {err}")

                #Record port counter after sending traffic
                send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
                if not send_cont_2:
                    self.result.addFailure(self, sys.exc_info())
                    print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

                receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
                if not receive_cont_2:
                    self.result.addFailure(self, sys.exc_info())
                    print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
    
                #checking counter update
                for each in self.config_data['traffic']['pkts_counter']:
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

                for each in self.config_data['traffic']["octets_counter"]:
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
                
            else:
                self.result.addFailure(self, sys.exc_info())
                self.fail("FAIL: wrong number of ip_src list provided")

            iteration+=1

        # verify whether traffic hits group-3
        if group_count == 3:
            iteration = 1
            for src in self.config_data['traffic']['in_pkt_header']['ip_src']:

                send_port_id = self.config_data['traffic']['send_port'][1]
                if iteration == 1:
                    receive_port_id= self.config_data['traffic']['receive_port'][3]
                if iteration == 2:
                    receive_port_id= self.config_data['traffic']['receive_port'][4]

                #Record port counter before sending traffic
                receive_cont_1 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
                if not receive_cont_1:  
                    self.result.addFailure(self, sys.exc_info())
                    print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
                send_cont_1 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
                if not send_cont_1:
                    self.result.addFailure(self, sys.exc_info())
                    print (f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

                print("sending packet to check if it hit group 3")
                pkt = simple_tcp_packet(ip_src=src, ip_dst=self.config_data['traffic']['in_pkt_header']['ip_dst'][2], pktlen=pktlen)
                send_packet(self, port_ids[self.config_data['traffic']['send_port'][1]], pkt, count=num)
                if iteration == 1:
                    try:
                        verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][3]][1])
                        print(f"PASS: Verification of packets passed, packets received as per group 3 : member 4")
                    except Exception as err:
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"FAIL: Verification of packets sent failed with exception {err}")
                   
                    #Record port counter after sending traffic
                    send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
                    if not send_cont_2:
                        self.result.addFailure(self, sys.exc_info())
                        print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

                    receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
                    if not receive_cont_2:
                        self.result.addFailure(self, sys.exc_info())
                        print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
                   
                    #checking counter update
                    for each in self.config_data['traffic']['pkts_counter']:
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

                    for each in self.config_data['traffic']["octets_counter"]:
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

                elif iteration  == 2 :
                    try:
                        verify_packet(self, pkt, port_ids[self.config_data['traffic']['receive_port'][4]][1])
                        print(f"PASS: Verification of packets passed, packets received as per group 3 : member 5")
                    except Exception as err:
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"FAIL: Verification of packets sent failed with exception {err}")

                    #Record port counter after sending traffic
                    send_cont_2 = gnmi_get_params_counter(self.gnmicli_params[send_port_id])
                    if not send_cont_2:
                        self.result.addFailure(self, sys.exc_info())
                        print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")

                    receive_cont_2 = gnmi_get_params_counter(self.gnmicli_params[receive_port_id])
                    if not receive_cont_2:
                        self.result.addFailure(self, sys.exc_info())
                        print (f"FAIL: unable to get counter of {self.config_data['port'][receive_port_id]['name']}")
                   
                    #checking counter update
                    for each in self.config_data['traffic']['pkts_counter']:
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

                    for each in self.config_data['traffic']["octets_counter"]:
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

                else:
                    self.result.addFailure(self, sys.exc_info())
                    self.fail("FAIL: wrong number of ip_src list provided")

                iteration+=1
      
        self.dataplane.kill()

    def tearDown(self):
        
        table = self.config_data['table'][1]
        
        print(f"Deleting rules")
        for del_action in table['del_action']:
            ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action.split(",")[0])
       
        table = self.config_data['table'][0]
        print("Deleting groups")
        for del_group in table['del_group']:
            ovs_p4ctl.ovs_p4ctl_del_group(table['switch'],table['name'],del_group)
                 
        print("Deleting members")    
        for del_member in table['del_member']:
            ovs_p4ctl.ovs_p4ctl_del_member(table['switch'],table['name'],del_member)

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")

 
