#!/bin/bash

shopt -s expand_aliases

if [ -z "$1" ] || [ -z "$2" ]
then
    echo "- Missing mandatory argument:"
    echo " - Usage: source pre_test.sh <SDE_INSTALL_PATH> <IPDK_RECIPE> <DEPEND_INSTALL>"
    return 0
fi

export SDE_INSTALL=$1
export IPDK_RECIPE=$2
export DEPEND_INSTALL=$3

echo "Killing qemu"
pkill -9 qemu

echo "killing infrap4d"
PID=`ps -eaf | grep infrap4d | grep -v grep | awk '{print $2}'`
if [[ "" !=  "$PID" ]]; then
  echo "killing $PID"
  kill -9 $PID
fi

echo "Sleeping for 2 seconds"
sleep 2

echo "Killing ovs"
pkill -9 ovs
echo "sleeping for 2 seconds"
sleep 2

echo "Removing any vhost users from /tmp"
rm -rf /tmp/vhost-user-*
rm -rf /tmp/intf/vhost-user-*

echo "Setting PATH"
export LD_LIBRARY_PATH=$IPDK_RECIPE/install/lib/:$SDE_INSTALL/lib:$SDE_INSTALL/lib64:$DEPEND_INSTALL/lib:$DEPEND_INSTALL/lib64
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SDE_INSTALL/lib/x86_64-linux-gnu
export PATH=$PATH:$IPDK_RECIPE/install/bin:$DEPEND_INSTALL/bin:$DEPEND_INSTALL/sbin
export RUN_OVS=$IPDK_RECIPE/install

source $IPDK_RECIPE/install/sbin/setup_env.sh $IPDK_RECIPE/install $SDE_INSTALL $DEPEND_INSTALL

echo "starting ovs"
mkdir -p $IPDK_RECIPE/install/var/run/openvswitch
rm -rf $IPDK_RECIPE/install/etc/openvswitch/conf.db
$IPDK_RECIPE/install/bin/ovsdb-tool create $IPDK_RECIPE/install/etc/openvswitch/conf.db $IPDK_RECIPE/install/share/openvswitch/vswitch.ovsschema
$IPDK_RECIPE/install/sbin/ovsdb-server  --remote=punix:$RUN_OVS/var/run/openvswitch/db.sock   --remote=db:Open_vSwitch,Open_vSwitch,manager_options  --pidfile --detach
$IPDK_RECIPE/install/sbin/ovs-vswitchd --detach --no-chdir unix:$RUN_OVS/var/run/openvswitch/db.sock --mlockall --log-file=/tmp/ovs-vswitchd.log

echo "set hugepages"
$IPDK_RECIPE/install/sbin/set_hugepages.sh

rm -rf /usr/share/stratum/dpdk/dpdk_skip_p4.conf
cp common/p4c_artifacts/pna_tcp_connection_tracking/dpdk_skip_p4.conf /usr/share/stratum/dpdk/ 


echo "run infrap4d"
$IPDK_RECIPE/install/sbin/infrap4d

echo "check if infrap4d is running"
ps_out=`ps -ef | grep 'infrap4d' | grep -v 'grep'`
echo $ps_out
result=$(echo $ps_out | grep "infrap4d")
if [[ "$result" != "" ]];then
    echo "infrap4d Running"
else
    echo "infrap4d Not Running"
fi
