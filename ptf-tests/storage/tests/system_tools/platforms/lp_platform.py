# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

import re
import time

from system_tools.config import LpConfig
from system_tools.const import (ACC_INTERNAL_IP, ACC_RPC_PATH, BRIDGE_ADDR,
                                CONTROLLERS_NUMBER, GRPC_CLI, LP_INTERNAL_IP,
                                LP_RPC_PATH, SPDK_BDEV_BASE,
                                SPDK_BDEV_BLOCK_SIZE, SPDK_BDEV_NUM_BLOCKS,
                                SPDK_REP, SPDK_SNQN_BASE, SPDK_VERSION)
from system_tools.errors import CommandException
from system_tools.log import logging
from system_tools.platforms.base_platform import BasePlatform
from system_tools.terminals import DeviceTerminal, SSHTerminal


class LPPlatform(BasePlatform):
    # Link Partner
    def __init__(self, controllers_number=CONTROLLERS_NUMBER):
        super().__init__(SSHTerminal(LpConfig()))
        self.imc_device = DeviceTerminal(self.terminal, "/dev/ttyUSB2")
        self.acc_device = DeviceTerminal(self.terminal, "/dev/ttyUSB0")
        self.controllers_number = controllers_number
        self.acc_rpc_path = ACC_RPC_PATH
        self.lp_rpc_path = LP_RPC_PATH
        self.grpc_cli = GRPC_CLI

    def set(self):
        super().set()
        self.imc_device.login()
        self.acc_device.login()
        self.set_internal_ips()
        self.set_spdk()
        self.set_nvmf_tgt(self.find_spdk_mask())
        self.create_pf_devices()
        self.prepare_acc()
        self.start_ssa()
        self.run_npi_transport()
        self.create_pf_device()

    def create_pf_device(self):
        nvmf_queues = 25
        try:
            logging.ptf_info("Start create pf device")
            self.terminal.execute(
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} nvmf_create_subsystem nqn.2019-07.io.spdk:npi-0.0 -a'"
            )
            time.sleep(5)
            cmd = (
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} --plugin npi nvmf_subsystem_add_listener"
                f" nqn.2019-07.io.spdk:npi-0.0 --trtype NPI --traddr 0.0 --max-qpairs {nvmf_queues}'"
            )
            self.terminal.execute(cmd)
            time.sleep(5)
            self.terminal.execute(
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} bdev_null_create NullPF {SPDK_BDEV_NUM_BLOCKS} {SPDK_BDEV_BLOCK_SIZE}'"
            )
            time.sleep(5)
            self.terminal.execute(
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} nvmf_subsystem_add_ns nqn.2019-07.io.spdk:npi-0.0  NullPF'"
            )
            time.sleep(5)
            logging.ptf_info("End create pf device")
        except CommandException:
            logging.error("Create PF device")

    def run_npi_transport(self):
        try:
            logging.ptf_info("Start run npi transport")
            cmd = (
                f"ssh root@200.1.1.3 'export MEV_NVME_DEVICE_MODE=HW_ACC_WITH_IMC "
                f"&& PYTHONPATH=/opt/ssa/rpc {self.acc_rpc_path} --plugin npi nvmf_create_transport -t npi'"
            )
            self.terminal.execute(cmd)
            logging.ptf_info("End run npi transport")
        except CommandException:
            logging.error("Run NPI transport")

    def start_ssa(self):
        try:
            logging.ptf_info("Start SSA on ACC")
            self.terminal.execute(
                "ssh root@200.1.1.3 'echo 2048 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages'"
            )
            cmd = (
                "ssh root@200.1.1.3 'ssa --logflag dma_utils --logflag npi"
                " --logflag nvme --logflag nvmf --logflag bdev_nvme -B 00:01.1 -m 0xEFFF --wait-for-rpc' &"
            )
            self.terminal.execute(cmd)
            time.sleep(10)
            logging.ptf_info("End start SSA on ACC")
            logging.ptf_info("Start set SSA")
            self.terminal.execute(
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} sock_impl_set_options -iposix --enable-zerocopy-send-server'"
            )
            time.sleep(2)
            self.terminal.execute(
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} iobuf_set_options --small-pool-count 12288 --large-pool-count 6144'"
            )
            time.sleep(2)
            self.terminal.execute(
                f"ssh root@200.1.1.3 '{self.acc_rpc_path} framework_start_init'"
            )
            time.sleep(1)
            logging.ptf_info("End set SSA")
        except CommandException:
            logging.error("Start SSA on ACC")

    def prepare_acc(self):
        try:
            logging.ptf_info("Start prepare acc")
            self.terminal.execute("ssh root@200.1.1.3 modprobe -r qat_lce_cpfxx")
            self.terminal.execute("ssh root@200.1.1.3 modprobe qat_lce_cpfxx")
            cpfdev = self.terminal.execute(
                "ssh root@200.1.1.3 lspci -D -d :1456 | cut -f1 -d' '"
            )
            mdev_uuid = self.terminal.execute(
                "ssh root@200.1.1.3 cat /proc/sys/kernel/random/uuid"
            )
            cmd = (
                f"ssh root@200.1.1.3 'echo {mdev_uuid} >"
                f" /sys/bus/pci/devices/{cpfdev}/mdev_supported_types/lce_cpfxx-mdev/create'"
            )
            self.terminal.execute(cmd)

            path = f"/sys/bus/mdev/devices/{mdev_uuid}/"
            self.terminal.execute(f"ssh root@200.1.1.3 'echo 0 > {path}enable'")
            self.terminal.execute(
                f"ssh root@200.1.1.3 'echo 15 > {path}dma_queue_pairs'"
            )
            self.terminal.execute(
                f"ssh root@200.1.1.3 'echo 15 > {path}cy_queue_pairs'"
            )
            self.terminal.execute(f"ssh root@200.1.1.3 'echo 1 > {path}enable'")

            self.terminal.execute(
                "ssh root@200.1.1.3 dma_sample 0"
            )  # test if DMA works
            self.terminal.execute("ssh root@200.1.1.3 modprobe vfio-pci")
            self.terminal.execute("ssh root@200.1.1.3 sysctl -w vm.nr_hugepages=2048")
            self.terminal.execute(
                "ssh root@200.1.1.3 'echo 8086 1458 > /sys/bus/pci/drivers/vfio-pci/new_id'"
            )
            logging.ptf_info("End prepare acc")
        except CommandException:
            logging.error("ACC prepare commands")

    def create_pf_devices(self):
        try:
            # create devices on lp
            logging.ptf_info("Start create pf devices on lp")
            for i in range(self.controllers_number):
                self.terminal.execute(
                    f"sudo {self.lp_rpc_path} bdev_null_create {SPDK_BDEV_BASE}{i} {SPDK_BDEV_NUM_BLOCKS} {SPDK_BDEV_BLOCK_SIZE}"
                )
                self.terminal.execute(
                    f"sudo {self.lp_rpc_path} nvmf_create_subsystem {SPDK_SNQN_BASE}:{i+1} --allow-any-host --max-namespaces {self.controllers_number}"
                )
                self.terminal.execute(
                    f"sudo {self.lp_rpc_path} nvmf_subsystem_add_ns {SPDK_SNQN_BASE}:{i+1}  {SPDK_BDEV_BASE}{i}"
                )
                cmd = (
                    f"sudo {self.lp_rpc_path} nvmf_subsystem_add_listener"
                    f" {SPDK_SNQN_BASE}:{i + 1} --trtype tcp --traddr 200.1.1.1 --trsvcid 4420"
                )
                self.terminal.execute(cmd)
            logging.ptf_info("End create pf devices on lp")
        except CommandException:
            logging.error("PF devices on lp is not created")

    def set_nvmf_tgt(self, mask):
        # Setup nvmf_tgt and create transport
        nr_hugepages = 2048
        try:

            logging.ptf_info("Start setup nvmf_tgt and create transport")
            self.terminal.execute(f"sudo sysctl -w vm.nr_hugepages={nr_hugepages}")
            time.sleep(0.5)
            self.terminal.execute(
                f"cd spdk && sudo ./build/bin/nvmf_tgt --cpumask {mask} --wait-for-rpc &"
            )
            time.sleep(2)
            self.terminal.execute(
                f"sudo {self.lp_rpc_path} sock_impl_set_options -iposix --enable-zerocopy-send-server"
            )
            time.sleep(2)
            self.terminal.execute(
                f"sudo {self.lp_rpc_path} iobuf_set_options --small-pool-count 12288 --large-pool-count 12288"
            )
            time.sleep(2)
            self.terminal.execute(f"sudo {self.lp_rpc_path} framework_start_init")
            time.sleep(2)
            self.terminal.execute(
                f"sudo {self.lp_rpc_path} nvmf_create_transport --trtype TCP --max-queue-depth=4096 --num-shared-buffers=8191"
            )
            time.sleep(2)
            logging.ptf_info("End setup nvmf_tgt and create transport")
        except CommandException:
            logging.error("Setup nvmf_tgt and create transport")

    def find_spdk_mask(self):
        cpus_to_use = self.get_cpus_to_use()
        logging.ptf_info("Start find mask for SPDK on LP")
        spdk_cpu_mask = 0
        for cpu in cpus_to_use:
            spdk_cpu_mask = self.terminal.execute(
                f"echo $(({spdk_cpu_mask} | (1 << {cpu})))"
            )
        logging.ptf_info("End find mask for SPDK on LP")
        return hex(int(spdk_cpu_mask))

    def set_spdk(self):
        # Download and configure spdk on LP
        try:
            logging.ptf_info("Start set spdk")
            self.terminal.execute(SPDK_REP)
            self.terminal.execute(f"cd spdk && git checkout {SPDK_VERSION}")
            self.terminal.execute("cd spdk && git submodule update --init")
            self._install_spdk_prerequisites()
            logging.ptf_info("End set spdk")
        except CommandException:
            logging.error("Download and configure spdk on LP")

    def _get_network_interfaces_names(self):
        raw = self.terminal.execute("ip a")
        return re.findall("\d: (.*?):", raw)

    def _get_acc_network_interfaces_names(self):
        raw = self.acc_device.execute("ip a", 60)
        return re.findall("\d: (.*?):", raw)

    def _set_acc_ip(self, ip, interface):
        self.acc_device.execute(f"ip a add {ip} dev {interface}")
        self.acc_device.execute(f"ip link set dev {interface} up")

    def _unset_acc_ip(self, ip, interface):
        self.acc_device.execute(f"ip a del {ip} dev {interface}")
        self.acc_device.execute(f"ip link set dev {interface} up")

    def _set_ip(self, ip, interface):
        self.terminal.execute(f"sudo ip a add {ip} dev {interface}")
        self.terminal.execute(f"sudo ip link set dev {interface} up")

    def _unset_ip(self, ip, interface):
        self.terminal.execute(f"sudo ip a del {ip} dev {interface}")
        self.terminal.execute(f"sudo ip link set dev {interface} up")

    def _is_lp_and_acc_ip_correct(self):
        try:
            self.terminal.execute("ping -w 4 -c 2 200.1.1.3")
            return True
        except:
            return False

    def _get_valid_interfaces(self, interfaces):
        return [interface for interface in interfaces if interface != "lo"]

    def set_internal_ips(self):
        logging.ptf_info("Start setting internal ips")
        if self._is_lp_and_acc_ip_correct():
            logging.ptf_info("Internal ips is setting correctly")
            return True
        self.imc_device.execute("python3 /usr/bin/scripts/cfg_acc_apf_x2.py", 10)
        self.acc_device.execute("systemctl stop NetworkManager")
        lp_interfaces = self._get_valid_interfaces(self._get_network_interfaces_names())
        acc_interfaces = self._get_valid_interfaces(
            self._get_acc_network_interfaces_names()
        )
        for lp_interface in lp_interfaces:
            if not self._is_lp_and_acc_ip_correct():
                self._set_ip(LP_INTERNAL_IP, lp_interface)
            for acc_interface in acc_interfaces:
                if not self._is_lp_and_acc_ip_correct():
                    self._set_acc_ip(ACC_INTERNAL_IP, acc_interface)
                if not self._is_lp_and_acc_ip_correct():
                    self._unset_acc_ip(ACC_INTERNAL_IP, acc_interface)
            if not self._is_lp_and_acc_ip_correct():
                self._unset_ip(LP_INTERNAL_IP, lp_interface)
        end = self._is_lp_and_acc_ip_correct()
        if end:
            logging.ptf_info("Internal ips is setting correctly")
        else:
            logging.error("Internal ips is not setting correctly")
        return self._is_lp_and_acc_ip_correct()

    @property
    def sma_port(self):
        return self.config.sma_port

    def clean(self):
        self.terminal.execute("sudo rm -rf spdk")
        return super().clean()


