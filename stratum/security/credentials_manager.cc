// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/lib/security/credentials_manager.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "stratum/glue/gtl/map_util.h"
#include "stratum/glue/logging.h"
#include "stratum/hal/lib/common/utils.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/utils.h"

DEFINE_string(ca_cert_file, "", "Path to CA certificate file");
DEFINE_string(server_key_file, "", "Path to gRPC server private key file");
DEFINE_string(server_cert_file, "", "Path to gRPC server certificate file");
DEFINE_string(client_key_file, "", "Path to gRPC client key file");
DEFINE_string(client_cert_file, "", "Path to gRPC client certificate file");

DEFINE_string(
    grpc_client_cert_req_type, "NO_REQUEST_CLIENT_CERT",
    "TLS server credentials option for client certificate verification. "
    "Available options are: NO_REQUEST_CLIENT_CERT, "
    "REQUEST_CLIENT_CERT_NO_VERIFY, REQUEST_CLIENT_CERT_AND_VERIFY, "
    "REQUIRE_CLIENT_CERT_NO_VERIFY, REQUIRE_CLIENT_CERT_AND_VERIFY");

namespace stratum {

using ::grpc::experimental::FileWatcherCertificateProvider;
using ::grpc::experimental::TlsChannelCredentialsOptions;
using ::grpc::experimental::TlsServerCredentials;
using ::grpc::experimental::TlsServerCredentialsOptions;

constexpr unsigned int CredentialsManager::kFileRefreshIntervalSeconds;

CredentialsManager::CredentialsManager() {}

CredentialsManager::~CredentialsManager() {}

::util::StatusOr<std::unique_ptr<CredentialsManager>>
CredentialsManager::CreateInstance(bool secure_only /*=false*/) {
  auto instance = absl::WrapUnique(new CredentialsManager());
  RETURN_IF_ERROR(instance->Initialize(secure_only));
  return std::move(instance);
}

std::shared_ptr<::grpc::ServerCredentials>
CredentialsManager::GenerateExternalFacingServerCredentials() const {
  return server_credentials_;
}

std::shared_ptr<::grpc::ChannelCredentials>
CredentialsManager::GenerateExternalFacingClientCredentials() const {
  return client_credentials_;
}

::util::Status CredentialsManager::Initialize(bool secure_only) {
  // Server credentials.
  if (FLAGS_ca_cert_file.empty() && FLAGS_server_key_file.empty() &&
      FLAGS_server_cert_file.empty()) {
    if (secure_only) {
      LOG(WARNING) << "No certificate/key files provided, cannot initiate gRPC "
                      "server credentials";
      server_credentials_ = nullptr;
    } else {
      LOG(WARNING) << "No certificate/key files provided, using insecure "
                      "server credentials!";
      server_credentials_ = ::grpc::InsecureServerCredentials();
    }
  } else {
    // Verify that the certificate files exist and are regular (non-symlink)
    // files If files are not present or not accesible, method will return with
    // nullptr
    if (stratum::hal::IsRegularFile(FLAGS_ca_cert_file) &&
        stratum::hal::IsRegularFile(FLAGS_server_cert_file) &&
        stratum::hal::IsRegularFile(FLAGS_server_key_file)) {
      auto certificate_provider =
          std::make_shared<FileWatcherCertificateProvider>(
              FLAGS_server_key_file, FLAGS_server_cert_file, FLAGS_ca_cert_file,
              kFileRefreshIntervalSeconds);
      auto tls_opts =
          std::make_shared<TlsServerCredentialsOptions>(certificate_provider);
      auto client_cert_verify_option = gtl::FindOrNull(
          client_cert_verification_map_, FLAGS_grpc_client_cert_req_type);
      if (client_cert_verify_option == nullptr) {
        return MAKE_ERROR(ERR_INVALID_PARAM)
               << "gRPC client certification verification option '"
               << FLAGS_grpc_client_cert_req_type << "' is invalid";
      }
      tls_opts->set_cert_request_type(*client_cert_verify_option);
      tls_opts->watch_root_certs();
      tls_opts->watch_identity_key_cert_pairs();
      server_credentials_ = TlsServerCredentials(*tls_opts);
    } else {
      LOG(ERROR) << "Cannot access certificate/key files. Unable to initiate "
                    "server_credentials.";
      server_credentials_ = nullptr;
    }
  }

  // Client credentials.
  if (FLAGS_ca_cert_file.empty() && FLAGS_client_key_file.empty() &&
      FLAGS_client_cert_file.empty()) {
    if (secure_only) {
      LOG(WARNING) << "No certificate/key files provided, cannot initiate gRPC "
                      "client credentials";
    } else {
      client_credentials_ = ::grpc::InsecureChannelCredentials();
      LOG(WARNING)
          << "No key files provided, using insecure client credentials!";
    }
  } else {
    // Verify that the certificate files exist and are regular (non-symlink)
    // files If files are not present or not accesible, method will return with
    // nullptr
    if (stratum::hal::IsRegularFile(FLAGS_ca_cert_file) &&
        stratum::hal::IsRegularFile(FLAGS_client_cert_file) &&
        stratum::hal::IsRegularFile(FLAGS_client_key_file)) {
      auto certificate_provider =
          std::make_shared<FileWatcherCertificateProvider>(
              FLAGS_client_key_file, FLAGS_client_cert_file, FLAGS_ca_cert_file,
              kFileRefreshIntervalSeconds);
      auto tls_opts = std::make_shared<TlsChannelCredentialsOptions>();
      tls_opts->set_certificate_provider(certificate_provider);
#if 0
      // Commenting out set_server_verification_option() since this was removed from
      // ::grpc::TlsChannelCredentialsOptions in v1.50.0. Including this file causes
      // a compilation error, since this API does not exist anymore. 
      // ::grpc::TlsChannelCredentialsOptions() now has set_verify_server_certs() instead
      // and the default is true.

      tls_opts->set_server_verification_option(GRPC_TLS_SERVER_VERIFICATION);
#endif
      tls_opts->watch_root_certs();
      if (!FLAGS_ca_cert_file.empty() && !FLAGS_client_key_file.empty()) {
        tls_opts->watch_identity_key_cert_pairs();
      }
      client_credentials_ = ::grpc::experimental::TlsCredentials(*tls_opts);
    } else {
      LOG(ERROR) << "Cannot access certificate/key files. Unable to initiate "
                    "client_credentials.";
      client_credentials_ = nullptr;
    }
  }

  return ::util::OkStatus();
}

::util::Status CredentialsManager::LoadNewServerCredentials(
    const std::string& root_certs, const std::string& cert_chain,
    const std::string& private_key) {
  ::util::Status status;
  // TODO(Kevin): Validate the provided key material if possible
  // TODO(max): According to the API of FileWatcherCertificateProvider, any key
  // and certifcate update must happen atomically. The below code does not
  // guarantee that.
  status.Update(WriteStringToFile(root_certs, FLAGS_ca_cert_file));
  status.Update(WriteStringToFile(cert_chain, FLAGS_server_cert_file));
  status.Update(WriteStringToFile(private_key, FLAGS_server_key_file));
  absl::SleepFor(absl::Seconds(kFileRefreshIntervalSeconds + 1));

  return status;
}

::util::Status CredentialsManager::LoadNewClientCredentials(
    const std::string& root_certs, const std::string& cert_chain,
    const std::string& private_key) {
  ::util::Status status;
  // TODO(Kevin): Validate the provided key material if possible
  // TODO(max): According to the API of FileWatcherCertificateProvider, any key
  // and certifcate update must happen atomically. The below code does not
  // guarantee that.
  status.Update(WriteStringToFile(root_certs, FLAGS_ca_cert_file));
  status.Update(WriteStringToFile(cert_chain, FLAGS_client_cert_file));
  status.Update(WriteStringToFile(private_key, FLAGS_client_key_file));
  absl::SleepFor(absl::Seconds(kFileRefreshIntervalSeconds + 1));

  return status;
}

}  // namespace stratum
