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
DPDK Hot Plug multi port test

"""

# in-built module imports
import time
import sys
from itertools import dropwhile

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
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_gnmi_params_hotplug, get_interface_ipv4_dict, get_interface_ipv4_dict_hotplug, get_interface_mac_dict_hotplug, get_interface_ipv4_route_dict_hotplug, create_port_vm_map
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4
from common.lib.telnet_connection import connectionManager


class Dpdk_Hot_Plug(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True # for verify_packets to ignore other packets received at the interface
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        try:
            self.vm_cred = test_params['vm_cred']
        except KeyError:
            self.vm_cred = ""
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)

        self.config_data = get_config_dict(config_json,vm_location_list=test_params['vm_location_list'])
        print(self.config_data)

        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.gnmicli_hotplug_params = get_gnmi_params_hotplug(self.config_data)
        print(self.gnmicli_hotplug_params)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)

    def runTest(self):
        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
        '''self.config_data['pb_bin'] = "common/p4c_artifacts/hot_plug/simple_l3.pb.bin"
        self.config_data['p4_info'] = "common/p4c_artifacts/hot_plug/p4Info.txt"'''

        result, vm_name = test_utils.vm_create_with_hotplug(self.config_data)
        print(result)
        print(vm_name)
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to create {vm_name}")

        print("Sleeping for 30 seconds for the vms to come up")
        time.sleep(30)
        
        vm=self.config_data['vm'][0]
        conn1 = connectionManager(vm['hotplug']['qemu-socket-ip'],vm['hotplug']['serial-telnet-port'],vm['vm_username'], password=vm['vm_password'])
        
        vm1=self.config_data['vm'][1]
        conn2 = connectionManager(vm1['hotplug']['qemu-socket-ip'],vm1['hotplug']['serial-telnet-port'],vm1['vm_username'], password=vm1['vm_password'])

        vm1_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result1 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        vm2_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn2, vm2_command_list)[0]
        result = result.split("\n")
        vm2result1 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        if not gnmi_cli_set_and_verify(self.gnmicli_hotplug_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure hotplug through gnmi")

        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result2 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        result = test_utils.sendCmd_and_recvResult(conn2, vm2_command_list)[0]
        result = result.split("\n")
        vm2result2 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        vm1interfaces = list(set(vm1result2) - set(vm1result1))
        vm1interfaces = [x.strip() for x in vm1interfaces]
        print("interfaces: ",vm1interfaces)
       
        vm2interfaces = list(set(vm2result2) - set(vm2result1))
        vm2interfaces = [x.strip() for x in vm2interfaces]
        print("interfaces: ",vm2interfaces)

        vm1interfaces.extend(vm2interfaces)
        self.interface_ip_list_hotplug = get_interface_ipv4_dict_hotplug(self.config_data,vm1interfaces)
        print("self.interface_ip_list_hotplug: ",self.interface_ip_list_hotplug)

        self.interface_ipv4_route_hotplug = get_interface_ipv4_route_dict_hotplug(self.interface_ip_list_hotplug)
        print("self.interface_ipv4_route_hotplug: ",self.interface_ipv4_route_hotplug)

        self.interface_mac_list_hotplug = get_interface_mac_dict_hotplug(self.config_data,vm1interfaces)
        print("self.interface_mac_list_hotplug: ", self.interface_mac_list_hotplug)

        port_list = self.config_data['port_list']
        port_ids = test_utils.add_port_to_dataplane(port_list)

        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)

        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        for table in self.config_data['table']:

            print(f"Scenario : l3 exact match : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")
       
        print("VM1: \n")
        result = test_utils.vm_interface_up(conn1, [self.interface_ip_list_hotplug[0]])

        result = test_utils.vm_interface_configuration(conn1, [self.interface_ip_list_hotplug[0]])
        result = test_utils.vm_route_configuration(conn1, list(self.interface_ipv4_route_hotplug[0].keys())[0], 
                list(self.interface_ip_list_hotplug[0].values())[0], list(self.interface_ipv4_route_hotplug[1].values())[0])
        result = test_utils.vm_ip_neigh_configuration(conn1, list(self.interface_mac_list_hotplug[0].keys())[0],
                list(self.interface_ip_list_hotplug[1].values())[0], list(self.interface_mac_list_hotplug[1].values())[0])


        print("VM2: \n")
        result = test_utils.vm_interface_up(conn2, [self.interface_ip_list_hotplug[1]])

        result = test_utils.vm_interface_configuration(conn2, [self.interface_ip_list_hotplug[1]])
        result = test_utils.vm_route_configuration(conn2, list(self.interface_ipv4_route_hotplug[1].keys())[0],
                list(self.interface_ip_list_hotplug[1].values())[0], list(self.interface_ipv4_route_hotplug[0].values())[0])
        result = test_utils.vm_ip_neigh_configuration(conn2, list(self.interface_mac_list_hotplug[1].keys())[0],
                list(self.interface_ip_list_hotplug[0].values())[0], list(self.interface_mac_list_hotplug[0].values())[0])
        time.sleep(10)


        port1=self.config_data['port'][1]
        if not test_utils.vm_to_vm_ping_test(conn1, port1['ip'].split('/')[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to ping from vm1 to vm2 ")

        port2=self.config_data['port'][0]
        if not test_utils.vm_to_vm_ping_test(conn2, port2['ip'].split('/')[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to ping from vm2 to vm1 ")



        conn1.close()
        conn2.close()

        self.dataplane.kill()


    def tearDown(self):
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")

