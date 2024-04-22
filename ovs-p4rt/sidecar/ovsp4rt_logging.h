// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_LOGGING_H_
#define OVSP4RT_LOGGING_H_

// P4c lib/log.h already defines the ERROR macro.
// Issue: https://github.com/p4lang/p4c/issues/2523
#ifdef ERROR
#undef ERROR
#endif

#include "glog/logging.h"  // IWYU pragma: export

// These are exported in open source glog but not base/logging.h
using ::google::ERROR;
using ::google::FATAL;
using ::google::INFO;
using ::google::WARNING;

#endif  // OVSP4RT_LOGGING_H_
