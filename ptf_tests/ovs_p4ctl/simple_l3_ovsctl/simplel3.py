"""
P4Runtime interface basic L2 tests
"""

import time
import sys
import logging

import random

import ptf.dataplane as dataplane
from ptf.testutils import *

from pdb import set_trace

import scapy.main
import scapy.contrib
from scapy.packet import *
from scapy.fields import *
from scapy.all import *

from ptf.base_tests import BaseTest
#from ptf import config
from ptf.testutils import *
from ptf.packet import *
#from ptf.thriftutils import *

import os
#import ptf.mask as mask

import warnings
import ovsp4ctl
from ovsp4ctl import ovs_p4ctl

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
#from common.utils import *


class SimpleL3Test(BaseTest):
    def compare_packets(self, in_pkts, out_pkts):
        for in_pkt in in_pkts:
            for out_pkt in out_pkts:
                if str(in_pkt.payload) == str(out_pkt.payload):
                    return 1
                else:
                    return 0

    def runTest(self):
        try:
            test_params = ptf.testutils.test_params_get()
            p4info_file = test_params['p4info']
            p4bin_file = test_params['p4bin']

            print("contacting grpc server")
            ovs_p4ctl.p4ctl_set_pipe('br0', p4bin_file, p4info_file)
            print("grpc server contacted and set the forwarding pipeline")

            #add rule saying that any packet with destination ip 1.1.1.1 should be sent to port with port id 0
            print("adding rule for port 0")
            ovs_p4ctl.p4ctl_add_entry('br0', 'ingress.ipv4_host', 'hdr.ipv4.dst_addr=1.1.1.1,action=ingress.send(0)')

            #add rule saying that any packet with destination ip 2.2.2.2 should be sent to port with port id 1
            print("adding rule for port 1")
            ovs_p4ctl.p4ctl_add_entry('br0', 'ingress.ipv4_host', 'hdr.ipv4.dst_addr=1.1.1.2,action=ingress.send(1)')

            #form packet with dst mac, src mac, dst ip, src ip and send it on port 1
            print("sending packet to check if rule on port 0 hits")
            pkt = Ether(dst="00:00:00:00:03:14", src="9e:ba:ce:98:d9:e8")/\
                IP(dst="1.1.1.1", src="192.168.1.10")/\
                TCP()/\
                Raw(load="0"*50)
            wrpcap("/tmp/in_rule1.pcap", pkt)

            #tcpdump
            p1 = subprocess.Popen(['tcpdump', '-w', '/tmp/capture1.pcap', '-i', 'TAP0'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            time.sleep(1)
            sendp(pkt, iface='TAP1')
            time.sleep(2)
            p1.terminate()
            status = None
            while status is None: 
                status = p1.poll()
            time.sleep(2)

            #forming 2nd packet
            print("sending packet to check if rule on port 1 hits")
            pkt = Ether(dst="00:00:00:00:03:14", src="9e:ba:ce:98:d9:e8")/\
                IP(dst="1.1.1.2", src="192.168.1.10")/\
                TCP()/\
                Raw(load="0"*50)
            wrpcap("/tmp/in_rule2.pcap", pkt)

            #tcpdump
            p2 = subprocess.Popen(['tcpdump', '-w', '/tmp/capture2.pcap', '-i', 'TAP1'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            time.sleep(1)
            sendp(pkt, iface='TAP0')
            time.sleep(2)
            p2.terminate()
            status = None
            while status is None:
                status = p2.poll()
            time.sleep(2)

            #deleting rule2
            print("deleting rule for port 1")
            ovs_p4ctl.p4ctl_del_entry('br0', 'ingress.ipv4_host', 'hdr.ipv4.dst_addr=1.1.1.2')

            #forming 3rd packet and sending to validate if rule 2 hits or not
            print("sending packet to check the rule deletion")
            pkt = Ether(dst="00:00:00:00:03:14", src="9e:ba:ce:98:d9:e8")/\
                IP(dst="1.1.1.2", src="192.168.1.10")/\
                TCP()/\
                Raw(load="0"*50)
            wrpcap("/tmp/in_rule3.pcap", pkt)

            #tcpdump
            p3 = subprocess.Popen(['tcpdump', '-w', '/tmp/capture3.pcap', '-i', 'TAP1'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            time.sleep(1)
            sendp(pkt, iface='TAP0')
            time.sleep(2)
            p3.terminate()
            status = None
            while status is None:
                status = p3.poll()
            time.sleep(2)

            #verifying packets for rule1
            in_rule1_pkts = rdpcap("/tmp/in_rule1.pcap")
            capture1_pkts = rdpcap("/tmp/capture1.pcap")
            valid = self.compare_packets(in_rule1_pkts, capture1_pkts)
            if valid:
                print ("verified sent and received packets for rule1")
            else:
                print ("sent and received packet doesn't match for rule1")

            #verifying packets for rule2
            in_rule2_pkts = rdpcap("/tmp/in_rule2.pcap")
            capture2_pkts = rdpcap("/tmp/capture2.pcap")
            valid = self.compare_packets(in_rule2_pkts, capture2_pkts)
            if valid:
                print ("verified sent and received packets for rule2")
            else:
                print ("sent and received packet doesn't match for rule2")

            #verifying the packet for rule3
            in_rule3_pkts = rdpcap("/tmp/in_rule3.pcap")
            capture3_pkts = rdpcap("/tmp/capture3.pcap")
            valid = self.compare_packets(in_rule3_pkts, capture3_pkts)
            if valid:
                print ("verified sent and received packets for rule3")
            else:
                print ("sent and received packet doesn't match for rule3")


        finally:
            pass

    def tearDown(self):
       #self.dataplane.flush()
       pass

