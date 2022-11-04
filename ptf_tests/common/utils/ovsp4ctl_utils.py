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
import sys
import io
import re
import time
from io import StringIO
import common.lib.ovs_p4ctl as ovs_p4ctl
from contextlib import redirect_stdout
from common.lib.local_connection import Local

def ovs_p4ctl_set_pipe(bridge, device_config, p4info):
    """
    set_pipe SWITCH DEVICE_CONFIG P4INFO
    Example:
    ovs_p4ctl.p4ctl_set_pipe('br0', '/root/cfgs/simple_l3/simple_l3.pb.bin',
            '/root/cfgs/simple_l3/p4Info.txt')

    """
    try:
        out = ovs_p4ctl.p4ctl_set_pipe(bridge, device_config, p4info)
        if out == None:
            print(f"PASS: ovs-p4ctl set pipe: {device_config}")
            return True

    except Exception as error:
        print(f"FAIL: ovs-p4ctl set pipe Failed with error: {error}")
        return False

    
def ovs_p4ctl_add_entry(bridge, tbl_name, flow):
    """
    add-entry SWITCH TABLE MATCH_KEY ACTION ACTION_DATA
    Example:
      ovs_p4ctl_add_entry('br0', 'ingress.ipv4_host', 'hdr.ipv4.dst_addr=2.2.2.2,action=ingress.send(1)')

    """
    try:
        out = ovs_p4ctl.p4ctl_add_entry(bridge, tbl_name, flow)
        if out == None:
            print(f"PASS: ovs-p4ctl add entry: {flow}")
            return True

    except Exception as error:
        print(f"FAIL: ovs-p4ctl add entry Failed with error: {error}")
        return False



def ovs_p4ctl_del_entry(bridge, tbl_name, match_key):
    """
    del-entry SWITCH TABLE MATCH_KEY
    Example:
      ovs_p4ctl_del_entry('br0', 'ingress.ipv4_host', 'hdr.ipv4.dst_addr=.2.2.2')

    """
    
    try:
        out = ovs_p4ctl.p4ctl_del_entry(bridge, tbl_name, match_key)
        if out == None:
            print(f"PASS: ovs-p4ctl del entry: {match_key}")
            return True

    except Exception as error:
        print(f"FAIL; ovs-p4ctl del entry Failed with error: {error}")
        return False



def ovs_p4ctl_add_member(bridge, tbl_name, member_details):

    """
    add-member SWITCH TABLE MEMBER_DETAILS
    Example:
      ovs_p4ctl_add_member('br0', 'ingress.as_sl3', 'action=ingress.send(2),member_id=1')      

    """

    try:
        out = ovs_p4ctl.p4ctl_add_member(bridge, tbl_name, member_details)
        if out == None:
            print(f"PASS: ovs-p4ctl add member {member_details}")
            return True

    except Exception as error:
        print(f"FAIL: ovs-p4ctl add member Failed with error: {error}")
        return False
        

def ovs_p4ctl_add_member_and_verify(bridge, tbl_name, member_details):

    """
    add-member SWITCH TABLE MEMBER_DETAILS
    Example:
      ovs_p4ctl_add_member_and_verify('br0', 'ingress.as_sl3', 'action=ingress.send(2),member_id=1')

    """

    action, action_data, mem_id = ovs_p4ctl.parse_profile_mem(member_details)
    try:
        out = ovs_p4ctl.p4ctl_add_member(bridge, tbl_name, member_details)
        if out == None:
            print(f"PASS: ovs-p4ctl add member {member_details}")
        f = io.StringIO()
        with redirect_stdout(f):
            out = ovs_p4ctl.p4ctl_get_member(bridge, tbl_name, f'member_id={mem_id}')

        result = f.getvalue()
        values = result.split("\n")

        dict_var = re.findall('[a-z|_|=|.|0-9]+', values[1].strip())
        member_returned = {}
        for i in dict_var:
            member_returned.update(dict([i.split("=")]))
        if out == None:
            print(f"PASS: ovs-p4ctl get details for member {mem_id} :  {member_returned}")
        keys = member_returned.keys()
        if (tbl_name == member_returned["action_profiles"]) and (action == member_returned["actions"]) and (' '.join(map(str, action_data)) == member_returned["dst_port"]):
            print("PASS : Returned member details matching with configured group")
        else:
            print("FAIL : Returned member details are not matching with configured group")
            return False
    except Exception as error:
        print(f"FAIL: ovs-p4ctl add member Failed with error: {error}")
        return False

    return True




