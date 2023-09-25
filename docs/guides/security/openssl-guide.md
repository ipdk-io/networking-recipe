# OpenSSL Guide

This document provides information about supported OpenSSL versions and
security details that you need to consider.

OpenSSL is a package dependency, as infrap4d uses the library for gRPC.

## End of Life for OpenSSL 1.1.1

OpenSSL 1.1.1 has reached End of Life (EOL) in September 2023. 

It is highly recommended that you upgrade OpenSSL from 1.1.1x to OpenSSL 3.x.
See the [official migration guide](https://www.openssl.org/docs/man3.0/man7/migration_guide.html)
for more information.

Beginning with Fedora 37, Ubuntu 22.04, and Rocky Linux 9.0, OpenSSL 3.0.x comes
standard and requires no further action.

Older distributions of Linux systems download and install OpenSSL 1.1.1 when
you run the `yum install` or `apt install` command. If you are using one of these
distributions, you will need to find an RPM or DEB package to install
or build OpenSSL 3.x from source.

Note that infrap4d will compile and run normally with OpenSSL 1.1.1, since
OpenSSL 3.0 is backward compatible. In the interest of following best security practices
and avoiding future security issues, we recommend upgrading to OpenSSL 3.0.
