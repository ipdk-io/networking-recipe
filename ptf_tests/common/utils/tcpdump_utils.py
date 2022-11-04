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
from common.lib.tcpdump import TcpDumpCap
from common.lib.local_connection import Local
import os,shutil


def tcpdump_start_pcap(interface, src_host ="", pkt_count=1 ):
    """
    TCPDUMP function to start packet capture in background and dump packet capture to /tmp dir
    e.g  << tcpdump -i TAP1 host 192.168.1.10 -nn -c 1 >> /tmp/TAP1/TAP1.pcap &

    :params interface: to start packet capture 
            src_host: to filter traffic with source ip
            pkt_count: Number of packets to capure with default value as 1 
    """
    tcpd = TcpDumpCap()

    cmdopt = []
    if interface:
        cmdopt.extend(['-i', interface])
    if src_host:
        cmdopt.extend(['src', src_host])
    if pkt_count:
        cmdopt.extend(['-c', pkt_count])
    cmdopt.extend(['-nn'])
    pcapdir = "/tmp/" + interface  
    if not os.path.exists(pcapdir):
       try:
           os.makedirs(pcapdir)
       except OSError as e:
           if e.errno != errno.EEXIST:
                raise
    pcapfile = pcapdir + "/" + interface + ".pcap"
    cmdopt.extend(['>>', pcapfile])
    if os.path.exists(pcapfile): os.remove(pcapfile)
    tcpd.TCPDUMP.tcpdump_start_capture(cmdopt)


def tcpdump_get_pcap(interface): 
    """
    Function to return captured packets in clear text
    param: interface name for file/interface identity
    return: type str, tcdump packet capture output

    """
    pcapdir = "/tmp/" + interface
    pcapfile = pcapdir + "/" + interface + ".pcap"
    if os.path.exists(pcapfile) and os.stat(pcapfile).st_size != 0:
        output = open(pcapfile).read()
        return output
    else: 
        print("Pcap file does not exist or empty")

def tcdump_match_str(superstring, substring):
    """
    Function to match all substring elements with superstring
    params: type str, superstring and substring
    return: value of True or false
    """
    superstringlist = list(superstring.split(" "))
    substringlist = list(substring.split(" "))
    result = True
    for str in substringlist:
        if str not in superstringlist:
            result = False
    return result

def tcpdump_remove_pcap_file(interface):
    """
    A function to remove pcap file directory e.g  remove /tmp/TAP1/

    :params interface: to start packet capture 
    """
    
    pcapdir = "/tmp/" + interface
    if os.path.exists(pcapdir):
        try:
            shutil.rmtree(pcapdir)
            return pcapdir
        except OSError as e:
            print(f"FAIL: Failed to remove {pcapdir}due to {e.strerror}")
            return False

def tcpdump_tear_down():
  
    tcpd = TcpDumpCap()
    tcpd.TCPDUMP.tcpdump_tear_down()
