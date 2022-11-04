# Copyright (c) 2022 Intel Corporation.
#
#SPDX-License-Identifier: Apache-2.0
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
2VM on local host with P4OVS and 2 NS VM on remote host with Stand OVS
Full Mesh ping
"""
# in-built module imports
import time
import sys
from itertools import dropwhile

# Unittest related imports
import unittest

# ptf related imports
from ptf.base_tests import BaseTest
from ptf.testutils import *

# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
import common.utils.ovs_utils as ovs_utils
import common.utils.gnmi_cli_utils as gnmi_cli_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict,get_gnmi_params_hotplug,get_interface_ipv4_dict_hotplug
from common.lib.telnet_connection import connectionManager

class LNT_4VM_FUll_MESH(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        test_params = test_params_get()
        config_json = test_params['config_json']
        
        try:
            self.vm_cred = test_params['vm_cred']
        except KeyError:
            self.vm_cred = ""
       
        self.config_data = get_config_dict(config_json,pci_bdf=test_params['pci_bdf'],
                    vm_location_list=test_params['vm_location_list'],
                          vm_cred=self.vm_cred, client_cred=test_params['client_cred'],
                                              remote_port = test_params['remote_port']) 
        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.tap_port_list =  gnmi_cli_utils.get_tap_port_list(self.config_data)
        self.link_port_list = gnmi_cli_utils.get_link_port_list(self.config_data)
        self.conn_obj_list = []
        
    def runTest(self):
        # ------------------------------------------------- #
        # Configure P4OVS, VM, VLAN adn VXLAN on Local Host #
        # ------------------------------------------------- #
        print ("Begin to configure P4OVS and VM on local host")
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
        if not gnmi_cli_utils.gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")
    
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
           globals()["conn"+str(vm_id+1)] = connectionManager("127.0.0.1", f"655{vm_id}", vm['vm_username'], vm['vm_password'])
           self.conn_obj_list.append(globals()["conn"+str(vm_id+1)])
           globals()["vm"+str(vm_id+1)+"_command_list"] = [f"ip addr add {port['ip']} dev {port['interface']}", f"ip link set dev {port['interface']} up", f"ip link set dev {port['interface']} address {port['mac_local']}", f"ip neigh add dev {port['interface']} {vm['remote_ip']} lladdr {vm['mac_remote']}"]
           vm_cmd_list.append(globals()["vm"+str(vm_id+1)+"_command_list"])
           vm_id+=1
        
        # configuring VMs
        for i in range(len(self.conn_obj_list)):
            print ("Configuring VM{i}....")
            test_utils.configure_vm(self.conn_obj_list[i], vm_cmd_list[i])

        #bring up TAP0
        if not gnmi_cli_utils.ip_set_dev_up(self.tap_port_list[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to bring up {self.tap_port_list[0]}")
        
        #Bring up control TAP1
        if not gnmi_cli_utils.ip_set_dev_up(self.link_port_list[0]['control-port']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to bring up {self.link_port_list[0]['control-port']")
   
        if not gnmi_cli_utils.ip_add_addr(self.link_port_list[0]['control-port'],
                                                       self.config_data['vxlan']['tep_ip'][0] ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure TEP IP for {self.link_port_list[0]['control-port']}")
        
        # generate p4c artifact
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
       
        # set pipe line
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], 
                                          self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
   
        #add a bridge to ovs
        if not ovs_utils.add_bridge_to_ovs(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add bridge {self.config_data['bridge']} to ovs")
        #bring up bridge
        if not gnmi_cli_utils.ip_set_dev_up(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']}")
    
        print ("Config VXLAN Port on local host With Remote Tunnel IP")
        if not ovs_utils.add_vxlan_port_to_ovs(self.config_data['bridge'],
                self.config_data['vxlan']['vxlan_name'][0],
                    self.config_data['vxlan']['tep_ip'][0].split('/')[0], 
                        self.config_data['vxlan']['tep_ip'][1].split('/')[0],
                            self.config_data['vxlan']['dst_port'][0]):
            
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add vxlan {self.config_data['vxlan']['vxlan_name'][0]} to bridge {self.config_data['bridge']}")
      
        for i in range(len(self.conn_obj_list)):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            #add vlan to TAP0, e.g. ip link add link TAP0 name vlan1 type vlan id 1
            if not gnmi_cli_utils.iplink_add_vlan_port(id, vlanname, self.tap_port_list[0]):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add vlan {vlanname} to {self.tap_port_list[0]}")

            #add vlan to the bridge, e.g. ovs-vsctl add-port br-int vlan1
            if not ovs_utils.add_vlan_to_bridge(self.config_data['bridge'], vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add vlan {vlanname} to {self.config_data['bridge']}")

            #bring up vlan 
            if not gnmi_cli_utils.ip_set_dev_up(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to bring up {vlanname}")
        
        # add linux networking match action rules
        for table in self.config_data['table']:
            print(f"Scenario : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        #-------------------------------------------------------------#
        # Configure standard OVS, Name Space and VXLAN on Remote Host #
        #-------------------------------------------------------------#
        print (f"Begin to configure standard OVS on remote host {self.config_data['client_hostname']}")
        if not ovs_utils.add_bridge_to_ovs(self.config_data['bridge'], remote=True,
                hostname=self.config_data['client_hostname'],
                        username=self.config_data['client_username'],
                                passwd=self.config_data['client_password']):

            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add bridge {self.config_data['bridge']} to \
                     ovs {self.config_data['bridge']} on {self.config_data['client_hostname']}" )
 
        # bring up the bridge
        if not gnmi_cli_utils.ip_set_dev_up(self.config_data['bridge'],remote=True,
                      hostname=self.config_data['client_hostname'],
                             username=self.config_data['client_username'],
                                    password=self.config_data['client_password']):

            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']}")
        
        # assign IP to the bridge
        if not gnmi_cli_utils.ip_add_addr(self.config_data['bridge'],self.config_data['vxlan']['tep_ip'][1], 
                    remote=True, hostname=self.config_data['client_hostname'],
                        username=self.config_data['client_username'],
                                passwd=self.config_data['client_password']):

            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']} on {self.config_data['client_hostname']}")
        
        # create ip netns
        for namespace in self.config_data['net_namespace']: 
            print (f"creating namespace {namespace['name']} on {self.config_data['client_hostname']}")
            if not test_utils.create_ipnetns_vm(namespace, remote=True,
                hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                    password=self.config_data['client_password']):

                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add VM namesapce {namespace['name']} on on {self.config_data['client_hostname']}")
            
            if not ovs_utils.add_port_to_ovs(self.config_data['bridge'], namespace['peer_name'],
                    remote=True, hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                                   password =self.config_data['client_password']):
            
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add port {namespace['peer_name']} to bridge {self.config_data['bridge']}")
            
        print (f"Configure vxlan port on remote host on {self.config_data['client_hostname']}")
        if not ovs_utils.add_vxlan_port_to_ovs(self.config_data['bridge'],
                self.config_data['vxlan']['vxlan_name'][0],
                    self.config_data['vxlan']['tep_ip'][1].split('/')[0], 
                        self.config_data['vxlan']['tep_ip'][0].split('/')[0],
                            self.config_data['vxlan']['dst_port'][0],remote=True, 
                                    hostname=self.config_data['client_hostname'],
                                        username=self.config_data['client_username'],
                                             password=self.config_data['client_password']):

            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add vxlan {self.config_data['vxlan']['vxlan_name'][0]} to \
                         bridge {self.config_data['bridge']} on on {self.config_data['client_hostname']}")

        print (f"bring up {self.config_data['remote_port'][0]} and add it to bridge {self.config_data['bridge']}")
        if not gnmi_cli_utils.ip_set_dev_up(self.config_data['remote_port'][0],remote=True, 
                        hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                    password=self.config_data['client_password']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['remote_port'][0]} on {self.config_data['client_hostname']}")

        if not ovs_utils.add_port_to_ovs(self.config_data['bridge'], self.config_data['remote_port'][0],
                    remote=True, hostname=self.config_data['client_hostname'],
                        username=self.config_data['client_username'],
                                password=self.config_data['client_password']):

            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add physical port {self.config_data['remote_port'][0]} to \
                              bridge {self.config_data['bridge']} on {self.config_data['client_hostname']}")

        # --------------------------------- #   
        #   Mesh Ping Test                  #
        # ----------------------------------#
        time.sleep(10)
        print ("Ping test executed from VM on remote host")
        for namespace in self.config_data['net_namespace']: 
            for ip in namespace['remote_ping']:
                test_utils.ip_ntns_exec_ping_test(namespace['name'], ip, remote=True,
                    hostname=self.config_data['client_hostname'],
                                username=self.config_data['client_username'],
                                        password=self.config_data['client_password'])
    
        print ("Ping test executed from VM on local host")
        for i in range(len(self.conn_obj_list)):
            print(f"Ping test from VM{i} to other VMs")
            for ip in self.config_data['vm'][i]['remote_ip']:
                if not test_utils.vm_to_vm_ping_test(self.conn_obj_list[i], ip):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"FAIL: Ping test failed for VM{i}")

        print ("close VM telnet session")
        for conn in self.conn_obj_list:
            conn.close()

    def tearDown(self):
        print ("Begin to teardown ...")
        print ("Delete p4ovs match action rules on local host")
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)
    
        print ("Delete vlan on local host")
        for i in range(len(self.conn_obj_list)):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            if not gnmi_cli_utils.iplink_del_port(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {vlanname}")

        print ("Delete ip netns on remote host")
        for namespace in self.config_data['net_namespace']:
            if not test_utils.del_ipnetns_vm(namespace,remote=True,
                hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                    password=self.config_data['client_password']):

                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete VM namesapce {namespace['name']} on {self.config_data['client_hostname']}")

        #remove local bridge
        if not ovs_utils.del_bridge_from_ovs(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete bridge {self.config_data['bridge']} from ovs")
        
        #remote remote bridge
        if not ovs_utils.del_bridge_from_ovs(self.config_data['bridge'],
               remote=True,
                hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                    passwd=self.config_data['client_password']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete bridge {self.config_data['bridge']} on {self.config_data['client_hostname']}")
    
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
