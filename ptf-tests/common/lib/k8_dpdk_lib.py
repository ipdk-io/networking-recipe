# Copyright (c) 2022 Intel Corporation.
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

from kubernetes import client, config
from kubernetes.stream import stream
import common.utils.log as log


class K8_DPDK:
    """
    Base class for K8 api s for dpdk backend
    """

    def __init__(self):
        """
        Constructor method
        """
        config.load_kube_config()
        self.apiinstance = client.CoreV1Api()
        self.pods = []

    def create_pod(self, pod_name: str, pod_body: dict, namespace="default"):
        """
        Method to create a pod
        params:
        pod_name: name of the pod
        pod_body: pod manifest in the form of dictionary
        namespace: namespace for creating pods

        returns:
        pod object
        """
        try:
            resp = self.apiinstance.create_namespaced_pod(
                namespace=namespace, body=pod_body
            )
        except Exception as err:
            log.failed(f"Exception occurred: {err}")
            return False

        self.pods.append(pod_name)

        return True

    def list_pod(self):
        """
        Method to list all pods
        returns:
        ret: String
        """
        ret = self.apiinstance.list_pod_for_all_namespaces(watch=False)
        """for item in ret.items:
            print(
            "%s\t%s\t%s" %
            (item.status.pod_ip,
             item.metadata.namespace,
             item.metadata.name))"""

        return ret

    def delete_pods(self):
        """
        Delete all pods
        returns:
        Boolean True/False
        """
        try:
            for pod in self.pods:
                self.apiinstance.delete_namespaced_pod(
                    name=pod, grace_period_seconds=0, namespace="default", force=True
                )
        except Exception as err:
            log.failed(f"Exception occurred: {err}")
            return False

        return True

    def delete_pod(self, pod_name: str):
        """
        Deletes pod: pod_name
        params:
        pod_name: String

        returns:
        Boolean True/False
        """
        try:
            self.apiinstance.delete_namespaced_pod(
                name=pod_name, grace_period_seconds=0, namespace="default"
            )
        except Exception as err:
            log.failed(f"Exception occurred: {err}")
            return False

        return True

    def execute_command(self, pod_name: str, cmd: list):
        """
        Method to execute command
        params:
        pod_name: String
        cmd: list

        returns:
        resp: String
        """
        try:
            resp = stream(
                self.apiinstance.connect_get_namespaced_pod_exec,
                pod_name,
                "default",
                command=cmd,
                stderr=True,
                stdin=False,
                stdout=True,
                tty=False,
            )

        except Exception as err:
            log.failed(f"Exception occurred: {err}")
            return False

        return resp

    def check_service_running(self, service_name):
        """
        Method to find if service 'service_name' is running
        params:
        service_name: String
        returns:
        Boolean True/False
        """
        all_services = self.apiinstance.list_service_for_all_namespaces()
        for item in all_services.items:
            if item.metadata.name == service_name:
                log.info(f"Service {service_name} running")
                return True

        log.info(f"Service {service_name} not running")
        return False
