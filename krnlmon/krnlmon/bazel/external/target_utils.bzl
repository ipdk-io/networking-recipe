# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

def _impl(repository_ctx):
    if "SDE_INSTALL" in repository_ctx.os.environ:
        target_utils_path = repository_ctx.os.environ["SDE_INSTALL"]
    else:
        repository_ctx.file("BUILD.bazel", "")
        return
    repository_ctx.symlink(target_utils_path, "target-utils")
    repository_ctx.symlink(
        Label("@//bazel:external/target_utils.BUILD"),
        "BUILD.bazel",
    )

configure_target_utils = repository_rule(
    implementation = _impl,
    local = True,
    environ = ["SDE_INSTALL"],
)
