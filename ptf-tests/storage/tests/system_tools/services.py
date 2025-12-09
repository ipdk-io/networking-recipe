# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

from abc import ABC, abstractmethod

from system_tools.config import TestConfig
from system_tools.const import CONTROLLERS_NUMBER, FIO_TARGET_PERFORMANCE
from system_tools.parsers import fio_performance_parser
from system_tools.platforms.host_platform import HostPlatform
from system_tools.platforms.lp_platform import (IPULinkPartnerPlatform,
                                                OPILinkPartnerPlatform)


class BaseService(ABC):
    @abstractmethod
    def __init__(self):
        self.tests_config = TestConfig()
        self.lp_platform = None
        self.host_platform = None

    def set(self):
        self.lp_platform.set()
        self.host_platform.set()

    def get_host_platform(self):
        return self.host_platform

    def get_lp_platform(self):
        return self.lp_platform

    def clean(self):
        self.lp_platform.clean()
        self.host_platform.clean()

    def fio_performance(self, run_num):
        pass_fio = 0
        for _ in range(run_num):
            result = self.host_platform.run_performance_fio()
            iops = fio_performance_parser(result)
            pass_fio = pass_fio + 1 if iops > FIO_TARGET_PERFORMANCE else pass_fio
        return pass_fio / run_num * 100


class IPUService(BaseService):
    def __init__(self):
        self.tests_config = TestConfig()
        self.lp_platform = IPULinkPartnerPlatform()
        self.host_platform = HostPlatform()


class OPIService(BaseService):
    def __init__(self, controllers_number=None):
        self.controllers_number = controllers_number if controllers_number else CONTROLLERS_NUMBER
        self.tests_config = TestConfig()
        self.lp_platform = OPILinkPartnerPlatform(self.controllers_number)
        self.host_platform = HostPlatform(self.controllers_number)
