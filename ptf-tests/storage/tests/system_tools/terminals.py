# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

from pathlib import Path
from threading import Thread
from time import sleep
from typing import Optional

from paramiko.client import AutoAddPolicy, SSHClient

from system_tools.errors import CommandException
from system_tools.log import logging


class SSHTerminal:
    """A class used to represent a session with an SSH server"""

    def __init__(self, config, *args, **kwargs):
        self.config = config
        self.client = SSHClient()

        self.client.load_system_host_keys()
        self.client.set_missing_host_key_policy(AutoAddPolicy)
        self.client.connect(
            config.ip_address,
            config.port,
            config.username,
            config.password,
            *args,
            **kwargs,
        )

    def execute(self, cmd: str, timeout: int = None) -> Optional[str]:
        """Simple function executes a command on the SSH server
        Returns list of the lines output
        """
        _, stdout, stderr = self.client.exec_command(cmd, timeout=timeout)  # nosec
        if stdout.channel.recv_exit_status():
            raise CommandException(stderr.read().decode())
        # if command is executed in the background don't wait for the output
        return (
            None if cmd.rstrip().endswith("&") else stdout.read().decode().rstrip("\n")
        )


class UnixSocketTerminal:
    def __init__(self, ssh_terminal, sock):
        self.ssh_terminal = ssh_terminal
        self.sock = sock
        tmp_path = Path("/tmp")
        self._filename = "socket_functions.py"
        self._dest_file = tmp_path.joinpath(self._filename)
        self._src_file = Path(__file__).parent.joinpath(self._filename)

    def _save_tmp_file(self):
        ssh_client = self.ssh_terminal.client
        sftp = ssh_client.open_sftp()
        sftp.put(str(self._src_file), str(self._dest_file))
        sftp.close()

    def _delete_tmp_file(self):
        self.ssh_terminal.execute(f"rm -rf {self._dest_file}")

    def _execute(self, cmd, wait_for_secs=1):
        cmd = (
            f"""cd /tmp && sudo python -c 'from socket_functions import *; """
            f"""out = send_command_over_unix_socket("{self.sock}", "{cmd}", {wait_for_secs}); """
            f"""print(out)'"""
        )
        return self.ssh_terminal.execute(cmd)

    @staticmethod
    def _clean_out(out):
        split_out = out.split("\r\n\x1b[?2004l\r")
        split_out = split_out[1] if len(split_out) == 2 else out
        split_out = split_out.split("\r\n\x1b[?2004h")
        split_out = split_out[0] if len(split_out) == 2 else split_out[0]
        return split_out.replace("\r", "")

    def execute(self, cmd, wait_for_secs=1):
        self._save_tmp_file()
        out = self._execute(cmd, wait_for_secs)
        self._delete_tmp_file()
        return self._clean_out(out)


class WaitDeviceThread(Thread):
    def __init__(self, terminal, dev):
        Thread.__init__(self)
        self.terminal = terminal
        self.dev = dev

    def run(self):
        cmd = f"""echo ' ' > {self.dev}"""
        self.terminal.execute(cmd)


class DeviceThread(WaitDeviceThread):
    def __init__(self, terminal, dev, cmd, lines):
        WaitDeviceThread.__init__(self, terminal, dev)
        self._cmd = cmd
        self._lines = lines

        self.response = None

    def run(self):
        self.response = self.terminal.execute(
            f"""echo '{self._cmd}' > {self.dev} && cat {self.dev} | head -n {self._lines}"""
        )


class DeviceTerminal:
    def __init__(self, ssh_terminal, dev):
        self.ssh_terminal = ssh_terminal
        self.dev = dev

    def execute(self, cmd, lines=0):
        cmd_thread = DeviceThread(self.ssh_terminal, self.dev, cmd, lines)
        cmd_thread.start()
        sleep(1)
        while cmd_thread.is_alive():
            waiting_thread = WaitDeviceThread(self.ssh_terminal, self.dev)
            waiting_thread.start()
            waiting_thread.join()
        cmd_thread.join()
        return cmd_thread.response

    def login(self):
        logging.ptf_info(f"Login device {self.dev}")
        for i in range(10):
            self.execute("root")

    def logout(self):
        self.execute("exit")
