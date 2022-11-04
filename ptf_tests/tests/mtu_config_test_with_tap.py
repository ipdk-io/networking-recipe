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
DPDK Port Config: MTU test with TAP ports 
"""

# in-built module imports
import time

# ptf related imports
import ptf
import ptf.dataplane as dataplane
import sys, unittest
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config

# scapy related imports
from scapy.packet import *
from scapy.fields import *
from scapy.all import *
#from scapy.all import sniff


# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple, get_interface_ipv4_dict
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params, ip_set_ipv4
from common.utils.tcpdump_utils import tcpdump_start_pcap, tcpdump_get_pcap, tcdump_match_str, tcpdump_tear_down

class TapPort_MTU_Config(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        config["relax"] = True # for verify_packets to ignore other packets received at the interface
        
        test_params = test_params_get()
        config_json = test_params['config_json']
        self.dataplane = ptf.dataplane_instance
        ptf.dataplane_instance = ptf.dataplane.DataPlane(config)

        self.config_data = get_config_dict(config_json)

        self.gnmicli_params = get_gnmi_params_simple(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)


    def runTest(self):
        src_mac = self.config_data['traffic']['in_pkt_header']['src_mac']
        dst_mac = self.config_data['traffic']['in_pkt_header']['dst_mac']
        ip_src = self.config_data['traffic']['in_pkt_header']['ip_src']
        ip_dst1 = self.config_data['traffic']['in_pkt_header']['ip_dst_1']
        ip_dst2 = self.config_data['traffic']['in_pkt_header']['ip_dst_2']
        pktsize1= self.config_data['traffic']['in_pkt_header']['pktsize_1']
        pktsize2= self.config_data['traffic']['in_pkt_header']['pktsize_2']

        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        gnmi_set_params(self.gnmicli_params)
        ip_set_ipv4(self.interface_ip_list)

        port_list = self.config_data['port_list']
        port_ids = test_utils.add_port_to_dataplane(port_list)

        for port_id, ifname in config["port_map"].items():
            device, port = port_id
            self.dataplane.port_add(ifname, device, port)

        ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info'])

        for table in self.config_data['table']:

           print(f"Scenario : Verifying MTU configuration on DPDK Tap Ports")
           print(f"Adding rules")
           for match_action in table['match_action']:
              ovs_p4ctl.ovs_p4ctl_add_entry(table['switch'],table['name'], match_action)

        for port in port_list:
            for port_details in self.config_data['port']:
                if port_details['name'] == port_list[0]:
                    assigned_mtu = port_details['mtu']
        mtudiff = int(pktsize1) - int(assigned_mtu) 

        print(f"sending packet from " + port_list[0] + " of size " + pktsize1 +"  i.e. greater than assigned MTU of " + assigned_mtu )
        tcpdump_start_pcap(interface=port_list[1], src_host=ip_src, pkt_count=1)
        pkt = simple_tcp_packet(eth_src=src_mac,eth_dst=dst_mac,ip_src=ip_src,ip_dst=ip_dst1,pktlen=int(pktsize1))
        try:
           send_packet(self, port_ids[self.config_data['traffic']['send_port'][0]][1], pkt)
        except Exception as err:
           errstr = "Message too long"
           result = tcdump_match_str(str(err), errstr)
           if result:
               print(f"PASS: Assertion verified as per difference in MTU and packet size")
           else:
               self.result.addFailure(self, sys.exc_info())
               self.fail(f"FAIL: Assertion not verified as per difference in MTU and packet size")
        else:
            try:
                time.sleep(1)
                verify_packet(self, pkt, port_id=port_ids[1][1], timeout=None)
            except Exception as err:
                outpcap = tcpdump_get_pcap(port_list[1])
                mtudiffstr = "truncated-ip " + str(mtudiff) + " bytes missing!"
                result =  tcdump_match_str(outpcap, mtudiffstr)
                if result == True :
                    print(f"PASS: Assertion verified as per difference in MTU and packet size")
                else:
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"FAIL: Assertion not verified as per difference in MTU and packet size")
            else:
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"FAIL: Assertion not verified as per difference in MTU and packet size")
            tcpdump_tear_down()

        pkt = simple_tcp_packet(eth_src=src_mac,eth_dst=dst_mac,ip_src=ip_src,ip_dst=ip_dst2,pktlen=int(pktsize1))
        print("sending packet from " + port_list[1] + " of size " + pktsize1  )
        send_packet(self, port_ids[self.config_data['traffic']['send_port'][1]], pkt)
        try:
            verify_packet(self, pkt, port_id=port_ids[0][1], timeout=None)
            print(f"PASS: Verification of packets passed, packet received as expected")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: Verification of packets sent failed with exception {err}")
                
             
        pkt = simple_tcp_packet(eth_src=src_mac,eth_dst=dst_mac,ip_src=ip_src,ip_dst=ip_dst1,pktlen=int(pktsize2))
        send_packet(self, port_ids[self.config_data['traffic']['send_port'][0]], pkt)
        print("sending packet from " + port_list[0] + " of size " + pktsize2  )
        try:
            verify_packet(self, pkt, port_id=port_ids[1][1], timeout=None)
            print(f"PASS: Verification of packets passed, packet received as expected")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: Verification of packets sent failed with exception {err}")


        pkt = simple_tcp_packet(eth_src=src_mac,eth_dst=dst_mac,ip_src=ip_src,ip_dst=ip_dst2,pktlen=int(pktsize2))
        send_packet(self, port_ids[self.config_data['traffic']['send_port'][1]], pkt)
        print("sending packet from " + port_list[1] + " of size " + pktsize2  )
        try:
            verify_packet(self, pkt, port_ids[0][1], timeout=None)
            print(f"PASS: Verification of packets passed, packet received as expected")
        except Exception as err:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: Verification of packets sent failed with exception {err}")

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

 

