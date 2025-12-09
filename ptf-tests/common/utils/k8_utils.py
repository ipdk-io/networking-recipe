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

from common.lib.k8_dpdk_lib import K8_DPDK
from common.lib.local_connection import Local
import common.utils.log as log


def create_pod(pod_name: str, pod_body: dict, namespace="default"):
    """
    Util function to create pod 'pod_name'
    params:
    pod_name: String

    returns:
    None
    """
    k8_api = K8_DPDK()
    k8_api.create_pod(pod_name, pod_body, namespace=namespace)


def delete_pod(pod_name):
    """
    Util function to delete pod 'pod_name'
    params:
    pod_name: String

    returns:
    None
    """
    k8_api = K8_DPDK()
    k8_api.delete_pod(pod_name)


def delete_pods_with_given_namespace(namespace="default"):
    """
    lists and deletes pods with given namespace
    params:
    namespace: string

    returns:
    Boolean True/False
    """
    pod_names = find_pods_with_namespace(namespace=namespace)
    k8_api = K8_DPDK()
    if not pod_names:
        log.info(f"No pods found with namespace = {namespace}")
        return True
    for pod_name in pod_names:
        log.info(f"Deleteing {pod_name}")
        k8_api.delete_pod(pod_name)
        time.sleep(5)
        if verify_pod_running(pod_name):
            log.failed(f"Failed to delete {pod_name}")
            return False

        return True


def execute_command(pod_name, cmds):
    """
    Util function to execute 'cmds' in pod 'pod_name'
    params:
    pod_name: String
    cmds: list

    return:
    resp: String
    """
    k8_api = K8_DPDK()
    resp = k8_api.execute_command(pod_name, cmds)
    return resp


def find_pod_ip(pod_name):
    """
    Util function to find ip of pod 'pod_name'
    params:
    pod_name: String

    return:
    pod_ip: String
    """
    k8_api = K8_DPDK()
    ret = k8_api.list_pod()
    for item in ret.items:
        if item.metadata.name == pod_name:
            log.info(f"Finding ip for {pod_name}")
            return item.status.pod_ip

    return None


def find_pods_with_namespace(namespace="default"):
    """
    Util function to find list of pods with a given namespace
    params:
    namespace: string

    returns:
    pod_names: list
    """
    k8_api = K8_DPDK()
    ret = k8_api.list_pod()
    pod_names = []
    for item in ret.items:
        if item.metadata.namespace == namespace:
            pod_names.append(item.metadata.name)

    return pod_names


def verify_pod_running(pod_name):
    """
    Util function to check if 'pod_name' is running
    params:
    pod_name: String

    return: Boolean True or False
    """
    k8_api = K8_DPDK()
    ret = k8_api.list_pod()

    for item in ret.items:
        if item.metadata.name == pod_name:
            log.info(f"test pod {pod_name} running")
            return True

    log.info(f"test pod {pod_name} not running")
    return False


def ping_and_verify_no_packet_loss(pod_name, dest_ip, count=10):
    """
    Util function to ping dest ip and verify if 0% packet loss
    params:
    pod_name: string
    dest_ip: string

    returns:
    Boolean True/False
    """
    k8_api = K8_DPDK()
    cmds = [
        "/bin/sh",
        "-c",
        f'ping -c 10 {dest_ip} | egrep -oi "[0-9]*% packet loss" | grep -oi [0-9]*',
    ]
    resp = k8_api.execute_command(pod_name, cmds)
    log.info(f"{resp}% packet loss")
    if not resp:
        return False
    elif resp != "0":
        return False
    else:
        return True


def check_service_status(service_name, expected_status=True):
    """
    Util function to check the status of service "service_name", default check expected_status=True, i.e. running
    params:
    service_name: String

    return:
    Boolean True or False
    """
    k8_api = K8_DPDK()
    log.info(f"Checking if service:{service_name} is running or not")
    ret = k8_api.check_service_running(service_name)

    if ret == expected_status:
        return True

    return False


def execute_iperf_setup(kube_iperf_path):
    """
    Util function to execute setup.sh located at kube_iperf_path/steps/

    params:
    kube_iperf_path: String

    return:
    Boolean True/False
    """
    local = Local()
    cmd = f"cd {kube_iperf_path}; ./steps/setup.sh"

    ret, _, _ = local.execute_command(cmd)

    log.info(ret)

    if "Server is running" in ret:
        log.passed("Successfully executed setup.sh")
        return True

    log.failed("Failed to execute setup.sh")
    return False


def execute_iperf_cleanup(kube_iperf_path, service_name="iperf3-server"):
    """
    Util function to execute cleanup.sh located at kube_iperf_path/steps/

    params:
    kube_iperf_path: String

    return:
    Boolean True/False
    """
    local = Local()
    cmd = f"cd {kube_iperf_path}; ./steps/cleanup.sh"

    ret, _, _ = local.execute_command(cmd)

    log.info(ret)

    if f'service "{service_name}" deleted' in ret:
        log.passed("Successfully executed cleanup.sh")
        return True

    log.failed("Failed to execute cleanup.sh")
    return False


def execute_iperf_client(kube_iperf_path):
    """
    Util function to execute run.sh located at kube_iperf_path/steps/

    params:
    kube_iperf_path: String

    return:
    rx_pkt, tx_pkt: String (in Mbytes)
    """
    local = Local()
    cmd = f"cd {kube_iperf_path}; ./steps/run.sh"

    ret, _, _ = local.execute_command(cmd)

    log.info(ret)

    rx_pkt = 0
    tx_pkt = 0
    for line in ret.split("\n"):
        if "sender" in line:
            rx_pkt = int(line.split()[7])
        elif "receiver" in line:
            tx_pkt = int(line.split()[7])
        else:
            continue

    return rx_pkt, tx_pkt
