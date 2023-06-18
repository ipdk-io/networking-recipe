// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovs_p4rt_tls_credentials.h"

#include <sys/stat.h>

#include <iostream>
#include <string>

namespace ovs_p4rt {

using ::grpc::experimental::FileWatcherCertificateProvider;
using ::grpc::experimental::TlsChannelCredentialsOptions;

bool IsRegularFile(const std::string& filename) {
  struct stat buf;
  int rc = lstat(filename.c_str(), &buf);
  return (rc == 0 && S_ISREG(buf.st_mode));
}

std::shared_ptr<::grpc::ChannelCredentials> GenerateClientCredentials() {
  // Verify that the certificate files exist and are regular (non-symlink) files
  // If files are not present or not accesible, load insecure credentials
  std::shared_ptr<::grpc::ChannelCredentials> client_credentials_;

  if (IsRegularFile(ca_cert_file) && IsRegularFile(client_cert_file) &&
      IsRegularFile(client_key_file)) {
    auto certificate_provider =
        std::make_shared<FileWatcherCertificateProvider>(
            client_key_file, client_cert_file, ca_cert_file,
            kFileRefreshIntervalSeconds);
    auto tls_opts = std::make_shared<TlsChannelCredentialsOptions>();
    tls_opts->set_certificate_provider(certificate_provider);
    tls_opts->watch_root_certs();
    if (!ca_cert_file.empty() && !client_key_file.empty()) {
      tls_opts->watch_identity_key_cert_pairs();
    }
    client_credentials_ = ::grpc::experimental::TlsCredentials(*tls_opts);
  } else {
    client_credentials_ = ::grpc::InsecureChannelCredentials();
    printf("Using insecure client credentials!\n");
  }
  return client_credentials_;
}

}  // namespace ovs_p4rt