class IPULinkPartnerPlatform(LPPlatform):
    def create_vf_devices(self):
        self.create_subsystems()
        self.create_controllers()
        self.add_remote_controllers()
        self.create_namespaces()

    def set(self):
        super().set()
        self.create_vf_devices()

    def create_subsystems(self):
        try:
            logging.ptf_info("Create subsystems")
            for i in range(1, self.controllers_number + 1):
                self.create_subsystem(i)
            time.sleep(5)
        except CommandException:
            logging.error("Subsystems didn't create")

    def create_subsystem(self, num):
        cmd = f"ssh root@200.1.1.3 {self.acc_rpc_path} nvmf_create_subsystem nqn.2019-07.io.spdk:npi-0.{num} -a"
        self.terminal.execute(cmd)
        time.sleep(2)

    def create_controllers(self):
        try:
            logging.ptf_info("Create controllers")
            for i in range(1, self.controllers_number + 1):
                self.create_controller(i)
            logging.ptf_info("End creating controllers")
        except CommandException:
            logging.error("Controllers didn't create")
        time.sleep(5)

    def create_controller(self, num):
        nvmf_queues = 25
        self.terminal.execute(
            f"ssh root@200.1.1.3 {self.acc_rpc_path} --plugin npi nvmf_subsystem_add_listener -t npi -a 0.{num} nqn.2019-07.io.spdk:npi-0.{num} --max-qpairs {nvmf_queues}"
        )
        time.sleep(2)

    def add_remote_controller(self, num):
        self.terminal.execute(
            f"ssh root@200.1.1.3 {self.acc_rpc_path} bdev_nvme_attach_controller -b Nvme{num} -t TCP -a 200.1.1.1 -f IPv4 -s 4420 -n nqn.2019-06.io.spdk:{num}"
        )
        time.sleep(2)

    def add_remote_controllers(self):
        try:
            logging.ptf_info("Add remote controllers")
            for i in range(1, self.controllers_number + 1):
                self.add_remote_controller(i)
            logging.ptf_info("End adding remote controllers")
        except CommandException:
            logging.error("Remote controllers didn't add")
        time.sleep(5)

    def create_namespace(self, num):
        self.terminal.execute(
            f"ssh root@200.1.1.3 {self.acc_rpc_path} nvmf_subsystem_add_ns nqn.2019-07.io.spdk:npi-0.{num} Nvme{num}n1"
        )
        time.sleep(2)

    def create_namespaces(self):
        time.sleep(10)
        try:
            logging.ptf_info("Create namespaces")
            for i in range(1, self.controllers_number + 1):
                self.create_namespace(i)
            logging.ptf_info("End creating namespaces")
        except CommandException:
            logging.error("Namespaces didn't create")


