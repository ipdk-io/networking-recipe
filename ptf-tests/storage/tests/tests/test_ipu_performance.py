# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

target = "ipu_performance"
import time

from system_tools.config import import_base_test
from system_tools.const import FIO_NUM_RUN, FIO_PERCENT_PASS
from system_tools.services import IPUService


BaseTest = import_base_test(target)


class TestIPUPerformance(BaseTest):
    def setUp(self):
        self.platforms_service = IPUService()
        self.platforms_service.set()
        self.lp_platform = self.platforms_service.get_lp_platform()
        self.host_platform = self.platforms_service.get_host_platform()

    def runTest(self):
        pass_percent = self.platforms_service.fio_performance(FIO_NUM_RUN)
        assert pass_percent >= FIO_PERCENT_PASS

    def tearDown(self):
        self.platforms_service.clean()
