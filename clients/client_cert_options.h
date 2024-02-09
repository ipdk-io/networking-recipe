// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef CLIENT_CERT_OPTIONS_H_
#define CLIENT_CERT_OPTIONS_H_

#include "gflags/gflags.h"

DECLARE_string(ca_cert_file);
DECLARE_string(client_cert_file);
DECLARE_string(client_key_file);
DECLARE_string(server_cert_file);
DECLARE_string(server_key_file);

void set_client_cert_defaults();

#endif  // CLIENT_CERT_OPTIONS_H_
