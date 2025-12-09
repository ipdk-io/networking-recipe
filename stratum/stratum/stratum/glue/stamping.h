// Copyright 2021-present Open Networking Foundation
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_GLUE_STAMPING_H_
#define STRATUM_GLUE_STAMPING_H_

namespace stratum {
// Populated via linkstamping.
extern const char kBuildHost[];
extern const char kBuildScmRevision[];
extern const char kBuildScmStatus[];
extern const char kBuildUser[];
extern const int kBuildTimestamp;
extern const bool kStampingEnabled;
}  // namespace stratum

#endif  // STRATUM_GLUE_STAMPING_H_