def ovs_p4ctl_get_member(bridge, tbl_name, member_id, member_details=None):
    """
    get-member SWITCH TABLE MEMBER_ID
    Example:
        ovs_p4ctl_get_member('br0', 'ingress.as_sl3', 'member_id=1')
        ovs_p4ctl_get_member('br0', 'ingress.as_sl3', 'member_id=1', 'action=ingress.send(10)')
    """

    try:
        f = io.StringIO()
        with redirect_stdout(f):
            out = ovs_p4ctl.p4ctl_get_member(bridge, tbl_name, member_id)
        result = f.getvalue() 
        values = result.split("\n")
        dict_var = re.findall('[a-z|_|=|.|0-9]+', values[1].strip())
        member_returned = {}
        for i in dict_var:
            member_returned.update(dict([i.split("=")]))

        if out == None:
            print(f"PASS: ovs-p4ctl get details for member {member_id} :  {member_returned}")
        
        if member_details != None:
            action, action_data, mem_id = ovs_p4ctl.parse_profile_mem(member_details + ',' + member_id)
            if (tbl_name == member_returned["action_profiles"]) and (action == member_returned["actions"]) and (' '.join(map(str, action_data)) == member_returned["dst_port"]):
                print("PASS : Returned member details matching with configured group")
            else:
                print("FAIL : Returned member details are not matching with configured group")
                return False
 
        return member_returned    

    except Exception as error:
        print(f"ovs-p4ctl get member Failed with error: {error}")
        return False 

def ovs_p4ctl_del_member(bridge, tbl_name, member_id):
    """
    del-member SWITCH TABLE MEMBER_ID
    Example:
        ovs_p4ctl_get_member('br0', 'ingress.as_sl3', 'member_id=1')
    """
 
    try:
        out = ovs_p4ctl.p4ctl_del_member(bridge, tbl_name, member_id)
        if out == None:
            print(f"PASS: ovs-p4ctl del member {member_id}")
            return True
    except Exception as error:
        print(f"FAIL: ovs-p4ctl del member Failed with error: {error}")
        return False



def ovs_p4ctl_add_group(bridge, tbl_name, group_details):
    """
    add-group SWITCH TABLE GROUP_DETAILS
    Example:
       ovs_p4ctl_add_group('br0', 'ingress.as_sl3', 'group_id=1 ,reference_members=(1),max_size=128')
    """

    try:
        out = ovs_p4ctl.p4ctl_add_group(bridge, tbl_name, group_details)
        if out == None:
            print(f"PASS: ovs-p4ctl add group {group_details}")
            return True

    except Exception as error:
        print(f"FAIL: ovs-p4ctl add group Failed with error: {error}")
        return False


def ovs_p4ctl_add_group_and_verify(bridge, tbl_name, group_details):
    """
    add-group SWITCH TABLE GROUP_DETAILS
    Example:
       ovs_p4ctl_add_group_and_verify('br0', 'ingress.as_sl3', 'group_id=1 ,reference_members=(1),max_size=128')
    """

    group_id, members, max_size = ovs_p4ctl.parse_profile_group(group_details)
    try:
        out = ovs_p4ctl.p4ctl_add_group(bridge, tbl_name, group_details)
        if out == None:
            print(f"PASS: ovs-p4ctl add group {group_details}")
        f = io.StringIO()
        with redirect_stdout(f):
            out = ovs_p4ctl.p4ctl_get_group(bridge, tbl_name, f'group_id={group_id}')
        result = f.getvalue()
        values = result.split("\n")
        group_returned = dict(substring.split("=") for substring in values[1].strip().split(" "))

        if out == None:
            print(f"PASS: ovs-p4ctl get details for group {group_id} :  {group_returned}")
        if (tbl_name == group_returned["action_profiles"]) and (members == group_returned["reference_members"]) and (max_size == group_returned["max_size"]):
            print("PASS: Returned group details matching with configured group")
        else:
            print("FAIL : Returned group details are not matching with configured group")
            return False
        
    except Exception as error:
        print(f"FAIL: ovs-p4ctl add group Failed with error: {error}")
        return False

    return True



