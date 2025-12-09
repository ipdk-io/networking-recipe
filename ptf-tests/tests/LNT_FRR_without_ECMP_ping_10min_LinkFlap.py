# Copyright (c) 2022 Intel Corporation.
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
LNT FRR without ECMP sending 10 minutes ping to 4VM on 2Hosts along with flaping ports
"""

# in-built module imports
import time
import sys
from datetime import datetime, timedelta

# Unittest related imports
import unittest

# ptf related imports
from ptf.base_tests import BaseTest
from ptf.testutils import *

# framework related imports
import common.utils.log as log
import common.utils.p4rtctl_utils as p4rt_ctl
import common.utils.test_utils as test_utils
import common.utils.ovs_utils as ovs_utils
import common.utils.gnmi_ctl_utils as gnmi_ctl_utils
from common.utils.config_file_utils import (
    get_config_dict,
    get_gnmi_params_simple,
    get_interface_ipv4_dict,
    get_gnmi_phy_with_ctrl_port,
)
from common.lib.telnet_connection import connectionManager


class LNT_FRR_without_ECMP_ping_10min_flap(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        test_params = test_params_get()
        config_json = test_params["config_json"]

        try:
            self.vm_cred = test_params["vm_cred"]
        except KeyError:
            self.vm_cred = ""

        self.config_data = get_config_dict(
            config_json,
            pci_bdf=test_params["lnt_pci_bdf"],
            vm_location_list=test_params["vm_location_list"],
            vm_cred=self.vm_cred,
            client_cred=test_params["client_cred"],
            remote_port=test_params["remote_port"],
        )
        self.gnmictl_params = get_gnmi_params_simple(self.config_data)
        # in the case of a physical port configured with a control port
        self.gnmictl_phy_ctrl_params = get_gnmi_phy_with_ctrl_port(self.config_data)
        self.tap_port_list = gnmi_ctl_utils.get_tap_port_list(self.config_data)
        self.link_port_list = gnmi_ctl_utils.get_link_port_list(self.config_data)
        self.interface_ip_list = get_interface_ipv4_dict(self.config_data)
        self.conn_obj_list = []

    def runTest(self):
        # Create ports using gnmi-ctl
        if not gnmi_ctl_utils.gnmi_ctl_set_and_verify(self.gnmictl_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi ctl ports")

        # Prepare frr service
        log.info(f"verify if frr is intalled and running")
        # Prepare frr service. "restart" doesn't work well
        if not test_utils.run_frr_service("stop"):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to stop frr service on local host")
        if not test_utils.run_frr_service(
            "stop",
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to stop frr service on {self.config_data['client_hostname']}"
            )
        time.sleep(5)
        log.info(f"Begin to verify if frr is intalled and running")
        if not test_utils.run_frr_service("start"):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to start frr service on local host")
        if not test_utils.run_frr_service(
            "start",
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to start frr service on {self.config_data['client_hostname']}"
            )

        # Create VMs
        result, vm_name = test_utils.vm_create(self.config_data["vm_location_list"])
        if not result:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"VM creation failed for {vm_name}")

        # Create telnet instance for VMs
        self.conn_obj_list = []
        vm_cmd_list = []
        vm_id = 0
        for vm, port in zip(self.config_data["vm"], self.config_data["port"]):
            globals()["conn" + str(vm_id + 1)] = connectionManager(
                "127.0.0.1", f"655{vm_id}", vm["vm_username"], vm["vm_password"]
            )
            self.conn_obj_list.append(globals()["conn" + str(vm_id + 1)])
            globals()["vm" + str(vm_id + 1) + "_command_list"] = [
                f"ip addr add {port['ip']} dev {port['interface']}",
                f"ip link set dev {port['interface']} up",
                f"ip link set dev {port['interface']} address {port['mac_local']}",
            ]
            vm_cmd_list.append(globals()["vm" + str(vm_id + 1) + "_command_list"])
            vm_id += 1

        # Configuring VMs
        for i in range(len(self.conn_obj_list)):
            log.info("Configuring VM....")
            test_utils.configure_vm(self.conn_obj_list[i], vm_cmd_list[i])

        # Bring up TAP ports
        if not gnmi_ctl_utils.ip_set_dev_up(self.tap_port_list[0]):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to bring up {self.tap_port_list[0]}")

        log.info("Begin to configure frr on local host")
        # Bring up local tep interfacce
        if not gnmi_ctl_utils.iplink_add_dev(
            self.config_data["vxlan"]["tep_intf"], "dummy"
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to add dev {self.config_data['vxlan']['tep_intf']} type dummy"
            )

        # Configure local tep interface in frr
        if not test_utils.vtysh_config_frr_router_interface(
            self.config_data["vxlan"]["tep_intf"],
            ip=self.config_data["vxlan"]["tep_ip"][0],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to config frr inteface {self.config_data['vxlan']['tep_intf']} on local"
            )
        # Configure local port in frr
        if not test_utils.vtysh_config_frr_router_interface(
            self.config_data["ecmp"]["local_ports"][0],
            ip=self.config_data["ecmp"]["local_ports_ip"][0],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to config frr inteface {self.config_data['ecmp']['local_ports_ip'][0]} on local"
            )

        # Configure local bgp attributes
        if not test_utils.vtysh_config_frr_bgp_attr(
            self.config_data["bgp"]["as_number"],
            neighbor=self.config_data["ecmp"]["remote_ports_ip"][0].split("/")[0],
            addr_family=self.config_data["bgp"]["addr_family"],
            network=self.config_data["vxlan"]["tep_ip"][0],
            router_id=self.config_data["vxlan"]["tep_ip"][0].split("/")[0],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to config Bgp on local")

        # Generate p4c artifact and create binary by using tdi pna arch
        if not test_utils.gen_dep_files_p4c_dpdk_pna_tdi_pipeline_builder(
            self.config_data
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        # Run Set-pipe command for set pipeline
        if not p4rt_ctl.p4rt_ctl_set_pipe(
            self.config_data["switch"],
            self.config_data["pb_bin"],
            self.config_data["p4_info"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")

        # Create bridge
        if not ovs_utils.add_bridge_to_ovs(self.config_data["bridge"]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add bridge {self.config_data['bridge']} to ovs")

        # Bring up bridge
        if not gnmi_ctl_utils.ip_set_dev_up(self.config_data["bridge"]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']}")

        # Configure VXLAN
        log.info(f"Configure VXLAN ")
        if not ovs_utils.add_vxlan_port_to_ovs(
            self.config_data["bridge"],
            self.config_data["vxlan"]["vxlan_name"][0],
            self.config_data["vxlan"]["tep_ip"][0].split("/")[0],
            self.config_data["vxlan"]["tep_ip"][1].split("/")[0],
            self.config_data["vxlan"]["dst_port"][0],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to add vxlan {self.config_data['vxlan']['vxlan_name'][0]} to bridge {self.config_data['bridge']}"
            )

        # Configure VLAN
        for i in range(len(self.conn_obj_list)):
            id = self.config_data["port"][i]["vlan"]
            vlanname = "vlan" + id
            # Add vlan to TAP0, e.g. ip link add link TAP0 name vlan1 type vlan id 1
            if not gnmi_ctl_utils.iplink_add_vlan_port(
                id, vlanname, self.tap_port_list[0]
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add vlan {vlanname} to {self.tap_port_list[0]}")

            # Add vlan to the bridge, e.g. ovs-vsctl add-port br-int vlan1
            if not ovs_utils.add_vlan_to_bridge(self.config_data["bridge"], vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(
                    f"Failed to add vlan {vlanname} to {self.config_data['bridge']}"
                )

            # Bring up vlan
            if not gnmi_ctl_utils.ip_set_dev_up(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to bring up {vlanname}")

        # Program rules
        log.info(f"Program rules")
        for table in self.config_data["table"]:
            log.info(f"Scenario : {table['description']}")
            log.info(f"Adding {table['description']} rules")
            for match_action in table["match_action"]:
                if not p4rt_ctl.p4rt_ctl_add_entry(
                    table["switch"], table["name"], match_action
                ):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to add table entry {match_action}")

        # Configure remote host
        log.info(
            f"Begin to configure standard OVS on remote host {self.config_data['client_hostname']}"
        )
        # Add bridge to ovs
        if not ovs_utils.add_bridge_to_ovs(
            self.config_data["bridge"],
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            passwd=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to add bridge {self.config_data['bridge']} to \
                     ovs {self.config_data['bridge']} on {self.config_data['client_hostname']}"
            )

        # Bring up the bridge
        if not gnmi_ctl_utils.ip_set_dev_up(
            self.config_data["bridge"],
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to bring up {self.config_data['bridge']}")

        # Create ip netns VMs
        for namespace in self.config_data["net_namespace"]:
            log.info(
                f"creating namespace {namespace['name']} on {self.config_data['client_hostname']}"
            )
            if not test_utils.create_ipnetns_vm(
                namespace,
                remote=True,
                hostname=self.config_data["client_hostname"],
                username=self.config_data["client_username"],
                password=self.config_data["client_password"],
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(
                    f"Failed to add VM namesapce {namespace['name']} on on {self.config_data['client_hostname']}"
                )
            # Add local port to ovs
            if not ovs_utils.add_port_to_ovs(
                self.config_data["bridge"],
                namespace["peer_name"],
                remote=True,
                hostname=self.config_data["client_hostname"],
                username=self.config_data["client_username"],
                password=self.config_data["client_password"],
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(
                    f"Failed to add port {namespace['peer_name']} to bridge {self.config_data['bridge']}"
                )
        # Configure remote vxlan
        log.info(
            f"Configure vxlan port on remote host on {self.config_data['client_hostname']}"
        )
        # Add xlan port to ovs
        if not ovs_utils.add_vxlan_port_to_ovs(
            self.config_data["bridge"],
            self.config_data["vxlan"]["vxlan_name"][0],
            self.config_data["vxlan"]["tep_ip"][1].split("/")[0],
            self.config_data["vxlan"]["tep_ip"][0].split("/")[0],
            self.config_data["vxlan"]["dst_port"][0],
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to add vxlan {self.config_data['vxlan']['vxlan_name'][0]} to \
                         bridge {self.config_data['bridge']} on on {self.config_data['client_hostname']}"
            )

        # Configure remote tep interface
        log.info(
            f"add {self.config_data['vxlan']['tep_intf']} on remote {self.config_data['client_hostname']}"
        )
        if not gnmi_ctl_utils.iplink_add_dev(
            self.config_data["vxlan"]["tep_intf"],
            "dummy",
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to add {self.config_data['vxlan']['tep_intf']}")

        # Configure remote frr
        log.info(
            "Begin to configure frr on remote {self.config_data['client_hostname']}"
        )
        # Configure remote frr router interface 1
        if not test_utils.vtysh_config_frr_router_interface(
            self.config_data["vxlan"]["tep_intf"],
            ip=self.config_data["vxlan"]["tep_ip"][1],
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to config frr inteface {self.config_data['vxlan']['tep_intf']} on {self.config_data['client_hostname']}"
            )

        # Configure remote frr router interface 2
        if not test_utils.vtysh_config_frr_router_interface(
            self.config_data["remote_port"][0],
            ip=self.config_data["ecmp"]["remote_ports_ip"][0],
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to config frr inteface {self.config_data['ecmp']['remote_ports_ip'][0]} on {self.config_data['client_hostname']}"
            )
        # Configure remote frr bgp attributes
        if not test_utils.vtysh_config_frr_bgp_attr(
            self.config_data["bgp"]["as_number"],
            neighbor=self.config_data["ecmp"]["local_ports_ip"][0].split("/")[0],
            addr_family=self.config_data["bgp"]["addr_family"],
            network=self.config_data["vxlan"]["tep_ip"][1],
            router_id=self.config_data["vxlan"]["tep_ip"][1].split("/")[0],
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to config Bgp on {self.config_data['client_hostname']}")

        # Check local host bgp route
        log.info("Chek if bgp route is built on local host")
        m, j = 15, 0
        while j <= m:
            if not test_utils.check_bgp_route():
                time.sleep(12)
                j += 1
            else:
                break
        if j > m:
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"FAIL: bgp route is not built on locahost after {j} tries")

        # Check remote host bgp route
        log.info(
            f"Chek if bgp route is built on remote host {self.config_data['client_hostname']}"
        )
        m, j = 15, 0
        while j <= m:
            if not test_utils.check_bgp_route(
                remote=True,
                hostname=self.config_data["client_hostname"],
                username=self.config_data["client_username"],
                password=self.config_data["client_password"],
            ):
                time.sleep(12)
                j += 1
            else:
                break

        if j > m:
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"FAIL: bgp route is not built on remote {self.config_data['client_hostname']} afer {j} tries"
            )

        # Sleep for system ready to send traffic
        log.info("Sleep before sending ping traffic")
        time.sleep(15)
        # Execute underlay ping
        log.info(f"Ping test for underlay network")
        for ip in self.config_data["ecmp"]["remote_ports_ip"]:
            ping_cmd = f"ping {ip.split('/')[0]} -c 10"
            if not test_utils.local_ping(ping_cmd):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f'FAIL: Ping "{ping_cmd}" test failed for underlay network')
            else:
                log.passed(f"{ping_cmd} succeed")

        # ping remote tep
        ping_cmd = f"ping {self.config_data['vxlan']['tep_ip'][1].split('/')[0]} -c 10"
        if not test_utils.local_ping(ping_cmd):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f'FAIL: Ping  "{ping_cmd}" test failed for underlay network')
        else:
            log.passed(f"{ping_cmd} succeed")

        j = 1
        # Define execution window time
        end_time = datetime.now() + timedelta(minutes=10)
        while end_time >= datetime.now():
            log.info(f"The {j} loop ping within 10 min")
            for i in range(len(self.conn_obj_list)):
                for flap in self.config_data["traffic"]["flap_order"]:
                    # Execute ping
                    log.info(f"Start ping test from VM{i} to other VMs")
                    if flap == "up":
                        num = self.config_data["traffic"]["number_pkts"][0]
                        log.info(
                            f"Bring UP interface {self.config_data['port'][i]['interface']} on VM{i}"
                        )
                        # Bring up port
                        if not test_utils.vm_change_port_status(
                            self.conn_obj_list[i],
                            self.config_data["port"][i]["interface"],
                            "up",
                        ):
                            self.result.addFailure(self, sys.exc_info())
                            self.fail(
                                f"FAIL: failed to bring up {self.config_data['port'][i]['interface']} on VM{i}"
                            )
                    if flap == "down":
                        num = 0
                        log.info(
                            f"Bring DOWN interface {self.config_data['port'][i]['interface']} on VM{i}"
                        )
                        # Bring down port
                        if not test_utils.vm_change_port_status(
                            self.conn_obj_list[i],
                            self.config_data["port"][i]["interface"],
                            "down",
                        ):
                            self.result.addFailure(self, sys.exc_info())
                            self.fail(
                                f"FAIL: failed to bring dwon {self.config_data['port'][i]['interface']} on VM{i}"
                            )
                    # Execute ping among VMs
                    for m in range(len(self.config_data["vm"][i]["remote_ip"])):
                        ip = self.config_data["vm"][i]["remote_ip"][m]

                        if m == 0:
                            log.info(
                                f"Verify local ping {ip} on VM{i}"
                            )  # The 1st IP is local VM ip

                        if m != 0:  # ping traffic is sent acroos physical link
                            log.info(f"Verify remote ping {ip} traffic from VM{i}")
                            log.info(
                                "Record host physical port counter before sending traffic"
                            )
                            # Record physical port counter before sending traffic
                            send_count_list_before = []
                            for send_port_id in self.config_data["traffic"][
                                "send_port"
                            ]:
                                # get physical port counter
                                send_cont = gnmi_ctl_utils.gnmi_get_params_counter(
                                    self.gnmictl_phy_ctrl_params[send_port_id]
                                )
                                if not send_cont:
                                    log.info(
                                        f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}"
                                    )
                                    self.result.addFailure(self, sys.exc_info())
                                    self.fail(f"FAIL: unable to get send counter")
                                log.passed(
                                    f"The port {self.gnmictl_phy_ctrl_params[send_port_id].split(',')[1].split(':')[1]} "
                                    + "counter is built successfully"
                                )
                                send_count_list_before.append(send_cont)

                        time.sleep(5)
                        if flap == "up":
                            n, max = 1, 2
                            while n <= max:
                                # Execute ping when port is up
                                if not test_utils.vm_ping_less_than_100_loss(
                                    self.conn_obj_list[i], ip
                                ):
                                    log.info(
                                        f"The {n} ping failed. will try one more time"
                                    )
                                    n += 1
                                else:
                                    break
                            if n > max:
                                self.result.addFailure(self, sys.exc_info())
                                self.fail(
                                    f"FAIL: after {n} try,ping test failed for VM{i}"
                                )
                        if flap == "down":
                            num = 0
                            # Execute ping when port is down
                            if not test_utils.vm_to_vm_ping_drop_test(
                                self.conn_obj_list[i], ip
                            ):
                                self.result.addFailure(self, sys.exc_info())
                                self.fail(
                                    f"FAIL: expect ping failure but success for VM{i}"
                                )
                        if m != 0:
                            log.info(
                                "Record host physical port counter before sending traffic"
                            )

                            # Record physical port counter after sending traffic
                            send_count_list_after = []
                            for send_port_id in self.config_data["traffic"][
                                "send_port"
                            ]:
                                send_cont = gnmi_ctl_utils.gnmi_get_params_counter(
                                    self.gnmictl_phy_ctrl_params[send_port_id]
                                )
                                if not send_cont:
                                    log.info(
                                        f"FAIL: unable to get counter of {self.config_data['port'][send_port_id]['name']}"
                                    )
                                    self.result.addFailure(self, sys.exc_info())
                                    self.fail(f"FAIL: unable to get send counter")
                                log.passed(
                                    f"The port {self.gnmictl_phy_ctrl_params[send_port_id].split(',')[1].split(':')[1]} "
                                    + "counter is built successfully"
                                )
                                send_count_list_after.append(send_cont)

                            # Check if pkts are forwarded on physical port
                            counter_type = "out-unicast-pkts"
                            stat_total = 0
                            for send_count_before, send_count_after in zip(
                                send_count_list_before, send_count_list_after
                            ):
                                stat = test_utils.compare_counter(
                                    send_count_after, send_count_before
                                )
                                if not stat[counter_type] >= 0:
                                    self.result.addFailure(self, sys.exc_info())
                                    self.fail(
                                        f"FAIL: Packets are not forwarded on physical port"
                                    )
                                stat_total = stat_total + stat[counter_type]

                            if stat_total >= num:
                                log.passed(
                                    f"Minimum {num} packets transmitted and {stat_total} recorded on the path"
                                )
                            else:
                                self.result.addFailure(self, sys.exc_info())
                                self.fail(
                                    f"FAIL: {num} packets expected but only {stat_total} transmitted"
                                )
            j += 1
        # Closing telnet session
        log.info(f"close VM telnet session")
        for conn in self.conn_obj_list:
            conn.close()

    def tearDown(self):
        # Unconfigure local host
        log.info("Unconfiguration on local host")
        log.info(f"Delete match action rules on local host")
        for table in self.config_data["table"]:
            log.info(f"Deleting {table['description']} rules")
            for del_action in table["del_action"]:
                p4rt_ctl.p4rt_ctl_del_entry(table["switch"], table["name"], del_action)

        # Delete local VLAN
        log.info(f"Delete vlan on local host")
        for i in range(len(self.conn_obj_list)):
            id = self.config_data["port"][i]["vlan"]
            vlanname = "vlan" + id
            if not gnmi_ctl_utils.iplink_del_port(vlanname):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {vlanname}")

        # Delete local tep interface
        log.info(f"Delete TEP interface")
        if not gnmi_ctl_utils.iplink_del_port(self.config_data["vxlan"]["tep_intf"]):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete {self.config_data['vxlan']['tep_intf']}")

        log.info("Unconfiguration on remote host")
        log.info(f"Delete ip netns on remote host")
        for namespace in self.config_data["net_namespace"]:
            # delete remote veth_host_vm port
            if not gnmi_ctl_utils.iplink_del_port(
                namespace["peer_name"],
                remote=True,
                hostname=self.config_data["client_hostname"],
                username=self.config_data["client_username"],
                passwd=self.config_data["client_password"],
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete {namespace['peer_name']}")
            # delete name space
            if not test_utils.del_ipnetns_vm(
                namespace,
                remote=True,
                hostname=self.config_data["client_hostname"],
                username=self.config_data["client_username"],
                password=self.config_data["client_password"],
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(
                    f"Failed to delete VM namesapce {namespace['name']} on {self.config_data['client_hostname']}"
                )

        # delete remote bridge
        if not ovs_utils.del_bridge_from_ovs(
            self.config_data["bridge"],
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            passwd=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to delete bridge {self.config_data['bridge']} on {self.config_data['client_hostname']}"
            )

        # delete tep
        if not gnmi_ctl_utils.iplink_del_port(
            self.config_data["vxlan"]["tep_intf"],
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            passwd=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to delete TEP1")

        # Delete remote host Ip
        remote_port_list = self.config_data["remote_port"]
        remote_port_ip_list = self.config_data["ecmp"]["remote_ports_ip"]
        for remote_port, remote_port_ip in zip(remote_port_list, remote_port_ip_list):
            if not gnmi_ctl_utils.ip_del_addr(
                remote_port,
                remote_port_ip,
                remote=True,
                hostname=self.config_data["client_hostname"],
                username=self.config_data["client_username"],
                passwd=self.config_data["client_password"],
            ):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to delete ip {remote_port_ip} on {remote_port}")

        log.info("Clean up frr configuration")
        if not test_utils.run_frr_service("stop"):
            self.result.addFailure(self, sys.exc_info())
            self.fail(f"Failed to stop frr service on local host")

        if not test_utils.run_frr_service(
            "stop",
            remote=True,
            hostname=self.config_data["client_hostname"],
            username=self.config_data["client_username"],
            password=self.config_data["client_password"],
        ):
            self.result.addFailure(self, sys.exc_info())
            self.fail(
                f"Failed to stop frr service on {self.config_data['client_hostname']}"
            )

        if self.result.wasSuccessful():
            log.info("Test has PASSED")
        else:
            log.info("Test has FAILED")
