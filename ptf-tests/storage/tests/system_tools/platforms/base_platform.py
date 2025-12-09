# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

from system_tools.config import DockerConfig
from system_tools.const import NODE
from system_tools.errors import CommandException, MissingDependencyException
from system_tools.log import logging


class BasePlatform:
    """A base class used to represent operating system with needed libraries"""

    def __init__(self, terminal):
        self.terminal = terminal
        self.config = self.terminal.config
        self.pms = "dnf" if self._is_dnf() else "apt-get" if self._is_apt() else None
        self.system = self._os_system()

    def set(self):
        self._install_kernel_headers()
        self.change_cpu_performance_scaling()
        if not self._is_docker():
            self.install_docker()
            self.set_docker()

    def change_cpu_performance_scaling(self):
        try:
            logging.ptf_info("Start changing cpu performance")
            for i in range(int(self.terminal.execute("nproc"))):
                freq = self.terminal.execute(
                    f"cat /sys/devices/system/cpu/cpu{i}/cpufreq/cpuinfo_max_freq"
                )
                scaling_max_freq = (
                    f"/sys/devices/system/cpu/cpu{i}/cpufreq/scaling_max_freq"
                )
                self.terminal.execute(f"echo -e '{freq}' | sudo tee {scaling_max_freq}")
                scaling_min_freq = (
                    f"/sys/devices/system/cpu/cpu{i}/cpufreq/scaling_min_freq"
                )
                self.terminal.execute(f"echo -e '{freq}' | sudo tee {scaling_min_freq}")

                scaling_governor = (
                    f"/sys/devices/system/cpu/cpu{i}/cpufreq/scaling_governor"
                )
                self.terminal.execute(
                    f"echo -e performance | sudo tee {scaling_governor}"
                )
                energy = f"/sys/devices/system/cpu/cpu{i}/cpufreq/energy_performance_preference"
                self.terminal.execute(f"echo -e performance | sudo tee {energy}")
            logging.ptf_info("End changing cpu performance")
        except CommandException:
            logging.error("If permission denied or file not exist is ok")

    def _install_kernel_headers(self):
        logging.ptf_info("Start installing kernel headers")
        raw = self.terminal.execute(f"sudo {self.pms} install -y kernel-headers")
        logging.ptf_info("Kernel headers installed")
        return raw

    def get_cpus_to_use(self):
        logging.ptf_info("Start get cpus to use")
        node_cpus = self.terminal.execute(
            f"numactl --cpunodebind {NODE} --show | grep physcpubind"
        ).split()
        return [int(cpu) for cpu in node_cpus[1:17]]  # first 16 cpus

    def get_ip_address(self):
        return self.config.ip_address

    def get_storage_dir(self):
        return self.config.storage_dir

    def _os_system(self) -> str:
        return self.terminal.execute("sudo cat /etc/os-release | grep ^ID=")[3:]

    def _is_dnf(self) -> bool:
        _, stdout, _ = self.terminal.client.exec_command(f"dnf --version")
        return not stdout.channel.recv_exit_status()

    def _is_apt(self) -> bool:
        _, stdout, _ = self.terminal.client.exec_command("apt-get --version")
        return not stdout.channel.recv_exit_status()

    def _is_docker(self) -> bool:
        _, stdout, _ = self.terminal.client.exec_command("sudo docker --version")
        return not stdout.channel.recv_exit_status()

    def _is_virtualization(self) -> bool:
        """Checks if VT-x/AMD-v support is enabled in BIOS"""

        expectations = ["vt-x", "amd-v", "full"]
        out = self.terminal.execute("lscpu | grep -i virtualization")
        for allowed_str in expectations:
            if allowed_str.upper() in out.upper():
                return True
        return False

    def _is_kvm(self) -> bool:
        """Checks if kvm modules are loaded"""

        expectations = ["kvm_intel", "kvm_amd"]
        out = self.terminal.execute("lsmod | grep -i kvm")
        for allowed_str in expectations:
            if allowed_str.upper() in out.upper():
                return True
        return False

    def _set_security_policies(self) -> bool:
        cmd = (
            "sudo setenforce 0"
            if self.system == "fedora"
            else "sudo systemctl stop apparmor"
        )
        _, stdout, stderr = self.terminal.client.exec_command(cmd)
        return (
            "disabled" in stdout.read().decode() or "disabled" in stderr.read().decode()
        )

    def install_docker(self):
        logging.ptf_info("Start installing docker")
        response = self.terminal.execute(f"sudo {self.pms} install -y docker")
        logging.ptf_info("Docker is installed")
        return response

    def set_docker(self):
        logging.ptf_info("Start setting docker service")
        docker_config = DockerConfig()
        filepath = "/etc/systemd/system/docker.service.d/http-proxy.conf"
        self.terminal.execute("sudo mkdir -p /etc/systemd/system/docker.service.d")
        # proxies
        env = (
            f"""[Service]\n"""
            f"""Environment="HTTP_PROXY="{docker_config.http_proxy}"\n"""
            f"""Environment="HTTPS_PROXY={docker_config.https_proxy}"\n"""
            f"""Environment="http_proxy={docker_config.http_proxy}"\n"""
            f"""Environment="https_proxy={docker_config.https_proxy}"\n"""
            f"""Environment="FTP_PROXY={docker_config.ftp_proxy}"\n"""
            f'''Environment="ftp_proxy={docker_config.ftp_proxy}"'''
        )
        self.terminal.execute(f"""echo -e '{env}' | sudo tee {filepath}""")
        self.terminal.execute("sudo systemctl daemon-reload")
        self.terminal.execute("sudo systemctl restart docker")
        # cgroups
        try:
            self.terminal.execute("sudo mkdir /sys/fs/cgroup/systemd")
            self.terminal.execute(
                "sudo mount -t cgroup -o none,name=systemd cgroup /sys/fs/cgroup/systemd"
            )
        except CommandException:
            pass

        logging.ptf_info("Docker service is setting")

    def _install_spdk_prerequisites(self):
        logging.ptf_info("Install spdk prerequisites")
        self.terminal.execute("cd spdk && sudo ./scripts/pkgdep.sh")
        self.terminal.execute("cd spdk && sudo ./configure --with-vfio-user")
        self.terminal.execute("cd spdk && sudo make")
        logging.ptf_info("spdk prerequisites installed")

    def _create_transports(self):
        logging.ptf_info("Create transports")
        directory = "cd spdk/scripts/"
        cmd1 = "./rpc.py -s /var/tmp/spdk2.sock nvmf_create_transport -t tcp"
        cmd2 = "./rpc.py -s /var/tmp/spdk2.sock nvmf_create_transport -t vfiouser"
        cmd3 = "./rpc.py nvmf_create_transport -t tcp"
        cmd4 = "./rpc.py nvmf_create_transport -t vfiouser"
        self.terminal.execute(f"cd {directory} && sudo {cmd1}")
        self.terminal.execute(f"cd {directory} && sudo {cmd2}")
        self.terminal.execute(f"cd {directory} && sudo {cmd3}")
        self.terminal.execute(f"cd {directory} && sudo {cmd4}")

    def check_system_setup(self):
        """Overwrite this method in specific platform if you don't want check all setup"""
        if not self._is_virtualization():
            raise MissingDependencyException("Virtualization may not be set properly")
        if not self._is_kvm():
            raise MissingDependencyException("KVM may not be set properly")
        if not self.pms:
            raise MissingDependencyException("Packet manager may not be installed")
        if not self._set_security_policies():
            raise MissingDependencyException("Security polices may not be set properly")
        if not self._is_docker():
            raise MissingDependencyException("Docker may not be installed")

    def get_pid_from_port(self, port: int):
        return self.terminal.execute(
            f"sudo netstat -anop | grep -Po ':{port}\s.*LISTEN.*?\K\d+(?=/)' || true"
        )

    def path_exist(self, path):
        return self.terminal.execute(f"test -e {path} || echo False") != "False"

    def kill_process_from_port(self, port: int):
        """Raise error if there is no process occupying specific port"""
        pid = self.get_pid_from_port(port)
        self.terminal.execute(f"sudo kill -9 {pid}")

    def clean(self):
        pass

    def is_port_free(self, port):
        return not bool(
            self.terminal.execute(f"sudo netstat -anop | grep ':{port} ' || true")
        )

    def is_app_listening_on_port(self, app_name, port):
        out = self.terminal.execute(f"sudo netstat -anop | grep ':{port} ' || true")
        return app_name in out
