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
util lib for parsing config json files
"""
import json
import os
import yaml
from typing import Union
import common.utils.log as log


def get_config_dict(
    config_json: dict,
    pci_bdf: str = "",
    vm_location_list: str = "",
    vm_cred: str = "",
    client_cred: str = "",
    remote_port: str = "",
) -> dict:
    """util function to convert json config file to dictionary and to add
       extra needed config parameters to dictionary

    expected directory structure:
    P4OVS
    |
    |
    ---- ptf-test
        |
        ---- common
        |   |
        |    | ---- config (JSON files)
        |    | ----- lib (gnmi, p4-ovsctl)
        |    | ----- utils ( parse jason, send/verify traffic, port config,
        |             vm support etc)
        |---- tests
              | -----exact-match.py
              | -----action-selector.py
             | -----port-types.py
             | -----hot-plug.py

    :param config_json:
    :param pci_bdf:
    :return: dictionary --> 'data'

    :param config_json: config json file name
    :type config_json: string e.g. "l2_exact_match_with_tap.json"
    :param pci_bdf: phy links bdf list
    :type pci_bdf: string e.g. '0000:5e:00.0,0000:00:5e.1'
    :param vm_location_list: complete path of vms needed
    :type vm_location_list: string e.g. '/home/VM/vm1.img,/home/VM/vm2.img'
    :param vm_cred: VM login credentials
    :type vm_cred: string
    :param client_cred: Remote host login credentials
    :type client_cred: string e.g. 'hostname,username,password'
    :return: all configuration data needed
    :rtype: dictionary
    """

    config_json = os.sep.join([os.getcwd(), "common", "config", config_json])

    with open(config_json) as config_json:
        data = json.load(config_json)

        port_list, phy_port_list = [], []
        for port in data["port"]:
            port_list.append(port["name"])
            if port["device"] == "physical-device":
                phy_port_list.append(port["name"])
        port_list.sort()

        data["port_list"] = port_list

        if pci_bdf:
            pci_bdf = [x.strip() for x in pci_bdf.split(",")]
            if len(pci_bdf) != len(phy_port_list):
                log.info(
                    f"No of pci bdf must be equal to or less than the no of "
                    f"ports defined in the config json file: "
                    f"{len(data['port_list'])}"
                )
                return None

            for port in data["port"]:
                if port["device"] == "physical-device":
                    port["pci-bdf"] = pci_bdf.pop(0)

        if vm_location_list:
            vm_location_list = [x.strip() for x in vm_location_list.split(",")]
            # Check if no of vms added in json file matches with the no of vms
            # passed in cli
            if len(vm_location_list) != len(data["vm"]):
                log.info(
                    "Mismatch in number of vms mentioned in json to the "
                    "number of vm images mentioned in cli args"
                )
                return None
            data["vm_location_list"] = vm_location_list
            for vm, location in zip(data["vm"], vm_location_list):
                vm["vm_image_location"] = location
                if not vm_cred:
                    vm_cred = "root,password"
                vm["vm_username"], vm["vm_password"] = [
                    x.strip() for x in vm_cred.split(",")
                ]

        if client_cred:
            client_cred = [x.strip() for x in client_cred.split(",")]
            data["client_hostname"] = client_cred[0]
            data["client_username"] = client_cred[1]
            data["client_password"] = client_cred[2]

        if "table" in data.keys():
            for table in data["table"]:
                if "match_action" in table.keys():
                    table["del_action"] = []
                    for match_action in table["match_action"]:
                        table["del_action"].append(
                            match_action.replace(" ", "").split(",action=")[0]
                        )
            for table in data["table"]:
                if "member_details" in table.keys():
                    table["del_member"] = []
                    for member_detail in table["member_details"]:
                        table["del_member"].append(member_detail.split(",")[1])

            for table in data["table"]:
                if "group_details" in table.keys():
                    table["del_group"] = []
                    for group_detail in table["group_details"]:
                        table["del_group"].append(group_detail.split(",")[0])

        if remote_port:
            data["remote_port"] = [x.strip() for x in remote_port.split(",")]

        #######################################################
        # Any future data structure can be added here #########
        #######################################################

        return data


def get_interface_ipv4_dict(data: dict) -> list:
    """
    util function to get a list of dictionary mapping interfaces with its corresponding ip
    :param data: dictionary obtained from config json file
    :return: list of dictionary --> [{"TAP0":"1.1.1.1/24"},
                                    {"TAP1":"2.2.2.2/24"},
                                    ...]
    """
    interface_ip_list: list = []
    for port in data["port"]:
        dev_type = get_device_type(port)
        if dev_type == "tap":
            interface_ip_list.append({port["name"]: port.setdefault("ip", "0.0.0.0")})
        if "control-port" in port:
            interface_ip_list.append(
                {port["control-port"]: port.setdefault("ip", "0.0.0.0")}
            )

    return interface_ip_list


def get_device_type(port: dict) -> Union[str, None]:
    """
    helper function to decide the device type: tap/vhost/link
    :params: port --> dictionary containing port details from json
    :returns: string --> tap / vhost / link
    """
    if port["device"] == "physical-device":
        return "link"
    elif port["device"] == "virtual-device" and port["port-type"] == "LINK":
        return "vhost"
    elif port["device"] == "virtual-device" and port["port-type"] == "TAP":
        return "tap"
    else:
        log.info("Invalid 'device' or 'port-type' in json")
        return None


def get_gnmi_params_simple(data: dict) -> str:
    """
    util function to parse 'data' dictionary and return list of 'params' string for gnmi-cli set/get
    :param data: dictionary obtained from config json file
    :return: list --> list of params
                --> ["device:virtual-device,name:net_vhost0,host:host1,device-type:VIRTIO_NET,queues:1,socket-path:/tmp/vhost-user-0,port-type:LINK",
                "device:virtual-device,name:net_vhost1,host:host2,device-type:VIRTIO_NET,queues:1,socket-path:/tmp/vhost-user-1,port-type:LINK",
                ...]
    """
    common: list = ["device", "name"]
    mandatory: dict = {
        "tap": [],
        "vhost": ["host-name", "device-type", "queues", "socket-path"],
        "link": ["pci-bdf"],
    }
    optional: list = [
        "pipeline-name",
        "mempool-name",
        "control-port",
        "mtu",
        "packet-dir",
    ]

    params: list = []

    for port in data["port"]:
        param = ""
        for field in common:
            param += f"{field}:{port[field]},"

        device_type = get_device_type(port)
        if not device_type:
            return None
        for field in mandatory[device_type]:
            param += f"{field}:{port[field]},"

        for field in optional:
            if field in port.keys():
                param += f"{field}:{port[field]},"

        param += f"port-type:{port['port-type']}"

        params.append(param)

    return params


def get_gnmi_phy_with_ctrl_port(data):
    """
    util function to parse 'data' dictionary and return list of 'params' string for gnmi-cli set/get
    It's an improvement for get_gnmi_params_simple to handel physical port come with control port. It
    strip out control TAP from physical port gnmi param as an individual port to build right target DP index
    :param data: dictionary obtained from config json file
    :return: list --> list of params
                --> ["device:virtual-device,name:net_vhost0,host:host1,device-type:VIRTIO_NET,queues:1,socket-path:/tmp/vhost-user-0,port-type:LINK",
                "device:virtual-device,name:net_vhost1,host:host2,device-type:VIRTIO_NET,queues:1,socket-path:/tmp/vhost-user-1,port-type:LINK",
                'device:physical-device,name:PORT0,pci-bdf:0000:af:00.0,control-port:TAP1,mtu:1500,packet-dir:network,port-type:LINK',
                'device:virtual-device,name:TAP0,pipeline-name:pipe,mempool-name:MEMPOOL0,mtu:1500,packet-dir:host,port-type:TAP'
                ...]
    """
    common = ["device", "name"]
    mandatory = {
        "tap": [],
        "vhost": ["host-name", "device-type", "queues", "socket-path"],
        "link": ["pci-bdf"],
    }
    optional = ["pipeline-name", "mempool-name", "control-port", "mtu", "packet-dir"]

    params = []

    for port in data["port"]:
        param = ""

        device_type = get_device_type(port)
        if not device_type:
            return None

        for field in common:
            param += f"{field}:{port[field]},"

        for field in mandatory[device_type]:
            param += f"{field}:{port[field]},"

        for field in optional:
            if field in port.keys():
                param += f"{field}:{port[field]},"

        param += f"port-type:{port['port-type']}"
        params.append(param)

        # This is a partial gnmi example 'device:physical-device,control-port:TAP1,port-type:LINK'
        # Below code is to strip out 'control-port:TAP1' as an individaul port and build its target ID
        # index at that order.
        if device_type == "link" and port.get("control-port"):
            ctrl_port = f"device:virtual-device,name:{port['control-port']},"

            for field in optional:
                if field in port.keys():
                    if field != "control-port" or field != "packet-dir":
                        ctrl_port += f"{field}:{port[field]},"

            ctrl_port += f"packet-dir:host:port-type:TAP"
            params.append(ctrl_port)

    return params


def get_gnmi_params_hotplug(data: dict, action: str = "add") -> Union[list, None]:
    """
    util function to parse 'data' dictionary and return a list of 'params' string for gnmi-cli set for hotplug
    :param data: dictionary obtained from config json file
    :action "add" or "del" ... based on what we are trying to do, add a hot plug port or delete one
    :return: list --> list of params
                --> ["device:virtual-device,name:net_vhost0,hotplug:add,
                  qemu-socket-ip:127.0.0.1,qemu-socket-port:6555,
                  qemu-vm-mac:00:e8:ca:11:aa:01,qemu-vm-netdev-id:netdev0,
                  qemu-vm-chardev-id:char1,native-socket-path:/tmp/intf/vhost-user-0,
                  qemu-vm-device-id:dev0",
                  ...]
    """
    data = create_port_vm_map(data)
    if action.lower() not in ["add", "del"]:
        log.info(
            f"get_gnmi_params_hotplug: Expected 'action' as 'add' or 'del', got {action}"
        )
        return None

    common = ["device", "name"]
    mandatory = [
        "qemu-socket-ip",
        "qemu-socket-port",
        "qemu-vm-mac-address",
        "qemu-vm-netdev-id",
        "qemu-vm-chardev-id",
        "native-socket-path",
        "qemu-vm-device-id",
    ]
    optional = []  # TBD
    params = []
    for port in data["port"]:
        param = ""
        device_type = get_device_type(port)
        if not device_type:
            return None
        if device_type != "vhost" or "qemu-hotplug-mode" not in port.keys():
            continue

        for field in common:
            param += f"{field}:{port[field]},"
        if action == "del":
            param += "qemu-hotplug-mode:del"
            params.append(param)
            continue
        param += "qemu-hotplug-mode:add,"

        for field in mandatory:
            param += f"{field}:{port['qemu-hotplug-mode'][field]},"

        for field in optional:
            if field in port.keys():
                param += f"{field}:{port['qemu-hotplug-mode'][field]},"

        param = ",".join(param.split(",")[:-1])

        params.append(param)

    if not params:
        log.info("No vhost port mentioned as 'qemu-hotplug-mode' in json")

    return params


def create_port_vm_map(data: dict) -> dict:
    """
    create vhost port and corresponding vm mapping
    adds vm specific entried to its corresponding port dictionary
    """
    for port in data["port"]:
        for vm in data["vm"]:
            if vm["port"] == port["name"]:
                for k in vm.keys():
                    port[k] = vm[k]
    return data


def get_interface_ipv4_dict_hotplug(data, interfaces):
    """
    util function to get a list of dictionary mapping hotplugged interfaces with its corresponding ip
    :param data: dictionary obtained from config json file
    :param interfaces: list of hutplugged interfaces
    :return: list of dictionary --> [{"ens4":"1.1.1.1/24"},
                                    {"ens5":"2.2.2.2/24"},
                                    ...]
    """
    interface_ip_list: list = []
    ips: list = []
    for port in data["port"]:
        device_type = get_device_type(port)
        if not device_type:
            return None
        if device_type != "vhost" or "qemu-hotplug-mode" not in port.keys():
            continue
        ips.append(port.setdefault("ip", "0.0.0.0"))

    for interface, ip in zip(interfaces, ips):
        interface_ip_list.append({interface: ip})

    return interface_ip_list


def get_interface_ipv4_route_dict_hotplug(interface_ip_list: list) -> list:
    interface_ipv4_route_list = []
    for interface_ip in interface_ip_list:
        for interface, ip in interface_ip.items():
            ip = ".".join(ip.split(".")[:-1]) + ".0"
            interface_ipv4_route_list.append({interface: ip})

    return interface_ipv4_route_list


def get_interface_mac_dict_hotplug(data: dict, interfaces: list) -> list:
    """
    util function to get a list of dictionary mapping hotplugged interfaces with its corresponding mac
    :param data: dictionary obtained from config json file
    :param interfaces: list of hutplugged interfaces
    :return: list of dictionary --> [{"ens4":"00:e8:ca:11:aa:01"},
                                    {"ens5":"00:e8:ca:11:aa:02"},
                                    ...]
    """
    interface_mac_list = []
    macs = []
    for port in data["port"]:
        device_type = get_device_type(port)
        if not device_type:
            return None
        if device_type != "vhost" or "qemu-hotplug-mode" not in port.keys():
            continue
        macs.append(port["qemu-hotplug-mode"].setdefault("qemu-vm-mac-address", ""))

    for interface, mac in zip(interfaces, macs):
        interface_mac_list.append({interface: mac})

    return interface_mac_list


def convert_yaml_to_dict_from_file(yaml_file):
    """
    util function to convert yaml file to python dictionary
    params:
    yaml_file: string

    returns:
    ret: dictionary
    """
    with open(yaml_file, "r") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)

    return data
