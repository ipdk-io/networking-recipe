# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

import os
from abc import ABC, abstractmethod

from dotenv import load_dotenv
from system_tools.const import LP_INTERNAL_IP, ACC_INTERNAL_IP

from system_tools.const import (DEFAULT_HOST_TARGET_SERVICE_PORT_IN_VM,
                                DEFAULT_MAX_RAMDRIVE, DEFAULT_MIN_RAMDRIVE,
                                DEFAULT_NQN, DEFAULT_NVME_PORT,
                                DEFAULT_QMP_PORT, DEFAULT_SMA_PORT,
                                DEFAULT_SPDK_PORT, DEFAULT_TARGETS,
                                STORAGE_DIR_PATH)


class BaseConfig(ABC):
    load_dotenv()

    @abstractmethod
    def __init__(self):
        pass

    def _getenv(self, env_name, alternative=None):
        env = os.getenv(env_name)
        return env if env else alternative

class DockerConfig(BaseConfig):
    def __init__(self):
        self.http_proxy = self._getenv("HTTP_PROXY")
        self.https_proxy = self._getenv("HTTPS_PROXY")
        self.ftp_proxy = self._getenv("FTP_PROXY")

class TestConfig(BaseConfig):
    __test__ = False
    def __init__(self):
        self.debug = self._getenv("DEBUG", "FALSE")
        self.nqn = self._getenv("NQN", DEFAULT_NQN)
        self.cmd_sender_platform = self._getenv("CMD_SENDER_PLATFORM", "ipu")
        self.targets = self._get_targets()

    def _get_targets(self):
        targets = self._getenv("TARGETS", DEFAULT_TARGETS)
        if isinstance(targets, str):
            chars_to_delete = """[]"' """
            for char in chars_to_delete:
                targets = targets.replace(char, "")
            targets = targets.split(",")
        return list(map(lambda x: x.lower(), targets))


class BasePlatformConfig(BaseConfig):
    def __init__(self, platform_name):
        self._platform_name = platform_name
        self.username = self._get_platform_property("USERNAME")
        self.password = self._get_platform_property("PASSWORD")
        self.ip_address = self._get_platform_property("IP_ADDRESS")
        self.port = self._get_platform_property("PORT", 22)
        self.workdir = os.getenv(
            "_".join([platform_name, "WORKDIR"]),
            f"/home/{self.username}/ipdk_tests_workdir",
        )

    @property
    def storage_dir(self):
        return os.path.join(self.workdir, STORAGE_DIR_PATH)

    def _get_platform_property(self, property_name, alternative=None):
        return self._getenv("_".join([self._platform_name, property_name]), alternative)


class MainPlatformConfig(BasePlatformConfig):
    def __init__(self, platform_name):
        username = self._getenv("_".join([platform_name, "USERNAME"]))
        super().__init__(platform_name) if username else super().__init__(
            "MAIN_PLATFORM"
        )


class LpConfig(MainPlatformConfig):
    def __init__(self):
        super().__init__("LP")
        self.sma_port = self._getenv("SMA_PORT", DEFAULT_SMA_PORT)


class HostConfig(MainPlatformConfig):
    def __init__(self):
        super().__init__("HOST")
        share_dir_path = self._getenv("SHARE_DIR_PATH", "shared")
        self.vm_share_dir_path = os.path.join(self.workdir, share_dir_path)
        self.host_target_service_port_in_vm = self._getenv(
            "HOST_TARGET_SERVICE_PORT_IN_VM", DEFAULT_HOST_TARGET_SERVICE_PORT_IN_VM
        )
        self.qemu_binary = self._getenv("QEMU_BINARY", None)


class BaseTest:
    pass


def import_base_test(target):
    target = target.lower()
    if target in TestConfig().targets or target == "all":
        from ptf.base_tests import BaseTest
    else:
        from system_tools.config import BaseTest
    return BaseTest
