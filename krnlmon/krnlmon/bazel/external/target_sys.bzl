# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

def _impl(repository_ctx):
    if "SDE_INSTALL" in repository_ctx.os.environ:
        install_path = repository_ctx.os.environ["SDE_INSTALL"]
    else:
        repository_ctx.file("BUILD.bazel", "")
        return
    repository_ctx.symlink(install_path, "target-sys")
    repository_ctx.symlink(
        Label("@//bazel:external/target_sys.BUILD"),
        "BUILD.bazel",
    )

configure_target_sys = repository_rule(
    implementation = _impl,
    local = True,
    environ = ["SDE_INSTALL"],
)
