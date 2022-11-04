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
#!/usr/bin/python

from common.lib.local_connection import Local
from common.lib.exceptions import ExecuteCMDException
import subprocess 

class TcpDumpCap(object):
    def __init__(self):
        """
        Constructor method
        """
        self.TCPDUMP = self._TCPDUMP()

    class _Common(object):
        cmd_prefix = None

        def form_cmd(self, cmd):
            """Combine command prefix with command
            :param cmd: command to combine with prefix
            :type cmd: str
            :return: command combined with prefix
            :rtype: str
            """
            return " ".join([self.cmd_prefix, cmd])

    class _TCPDUMP(_Common):
        def __init__(self):
            """
            Constructor method
            """
            self.local = Local()
            cmd = "which tcpdump"
            output, return_code, _ = self.local.execute_command(cmd)
            if return_code: 
                print(f"Error: Tcpdump is not installed")

            self.cmd_prefix = 'tcpdump'


        def tcpdump_start_capture(self, params):
            """
            tcpdump command to start capture in background and supressed stdout
            :param params: list of parameters for tcpdump capture 
            :type params: list
            :return: none, exit code of executing tcpdump with the parameters 
            """
            paramstr = " "
            params = paramstr.join(map(str, params))
            cmd = self.form_cmd(params + " 2> /dev/null &")
            output, return_code, _ = self.local.execute_command(cmd)
            if return_code:
                print(f"Failed to run the tcpdump capture command")
                return False
            else:    
                return output


        def tcpdump_tear_down(self):
            """
            Function
            """
            cmd = "pkill -9 tcpdump"
            output, return_code, _ = self.local.execute_command(cmd)
            if not return_code:
                print("tcpdump process terminated")
            else:
                print("No tcpdump process to terminate")
                    
