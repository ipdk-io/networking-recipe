#!/bin/bash
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

SCRIPT_PATH=$(realpath "$(dirname -- "${BASH_SOURCE[0]}")")
PTF_TESTS_PATH=$(dirname "$SCRIPT_PATH")
cd -- "$PTF_TESTS_PATH" || exit 1

# create python environment
echo 'create python environment'
echo 'update pms'
sudo apt update || sudo dnf update
echo 'create venv'
sudo apt install -y python3 || sudo dnf -y install python3
sudo apt install -y python3-pip || sudo dnf -y install python3-pip
python3 -m venv venv
# shellcheck disable=SC1091,SC1090
echo 'venv activate'
source venv/bin/activate
python3 -m pip install --upgrade pip
echo 'install requirements'
python3 -m pip install -r requirements.txt
echo 'end'
