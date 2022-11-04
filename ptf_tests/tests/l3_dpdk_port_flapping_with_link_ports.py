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
DPDK Link Flapping Test Case
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
import common.lib.port_config as port_config

class Dpdk_Link_Flapping(BaseTest):

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
        self.config_data = get_config_dict(config_json, vm_location_list=test_params['vm_location_list'])

    def runTest(self):
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
        
        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
   

        table = self.config_data['table'][0]
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
        self.conn_obj_list = []
        vm_cmd_list = []
        vm_id = 0
        for vm, port in zip(self.config_data['vm'], self.config_data['port']):
           globals()["conn"+str(vm_id+1)] = connectionManager("127.0.0.1", f"655{vm_id}", vm['vm_username'], vm['vm_password'], timeout=20)
           self.conn_obj_list.append(globals()["conn"+str(vm_id+1)])
           globals()["vm"+str(vm_id+1)+"_command_list"] = [f"ip link set dev {port['interface']} down", f"ip link set dev {port['interface']} address {port['mac']}", f"ip link set dev {port['interface']} up", f"ip addr add {port['ip_address']} dev {port['interface']}", f"ip neigh add dev {port['interface']}  {vm['remote_ip']} lladdr {vm['mac_remote']}"]
           vm_cmd_list.append(globals()["vm"+str(vm_id+1)+"_command_list"])
           vm_id+=1 

        # configuring VMs
        for i in range(len(self.conn_obj_list)):
            print (f"Configuring VM{i}....")
            test_utils.configure_vm(self.conn_obj_list[i], vm_cmd_list[i])
       
        port_list = self.config_data['port_list']
        port_list[0] = test_utils.get_port_name_from_pci_bdf(self.capture_port)
        phy_interface1 = port_list[0]
        port_ids = test_utils.add_port_to_dataplane(port_list)

        # Now bring PHY interface down
        print('Bring PHY1 interface down...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.iplink_enable_disable_link(phy_interface1, status_to_change='down')


        # Now change Mac address 
        print('Change Mac of PHY1 interface...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.ip_link_set_mac(phy_interface1, self.config_data['vm'][0]['mac_remote'])

        # Now bring PHY interface up
        print('Bring PHY1 interface up...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.iplink_enable_disable_link(phy_interface1, status_to_change='up')


        # Now Give Ip address
        print('Give Ip address on  PHY1 interface...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.ipaddr_ipv4_set(phy_interface1, self.config_data['port'][1]['ip_address'])


        # Now bring PHY interface up
        print('Bring PHY1 interface up...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.iplink_enable_disable_link(phy_interface1, status_to_change='up')

        # Now Add neighbour entry 
        print('Add neigh entry on  PHY1 interface...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.ip_neigh_add(phy_interface1, self.config_data['port'][1]['remote_ip'], self.config_data['port'][1]['mac_local'])

        # ping test between VM to link port
        time.sleep(5)
        print("------------------------------")
        print("Ping Test from VM to Link port")
        print("------------------------------")
        result = test_utils.vm_to_vm_ping_test(self.conn_obj_list[0], self.config_data['vm'][0]['remote_ip'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail("FAIL: Ping test failed for VM0")

        print("------------------------------")
        print("Ping Test from Link port to VM")
        print("------------------------------")
        #Ping from link port to vhost
        pingcmd = f"ping -I {phy_interface1}  {self.config_data['port'][1]['remote_ip']} -c {self.config_data['num']}"
        result = test_utils.local_ping(pingcmd)
        print(f"PASS: Ping successful to destination {self.config_data['port'][1]['remote_ip']}")

        # Now bring PHY interface down for flapping
        print("-----------------------------------------")
        print('Bring PHY1 interface down for flapping...')
        print("-----------------------------------------")
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.iplink_enable_disable_link(phy_interface1, status_to_change='down')
        
        
        # ping test between VM to link port
        print("---------------------------------------------------------")
        print("Ping test from VM to Link port after down the phy port...")
        print("---------------------------------------------------------")
        result = test_utils.vm_to_vm_ping_drop_test(self.conn_obj_list[0], self.config_data['vm'][0]['remote_ip'])
        
        # Now bring PHY interface up
        print("---------------------------------")
        print('Now bringing up PHY1 interface...')
        print("---------------------------------")
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.iplink_enable_disable_link(phy_interface1, status_to_change='up')

        # ping test between VM to link port after up the port
        time.sleep(5)
        print("----------------------------------------------------------------")
        print("Ping test should work from VM to Link port after up interface...")
        print("----------------------------------------------------------------")
        result = test_utils.vm_to_vm_ping_test(self.conn_obj_list[0], self.config_data['vm'][0]['remote_ip'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail("FAIL: Ping test failed for VM0")

              
        print ("close VM telnet session")
        for conn in self.conn_obj_list:
            conn.close()

    def tearDown(self):
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)

        port_list = self.config_data['port_list']
        port_list[0] = test_utils.get_port_name_from_pci_bdf(self.capture_port)
        phy_interface1 = port_list[0]

        # Delete IP Address from Phy Interface
        print('Delete Ip address from Phy Interface...')
        portconfig_obj = port_config.PortConfig()
        portconfig_obj.Ip.ipaddr_ipv4_del(phy_interface1, self.config_data['port'][1]['ip_address'])

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
        
