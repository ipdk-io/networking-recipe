#!/bin/bash

shopt -s expand_aliases

if [ -z "$1" ] || [ -z "$2" ]
then
    echo "- Missing mandatory argument:"
    echo " - Usage: source pre_test.sh <SDE_INSTALL_PATH> <P4OVS_INSTALL_PATH> [P4OVS_DEPS_INSTALL_PATH]"
    return 0
fi

export SDE_INSTALL=$1
export P4OVS=$2

echo "Killing qemu"
pkill -9 qemu

echo "killing ovs"
pkill -9 ovs

echo "sleeping for 2 seconds"
sleep 2

echo "removing any vhost users from /tmp"
rm -rf /tmp/vhost-user-*
rm -rf /tmp/intf/vhost-user-*

echo "changing directory to $P4OVS"
cd $P4OVS

echo "sourcing p4ovs_env_setup.sh"
if [ $3 ]
then
  export P4OVS_DEPS_INSTALL=$3
  source p4ovs_env_setup.sh $SDE_INSTALL $P4OVS_DEPS_INSTALL
else
  source p4ovs_env_setup.sh $SDE_INSTALL
fi

echo "set hugepages"
./set_hugepages.sh
echo "starting OVS"
if [ $3 ]
then
  ./run_ovs.sh $SDE_INSTALL
else
  ./run_ovs.sh
fi

echo "sleeping for 5 seconds"
sleep 5

echo "setting the alias for PATH and LD_LIBRARY_PATH"
alias sudo='sudo PATH="$PATH" HOME="$HOME" LD_LIBRARY_PATH="$LD_LIBRARY_PATH"'

echo "changing directory to ptf_tests"
cd -
