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
LNT Hot Plug Add/Remove multi port
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
import common.utils.ovs_utils as ovs_utils
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_gnmi_params_hotplug, get_interface_ipv4_dict, get_interface_ipv4_dict_hotplug, get_interface_mac_dict_hotplug, get_interface_ipv4_route_dict_hotplug, create_port_vm_map
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, iplink_add_vlan_port, ip_set_dev_up, get_tap_port_list, iplink_del_port
from common.lib.telnet_connection import connectionManager


class LNT_Hot_Plug(BaseTest):

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
        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.gnmicli_hotplug_params = get_gnmi_params_hotplug(self.config_data)
        self.gnmicli_hotplug_delete_params = get_gnmi_params_hotplug(self.config_data,action="del")
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)

    def runTest(self):
        # Compile linux networking p4 prog and generate pb.bin
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # Create VMs 
        result, vm_name = test_utils.vm_create_with_hotplug(self.config_data)
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to create {vm_name}")

        print("Sleeping for 30 seconds for the vms to come up")
        time.sleep(30)
   
        vm=self.config_data['vm'][0]
        conn1 = connectionManager(vm['hotplug']['qemu-socket-ip'],vm['hotplug']['serial-telnet-port'],vm['vm_username'], password=vm['vm_password'])
        vm1=self.config_data['vm'][1]
        conn2 = connectionManager(vm1['hotplug']['qemu-socket-ip'],vm1['hotplug']['serial-telnet-port'],vm1['vm_username'], password=vm1['vm_password'])

        # Get the list of interfaces on VMs
        vm1_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result1 = list(dropwhile(lambda x: 'lo\r' not in x, result))
        vm2_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn2, vm2_command_list)[0]
        result = result.split("\n")
        vm2result1 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        # Create ports using gnmi cli
        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        # Hotplug port to VM
        if not gnmi_cli_set_and_verify(self.gnmicli_hotplug_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure hotplug through gnmi")

        # Verify ports are added to VM
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result2 = list(dropwhile(lambda x: 'lo\r' not in x, result))
        result = test_utils.sendCmd_and_recvResult(conn2, vm2_command_list)[0]
        result = result.split("\n")
        vm2result2 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        vm1interfaces = list(set(vm1result2) - set(vm1result1))
        vm1interfaces = [x.strip() for x in vm1interfaces]
        vm2interfaces = list(set(vm2result2) - set(vm2result1))
        vm2interfaces = [x.strip() for x in vm2interfaces]

        if not vm1interfaces:
            print("FAIL: Hotplug add failed for vm1")
            self.result.addFailure(self, sys.exc_info())
            self.fail("Fail to add hotplug through gnmi")
        print("PASS: Added hotplug interface for vm1 ",vm1interfaces)
        if not vm2interfaces:
            print("FAIL: Hotplug add failed for vm2")
            self.result.addFailure(self, sys.exc_info())
            self.fail("Fail to add hotplug through gnmi")
        print("PASS: Added hotplug interface for vm2 ",vm2interfaces)

        # Configure Ip address on VMs
        cmd = ["ip address add " + self.config_data['port'][0]['ip'] + " dev " + vm1interfaces[0]]
        test_utils.sendCmd_and_recvResult(conn1,cmd)
        cmd = ["ip link set dev " + vm1interfaces[0] + " up"]
        test_utils.sendCmd_and_recvResult(conn1,cmd)
        cmd = ["ip address add " + self.config_data['port'][1]['ip'] + " dev " + vm1interfaces[0]]
        test_utils.sendCmd_and_recvResult(conn2,cmd)
        cmd = ["ip link set dev " + vm2interfaces[0] + " up"]
        test_utils.sendCmd_and_recvResult(conn2,cmd)

        # Enable the pipeline
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Configure rules
        for table in self.config_data['table']:
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Add a bridge to ovs
        ovs_utils.add_bridge_to_ovs(self.config_data['bridge'] )
        # Bring up bridge
        ip_set_dev_up(self.config_data['bridge'])
        # Bring up TAP0
        self.tap_port_list = get_tap_port_list(self.config_data)
        ip_set_dev_up(self.tap_port_list[0])
        for i in range(2):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            # Add vlan to TAP0
            iplink_add_vlan_port(id, vlanname, self.tap_port_list[0])
            # Add vlan to the bridge
            ovs_utils.add_vlan_to_bridge(self.config_data['bridge'], vlanname)
            # Bring up vlan
            ip_set_dev_up(vlanname)

        # Send ping traffic on VMs
        port1=self.config_data['port'][1]
        if not test_utils.vm_to_vm_ping_test(conn1, port1['ip'].split('/')[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to ping from vm1 to vm2 ")

        # Hotplug remove
        if not gnmi_set_params(self.gnmicli_hotplug_delete_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to remove hotplug through gnmi")

        # Verify ports are removed on VMs
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result2 = list(dropwhile(lambda x: 'lo\r' not in x, result))
        result1 = test_utils.sendCmd_and_recvResult(conn2, vm2_command_list)[0]
        result1 = result1.split("\n")
        vm2result2 = list(dropwhile(lambda x: 'lo\r' not in x, result1))

        vm1interfaces2 = list(set(vm1result2) - set(vm1result1))
        vm1interfaces2 = [x.strip() for x in vm1interfaces2]
        if  vm1interfaces2:
            self.result.addFailure(self, sys.exc_info())
            self.fail("Fail to del hotplug through gnmi")
        print("PASS: Deleted hotplug interface from vm1 ",vm1interfaces)
        vm2interfaces2 = list(set(vm2result2) - set(vm2result1))
        vm2interfaces2 = [x.strip() for x in vm2interfaces2]
        if  vm2interfaces2:
            self.result.addFailure(self, sys.exc_info())
            self.fail("Fail to del hotplug through gnmi")
        print("PASS: Deleted hotplug interface from vm2 ",vm2interfaces)
   
        conn1.close()
        conn2.close()

        self.dataplane.kill()


    def tearDown(self):
        # delete table entries
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)

        # delete bridge
        if not ovs_utils.del_bridge_from_ovs(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete bridge {self.config_data['bridge']} from ovs")

        # delete vlan
        for i in range(2):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            if not iplink_del_port(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {vlanname}")

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")

