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
DPDK port flapping test with vHost ports
"""
import unittest

# ptf related import
from ptf import config  # importing global dict where all config data is present
from ptf import testutils
from ptf.base_tests import BaseTest
# scapy related imports
from scapy.all import *
# framework related imports
import common.utils.config_file_utils as config_file_utils
import common.utils.gnmi_cli_utils as gnmi_cli_utils
import common.utils.ovsp4ctl_utils as ovsp4ctl_utils
import common.utils.test_utils as test_utils
import common.lib.telnet_connection as telnet_connection


class DpdkPortFlappingVhost(BaseTest):
    """L3 DPDK port flapping test with vHost ports
    to write new testcase, we need to have parent class as BaseTest
    which in turn uses unittest.TestCase, used to create new test cases
    """

    def setUp(self):
        """ It brings up test environment needed to run test
        It performs below operations:
        1. Enables logging from "Dataplane.BaseTest" class
        2. Get instance to store results of set of tests using
           unittest.TestResult() class
        3. get all test_params from config dictionary
        4. get json config file, VM credentials, vm location list
        5. Prepare list of gnmi_cli cmds needed to create vHost ports
        :return: None
        :rtype: None
        """
        # Enables logging
        BaseTest.setUp(self)
        # Store results of set of tests
        self.result = unittest.TestResult()
        # for verify_packets to ignore other packets received at the interface
        config["relax"] = True

        # get test_params from config dictionary
        test_params = testutils.test_params_get()
        # get json config file, VM credentials, vm location list
        config_json = test_params['config_json']  # converted to dict format
        try:
            self.vm_cred = test_params['vm_cred']
        except KeyError:
            self.vm_cred = ""
        vm_location_list = test_params['vm_location_list']
        self.config_data = config_file_utils.get_config_dict(config_json,
                                                             "",
                                                             vm_location_list,
                                                             self.vm_cred)
        # Prepare list of gnmi_cli cmds needed to create vHost ports
        self.gnmicli_params_list = config_file_utils.get_gnmi_params_simple(
            self.config_data)

    def runTest(self):
        """ It performs test execution. below steps executed:
        1. Generate deps files and pb.bin file
        2.set vHost interfaces using gnmi_cli set
        3. Set forwarding pipeline
        4. Create VMs
        5. Get telnet instance for created VMs and login
        6. Configure VMs
        7. Add rules for table L3 Exact Match
        8. Ping test between two VMs
        9. Port flapping test
        :return: None
        :rtype: None
        """
        # Generate deps files and pb.bin file
        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(
                self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # set vHost interfaces using gnmi_cli set
        if not gnmi_cli_utils.gnmi_cli_set_and_verify(self.gnmicli_params_list):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")

        # Set forwarding pipeline
        if not ovsp4ctl_utils.ovs_p4ctl_set_pipe(self.config_data['switch'],
                                                 self.config_data['pb_bin'],
                                                 self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Create VMs
        result, vm_name = test_utils.vm_create(
            self.config_data['vm_location_list'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f'Failed to create VM: {vm_name}')

        # Get telnet instance for created VMs
        vm_id = 0
        for vm, port in zip(self.config_data['vm'], self.config_data['port']):
            globals()[
                "conn" + str(vm_id + 1)] = telnet_connection.connectionManager(
                "127.0.0.1", f"655{vm_id}", vm['vm_username'],
                vm['vm_password'])
            globals()["vm" + str(vm_id + 1) + "_command_list"] = [
                f"ip addr add {port['ip_address']} dev {port['interface']}",
                f"ip link set dev {port['interface']} up",
                f"ip link set dev {port['interface']} address {port['mac_local']}",
                f"ip route add {vm['dst_nw']} via {vm['dst_gw']} dev {port['interface']}",
                f"ip neigh add dev {port['interface']} {vm['remote_ip']} lladdr {vm['mac_remote']}"]
            vm_id += 1

        # Configure VMs
        test_utils.configure_vm(conn1, vm1_command_list)
        test_utils.configure_vm(conn2, vm2_command_list)

        # Add rules for table L3 Exact Match
        for table in self.config_data['table']:
            print(f"Scenario : l3 dpdk port flapping with vHost ports: "
                  f"{table['description']}")

            # Add rules for table into forwarding pipe
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovsp4ctl_utils.ovs_p4ctl_add_entry(table['switch'],
                                                          table['name'],
                                                          match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Ping test between two VMs
        print(f'Ping test from VM1 to VM2...')
        result = test_utils.vm_to_vm_ping_test(conn1, self.config_data['vm'][
            0]['remote_ip'])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            print("FAIL: Ping test failed for VM1")
        # Port flapping test
        print('Test VM2 port flapping while traffic from VM1 -> VM2 running')
        result = test_utils.vm_port_flapping(conn1, self.config_data,
                                             self.result)
        if not result:
            self.result.addFailure(self, sys.exc_info())
            print("FAIL: L3 DPDK port flapping with vHost port failed")

        # close telnet connections
        conn1.close()
        conn2.close()

    def tearDown(self):
        """It removes p4ovs pipeline table entries, rules
        :return: None
        :rtype: None
        """
        # delete table entries
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovsp4ctl_utils.ovs_p4ctl_del_entry(table['switch'],
                                                   table['name'], del_action)
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
