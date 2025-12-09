#!/bin/bash
# Copyright 2020-present Open Networking Foundation
# Copyright 2023,2025 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

show_help() {
    cat << EOF
Usage: $(basename $0) [options]

This script accepts configuration through environment variables:

Environment Variables:
  SAN_DNS        DNS names to include in the certificate (comma or space separated)
  SAN_IP         IP addresses to include in the certificate (comma or space separated)
  COMMON_NAME    Common Name for the certificate

Examples:
  # Generate certificate with default settings (default CN=localhost)
  ./$(basename $0)

  # Generate certificate with IP addresses
  COMMON_NAME=10.10.0.2 ./$(basename $0)
  SAN_IP="192.168.1.1,1.2.3.4" COMMON_NAME=10.10.0.2 ./$(basename $0)

  # Generate certificate with a DNS names
  COMMON_NAME="example.com" ./$(basename $0)
  SAN_DNS="example.com,test.org,another.domain" COMMON_NAME="ipdk.io" ./$(basename $0)
    
  # Combine DNS names and IP addresses
  SAN_DNS="example.com" SAN_IP="192.168.1.1" COMMON_NAME="ipdk.io" ./$(basename $0)

Options:
  -h, --help     Display this help message and exit
EOF
    exit 0
}

# Check for help flag
if [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
    show_help
fi

set -e

THIS_DIR=$(dirname "${BASH_SOURCE[0]}")
COMMON_NAME=${COMMON_NAME:-"localhost"}

# Additional SANs can be passed as environment variables
SAN_DNS=${SAN_DNS:-""}
SAN_IP=${SAN_IP:-""}

echo "Creating certificates for CN=$COMMON_NAME"

mkdir -p "$THIS_DIR/certs"
rm -rf "$THIS_DIR/certs/"*

# Create temporary server config with environment variables replaced
SERVER_CONF_FILE="$(mktemp)"
sed "s/\${COMMON_NAME}/$COMMON_NAME/g" "$THIS_DIR/grpc-server.conf" > "$SERVER_CONF_FILE"

# Populate SANs
if [[ -n "$SAN_DNS" ]]; then
    # Split the SAN_DNS string by commas or spaces
    IFS=', ' read -r -a dns_array <<< "$SAN_DNS"
    dns_index=2  # Start from 2 since DNS.1 is already set to COMMON_NAME
    for dns in "${dns_array[@]}"; do
        echo "DNS.$dns_index = $dns" >> "$SERVER_CONF_FILE"
        ((dns_index++))
    done
fi

if [[ -n "$SAN_IP" ]]; then
    # Split the SAN_IP string by commas or spaces
    IFS=', ' read -r -a ip_array <<< "$SAN_IP"
    ip_index=1
    for ip in "${ip_array[@]}"; do
        echo "IP.$ip_index = $ip" >> "$SERVER_CONF_FILE"
        ((ip_index++))
    done
fi

echo "=== Generating CA certificate ==="
# Generate CA private key and certificate
openssl genrsa -out "$THIS_DIR/certs/ca.key" 4096
openssl req -new -x509 -key "$THIS_DIR/certs/ca.key" -out "$THIS_DIR/certs/ca.crt" \
    -config "$THIS_DIR/ca.conf" -days 365 -sha512
echo "CA certificate generated successfully"

echo "=== Generating server certificate ==="
# Generate server private key and CSR
openssl genrsa -out "$THIS_DIR/certs/stratum.key" 4096
openssl req -new -key "$THIS_DIR/certs/stratum.key" -out "$THIS_DIR/certs/stratum.csr" \
    -config "$SERVER_CONF_FILE" -sha512

# Sign server certificate with CA
openssl x509 -req -in "$THIS_DIR/certs/stratum.csr" -CA "$THIS_DIR/certs/ca.crt" \
    -CAkey "$THIS_DIR/certs/ca.key" -CAcreateserial -out "$THIS_DIR/certs/stratum.crt" \
    -days 30 -sha512 -extfile "$SERVER_CONF_FILE" -extensions server_ext
echo "Server certificate generated successfully"

echo "=== Generating client certificate ==="
# Generate client private key and CSR
openssl genrsa -out "$THIS_DIR/certs/client.key" 4096
openssl req -new -key "$THIS_DIR/certs/client.key" -out "$THIS_DIR/certs/client.csr" \
    -config "$THIS_DIR/grpc-client.conf" -sha512

# Sign client certificate with CA
openssl x509 -req -in "$THIS_DIR/certs/client.csr" -CA "$THIS_DIR/certs/ca.crt" \
    -CAkey "$THIS_DIR/certs/ca.key" -CAcreateserial -out "$THIS_DIR/certs/client.crt" \
    -days 30 -sha512 -extfile "$THIS_DIR/grpc-client.conf" -extensions client_ext
echo "Client certificate generated successfully"

# Install CA certificate to system trust store
echo "=== Installing CA certificate to system trust store ==="
cp "$THIS_DIR/certs/ca.crt" /etc/pki/ca-trust/source/anchors/
update-ca-trust extract
echo "CA certificate installation complete"

# Cleanup
rm "$SERVER_CONF_FILE"

echo "=== Certificate generation completed successfully ==="
echo ""
