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
from common.lib.telnet_connection import connectionManager
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4


class L3_Action_Profile_Vhost(BaseTest):

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

        self.PASSED = True

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

        
        #create VMs
        result, vm_name = test_utils.vm_create(self.config_data['vm_location_list'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"VM creation failed for {vm_name}")

        # create telnet instance for VMs created
        time.sleep(30)
        vm_id = 0
        for vm, port in zip(self.config_data['vm'], self.config_data['port']):
           globals()["conn"+str(vm_id+1)] = connectionManager("127.0.0.1", f"655{vm_id}", vm['vm_username'], vm['vm_password'], timeout=30)
           globals()["vm"+str(vm_id+1)+"_command_list"] = [f"ip addr add {port['ip']} dev {port['interface']}", f"ip link set dev {port['interface']} up", f"ip link set dev {port['interface']} address {port['mac']}" , f"ip route add 0.0.0.0/0 via {vm['dst_gw']} dev {port['interface']}"]
           vm_id+=1


        table = self.config_data['table'][0]
        
        print(f"##########  Scenario : {table['description']} ##########")

        function_dict = {
                'table_for_configure_member' : ovs_p4ctl.ovs_p4ctl_add_member_and_verify,
                'table_for_ipv4' : ovs_p4ctl.ovs_p4ctl_add_entry
                }
        table_entry_dict = {
                'table_for_configure_member' : 'member_details',
                'table_for_ipv4' : 'match_action'
                }

        for table in self.config_data['table']:
            print(f"Scenario : l3 verify traffic with action profile : {table['description']}")
            print(f"Adding {table['description']} rules")
            for match_action in table[table_entry_dict[table['description']]]:
                function_dict[table['description']](table['switch'],table['name'], match_action)

        time.sleep(100)
        # Configuring VMs
        print("Configuring VM0 ....")
        test_utils.configure_vm(conn1, vm1_command_list)

        print("Configuring VM1 ....")
        test_utils.configure_vm(conn2, vm2_command_list)

        # Verify Unicast traffic from VM0 to VM1
        print("Verify Unicast traffic from VM0 to VM1")
        for src in self.config_data['traffic']['in_pkt_header']['ip_src']:
            dst_ip=self.config_data['traffic']['in_pkt_header']['ip_dst'][0]
            print("Sending Unicast Traffic")
            try:
                pkt = test_utils.vm_to_vm_ping_test(conn1, dst_ip)
            except Exception as err:
                print(f"FAIL: Verification of packets sent failed with exception {err}")
                self.PASSED = False

        #verify Multicast Traffic
        print("Verify Multicast Traffic")
        sender_vm_id = self.config_data['traffic']['send_port'][1]
        receiver_vm_id= self.config_data['traffic']['receive_port'][1]
        sender_conn, receiver_conn = eval("conn"+str(sender_vm_id+1)), eval("conn"+str(receiver_vm_id+1))
        
        send_result = test_utils.send_scapy_traffic_from_vm(sender_vm_id, sender_conn, receiver_conn, self.config_data,'multicast')
        if send_result:
            result = test_utils.verify_scapy_traffic_from_vm(receiver_vm_id,conn2,self.config_data,'multicast')
            if not result:
                print(f"FAIL: Multicast Traffic not received on receiver_vm VM{receiver_vm_id}")
                self.result.addFailure(self, sys.exc_info())
            else:
                print(f"PASS: Multicast Traffic received on receiver_vm VM{receiver_vm_id}")
        else:
            print(f"FAIL: Multicast Packets not sent from sender_vm VM{sender_vm_id}")
            self.result.addFailure(self, sys.exc_info())

        print("Sleeping for 3 seconds")
        time.sleep(3)
        # Verify Broadcast Traffic
        print("Verify Broadcast Traffic")
        sender_vm_id = self.config_data['traffic']['send_port'][2]
        receiver_vm_id= self.config_data['traffic']['receive_port'][2]
        sender_conn, receiver_conn = eval("conn"+str(sender_vm_id+1)), eval("conn"+str(receiver_vm_id+1))
        send_result = test_utils.send_scapy_traffic_from_vm(sender_vm_id, sender_conn, receiver_conn, self.config_data,'broadcast')
        if send_result:
            result = test_utils.verify_scapy_traffic_from_vm(receiver_vm_id,receiver_conn,self.config_data,'broadcast')
            if not result:
                self.result.addFailure(self, sys.exc_info())
                print(f"FAIL: Broadcast Traffic not received on receiver_vm VM{receiver_vm_id} ")
                self.result.addFailure(self, sys.exc_info())
            else:
                print(f"PASS: Broadcast Traffic Received on receiver_vm VM{receiver_vm_id}")
        else:
            print(f"FAIL: Broadcast Packets not sent from sender_vm VM{sender_vm_id}")
            self.result.addFailure(self, sys.exc_info())

        conn1.close()
        conn2.close()

    def tearDown(self):

        table = self.config_data['table'][0]
        print("Deleting members")
        for del_member in table['del_member']:
            ovs_p4ctl.ovs_p4ctl_del_member(table['switch'],table['name'],del_member)

        table = self.config_data['table'][1]
        print(f"Deleting rules")
        for del_action in table['del_action']:
            ovs_p4ctl.ovs_p4ctl_del_entry(table['switch'], table['name'], del_action.split(",")[0])

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")

 
