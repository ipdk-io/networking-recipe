// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "client_cert_options.h"

#include "gflags/gflags.h"

#define DEFAULT_CERTS_DIR "/usr/share/stratum/certs/"

void set_client_cert_defaults() {
  FLAGS_ca_cert_file = DEFAULT_CERTS_DIR "ca.crt";
  FLAGS_server_key_file = DEFAULT_CERTS_DIR "stratum.key";
  FLAGS_server_cert_file = DEFAULT_CERTS_DIR "stratum.crt";
  FLAGS_client_key_file = DEFAULT_CERTS_DIR "client.key";
  FLAGS_client_cert_file = DEFAULT_CERTS_DIR "client.crt";
}
