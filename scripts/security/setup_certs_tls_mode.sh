# Copyright (c) 2023 Intel Corporation.
# SPDX-License-Identifier: Apache-2.0
#! /bin/bash

# This script generates TLS certificates using OpenSSL library.
# In order to support TLS-mode for gRPC communication, certificates
# and keys are generated and stored in /usr/share/stratum/certs
# folder. The CredentialsManager will use this location to initiate
# the server.
#
# Note this generates certificates for "localhost"
# This may not be desirable for most production systems. Users
# are recommended to re-generate certificates for specific
# server IPs or use FQDN in CN/SAN list.

echo "#### \
THIS SCRIPT DOES ONE-TIME CERT GENERATION TO ENABLE TLS MODE\
####"

CERTS_DIR_LOCATION=/usr/share/stratum/certs/

echo "Generating certificates for 'localhost'"
COMMON_NAME=localhost $IPDK_RECIPE/stratum/stratum/tools/tls/generate-certs.sh

if [ $? -eq 0 ]; then
   echo "Deleting old installed certificates"
   rm -rf $CERTS_DIR_LOCATION
   cp -pr $IPDK_RECIPE/stratum/stratum/tools/tls/certs/ $CERTS_DIR_LOCATION
   echo "Certificates generated and installed successfully in " $CERTS_DIR_LOCATION
else
   echo "Failed to generate certificates to enable TLS mode"
fi
