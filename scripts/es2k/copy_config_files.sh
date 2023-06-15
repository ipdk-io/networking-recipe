#Copyright (C) 2021-2022 Intel Corporation
#SPDX-License-Identifier: Apache-2.0

#! /bin/bash
set -e

if [ -z "$1" ]
then
    echo "- Missing mandatory argument: P4CP_SOURCE"
    echo " - Usage: sudo copy_config_files.sh <P4CP_SOURCE> <SDE_INSTALL> <P4CP_INSTALL>"
    return 0
fi

if [ -z "$2" ]
then
    echo "- Missing mandatory argument: SDE_INSTALL"
    echo " - Usage: sudo copy_config_files.sh <P4CP_SOURCE> <SDE_INSTALL> <P4CP_INSTALL>"
    return 0
fi

if [ -z "$3" ]
then
    echo "- Missing mandatory argument: P4CP_INSTALL"
    echo " - Usage: sudo copy_config_files.sh <P4CP_SOURCE> <SDE_INSTALL> <P4CP_INSTALL>"
    return 0
fi

export P4CP_SOURCE=$1
export SDE_INSTALL=$2
export P4CP_INSTALL=$3

#... Create required directories and copy the config files ...#
cd $P4CP_SOURCE
sudo mkdir -p /etc/stratum/
sudo mkdir -p /var/log/stratum/
sudo mkdir -p /usr/share/stratum/es2k
sudo cp $P4CP_INSTALL/share/stratum/es2k/es2k_port_config.pb.txt /usr/share/stratum/es2k/
sudo cp $P4CP_INSTALL/share/stratum/es2k/es2k_skip_p4.conf /usr/share/stratum/es2k/
sudo cp $SDE_INSTALL/share/target_sys/zlog-cfg /usr/share/target_sys/
set +e