class OPILinkPartnerPlatform(LPPlatform):
    def __init__(self, controllers_number=CONTROLLERS_NUMBER):
        super().__init__(controllers_number)
        self.bridge_addr = BRIDGE_ADDR

    def install_opi_prerequsites(self):
        logging.ptf_info("Start install opi prerequsites")
        self.terminal.execute(
            f"""sudo {self.pms} install -y git ethtool docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin grpc-cli"""
        )
        logging.ptf_info("End install opi prerequsites")

    def set(self):
        super().set()
        self.install_opi_prerequsites()
        self.setup_socket_connection()
        self.setup_opi_bridge()
        self.create_vf_devices()

    def setup_opi_bridge(self):
        try:
            logging.ptf_info("Start setup opi bridge")
            self.terminal.execute("git clone https://github.com/opiproject/opi-intel-bridge.git")
            filename = "docker-compose.yml"
            with open(f"static/{filename}", "r") as file:
                content = file.read()
                self.terminal.execute(f"""echo '{content}' > opi-intel-bridge/{filename}""")

            # sometimes docker is not running
            cmd = """sudo systemctl start docker"""
            self.terminal.execute(cmd)
            time.sleep(3)

            cmd = """cd opi-intel-bridge && sudo screen -dm bash -c 'docker compose up'"""
            self.terminal.execute(cmd)
            time.sleep(10)
            logging.ptf_info("End setup opi bridge")
        except CommandException:
            logging.ptf_info("Setup opi bridge is failed")

    def setup_socket_connection(self):
        # Setup ssa socket connection between LP ACC
        logging.ptf_info("Start setup ssa socket connection")
        cmd = """sudo rm -f /var/tmp/spdk.ssa.sock ; sudo screen -dm bash -c 'ssh -L /var/tmp/spdk.ssa.sock:/var/tmp/spdk.sock root@200.1.1.3'"""
        self.terminal.execute(cmd)
        logging.ptf_info("End setup ssa socket connection")

    def create_subsystems(self):
        for i in range(1, self.controllers_number + 1):
            self.create_subsystem(i)

    def create_subsystem(self, num):
        cmd = f"""{self.grpc_cli} call --json_input --json_output {self.bridge_addr} CreateNvmeSubsystem "{{nvme_subsystem : {{spec : {{nqn: 'nqn.2019-07.io.spdk:npi-0.{num}', serial_number: 'myserial3', model_number: 'mymodel3', max_namespaces: 11}} }}, nvme_subsystem_id : 'subsystem{num}' }}" """
        self.terminal.execute(cmd)
        time.sleep(1)

    def create_controllers(self):
        for i in range(1, self.controllers_number + 1):
            self.create_controller(i)

    def create_controller(self, num):
        cmd = f"""{self.grpc_cli} call --json_input --json_output {self.bridge_addr} CreateNvmeController "{{parent: '//storage.opiproject.org/subsystems/subsystem{num}', nvme_controller : {{spec : {{nvme_controller_id: {num}, pcie_id : {{physical_function : 0, virtual_function : {num}, port_id: 0 }}, max_nsq:24, max_ncq:24, 'trtype': 'NVME_TRANSPORT_PCIE' }} }}, nvme_controller_id : 'controller{num}'}}" """
        self.terminal.execute(cmd)
        time.sleep(1)

    def create_remote_controllers(self):
        for i in range(1, self.controllers_number + 1):
            self.create_remote_controller(i)

    def create_remote_controller(self, num):
        cmd = f"""{self.grpc_cli} call --json_input --json_output {self.bridge_addr} CreateNvmeRemoteController "{{nvme_remote_controller : {{multipath: 'NVME_MULTIPATH_DISABLE'}}, nvme_remote_controller_id: 'nvmetcp{num}'}}" """
        self.terminal.execute(cmd)
        time.sleep(1)

    def create_paths(self):
        for i in range(1, self.controllers_number + 1):
            self.create_path(i)

    def create_path(self, num):
        cmd = f"""{self.grpc_cli} call --json_input --json_output {self.bridge_addr} CreateNvmePath "{{nvme_path : {{controller_name_ref: '//storage.opiproject.org/volumes/nvmetcp{num}', traddr:'200.1.1.1', trtype:'NVME_TRANSPORT_TCP', fabrics:{{subnqn:'nqn.2019-06.io.spdk:{num}', trsvcid:'4420', adrfam:'NVME_ADRFAM_IPV4'}}}}, nvme_path_id: 'nvmetcp{num}path0'}}" """
        self.terminal.execute(cmd)
        time.sleep(1)

    def create_namespaces(self):
        for i in range(1, self.controllers_number + 1):
            self.create_namespace(i)

    def create_namespace(self, num):
        cmd = f"""{self.grpc_cli} call --json_input --json_output {self.bridge_addr} CreateNvmeNamespace "{{parent: '//storage.opiproject.org/subsystems/subsystem{num}', nvme_namespace : {{spec : {{volume_name_ref : 'nvmetcp{num}n1', 'host_nsid' : '1', uuid:{{value : '1b4e28ba-2fa1-11d2-883f-b9a761bde3fb'}}, nguid: '1b4e28ba-2fa1-11d2-883f-b9a761bde3fb', eui64: 1967554867335598546 }} }}, nvme_namespace_id: 'namespace{num}'}}" """
        self.terminal.execute(cmd)
        time.sleep(1)

    def create_vf_devices(self):
        self.create_subsystems()
        self.create_controllers()
        self.create_remote_controllers()
        self.create_paths()
        self.create_namespaces()

    def clean(self):
        super().clean()
        self.terminal.execute("sudo rm -rf opi-intel-bridge")
