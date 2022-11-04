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
DPDK Connection Tracking with vhost Port for Netperf
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

class Connection_Track(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True # for verify_packets to ignore other packets received at the interface
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)
        self.config_data = get_config_dict(config_json, vm_location_list=test_params['vm_location_list'])
        self.gnmicli_params = get_gnmi_params_simple(self.config_data)


    def runTest(self):
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
        
        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")


        # get port list and add to dataplane
        port_list = self.config_data['port_list']
        port_ids = test_utils.add_port_to_dataplane(port_list)
        
        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)
        
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")


        table = self.config_data['table'][0]
        print(f"Rule Creation : {table['description']}")
        print(f"Adding {table['description']} rules")
        for match_action in table['match_action']:
            if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add table entry {match_action}")

        table = self.config_data['table'][1]
        print(f"Rule Creation : {table['description']}")
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
           globals()["vm"+str(vm_id+1)+"_command_list"] = [f"ip addr add {port['ip_address']} dev {port['interface']}", f"ip link set dev {port['interface']} up", f"ip link set dev {port['interface']} address {port['mac_local']}", f"ip route add {port['ip_route']} via {port['ip_add_route']} dev {port['interface']}",f"ip neigh add dev {port['interface']}  {vm['remote_ip']} lladdr {vm['mac_remote']}"]
           vm_cmd_list.append(globals()["vm"+str(vm_id+1)+"_command_list"])
           vm_id+=1
          

        # configuring VMs
        for i in range(len(self.conn_obj_list)):
            print (f"Configuring VM{i}....")
            test_utils.configure_vm(self.conn_obj_list[i], vm_cmd_list[i])
            print (f"execute ethtool {self.config_data['port'][i]['interface']} offload on VM{i}")
            if not test_utils.vm_ethtool_offload(self.conn_obj_list[i],self.config_data['port'][i]['interface'] ):
                  self.result.addFailure(self, sys.exc_info())
                  self.fail(f"FAIL: failed to set ethtool offload {self.config_data['port'][i]['interface']} on VM{i}")

        # Netperf and netserver for local host
        time.sleep(30)
        for i in range(len(self.conn_obj_list)):
            if not test_utils.vm_check_netperf(self.conn_obj_list[i], f"VM{i}"):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: netperf is not install on VM{i}")
        i=0
        print (f"Start netserver on VM{i}")
        if not test_utils.vm_start_netserver(self.conn_obj_list[i]):
              self.result.addFailure(self, sys.exc_info())
              self.fail(f"FAIL: failed to start netserver on VM{i}")
        print (f"netserver started on VM{i}")      
        
        #send netperf from local VM
        i=1
        print(f"Initially execute netperf on VM{i}")
        if not test_utils.vm_netperf_client(self.conn_obj_list[i], self.config_data['vm'][i]['remote_ip'],
                self.config_data['netperf']['testlen'], self.config_data['netperf']['testname'], option = self.config_data['netperf']['cmd_option']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: failed to start netperf")

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
        

 

