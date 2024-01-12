// Copyright 2018 Google LLC
// Copyright 2018-present Open Networking Foundation
// Copyright 2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_LIB_SECURITY_CREDENTIALS_MANAGER_H_
#define STRATUM_LIB_SECURITY_CREDENTIALS_MANAGER_H_

#include <memory>
#include <string>

#include "grpc/grpc_security_constants.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/security/server_credentials.h"
#include "grpcpp/security/tls_credentials_options.h"
#include "stratum/glue/status/status.h"
#include "stratum/glue/status/statusor.h"

// Declaring the flags in the header file, so that all other files that
// "#include" the header file already get the flags declared. This is one of
// the recommended methods of using gflags according to documentation
DECLARE_string(ca_cert_file);
DECLARE_string(server_key_file);
DECLARE_string(server_cert_file);
DECLARE_string(client_key_file);
DECLARE_string(client_cert_file);
DECLARE_string(grpc_client_cert_req_type);

namespace stratum {

// CredentialsManager manages the server credentials for (external facing) gRPC
// servers. It handles starting and shutting down TSI as well as generating the
// server credentials. This class is supposed to be created once for each
// binary.
class CredentialsManager {
 public:
  virtual ~CredentialsManager();

  // Generates server credentials for an external facing gRPC server.
  virtual std::shared_ptr<::grpc::ServerCredentials>
  GenerateExternalFacingServerCredentials() const;

  // Generates client credentials for contacting an external facing gRPC server.
  virtual std::shared_ptr<::grpc::ChannelCredentials>
  GenerateExternalFacingClientCredentials() const;

  // Loads new server credentials.
  virtual ::util::Status LoadNewServerCredentials(const std::string& ca_cert,
                                                  const std::string& cert,
                                                  const std::string& key);

  // Loads new client credentials.
  virtual ::util::Status LoadNewClientCredentials(const std::string& ca_cert,
                                                  const std::string& cert,
                                                  const std::string& key);

  // Factory method for creating an instance of this class.
  // Updated to take a 'secure_only' parameter. If the parameter is true,
  // CredentialsManager will only attempt to initiate using TLS certificates.
  // The default value is false, for backward compatibility.
  static ::util::StatusOr<std::unique_ptr<CredentialsManager>> CreateInstance(
      bool secure_only = false);

  // CredentialsManager is neither copyable nor movable.
  CredentialsManager(const CredentialsManager&) = delete;
  CredentialsManager& operator=(const CredentialsManager&) = delete;

 protected:
  // Default constructor. To be called by the Mock class instance as well as
  // CreateInstance().
  CredentialsManager();

 private:
  static constexpr unsigned int kFileRefreshIntervalSeconds = 1;

  const std::map<std::string, grpc_ssl_client_certificate_request_type>
      client_cert_verification_map_ = {
          {"NO_REQUEST_CLIENT_CERT", GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE},
          {"REQUEST_CLIENT_CERT_NO_VERIFY",
           GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_BUT_DONT_VERIFY},
          {"REQUEST_CLIENT_CERT_AND_VERIFY",
           GRPC_SSL_REQUEST_CLIENT_CERTIFICATE_AND_VERIFY},
          {"REQUIRE_CLIENT_CERT_NO_VERIFY",
           GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_BUT_DONT_VERIFY},
          {"REQUIRE_CLIENT_CERT_AND_VERIFY",
           GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY}};

  // Function to initialize the credentials manager.
  ::util::Status Initialize(bool secure_only);
  std::shared_ptr<::grpc::ServerCredentials> server_credentials_;
  std::shared_ptr<::grpc::ChannelCredentials> client_credentials_;

  friend class CredentialsManagerTest;
};

}  // namespace stratum

#endif  // STRATUM_LIB_SECURITY_CREDENTIALS_MANAGER_H_