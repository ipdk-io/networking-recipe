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
DPDK L3 Exact Match (match fields, actions) with vHost

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
from common.lib.telnet_connection import connectionManager


class Control_Port_Counter(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        
        try:
            self.vm_cred = test_params['vm_cred']
        except KeyError:
            self.vm_cred = ""
            
        self.config_data = get_config_dict(config_json,vm_location_list=test_params['vm_location_list'],vm_cred=self.vm_cred)
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

        # set pipe line
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
        
        #bring up control TAP
        ip_set_ipv4(self.interface_ip_list)
        
        # add l3 exact match forward rules
        for table in self.config_data['table']:
            print(f"Scenario : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # create VMs
        result, vm_name = test_utils.vm_create(self.config_data['vm_location_list'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"VM creation failed for {vm_name}")
       
        # create telnet instance for VMs created
        vm_id = 0
        for vm, port in zip(self.config_data['vm'], self.config_data['port']):
           globals()["conn"+str(vm_id+1)] = connectionManager("127.0.0.1", f"655{vm_id}", vm['vm_username'], vm['vm_password'])
           globals()["vm"+str(vm_id+1)+"_command_list"] = [f"ip addr add {port['ip_address']} dev {port['interface']}", f"ip link set dev {port['interface']} up", f"ip link set dev {port['interface']} address {port['mac_local']}" , f"ip route add {vm['dst_nw']} via {vm['dst_gw']} dev {port['interface']}", f"ip neigh add dev {port['interface']} {vm['remote_ip']} lladdr {vm['mac_remote']}"]
           vm_id+=1
      
        # configuring VMs
        print("Configuring VM0 ....")
        test_utils.configure_vm(conn1, vm1_command_list)

        print(f"Adding control to ovs bridge")
        num = self.config_data['traffic']['number_pkts'][0]
        result = test_utils.ovs_add_ctrl_port_to_bridge(self.config_data["dump_bridge"], self.control_port, self.config_data["p4_device_id"])
        if result:
            print (f"PASS: control port are added in ovs")
        else:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: failed to add control port to ovs")
    
        time.sleep(5) # wait for sometime to pybass traffic noise when bring up TAP

        print("Ping test from VM0 to control port TAP0")
        # It trigger icmp. using icmp default packet size
        pktlen = self.config_data['traffic']['packet_size'][0]
        vm_ping_num = num * 2 # vm ping has a double dumy ping 
        num_buffer = vm_ping_num + self.config_data['traffic']['count_buffer'][0] + 1
        bytes, bytes_buffer = pktlen * vm_ping_num, pktlen * num_buffer

        counter1= test_utils.get_ovs_port_dump(self.config_data["dump_bridge"], self.control_port)
        result = test_utils.vm_to_vm_ping_drop_test(conn1, self.config_data['traffic']['ip_dst'][0],count=num )
        if result:
            print("PASS: expected ping failure BUT at lease it trigger counter")
        counter2 = test_utils.get_ovs_port_dump(self.config_data["dump_bridge"], self.control_port)
      
        pkt_count = counter2[self.control_port[0]]['tx']['pkts'] - counter1[self.control_port[0]]['tx']['pkts']
        if pkt_count not in range(vm_ping_num, num_buffer):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAILED: expected {vm_ping_num} tx pkts counter but {pkt_count} ")
        else:
            print(f"PASS: tx pkts counter increased by {vm_ping_num}")
    
        bytes_count = counter2[self.control_port[0]]['tx']['bytes'] - counter1[self.control_port[0]]['tx']['bytes']
        if bytes_count not in range(bytes, bytes_buffer):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAILED: expected {bytes} tx bytes counter but {bytes_count}")
        else:
            print(f"PASS: tx bytes counter increased by {bytes_count}")
        
        print ("Sending traffic from control TAP to vhost ")
        # It triger ARP. Using arp default packet size
        pktlen = self.config_data['traffic']['packet_size'][1]
        num_buffer =  num + self.config_data['traffic']['count_buffer'][0] + 1
        bytes, bytes_buffer = pktlen * num, pktlen * num_buffer
        
        #Record counter before sending traffic
        counter1= test_utils.get_ovs_port_dump(self.config_data["dump_bridge"], self.control_port)
        #Ping from control TAP to vhost
        pingcmd = f"ping -I {self.control_port[0]}  {self.config_data['traffic']['ip_dst'][1]} -c {num}"
        result = test_utils.local_ping(pingcmd)
        if not result:
            print("PASS: expected ping failure BUT at lease it trigger counter") 
          
        #Record counter afteer sending traffic
        counter2 = test_utils.get_ovs_port_dump(self.config_data["dump_bridge"], self.control_port)
    
        pkt_count = counter2[self.control_port[0]]['rx']['pkts'] - counter1[self.control_port[0]]['rx']['pkts']
        if pkt_count not in range(num, num_buffer):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAILED: expected {num} tx pkts counter but {pkt_count} ")
        else:
            print(f"PASS: tx pkts counter increased by {num}")
        
        bytes_count = counter2[self.control_port[0]]['rx']['bytes'] - counter1[self.control_port[0]]['rx']['bytes']
        if bytes_count not in range(bytes, bytes_buffer):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAILED: expected {bytes} tx bytes counter but {bytes_count}")
        else:
            print(f"PASS: rx bytes counter increased by {bytes_count}")
               
        # close telnet connections
        conn1.close()
   
    def tearDown(self):

        # delete table entries
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
