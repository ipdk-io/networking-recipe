// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_TLS_CREDENTIALS_H_
#define OVSP4RT_TLS_CREDENTIALS_H_

#include <string>
#include <grpcpp/grpcpp.h>
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/security/tls_credentials_options.h"

#define DEFAULT_CERTS_DIR "/usr/share/stratum/certs/"

namespace ovs_p4rt_cpp {

static std::string ca_cert_file = DEFAULT_CERTS_DIR "ca.crt";
static std::string client_key_file = DEFAULT_CERTS_DIR "client.key";
static std::string client_cert_file = DEFAULT_CERTS_DIR "client.crt";

static constexpr unsigned int kFileRefreshIntervalSeconds = 1;

// Checks whether filename is a regular file and not a symlink
static bool IsRegularFile(const std::string& filename);

std::shared_ptr<::grpc::ChannelCredentials> GenerateClientCredentials();

}  // namespace ovs_p4rt_cpp

#endif  // OVSP4RT_TLS_CREDENTIALS_H_