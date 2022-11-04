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
import subprocess


class Local(object):
    """This class helps run commands on local server
    """
    def __init__(self):
        self.process = None

    def execute_command(self, command):
        """ To execute localhost commands and return all outputs
        :param command: command to execute locally
        :type command: string e.g. "ls -lh"
        :return: cmd output, exit code, and error logs
        :rtype: tuple e.g. out,return_code,error
        """

        self.process = subprocess.Popen('/bin/bash', stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        out, err = self.process.communicate(command.encode('utf-8'))

        if err:
            err = err.decode('utf-8')
        return out.decode('utf-8'), self.process.returncode, err

    def tear_down(self):
        """
        Yet to implement
        :return: None
        :rtype: None
        """
        pass
