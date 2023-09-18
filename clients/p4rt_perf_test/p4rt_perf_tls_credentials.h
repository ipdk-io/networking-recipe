// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef P4RT_PERF_TLS_CREDENTIALS_H_
#define P4RT_PERF_TLS_CREDENTIALS_H_

#include <grpcpp/grpcpp.h>

#include <string>

#define DEFAULT_CERTS_DIR "/usr/share/stratum/certs/"

static std::string ca_cert_file = DEFAULT_CERTS_DIR "ca.crt";
static std::string client_key_file = DEFAULT_CERTS_DIR "client.key";
static std::string client_cert_file = DEFAULT_CERTS_DIR "client.crt";

static constexpr unsigned int kFileRefreshIntervalSeconds = 1;

// Checks whether filename is a regular file and not a symlink
bool IsRegularFile(const std::string& filename);

std::shared_ptr<::grpc::ChannelCredentials> GenerateClientCredentials();

#endif  // P4RT_PERF_TLS_CREDENTIALS_H_
