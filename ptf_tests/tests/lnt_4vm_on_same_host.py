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
LNT 4VM Mesh ping on same Host
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

# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
import common.utils.ovs_utils as ovs_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params,iplink_add_vlan_port,ip_set_dev_up, get_tap_port_list,iplink_del_port
from common.lib.telnet_connection import connectionManager

class LNT_4VM_Same_Host(BaseTest):

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
        self.tap_port_list = get_tap_port_list(self.config_data)
    
    def runTest(self):
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")
    
        print ("Start to crate VM")
        result, vm_name = test_utils.vm_create(self.config_data['vm_location_list'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"VM creation failed for {vm_name}")
       
        print ("Initiate telnet instance for each VM created")
        self.conn_obj_list = []
        vm_cmd_list = []
        vm_id = 0
        for vm, port in zip(self.config_data['vm'], self.config_data['port']):
           globals()["conn"+str(vm_id+1)] = connectionManager("127.0.0.1", f"655{vm_id}", vm['vm_username'], vm['vm_password'])
           self.conn_obj_list.append(globals()["conn"+str(vm_id+1)])
           globals()["vm"+str(vm_id+1)+"_command_list"] = [f"ip addr add {port['ip_address']} dev {port['interface']}", f"ip link set dev {port['interface']} up", f"ip link set dev {port['interface']} address {port['mac_local']}", f"ip neigh add dev {port['interface']} {vm['remote_ip']} lladdr {vm['mac_remote']}"]
           vm_cmd_list.append(globals()["vm"+str(vm_id+1)+"_command_list"])
           vm_id+=1
        
        # configuring VMs
        for i in range(len(self.conn_obj_list)):
            print (f"Configuring VM{i}....")
            test_utils.configure_vm(self.conn_obj_list[i], vm_cmd_list[i])
            
        #bring up TAP0
        if not ip_set_dev_up(self.tap_port_list[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to bring up {self.tap_port_list[0]}")

        # set pipe line
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
        
        # add linux networking match action rules
        for table in self.config_data['table']:
            print(f"Scenario : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")
                    
        #add a bridge to ovs
        if not ovs_utils.add_bridge_to_ovs(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add bridge {self.config_data['bridge']} to ovs")
        #bring up bridge
        if not ip_set_dev_up(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']}")

        for i in range(len(self.conn_obj_list)):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            #add vlan to TAP0
            if not iplink_add_vlan_port(id, vlanname, self.tap_port_list[0]):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add vlan {vlanname} to {self.tap_port_list[0]}")

            #add vlan to the bridge
            if not ovs_utils.add_vlan_to_bridge(self.config_data['bridge'], vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add vlan {vlanname} to {self.config_data['bridge']}")

            #bring up vlan 
            if not ip_set_dev_up(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to bring up {vlanname}")
        
        # ping test between VMs
        for i in range(len(self.conn_obj_list)):
            print(f"Ping test from VM{i} to other VMs")
            for ip in self.config_data['vm'][i]['remote_ip']:
                result = test_utils.vm_to_vm_ping_test(self.conn_obj_list[i], ip)
                if not result:
                    self.result.addFailure(self, sys.exc_info())
                    print("FAIL: Ping test failed for VM0")
    
        # close telnet connections
        for conn in self.conn_obj_list:
            conn.close()
        
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
        for i in range(len(self.conn_obj_list)):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            if not iplink_del_port(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {vlanname}")

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
