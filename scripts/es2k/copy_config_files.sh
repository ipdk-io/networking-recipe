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

export P4CP_INSTALL=$1
export SDE_INSTALL=$2

SOURCE_DIR=${P4CP_INSTALL}/share/stratum/es2k
TARGET_DIR=/usr/share/stratum/es2k
TLS_CERTS_DIR=/usr/share/stratum

#... Create required directories and copy the config files ...#
cd $P4CP_INSTALL
sudo mkdir -p /etc/stratum/
sudo mkdir -p /var/log/stratum/
sudo mkdir -p /usr/share/stratum/es2k
sudo mkdir -p /usr/share/target_sys/
sudo mkdir -p /usr/share/bf_rt_shared

sudo cp ${SOURCE_DIR}/es2k_port_config.pb.txt /usr/share/stratum/es2k/
sudo cp ${SOURCE_DIR}/es2k_skip_p4.conf /usr/share/stratum/es2k/

#... Install files required for TLS certificate generation ...#
sudo cp ${SOURCE_DIR}/ca.conf ${TLS_CERTS_DIR}/
sudo cp ${SOURCE_DIR}/grpc-client.conf ${TLS_CERTS_DIR}/
sudo cp ${P4CP_INSTALL}/sbin/generate-certs.sh ${TLS_CERTS_DIR}/

sudo cp $SDE_INSTALL/share/target_sys/zlog-cfg /usr/share/target_sys/

#... Install packetIO json file ...#
sudo cp $SDE_INSTALL/share/bf_rt_shared/tdi_pktio.json /usr/share/bf_rt_shared/

set +e
