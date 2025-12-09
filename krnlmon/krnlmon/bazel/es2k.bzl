# Copyright 2018-present Open Networking Foundation
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

# This Starlark rule imports the ES2K SDE shared libraries and headers.
# The ES2K_INSTALL or ES2K_INSTALL_TAR environment variable needs to be set;
# otherwise, the Stratum rules for ES2K platforms cannot be built.

def _impl(repository_ctx):
    if "ES2K_INSTALL" in repository_ctx.os.environ:
        es2k_sde_path = repository_ctx.os.environ["ES2K_INSTALL"]
    elif "SDE_INSTALL" in repository_ctx.os.environ:
        es2k_sde_path = repository_ctx.os.environ["SDE_INSTALL"]
    else:
        repository_ctx.file("BUILD", "")
        return
    target = repository_ctx.read(es2k_sde_path + "/share/TARGET").strip().upper()
    print("Target is: '" + target + "'")
    if target != "ES2K":
        print("SDE is not ES2K")
        repository_ctx.file("BUILD", "")
        return
    repository_ctx.symlink(es2k_sde_path, "es2k-bin")
    repository_ctx.symlink(Label("@//bazel:external/es2k.BUILD"), "BUILD")

es2k_configure = repository_rule(
    implementation = _impl,
    local = True,
    environ = ["ES2K_INSTALL", "SDE_INSTALL"],
)
