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
This lib implements functionalities to run ovs utility commands like ovs-vsctl
Usage:
ovs = Ovs(self.connection)
ovs.vsctl.get_ver()
"""

import re
from common.lib.exceptions import ExecuteCMDException


class Ovs(object):
    """
    Implements subclasses like _Cli, _Vsctl, _Dpctl and _Ofctl which implements
    list of APIs to execute ovs utility commands
    """
    def __init__(self, connection):
        self.vsctl = self._Vsctl(connection)
        self.dpctl = self._Dpctl(connection)
        self.ofctl = self._Ofctl(connection)

    class _Cli(object):
        cmd_prefix = None

        def __init__(self, connection):
            self.connection = connection

        def show(self, bridge=""):
            """Get configuration

            :param bridge: bridge name to show summary of configuration or
            empty for global config
            :type bridge: string e.g. br1
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"show {bridge}")
            return self.connection.execute_command(cmd)

        def form_cmd(self, cmd):
            """Combine command prefix with command

            :param cmd: command to combine with prefix
            :type cmd: str
            :return: command combined with prefix
            :rtype: str
            """
            return " ".join([self.cmd_prefix, cmd])

    class _Vsctl(_Cli):
        cmd_prefix = "ovs-vsctl"

        def add_br(self, name):
            """Add bridge with given name

            :param name: name of bridge
            :type name: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"add-br {name}")
            return self.connection.execute_command(cmd)

        def del_br(self, name):
            """Delete bridge with given name

            :param name: name of bridge to del
            :type name: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"del-br {name}")
            return self.connection.execute_command(cmd)

        def add_port(self, bridge, port):
            """Add given port to bridge or create one if not exist

            :param bridge: bridge to add to
            :type bridge: str
            :param port: port name to add (either existing if or new)
            :type port: LinuxAdapter | str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"add-port {bridge} {port}")
            return self.connection.execute_command(cmd)
        
        def add_port_vxlan_type(self, bridge, port, local_ip, remote_ip,
                                dst_port):
            """
            
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
            :return: it returns cmd output, error code, and error
            :rtype: tuple e.g. out, e_code, err
            """
            cmd = self.form_cmd(f"add-port {bridge} {port} -- set interface "
                                f"{port} type=vxlan options:local_ip={local_ip}"
                                f" options:remote_ip={remote_ip}"
                                f" options:dst_port={dst_port}")
            return self.connection.execute_command(cmd)
        
        def add_p4_device(self, id):
            """Add bridge with given name

            :param name: name of bridge
            :type name: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"add-p4-device {id}")
            return self.connection.execute_command(cmd)
        
        def add_br_p4(self, bridge, id):
            """Add bridge with given name

            :param name: name of bridge
            :type name: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"add-br-p4 {bridge} {id}")

        def add_vlan_to_bridge(self, bridge, vlan):
            """Add given vlan to bridge or create one if not exist

            :param bridge: an existing bridge
            :type bridge: str
            :param vlan: vlan name to add
            :type vlan: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"add-port {bridge} {vlan}")

            return self.connection.execute_command(cmd)

        def del_port(self, bridge, port):
            """Delete given port on bridge

            :param bridge: bridge to del from
            :type bridge: str
            :param port: port del
            :type port: LinuxAdapter | str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"del-port {bridge} {port}")
            return self.connection.execute_command(cmd)

        def get_ver(self):
            """Get version of OvS

            :return: OvS version
            :rtype: str
            """
            cmd = self.form_cmd("-V")
            out, r_code, error = self.connection.execute_command(cmd)
            if r_code:
                raise ExecuteCMDException("Failed to fetch ovs version.")
            else:
                ver = re.search(r"\d+\.\d+\.\d+", out)
                if ver:
                    return ver.group()
                else:
                    print("Cannot get version of OvS")
                    return ""

        def set_vlan_tag(self, interface, vlan):
            """
            Set VLAN tag on interface

            :param interface: Interface to set the vlan tag
            :type interface: str
            :param vlan: VLAN id to set
            :type vlan: int
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"set port {interface} tag={vlan}")
            return self.connection.execute_command(cmd)

        def set_vlan_trunk(self, interface, vlans):
            """
            Set VLAN tag on interface

            :param interface: Interface to set the vlan trunks
            :type interface: str
            :param vlans: VLAN ids to set
            :type vlans: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"set port {interface} trunks={vlans}")
            return self.connection.execute_command(cmd)

    class _Dpctl(_Cli):
        cmd_prefix = "ovs-dpctl"

        def show(self, bridge=""):
            """ Helps to show linux bridge overall summary

            :param bridge: linux bridge name
            :type bridge: string e.g. br1
            :return: None
            :rtype: None
            """
            if not bridge:
                raise RuntimeError(
                    "Dpctl show require at least one argument - bridge name")
            super(Ovs._Dpctl, self).show(
                bridge)  # pylint: disable=protected-access

        def dump_flows(self, bridge=""):
            """Get all flow entries in datapath dp's flow table.

            :param bridge: bridge to get flows of
            :type bridge: str
            :return: flows output
            :rtype: str
            """
            cmd = self.form_cmd(f"dump-flows {bridge}")
            return self.connection.execute_command(cmd)

    class _Ofctl(_Cli):
        cmd_prefix = "ovs-ofctl"

        def del_flows(self, bridge, port_name):
            """Delete flow entries from the port.

            :param bridge: linux bridge name
            :type bridge: string e.g. br1
            :param port_name: port name
            :type port_name: string e.g. port1
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error
            """
            cmd = self.form_cmd(f"del-flows {bridge} in_port={port_name}")
            return self.connection.execute_command(cmd)

        def dump_flows(self, bridge):
            """
            Dump the flows on the bridge

            :param bridge: bridge name
            :type bridge: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error

            """
            cmd = self.form_cmd(f"dump-flows {bridge}")
            return self.connection.execute_command(cmd)

        def dump_port(self,bridge):
            """
            Dump the port on the bridge

            :param bridge: bridge name
            :type bridge: str
            :return: output, error code and error
            :rtype: tuple e.g. out,e_code,error

            """
            cmd = self.form_cmd(f"dump-ports {bridge}")
            return self.connection.execute_command(cmd)
