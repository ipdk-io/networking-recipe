# OpenSSL Guide

This document provides information about supported OpenSSL versions and
security details that you need to consider.

OpenSSL is a package dependency as infrap4d uses the library for gRPC.

## End of Life for OpenSSL 1.1.1

OpenSSL 1.1.1 has reached End of Life (EOL) in September 2023. 

It is highly recommended that you upgrade OpenSSL from 1.1.1x to OpenSSL 3.x.
The official migration guide is [available here](https://www.openssl.org/docs/man3.0/man7/migration_guide.html).

Starting Fedora 37, Ubuntu 22.04 and Rocky Linux 9.0 OpenSSL 3.0.x comes
standard and requires no further action.

Older distributions of Linux systems download and install OpenSSL 1.1.1 when
running `yum install` or `apt install` commands. If you are using these
operating systems, you will need to upgrade the installed package (either find
an RPM to install upgrade or compile from source).

Note that infrap4d will compile and run normally with OpenSSL 1.1.1 since
OpenSSL 3.0 is backwards-compatible. In the interest of best security practices
and future security risks, we still recommend upgrading to OpenSSL 3.0.
