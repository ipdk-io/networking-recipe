// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "stratum/lib/security/credentials_manager.h"

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "absl/strings/ascii.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "stratum/glue/gtl/map_util.h"
#include "stratum/glue/logging.h"
#include "stratum/glue/status/statusor.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/utils.h"

DEFINE_string(ca_cert_file, "", "Path to CA certificate file");
DEFINE_string(server_key_file, "", "Path to gRPC server private key file");
DEFINE_string(server_cert_file, "", "Path to gRPC server certificate file");
DEFINE_string(client_key_file, "", "Path to gRPC client key file");
DEFINE_string(client_cert_file, "", "Path to gRPC client certificate file");
DEFINE_string(client_cert_req_type, "no_request",
              "Client certificate request type");

namespace stratum {

using ::grpc::experimental::FileWatcherCertificateProvider;
using ::grpc::experimental::TlsChannelCredentialsOptions;
using ::grpc::experimental::TlsServerCredentials;
using ::grpc::experimental::TlsServerCredentialsOptions;

constexpr unsigned int CredentialsManager::kFileRefreshIntervalSeconds;

namespace {

const std::map<const std::string, grpc_ssl_client_certificate_request_type>
    client_cert_req_type_map = {
        {"no_request", GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE},
        {"request_no_verify",
         GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_BUT_DONT_VERIFY},
        {"request_and_verify", GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_AND_VERIFY},
        {"require_no_verify",
         GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_BUT_DONT_VERIFY},
        {"require_and_verify",
         GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY}};

// Returns the value of the client_cert_req_type command-line flag.
::util::StatusOr<grpc_ssl_client_certificate_request_type>
GetCertRequestType() {
  std::string flag_value = absl::AsciiStrToLower(FLAGS_client_cert_req_type);
  auto option = gtl::FindOrNull(client_cert_req_type_map, flag_value);
  if (option == nullptr) {
    return MAKE_ERROR(ERR_INVALID_PARAM)
           << "--client_cert_req_type '" << FLAGS_client_cert_req_type
           << "' is invalid";
  }
  return *option;
}

// Client files exist and are regular (non-symlink) files.
bool ClientFilesOk() {
  return (IsRegularFile(FLAGS_ca_cert_file) &&
          IsRegularFile(FLAGS_client_cert_file) &&
          IsRegularFile(FLAGS_client_key_file));
}

// Server files exist and are regular (non-symlink) files.
bool ServerFilesOk() {
  return (IsRegularFile(FLAGS_ca_cert_file) &&
          IsRegularFile(FLAGS_server_cert_file) &&
          IsRegularFile(FLAGS_server_key_file));
}

}  // namespace

CredentialsManager::CredentialsManager(bool secure_only)
    : secure_only_(secure_only) {}

CredentialsManager::~CredentialsManager() {}

::util::StatusOr<std::unique_ptr<CredentialsManager>>
CredentialsManager::CreateInstance(bool secure_only) {
  auto instance = absl::WrapUnique(new CredentialsManager(secure_only));
  RETURN_IF_ERROR(instance->Initialize());
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

::util::Status CredentialsManager::Initialize() {
  RETURN_IF_ERROR(InitializeServerCredentials());
  return InitializeClientCredentials();
}

::util::Status CredentialsManager::InitializeServerCredentials() {
  if (FLAGS_ca_cert_file.empty() && FLAGS_server_key_file.empty() &&
      FLAGS_server_cert_file.empty()) {
    // No server credentials files
    if (secure_only_) {
      LOG(WARNING) << "No key/certificate files specified; "
                      "cannot initialize server credentials.";
    } else {
      LOG(WARNING) << "No key/certificate files specified; "
                      "using insecure server credentials!";
      server_credentials_ = ::grpc::InsecureServerCredentials();
    }
  } else if (secure_only_ && !ServerFilesOk()) {
    LOG(ERROR) << "Unable to access certificate/key files; "
                  "cannot initialize server credentials.";
  } else {
    // Initialize the server credentials
    auto certificate_provider =
        std::make_shared<FileWatcherCertificateProvider>(
            FLAGS_server_key_file, FLAGS_server_cert_file, FLAGS_ca_cert_file,
            kFileRefreshIntervalSeconds);
    auto tls_opts =
        std::make_shared<TlsServerCredentialsOptions>(certificate_provider);
    auto request_type = GetCertRequestType();
    if (!request_type.ok()) return request_type.status();
    tls_opts->set_cert_request_type(request_type.ValueOrDie());
    tls_opts->watch_root_certs();
    tls_opts->watch_identity_key_cert_pairs();
    server_credentials_ = TlsServerCredentials(*tls_opts);
  }
  return ::util::OkStatus();
}

::util::Status CredentialsManager::InitializeClientCredentials() {
  if (FLAGS_ca_cert_file.empty() && FLAGS_client_key_file.empty() &&
      FLAGS_client_cert_file.empty()) {
    // No client credentials files
    if (secure_only_) {
      LOG(WARNING) << "No key/certificate files specified; "
                      "cannot initialize client credentials.";
    } else {
      LOG(WARNING) << "No key/certificate files specified; "
                      "using insecure client credentials!";
      client_credentials_ = ::grpc::InsecureChannelCredentials();
    }
  } else if (secure_only_ && !ClientFilesOk()) {
    LOG(ERROR) << "Unable to access certificate/key files; "
                  "cannot initialize client credentials.";
  } else {
    // Initialize the client credentials
    auto certificate_provider =
        std::make_shared<FileWatcherCertificateProvider>(
            FLAGS_client_key_file, FLAGS_client_cert_file, FLAGS_ca_cert_file,
            kFileRefreshIntervalSeconds);
    auto tls_opts = std::make_shared<TlsChannelCredentialsOptions>();
    tls_opts->set_certificate_provider(certificate_provider);
    tls_opts->watch_root_certs();
    if (!FLAGS_ca_cert_file.empty() && !FLAGS_client_key_file.empty()) {
      tls_opts->watch_identity_key_cert_pairs();
    }
    client_credentials_ = ::grpc::experimental::TlsCredentials(*tls_opts);
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
