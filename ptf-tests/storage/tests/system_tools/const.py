# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

STORAGE_DIR_PATH = "ipdk/build/storage"
DEFAULT_NQN = "nqn.2016-06.io.spdk:cnode0"

LP_INTERNAL_IP = "200.1.1.1/24"
ACC_INTERNAL_IP = "200.1.1.3/24"

DEFAULT_SPDK_PORT = 5260
DEFAULT_NVME_PORT = 4420
DEFAULT_SMA_PORT = 8080
DEFAULT_QMP_PORT = 5555
DEFAULT_HOST_TARGET_SERVICE_PORT_IN_VM = 50051
DEFAULT_MAX_RAMDRIVE = 64
DEFAULT_MIN_RAMDRIVE = 1
FIO_COMMON = {
    "runtime": 1,
    "numjobs": 1,
    "time_based": 1,
    "group_reporting": 1,
}
FIO_IO_PATTERNS = [
    "RANDRW",
    "RANDREAD",
    "WRITE",
    "READWRITE",
    "RANDWRITE",
    "READ",
    "TRIM",
]
DEFAULT_TARGETS = [
    "kvm",
]
SPDK_VERSION = "v23.05"
CONTROLLERS_NUMBER = 8
SPDK_REP = "git clone https://github.com/spdk/spdk.git"
SPDK_BDEV_NUM_BLOCKS = 1024
SPDK_BDEV_BLOCK_SIZE = 4096
SPDK_SNQN_BASE = "nqn.2019-06.io.spdk"
SPDK_BDEV_BASE = "Null"

FIO_TARGET_PERFORMANCE = 2_000_000
FIO_NUM_RUN = 10
FIO_PERCENT_PASS = 50

NODE = 1
BRIDGE_ADDR = "127.0.0.1:50051"
GRPC_CLI = 'env -i grpc_cli'
ACC_RPC_PATH = "/opt/ssa/rpc.py"
LP_RPC_PATH = "/home/berta/spdk/scripts/rpc.py"
