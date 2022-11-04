# Copyright (c) 2022 Intel Corporation.
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
LNT ECMP 2 local Hot Plug VM and 2 remote name space VM with netperf traffic
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
from ptf import config

# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.ovs_utils as ovs_utils
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_gnmi_params_hotplug, get_interface_ipv4_dict, get_interface_ipv4_dict_hotplug, get_interface_mac_dict_hotplug, get_interface_ipv4_route_dict_hotplug, create_port_vm_map
import common.utils.gnmi_cli_utils as gnmi_cli_utils
from common.lib.telnet_connection import connectionManager

class LNT_ECMP_2HotPlugVM_2Host_Netperf(BaseTest):

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
      
        self.config_data = get_config_dict(config_json,pci_bdf=test_params['pci_bdf'],
            vm_location_list=test_params['vm_location_list'],
                vm_cred=self.vm_cred, client_cred=test_params['client_cred'],
                                              remote_port = test_params['remote_port']) 
        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.gnmicli_hotplug_params = get_gnmi_params_hotplug(self.config_data)
        self.gnmicli_hotplug_delete_params = get_gnmi_params_hotplug(self.config_data,action="del")
        self.tap_port_list =  gnmi_cli_utils.get_tap_port_list(self.config_data)
        self.link_port_list = gnmi_cli_utils.get_link_port_list(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)
        self.conn_obj_list = []
         
    def runTest(self):
        
        print ("Begin to configure P4OVS and VM on local host")
        result, vm_name = test_utils.vm_create_with_hotplug(self.config_data)
        print(result)
        print(vm_name)
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to create {vm_name}")

        print("Sleeping for 30 seconds for the vms to come up")
        time.sleep(30)
        
        vm=self.config_data['vm'][0]
        conn1 = connectionManager(vm['hotplug']['qemu-socket-ip'],vm['hotplug']['serial-telnet-port'],
                                vm['vm_username'], password=vm['vm_password'], timeout=self.config_data['netperf']['testlen']+5)
        self.conn_obj_list.append(conn1)

        vm1=self.config_data['vm'][1]
        conn2 = connectionManager(vm1['hotplug']['qemu-socket-ip'],vm1['hotplug']['serial-telnet-port'],
                                vm1['vm_username'], password=vm1['vm_password'], timeout=self.config_data['netperf']['testlen']+5)
        self.conn_obj_list.append(conn2)

        vm1_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn1, vm1_command_list)[0]
        result = result.split("\n")
        vm1result1 = list(dropwhile(lambda x: 'lo\r' not in x, result))
      
        vm2_command_list = ["ip a | egrep \"[0-9]*: \" | cut -d ':' -f 2"]
        result = test_utils.sendCmd_and_recvResult(conn2, vm2_command_list)[0]
        result = result.split("\n")
        vm2result1 = list(dropwhile(lambda x: 'lo\r' not in x, result))

        if not gnmi_cli_utils.gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        if not gnmi_cli_utils.gnmi_cli_set_and_verify(self.gnmicli_hotplug_params):
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
       
        time.sleep(10)
        # bring up and config VM
        for i in range(len(self.interface_ip_list_hotplug)):
            print (f" bring up and configure VM{i} interface")
            if not test_utils.vm_interface_up(self.conn_obj_list[i], [self.interface_ip_list_hotplug[i]]):
                self.result.addFailure(self, sys.exc_info())
                self.fail("Failed to bring up {self.interface_ip_list_hotplug[i]}")
        
            if not test_utils.vm_interface_configuration(self.conn_obj_list[i], [self.interface_ip_list_hotplug[i]]):
                self.result.addFailure(self, sys.exc_info())
                self.fail("Failed to configure {self.interface_ip_list_hotplug[i]}")
                                    
        # Bring up TAP ports 
        if not gnmi_cli_utils.ip_set_dev_up(self.tap_port_list[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to bring up {self.tap_port_list[0]}")

        # configure IP on TEP and TAP ports
        if not gnmi_cli_utils.iplink_add_dev(self.config_data['vxlan']['tep_intf'], "dummy"):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add dev {self.config_data['vxlan']['tep_intf']} type dummy")
        gnmi_cli_utils.ip_set_ipv4([{self.config_data['vxlan']['tep_intf']: self.config_data['vxlan']['tep_ip'][0]}])
        ecmp_local_ports = {}
        for i in range(len(self.config_data['ecmp']['local_ports'])):
            ecmp_local_ports[self.config_data['ecmp']['local_ports'][i]] =  self.config_data['ecmp']['local_ports_ip'][i]
        gnmi_cli_utils.ip_set_ipv4([ecmp_local_ports])
        
        # generate p4c artifact
        if not test_utils.gen_dep_files_p4c_dpdk_pna_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")
       
        # Set pipe line
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], 
                                          self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Create bridge
        if not ovs_utils.add_bridge_to_ovs(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add bridge {self.config_data['bridge']} to ovs")
        # Bring up bridge
        if not gnmi_cli_utils.ip_set_dev_up(self.config_data['bridge']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']}")

        print (f"Configure VXLAN ")
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

        # Program rules
        print (f"Program rules")
        for table in self.config_data['table']:
            print(f"Scenario : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Remote host configuration Start 
        print (f"\nConfigure standard OVS on remote host {self.config_data['client_hostname']}")
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

        # create ip netns VMs
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

        print (f"Add device TEP1")
        if not gnmi_cli_utils.iplink_add_dev("TEP1", "dummy",remote=True,
           hostname=self.config_data['client_hostname'],username=self.config_data['client_username'],
           password=self.config_data['client_password']):
             self.result.addFailure(self, sys.exc_info())
             self.fail(f"Failed to add TEP1")

        print (f"Bring up remote ports")
        remote_port_list = ["TEP1"] + self.config_data['remote_port']
        remote_port_ip_list = [self.config_data['vxlan']['tep_ip'][1]] + self.config_data['ecmp']['remote_ports_ip']
        for remote_port,remote_port_ip in zip(remote_port_list,remote_port_ip_list):
            if not gnmi_cli_utils.ip_set_dev_up(remote_port,remote=True, 
                        hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                    password=self.config_data['client_password']):
               self.result.addFailure(self, sys.exc_info())
               self.fail(f"Failed to bring up {remote_port} on {self.config_data['client_hostname']}")
            if not gnmi_cli_utils.ip_add_addr(remote_port,remote_port_ip,remote=True,
              hostname=self.config_data['client_hostname'],username=self.config_data['client_username'],
              passwd=self.config_data['client_password']):
               self.result.addFailure(self, sys.exc_info())
               self.fail("Failed to configure IP {remote_port_ip} for {remote_port}")
        # Remote host configuration End
        dst = self.config_data['vxlan']['tep_ip'][0].split('/')[0]
        nexthop_list,device_list,weight_list = [],[],[]
        for i in self.config_data['ecmp']['local_ports_ip']:
            nexthop_list.append(i.split('/')[0])
            weight_list.append(1)
        for i in self.config_data['remote_port']:
            device_list.append(i.split('/')[0])
        if not gnmi_cli_utils.iproute_add(dst, nexthop_list, device_list, weight_list, remote=True,
            hostname=self.config_data['client_hostname'], username=self.config_data['client_username'],
            password=self.config_data['client_password']):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add route")
                
        # configure static routes for underlay 
        dst = self.config_data['vxlan']['tep_ip'][1].split('/')[0]
        nexthop_list,device_list,weight_list = [],[],[]
        for i in self.config_data['ecmp']['remote_ports_ip']:
            nexthop_list.append(i.split('/')[0])
            weight_list.append(1)
        for i in self.config_data['ecmp']['local_ports']:
            device_list.append(i.split('/')[0])
        if not gnmi_cli_utils.iproute_add(dst, nexthop_list, device_list, weight_list):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add route")
        
        print("prepare netserver on local VM")
        for i in range(len(self.conn_obj_list)):
            print (f"execute ethtool {self.config_data['port'][i]['interface']} offload on VM{i}")
            if not test_utils.vm_ethtool_offload(self.conn_obj_list[i],self.config_data['port'][i]['interface'] ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: failed to set ethtool offload {self.config_data['port'][i]['interface']} on VM{i}")
                
            print (f"execute change {self.config_data['port'][i]['interface']} mtu on VM{i}")
            if not test_utils.vm_change_mtu(self.conn_obj_list[i], 
                                      self.config_data['port'][i]['interface'], self.config_data['netperf']['mtu']):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: failed change mtu for {self.config_data['port'][i]['interface']} on VM{i}")
            
            if not test_utils.vm_check_netperf(self.conn_obj_list[i], f"VM{i}"):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: netperf is not install on VM{i}")
                 
            print (f"Start netserver on VM{i}")
            if not test_utils.vm_start_netserver(self.conn_obj_list[i]):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: failed to start netserver on VM{i}")
                
        print("prepare netserver on remote name sapce")
        if not test_utils.host_check_netperf(remote=True,
                    hostname=self.config_data['client_hostname'], username=self.config_data['client_username'], 
                                                                       password=self.config_data['client_password'] ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: nertperf is not installed on {namespace['name']}")
        for namespace in self.config_data['net_namespace']:
            if not test_utils.ipnetns_eth_offload(namespace['name'], namespace['veth_if'], 
                        remote=True, hostname=self.config_data['client_hostname'],
                               username=self.config_data['client_username'], password=self.config_data['client_password'] ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: failed to set ethtool offload for {namespace['veth_if']} on {namespace['name']}")
            
            if not test_utils.ipnetns_change_mtu(namespace['name'], namespace['veth_if'],mtu=1450, 
                    remote=True,hostname=self.config_data['client_hostname'], 
                         username=self.config_data['client_username'], password=self.config_data['client_password']):
                
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: failed change mtu for {namespace['veth_if']} on {namespace['name']}")
            
            print (f"start netserver on {namespace['name']} on {self.config_data['client_hostname']}")
            if not test_utils.ipnetns_netserver(namespace['name'], remote=True, 
                            hostname=self.config_data['client_hostname'],
                                 username=self.config_data['client_username'], password=self.config_data['client_password'] ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: failed to netserver on {namespace['name']}")
                
        print("\nSleep before sending netperf traffic")
        time.sleep(20) 
        #send netperf from local VM 
        for i in range(len(self.conn_obj_list)):
            for testname in self.config_data['netperf']['testname']:
                print(f"execute netperf {testname} on VM{i}")
                for ip in self.config_data['vm'][i]['remote_ip']:
                    print(f"execute netperf -H {ip} -l {self.config_data['netperf']['testlen']} -t {testname} " +
                          f"{self.config_data['netperf']['cmd_option']} on VM{i}")
                    if not test_utils.vm_netperf_client(self.conn_obj_list[i], ip, 
                            self.config_data['netperf']['testlen'], testname,option = self.config_data['netperf']['cmd_option']):
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"FAIL: failed to get netperf data on VM{i}")
                    time.sleep(10)
                     
        # send netperf from remote name space VM
        for namespace in self.config_data['net_namespace']:                      
            for testname in self.config_data['netperf']['testname']:
                print(f"execute netperf {testname} on net namespace {namespace['name']}")
                for ip in namespace['remote_ip']:
                    if not test_utils.ipnetns_netperf_client(namespace['name'], ip, self.config_data['netperf']['testlen'], 
                        testname, remote=True, option = self.config_data['netperf']['cmd_option'], 
                            hostname=self.config_data['client_hostname'], username=self.config_data['client_username'], 
                                                                                  password=self.config_data['client_password'] ):
                        
                        self.result.addFailure(self, sys.exc_info())
                        self.fail(f"FAIL: failed to get netperf on name space {namespace['name']}")
    
        #Verify if the traffic is load balanced
        send_port_id= self.config_data['traffic']['send_port'][0]
        #Record port counter before sending traffic
        send_count_list_before = []
        for send_port_id in self.config_data['traffic']['send_port']:
            send_cont = gnmi_cli_utils.gnmi_get_params_counter(self.gnmicli_params[send_port_id])
            if not send_cont:
               self.result.addFailure(self, sys.exc_info())
               print (f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")
            send_count_list_before.append(send_cont)
            
        #Send ping traffic across ecmp links from VM
        print("Send netperf traffic to verify load balancing")
        netperf_rslt = test_utils.vm_netperf_client(self.conn_obj_list[0], self.config_data['vm'][0]['remote_ip'][1],
                self.config_data['netperf']['testlen'], self.config_data['netperf']['testname'][0],
                                                           option = self.config_data['netperf']['cmd_option'])
        if not netperf_rslt:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: failed to start netserver on VM0")
        
        #Record port counter after sending traffic
        send_count_list_after = []
        for send_port_id in self.config_data['traffic']['send_port']:
            send_cont = gnmi_cli_utils.gnmi_get_params_counter(self.gnmicli_params[send_port_id])
            if not send_cont:
               self.result.addFailure(self, sys.exc_info())
               print(f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}")
            send_count_list_after.append(send_cont)
        #check if icmp pkts are forwarded on both ecmp links
        counter_type = "out-unicast-pkts"
        stat_total = 0
        for send_count_before,send_count_after in zip(send_count_list_before,send_count_list_after):
            stat = test_utils.compare_counter(send_count_after,send_count_before)
            if not stat[counter_type] > 0:
                print(f"FAIL: Packets are not forwarded on one of the ecmp links")
                self.result.addFailure(self, sys.exc_info())
            stat_total = stat_total + stat[counter_type]
            
        #calculate netperf packet
        num =  netperf_rslt['send_sckt_byte']/netperf_rslt['sned_msg_byte']
        
        if stat_total >= num:
            print(f"PASS: Minimum {num} packets expected and {stat_total} received")
        else:
            print(f"FAIL: {num} packets expected but {stat_total} received")
            self.result.addFailure(self, sys.exc_info())
                
        print ("close VM telnet session")
        for conn in self.conn_obj_list:
            conn.close()
            
    def tearDown(self):

        print("\nUnconfiguration on local host")
        print (f"Delete p4ovs match action rules on local host")
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action)

        print (f"Delete vlan on local host")
        for i in range(len(self.conn_obj_list)):
            id = self.config_data['port'][i]['vlan']
            vlanname = "vlan"+id
            if not gnmi_cli_utils.iplink_del_port(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {vlanname}")

        print (f"Delete TEP interface")
        if not gnmi_cli_utils.iplink_del_port(self.config_data['vxlan']['tep_intf']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete {self.config_data['vxlan']['tep_intf']}")

        print (f"Delete ip route")
        if not gnmi_cli_utils.iproute_del(self.config_data['vxlan']['tep_ip'][1].split('/')[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete route to {self.config_data['vxlan']['tep_ip'][1].split('/')[0]}")

        print("\nUnconfiguration on remote host")
        print (f"Delete ip netns on remote host")
        for namespace in self.config_data['net_namespace']:
            # delete remote veth_host_vm port
            if not gnmi_cli_utils.iplink_del_port(namespace['peer_name'],remote=True,
                hostname=self.config_data['client_hostname'],
                        username=self.config_data['client_username'],
                                     passwd=self.config_data['client_password']):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {namespace['peer_name']}")
                
            #delete name space
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

        #remote bridge
        if not ovs_utils.del_bridge_from_ovs(self.config_data['bridge'],
               remote=True,
                hostname=self.config_data['client_hostname'],
                            username=self.config_data['client_username'],
                                    passwd=self.config_data['client_password']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete bridge {self.config_data['bridge']} on {self.config_data['client_hostname']}")
        
        # delete tep
        if not gnmi_cli_utils.iplink_del_port("TEP1",remote=True,hostname=self.config_data['client_hostname'],
           username=self.config_data['client_username'],passwd=self.config_data['client_password']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete TEP1")

        # Delete route
        if not gnmi_cli_utils.iproute_del(self.config_data['vxlan']['tep_ip'][0].split('/')[0],remote=True,
          hostname=self.config_data['client_hostname'],username=self.config_data['client_username'],
          password=self.config_data['client_password']):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete route to {self.config_data['vxlan']['tep_ip'][0].split('/')[0]}")

        # Delete remote host Ip
        remote_port_list = self.config_data['remote_port']
        remote_port_ip_list = self.config_data['ecmp']['remote_ports_ip']
        for remote_port,remote_port_ip in zip(remote_port_list,remote_port_ip_list):
            if not gnmi_cli_utils.ip_del_addr(remote_port,remote_port_ip,remote=True,
              hostname=self.config_data['client_hostname'],username=self.config_data['client_username'],
              passwd=self.config_data['client_password']):
               self.result.addFailure(self, sys.exc_info())
               self.fail(f"Failed to delete ip {remote_port_ip} on {remote_port}")

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
