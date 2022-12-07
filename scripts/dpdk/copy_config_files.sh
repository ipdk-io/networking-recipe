#Copyright (C) 2021-2022 Intel Corporation
#SPDX-License-Identifier: Apache-2.0

#! /bin/bash
set -e

if [ -z "$1" ]
then
    echo "- Missing mandatory argument: IPDK_RECIPE"
    echo " - Usage: sudo copy_config_files.sh <IPDK_RECIPE> <SDE_INSTALL>"
    return 0
fi

if [ -z "$2" ]
then
    echo "- Missing mandatory argument: SDE_INSTALL"
    echo " - Usage: sudo copy_config_files.sh <IPDK_RECIPE> <SDE_INSTALL>"
    return 0
fi

export IPDK_RECIPE=$1
export SDE_INSTALL=$2

#... Create required directories and copy the config files ...#
cd $IPDK_RECIPE
sudo mkdir -p /etc/stratum/
sudo mkdir -p /var/log/stratum/
sudo mkdir -p /usr/share/stratum/dpdk
sudo cp ./install/share/stratum/dpdk/dpdk_port_config.pb.txt /usr/share/stratum/dpdk/
sudo cp ./install/share/stratum/dpdk/dpdk_skip_p4.conf /usr/share/stratum/dpdk/
sudo cp $SDE_INSTALL/share/target_sys/zlog-cfg /usr/share/target_sys/
set +e
