// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "gflags/gflags.h"

DEFINE_bool(incompatible_enable_tdi_legacy_bytestring_responses, true,
            "Enables the legacy padded byte string format in P4Runtime "
            "responses for Stratum-tdi. The strings are left unchanged from "
            "the underlying SDE.");
