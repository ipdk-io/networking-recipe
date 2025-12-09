# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

import time

from system_tools.config import HostConfig
from system_tools.const import CONTROLLERS_NUMBER
from system_tools.log import logging
from system_tools.platforms.base_platform import BasePlatform
from system_tools.terminals import SSHTerminal


class HostPlatform(BasePlatform):
    def __init__(self, controllers_number=CONTROLLERS_NUMBER):
        super().__init__(SSHTerminal(HostConfig()))
        self.controllers_number = controllers_number

    def set(self):
        super().set()
        self.install_prerequsites()
        nvme_pf_bdf = self.get_nvme_pf_bdf()
        self.bind_pf(nvme_pf_bdf)
        self.create_vfs(nvme_pf_bdf)
        self.bind_vfs(nvme_pf_bdf)

    def get_nvme_pf_bdf(self):
        # On SUT find the address of PF
        pfs = self.terminal.execute("""lspci -D -d 8086:1457 | cut -d " " -f1""")
        return pfs.split("\n")[0]

    def bind_pf(self, nvme_pf_bdf):
        logging.ptf_info("PF binding")
        return self.terminal.execute(
            f"""echo {nvme_pf_bdf} | sudo tee /sys/bus/pci/drivers/nvme/bind"""
        )

    def install_prerequsites(self):
        logging.ptf_info("Start install host prerequsites")
        self.terminal.execute(
            f"""sudo {self.pms} install -y git nvme-cli pciutils numactl fio"""
        )
        logging.ptf_info("End install host prerequsites")

    def create_vfs(self, nvme_pf_bdf):
        logging.ptf_info("Create VFs on SUT")
        filepath = f"/sys/bus/pci/devices/{nvme_pf_bdf}/sriov_drivers_autoprobe"
        self.terminal.execute(f"""echo 0 | sudo tee {filepath}""")
        filepath = f"/sys/bus/pci/drivers/nvme/{nvme_pf_bdf}/sriov_numvfs"
        self.terminal.execute(f"""echo {self.controllers_number} | sudo tee {filepath}""")
        logging.ptf_info("End creating VFs on SUT")

    def bind_vfs(self, nvme_pf_bdf):
        logging.ptf_info("Bind VFs to NVMe driver")
        for i in range(self.controllers_number):
            filepath = f"/sys/bus/pci/devices/{nvme_pf_bdf}/virtfn{i}/driver_override"
            self.terminal.execute(f"""echo nvme | sudo tee {filepath}""")
            filepath = "/sys/bus/pci/drivers/nvme/bind"
            virtfn = self.terminal.execute(
                f"""echo $(basename "$(realpath "/sys/bus/pci/devices/{nvme_pf_bdf}/virtfn{i}")")"""
            )
            self.terminal.execute(f"""echo {virtfn} | sudo tee {filepath}""")
            time.sleep(7)
        logging.ptf_info("End binding VFs to NVMe driver")

    def run_performance_fio(self, num=CONTROLLERS_NUMBER):
        logging.ptf_info("Start fio")
        cpus_to_use = self.get_cpus_to_use()
        filenames = ""
        for i in range(2, num + 2):
            filenames = filenames + f"--filename=/dev/nvme{i}n1 "
        fio = f"""
        sudo taskset -c {cpus_to_use[0]}-{cpus_to_use[-1]} fio --name=test --rw=randwrite {filenames}\
        --numjobs=12 --group_reporting --runtime=20 --time_based=1 --io_size=4096 --iodepth=2048 --ioengine=libaio --ramp_time 5
        """
        response = self.terminal.execute(f"{fio}")
        logging.ptf_info("End fio")
        return response
