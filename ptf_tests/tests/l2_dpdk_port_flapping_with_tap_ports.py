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
DPDK port flapping test with TAP ports
"""
import unittest

# ptf related import
import ptf
from ptf import config  # importing global dict where all config data is present
from ptf import dataplane
from ptf import testutils
from ptf.base_tests import BaseTest

# scapy related imports
from scapy.all import *

# framework related imports
import common.utils.config_file_utils as config_file_utils
import common.utils.gnmi_cli_utils as gnmi_cli_utils
import common.utils.ovsp4ctl_utils as ovsp4ctl_utils
import common.utils.test_utils as test_utils
import common.lib.port_config as port_config


class DpdkPortFlapping(BaseTest):
    """L2 DPDK port flapping test with TAP ports
    to write new testcase, we need to have parent class as BaseTest
    which in turn uses unittest.TestCase, used to create new test cases
    """
    def setUp(self):
        """It brings up test environment needed to run test
        It performs below operations:
        1. Enables logging from "Dataplane.BaseTest" class
        2. Get instance to store results of set of tests using
           unittest.TestResult() class
        3. get all test_params from config dictionary
        4. get json config file and covert to dict
        5. Prepare list of gnmi_cli cmds needed to create vHost ports
        6.get a list of dict which maps TAP interfaces with its corresponding ip
        7. Initialize DataPlane instance
        :return:
        :rtype:
        """
        # Enables logging
        BaseTest.setUp(self)
        # Store results of set of tests
        self.result = unittest.TestResult()
        # for verify_packets to ignore other packets received at the interface
        config["relax"] = True

        # get test_params from config dictionary
        test_params = testutils.test_params_get()
        # get json config file and convert to dict
        config_json = test_params[
            'config_json']  # "--test_params" parameter passed from cmd line
        self.config_data = config_file_utils.get_config_dict(config_json)
        # Prepare list of gnmi_cli cmds needed to create vHost ports
        self.gnmicli_params_list = config_file_utils.get_gnmi_params_simple(
            self.config_data)
        # get a list of dict which maps TAP interfaces with its corresponding ip
        self.interface_ip_list = config_file_utils.get_interface_ipv4_dict(
            self.config_data)

        # global DataPlane instance used by all tests
        # initializing DataPlane class which starts thread
        # Provides methods to send and receive packets on dataplane
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)
        self.dataplane = ptf.dataplane_instance

    def runTest(self):
        """It performs test execution. below steps executed:
        1. Generate deps files and pb.bin file
        2. set TAP interfaces using gnmi_cli set and bring up interfaces
        3. Add TAP ports to dataplane
        4. Set forwarding pipeline
        5. Add rules for table into forwarding pipe for eth_dst and eth_src
        6. Create simple TCP packet for both dst and src
        Port Flapping:
        7. Send 1-1 pkt to TAP1 & TAP2 port and verify pkt received status
        8. Bring TAP interfaces down
        9. Send 1-1 pkt to TAP1 & TAP2 port and verify pkt received status
        10. Bring TAP interfaces up
        11. Send 1-1 pkt to TAP1 & TAP2 port and verify pkt received status

        :return: None
        :rtype: None
        """
        # Generate deps files and pb.bin file
        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(
                self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # set TAP interfaces using gnmicli set
        if not gnmi_cli_utils.gnmi_cli_set_and_verify(self.gnmicli_params_list):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")
        # Assign IP to TAP interfaces and bring up
        gnmi_cli_utils.ip_set_ipv4(self.interface_ip_list)

        # Update "port_map" dictionary which maps port ids to iface name
        # port_ids:list of tuples where every tuple represents port_id
        # port_id:combination of (device_number, port_number)
        port_ids = test_utils.add_port_to_dataplane(
            self.config_data['port_list'])

        # Add TAP ports to dataplane
        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)

        # Set forwarding pipeline
        if not ovsp4ctl_utils.ovs_p4ctl_set_pipe(self.config_data['switch'],
                                                 self.config_data['pb_bin'],
                                                 self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        tcp_packet_list = []
        for table in self.config_data['table']:
            print(f"Scenario : l2 dpdk port flapping with tap ports: "
                  f"{table['description']}")

            # Add rules for table into forwarding pipe
            print(f"Adding {table['description']} rules")
            for match_action in table['match_action']:
                if not ovsp4ctl_utils.ovs_p4ctl_add_entry(table['switch'],
                                                          table['name'],
                                                          match_action):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

            # Generate 1st simple TCP packet applicable as Eth/IP/TCP pkt
            if table['description'] == 'table_for_dst_mac':
                pkt = ptf.testutils.simple_tcp_packet(
                    eth_dst=self.config_data['traffic']['in_pkt_header'][
                        'eth_dst_1'])
            else:
                pkt = ptf.testutils.simple_tcp_packet(
                    eth_src=self.config_data['traffic']['in_pkt_header'][
                        'eth_src_1'])
            tcp_packet_list.append(pkt)

            # Send 1 packet to TAP1 port and verify whether pkt is received as
            # per rule1
            ptf.testutils.send_packet(self, port_ids[
                self.config_data['traffic']['send_port'][0]], pkt)
            try:
                ptf.testutils.verify_packet(self, pkt, port_ids[
                    self.config_data['traffic']['receive_port'][0]])
                print(f"PASS: Verification of packet passed, packet received "
                      f"as per rule 1")
            except Exception as e:
                self.fail(f"FAIL: Verification of packet sent failed with "
                          f"exception {e}")

            # Generate 2nd simple TCP packet applicable as Eth/IP/TCP pkt
            if table['description'] == 'table_for_dst_mac':
                pkt = ptf.testutils.simple_tcp_packet(
                    eth_dst=self.config_data['traffic']['in_pkt_header'][
                        'eth_dst_2'])
            else:
                pkt = ptf.testutils.simple_tcp_packet(
                    eth_src=self.config_data['traffic']['in_pkt_header'][
                        'eth_src_2'])
            tcp_packet_list.append(pkt)

            # Send pkt to TAP2 port and verify whether packet is dropped as
            # per rule2
            ptf.testutils.send_packet(self, port_ids[
                self.config_data['traffic']['send_port'][1]], pkt)
            try:
                ptf.testutils.verify_no_packet(self, pkt, port_ids[
                    self.config_data['traffic']['receive_port'][1]])
                print(f"PASS: Verification of packet passed, packet dropped "
                      f"as per rule 2")
            except Exception as e:
                self.fail(f"FAIL: Verification of packet sent failed with "
                          f"exception {e}")

        # Now bring TAP interfaces down
        print('Bring TAP interfaces down...')
        portconfig_obj = port_config.PortConfig()
        for interface_ipv4_dict in self.interface_ip_list:
            for interface, ip in interface_ipv4_dict.items():
                portconfig_obj.Ip.iplink_enable_disable_link(interface,
                                                             status_to_change='down')
        print('TAP interfaces are down!!!')
        time.sleep(5)

        # Now resend packets to verify that traffic should not be forwarded
        print('Resend traffic to verify traffic is not getting forwarded...')
        pkt_counter = 0
        for table in self.config_data['table']:
            # Send first packet to TAP1 port
            try:
                ptf.testutils.send_packet(self, port_ids[
                    self.config_data['traffic']['send_port'][0]],
                                          tcp_packet_list[pkt_counter])
                ptf.testutils.verify_no_packet(self,
                                               tcp_packet_list[pkt_counter],
                                               port_ids[
                                                   self.config_data['traffic'][
                                                       'receive_port'][0]])
                print(f"PASS: Verification of packet passed, packet didn't "
                      f"received as port is down")
            except OSError as OE:
                if "Network is down" in str(OE):
                    print(f'Pass: Traffic send failed as Ports are down.')
            except Exception as e:
                self.fail(f"FAIL: Verification of packet sent failed with "
                          f"exception {e}")
            pkt_counter = pkt_counter + 1

            # Send 2nd packet to TAP2 port
            try:
                ptf.testutils.send_packet(self, port_ids[
                    self.config_data['traffic']['send_port'][1]],
                                          tcp_packet_list[pkt_counter])
                ptf.testutils.verify_no_packet(self,
                                               tcp_packet_list[pkt_counter],
                                               port_ids[
                                                   self.config_data['traffic'][
                                                       'receive_port'][1]])
                print(f"PASS: Verification of packet passed, packet didn't "
                      f"received as port is down")
            except OSError as OE:
                if "Network is down" in str(OE):
                    print(f'Pass: Traffic send failed as Ports are down.')
            except Exception as e:
                self.fail(f"FAIL: Verification of packet sent failed with "
                          f"exception {e}")
            pkt_counter = pkt_counter + 1

        # Now bring TAP interfaces up
        print(f'Bringing TAP interfaces up...')
        for interface_ipv4_dict in self.interface_ip_list:
            for interface, ip in interface_ipv4_dict.items():
                portconfig_obj.Ip.iplink_enable_disable_link(interface,
                                                             status_to_change='up')
        self.dataplane.flush()
        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_up(device, port)
        # Get Port status
        status = test_utils.get_port_status(self.interface_ip_list)
        if not status:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: Port up status verification failed")
        time.sleep(5)
        print(f'TAP interfaces are up.')

        # Now resend packets to verify that traffic should be sent
        print('Resend traffic to verify traffic is getting forwarded...')
        pkt_counter = 0
        for table in self.config_data['table']:
            print(port_ids[self.config_data['traffic']['send_port'][0]])
            # Send first packet to TAP1 port
            ptf.testutils.send_packet(self, port_ids[self.config_data[
                'traffic']['send_port'][0]], tcp_packet_list[pkt_counter])
            try:
                ptf.testutils.verify_packet(self, tcp_packet_list[pkt_counter],
                                            port_ids[
                                                self.config_data['traffic'][
                                                    'receive_port'][0]])
                print(f"PASS: Verification of packet passed, packet received "
                      f"as per rule1")
            except Exception as e:
                self.fail(f"FAIL: Verification of packet sent failed with "
                          f"exception {e}")
            pkt_counter = pkt_counter + 1

            # Send 2nd packet to TAP2 port
            ptf.testutils.send_packet(self, port_ids[
                self.config_data['traffic']['send_port'][1]],
                                      tcp_packet_list[pkt_counter])
            try:
                ptf.testutils.verify_no_packet(self,
                                               tcp_packet_list[pkt_counter],
                                               port_ids[
                                                   self.config_data['traffic'][
                                                       'receive_port'][1]])
                print(f"PASS: Verification of packet passed, packet dropped as "
                      f"per rule1")
            except Exception as e:
                self.fail(f"FAIL: Verification of packet sent failed with "
                          f"exception {e}")
            pkt_counter = pkt_counter + 1

        # Stop data-plane thread
        self.dataplane.kill()

    def tearDown(self):
        """It removes p4ovs pipeline table entries, rules
        :return: None
        :rtype: None
        """
        for table in self.config_data['table']:
            print(f"Deleting {table['description']} rules")
            for del_action in table['del_action']:
                ovsp4ctl_utils.ovs_p4ctl_del_entry(table['switch'],
                                                   table['name'], del_action)
        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")
