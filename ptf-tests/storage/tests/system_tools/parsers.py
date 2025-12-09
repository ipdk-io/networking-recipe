# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

import re


def fio_performance_parser(fio):
    iops = re.search("IOPS=\d*[a-z]*", fio).group().split("=")[1]
    iops = re.sub("k|K", "000", iops)
    iops = re.sub("m|M", "000_000", iops)
    return int(iops)