def ovs_p4ctl_del_group(bridge, tbl_name, group_id):
    """
    del-group SWITCH TABLE GROUP_ID
    Example:
       ovs_p4ctl_del_group('br0', 'ingress.as_sl3', 'group_id=4')
    """

    try:
        out = ovs_p4ctl.p4ctl_del_group(bridge, tbl_name, group_id)
        if out == None:
            print(f"PASS: ovs-p4ctl del group {group_id}")
            return True
    except Exception as error:
        print(f"FAIL: ovs-p4ctl del group Failed with error: {error}")
        return False



def ovs_p4ctl_get_group(bridge, tbl_name, group_id, group_details=None):
    """
    get-group SWITCH TABLE GROUP_ID
    Example:
       ovs_p4ctl_get_group('br0', 'ingress.as_sl3', 'group_id=4')
       ovs_p4ctl_get_group('br0', 'ingress.as_sl3', 'group_id=4', 'reference_members=(1),max_size=128' )
    """

    try:
        f = io.StringIO()
        with redirect_stdout(f):
            out = ovs_p4ctl.p4ctl_get_group(bridge, tbl_name, group_id)
        result = f.getvalue()
        values = result.split("\n")
        group_returned = dict(substring.split("=") for substring in values[1].strip().split(" "))

        if out == None:
            print(f"PASS: ovs-p4ctl get details for group {group_id} :  {group_returned}")
            
        if group_details != None:
            group_id, members, max_size = ovs_p4ctl.parse_profile_group(group_id + ',' + group_details)

            if (tbl_name == group_returned["action_profiles"]) and (members == group_returned["reference_members"]) and (max_size == group_returned["max_size"]):
                print("PASS: Returned group details matching with configured group")
            else:
                print("FAIL: Returned group details are not matching with configured group")
                return False


        return group_returned
    
    except Exception as error:
        print(f"FAIL: ovs-p4ctl get group Failed with error: {error}")
        return False
    
def ovs_p4ctl_get_counter_data(bridge, cnt_tbl_name, flow):
    """
    A function to build counter dictionary based on table and flow received
    :return a counter dictionary
    """
    counter ={}
    try:
        #The called function doesn't return value but print out the value.
        #Thus using IO steam to capture the print out and parse it to 
        #build counter data
        save_stdout = sys.stdout
        output = StringIO()
        sys.stdout = output
        ovs_p4ctl.p4ctl_get_counter_entry(bridge, cnt_tbl_name, flow)
        sys.stdout = save_stdout
        result = output.getvalue().split(",", 2)
        for item in result[0:2]:
            counter[item.split("=")[0].strip()] = item.split("=")[1].strip()
       
        for item in result[2].split("=",1)[1].replace("(","").replace(")","").split(","):
            counter[item.split("=")[0].strip()] = int(item.split("=")[1].strip())
            
        return counter
    except Exception as error:
        print(f"FAIL: ovs-p4ctl add group Failed with error: {error}")
        return False
        
def ovs_p4ctl_get_counter_table_and_id():
    """
    :Function to retieve all counter table name and their if from P4 information
    :return a list of tuple of table name and its id.
    """
    table_name_and_id = []
    try:
        client=ovs_p4ctl.P4RuntimeClient(device_id=1)
        p4info =client.get_p4info()
        for i in range(len(p4info.counters)):
            table_name = p4info.counters[i].preamble.name
            counter_id = p4info.counters[i].preamble.id 
            table_name_and_id.append((table_name, counter_id))
        client.tear_down()
    except Exception as error:
        print(f"FAIL: unable to get p4info to: {error}")
        return False
    return table_name_and_id

def ovs_p4ctl_reset_counter_entry( bridge, cnt_tbl_name, flow):
    """
    A utility function to reset counter
    reset-counter-entry SWITCH COUNTER_TABLE RESET_COUNTER_FLOW
    Example: 
       ovs-p4ctl reset-counter br0 \
           ipv4_host_tbl_flow_counter_packets counter_id=303591076,index=1
    """
    ovs_p4ctl.p4ctl_reset_counter_entry(bridge, cnt_tbl_name, flow)
