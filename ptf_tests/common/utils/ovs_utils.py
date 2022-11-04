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
This lib should contain utility functions related to ovs commands like
ovs-vsctl, ovs-ofctl and ovs-dpctl.
This lib makes use of functions from library common/lib/ovs.py.
"""

from common.lib.local_connection import Local
from common.lib.ssh import Ssh
from common.lib.ovs import Ovs
import common.lib.port_config as port_config


def get_connection_object(remote=False, hostname="", username="", passwd=""):
    """ Get connection object needed either for localhost or remote host
    commands execution

    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param passwd: remote host password, not required for DUT
    :type passwd: string e.g. cloudsw
    :return: connection object either SSHParamiko or Local class
    :rtype: Object e.g. connection
    """
    if remote:
        connection = Ssh(hostname, username, passwd)
        connection.setup_ssh_connection()
    else:
        connection = Local()
    return connection


def get_ovsctl_version(remote=False, hostname="", username="", passwd=""):
    """ Get current Ovs version
    """
    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname=hostname,username=username,passwd=passwd)
    ovs = Ovs(connection)
    # Execute needed ovs command
    version = ovs.vsctl.get_ver()
    if version:
        print(version)
    # Close connection
    connection.tear_down()

def add_bridge_to_ovs(bridge_name, remote=False, hostname="", username="",
                      passwd=""):
    """ Add bridge to ovs

    :param bridge_name: name of the bridge to add
    :type bridge_name: string e.g. br-int
    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param passwd: remote host password, not required for DUT
    :type passwd: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """
    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname=hostname,username=username,passwd=passwd)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.add_br(bridge_name)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to add bridge to ovs, error is:{err}')
        return False
    else:
        print('Successfully added bridge to ovs')
        return True

def del_bridge_from_ovs(bridge_name, remote=False, hostname="", username="",
                      passwd=""):
    """ Delete bridge from ovs

    :param bridge_name: name of the bridge to add
    :type bridge_name: string e.g. br-int
    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param passwd: remote host password, not required for DUT
    :type passwd: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """
    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname=hostname, username=username, passwd=passwd)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.del_br(bridge_name)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to delete bridge to ovs, error is:{err}')
        return False
    else:
        print('Successfully delete bridge to ovs')
        return True

def ovs_bridge_up(bridge_name, remote=False, hostname="", username="",
                  password=""):
    """ Make ovs bridge up and running

    :param bridge_name: name of the bridge to make up
    :type bridge_name: string e.g. br-int
    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param password: remote host password, not required for DUT
    :type password: string e.g. cloudsw
    :return: None
    :rtype: None
    """
    pc = port_config.PortConfig(remote, hostname, username, password)
    pc.Ip.iplink_enable_disable_link(bridge_name)
    pc.Ip.tear_down()
    print(f'bridge {bridge_name} is UP')


def add_vxlan_port_to_ovs(bridge, port, local_ip, remote_ip, dst_port,
                          remote=False, hostname="", username="", password=""):
    """Add vxlan port to ovs bridge

    :param bridge: Name of the bridge
    :type bridge: string e.g. br-int
    :param port:  name of vxlan port to add
    :type port: string e.g. vxlan1
    :param local_ip: local tunnel IP
    :type local_ip: string e.g. 40.1.1.1
    :param remote_ip: remote tunnel IP
    :type remote_ip: string e.g. 40.1.1.2
    :param dst_port: dst vxlan port
    :type dst_port: integer e.g. 4789
    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param password: remote host password, not required for DUT
    :type password: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """

    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname=hostname,username=username,passwd=password)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.add_port_vxlan_type(bridge, port, local_ip,
                                                    remote_ip, dst_port)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to add vxlan port to ovs, error is:{err}')
        return False
    else:
        print('Successfully added vxlan port to ovs')
        return True
    
def add_vlan_to_bridge(bridge, vlan,
                          remote=False, hostname="", username="", password=""):
    """Add vlan port to ovs bridge

    :param bridge: Name of the bridge
    :type bridge: string e.g. br-int
    :param vlan:  name of vlan port to add
    :type vlan: string e.g. vlan1
    :param local_ip: local tunnel IP
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param password: remote host password, not required for DUT
    :type password: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """
   
    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname=hostname, username=username, passwd=password)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.add_vlan_to_bridge(bridge, vlan)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to add vlan port to ovs, error is:{err}')
        return False
    else:
        print('Successfully added vlan port to ovs')
        return True


def add_port_to_ovs(bridge, port_to_add, remote=False, hostname="",
                    username="", password=""):
    """ Add given port to ovs bridge

    :param bridge: name of bridge
    :type bridge: string e.g. br-int
    :param port_to_add: name of port
    :type port_to_add: string e.g. port1
    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param password: remote host password, not required for DUT
    :type password: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """
    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname, username, password)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.add_port(bridge, port_to_add)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to add port to ovs, error is:{err}')
        return False
    else:
        print(f'Successfully added {port_to_add} port to ovs')
        return True


def del_port_from_ovs(bridge, port_to_delete, remote=False, hostname="",
                      username="", password=""):
    """ Delete given port from ovs including vxlan type too

    :param bridge: Name of the bridge
    :type bridge: string e.g. br-int
    :param port_to_delete: name of port to delete
    :type port_to_delete: string e.g. vxlan1
    :param remote: set value to True enables remote host cmd execution
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param password: remote host password, not required for DUT
    :type password: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """

    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname=hostname,username=username,passwd=password)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.del_port(bridge, port_to_delete)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to delete port from ovs, error is:{err}')
        return False
    else:
        print(f'Successfully delete {port_to_delete} port from ovs')
        return True


def del_ovs_bridge(bridge, remote=False, hostname="", username="", password=""):
    """ delete ovs bridge

    :param bridge: name of bridge to delete
    :type bridge: string e.g. br-int
    :type remote: boolean e.g. remote=True
    :param hostname: remote host IP address, not required for DUT host
    :type hostname: string e.g. 10.233.132.110
    :param username: remote host username, not required for DUT
    :type username: string e.g. root
    :param password: remote host password, not required for DUT
    :type password: string e.g. cloudsw
    :return: exit status
    :rtype: boolean e.g. True on success or False on failure
    """

    # Establish connection with local/remote host
    connection = get_connection_object(remote, hostname, username, password)
    ovs = Ovs(connection)
    # Execute needed ovs command
    out, rcode, err = ovs.vsctl.del_br(bridge)
    # Close connection
    connection.tear_down()
    # work on output data
    if rcode:
        print(f'failed to delete ovs bridge, error is:{err}')
        return False
    else:
        print(f'Successfully deleted ovs bridge')
        return True
