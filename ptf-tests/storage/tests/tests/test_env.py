# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

target = "all"

from system_tools.config import HostConfig, LpConfig, import_base_test
from system_tools.terminals import SSHTerminal

BaseTest = import_base_test(target)


class TestTerminalConnect(BaseTest):
    def setUp(self):
        self.host_terminal = SSHTerminal(HostConfig())
        self.lp_terminal = SSHTerminal(LpConfig())

    def runTest(self):
        self.assertEqual(
            self.host_terminal.execute("whoami"),
            self.host_terminal.config.username,
        )
        self.assertEqual(
            self.lp_terminal.execute("whoami"),
            self.lp_terminal.config.username,
        )

    def tearDown(self):
        pass


class TestTerminalConnectHasRootPrivileges(BaseTest):
    def setUp(self):
        self.host_terminal = SSHTerminal(HostConfig())
        self.lp_terminal = SSHTerminal(LpConfig())

    def runTest(self):
        self.assertEqual(
            self.host_terminal.execute("sudo whoami"),
            "root",
        )
        self.assertEqual(
            self.lp_terminal.execute("sudo whoami"),
            "root",
        )

    def tearDown(self):
        pass
