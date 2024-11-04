#!/bin/bash
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#
# Reports unit test code coverage for KRNLMON on ES2K.
#
# You must build the tests with coverage enabled before
# running this script. For example:
#
#   cmake --preset es2k -DTEST_COVERAGE=ON
#   cmake --build build -j5 --target krnlmon-unit-tests
#

set -e

# Folder under which to search for .gcda files.
INPUT_DIR=build/krnlmon/krnlmon/

# Folder in which the report will be generated.
OUTPUT_DIR=build/Coverage/krnlmon/es2k

# Label identifying the tests to report on.
TEST_LABEL=krnlmon

# Select tests with the specified label, run the tests,
# and compute coverage of the source code.
(cd build; ctest -L ${TEST_LABEL} -T test -T coverage)

# Make sure the output directory exists.
mkdir -p ${OUTPUT_DIR}

# Empty it for good measure.
rm -fr ${OUTPUT_DIR:?}/*

# Capture the coverage data, excluding files in directories
# that don't interest us.
lcov --capture \
     --directory ${INPUT_DIR} \
     --output-file ${OUTPUT_DIR}/coverage.info \
     --exclude '/opt/deps/*' \
     --exclude '/usr/include/*'

# Generate html coverage report.
genhtml ${OUTPUT_DIR}/coverage.info \
        --output-directory ${OUTPUT_DIR}

echo "Coverage report is in ${OUTPUT_DIR}."	
