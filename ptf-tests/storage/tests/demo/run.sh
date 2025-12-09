#!/bin/bash
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

SCRIPT_PATH=$(realpath "$(dirname -- "${BASH_SOURCE[0]}")")
PTF_TESTS_PATH=$(dirname "$SCRIPT_PATH")
TESTS_PATH="$PTF_TESTS_PATH"/tests
# shellcheck disable=SC1091,SC1090

# flags
# c - create venv and download requirement
# v - run venv
# i - set ips
# m - create pf and vf
# f - fio
while getopts 'cvimf' OPTION; do
  case "$OPTION" in
    c)
      bash "${PTF_TESTS_PATH}"/scripts/create_python_environment.sh
      ;;
    v)
      source "${PTF_TESTS_PATH}"/venv/bin/activate
      ;;
    i)
      env PYTHONPATH="$PTF_TESTS_PATH" python "$PTF_TESTS_PATH"/demo/set_ips.py
      ;;
    m)
      env PYTHONPATH="$PTF_TESTS_PATH" python "$PTF_TESTS_PATH"/demo/create_pf_vf.py
      ;;
    f)
      env PYTHONPATH="$PTF_TESTS_PATH" python "$PTF_TESTS_PATH"/demo/fio.py
      ;;
    ?)
      echo "script usage: $(basename \$0) [-v]" >&2
      exit 1
      ;;
  esac
done
