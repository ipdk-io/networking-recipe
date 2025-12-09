# TargetOptions Class

## Background

Three changes were made to common files in order to support the TDI targets.

1. When built for the DPDK target, `ConfigMonitoringService::DoSet()` no
   longer pushes the updated chassis configuration to the switch.

2. When built for the DPDK target, `P4Service::SetForwardingPipelineConfig()`
   returns an error if an attempt is made to overwrite an existing pipeline.

3. `ConfigMonitoringService::PushSavedChassisConfig()` returns an error if
   the configuration file path is a symlink. This change was made to mitigate
   a security weakness.

Changes 1 and 2 are controlled by `#ifdef DPDK_TARGET`. Change 3 is
not conditionalized. It is currently disabled via `#if 0` because it breaks
unit test cases that use symlinks to manage the test environment.

## Problem statement

- The Stratum code has very few compile-time conditionals, none of which are
  in common code. The DPDK_TARGET conditionals go against this convention.

- Compile-time conditionals complicate the Bazel build and lead to
  recompilation when we change targets.
  This is undesirable, especially in common source files.

- Change 3 is sensitive to which target is being built *and* whether the code
  is being compiled for unit test purposes. We do not want to introduce more
  compile-time conditionals, and we do not want to have to recompile a file
  for unit testing.

## Proposed solution

- Introduce a parameter object (`TargetOptions`) that can be passed to the
  `ConfigMonitoringService` and `P4Service` classes to influence their
  behavior.

- Define Boolean members to enable/disable each of the TDI changes.

- Instantiate the object in the target's main class, initialize its member
  values according to the needs of the target, and pass the object to the
  service classes when instantiating them.

- Have each option default to a value that maintains backward compatibility
  with previous versions of Stratum.

- Make the TargetOptions parameter optional, with the default options being
  used if the parameter is not specified. This removes the need to update
  legacy code.

## Source code

Changes:

- Create `target_options.cc` and `target_options.h`.

- Implement`target_options` parameter and `target_options_` member variable
in the `DpdkHal`, `Es2kHal`, `ConfigMonitoringService`, and `P4Service` classes.

- Modify `DpdkMain` and `Es2kMain` to create and initialize a `TargetOptions`
object and specify it when instantiating the `DpdkHal` and `Es2kHal` objects.

- Modify `DpdkHal` and `Es2kHal` to specify the `target_options` parameter
when creating the `ConfigMonitoringService` and `P4Service` objects.

- Modify `ConfigMonitoringService` and `P4Service` to use the appropriate
member of the `TargetOptions` class, instead of a compile-time conditional,
to enable the new behavior in those classes.

- Remove logic to define the `DPDK_TARGET` from the Bazel build files.

Notes:

- The security mitigation in `ConfigMonitoringService` has been reenabled.
  The unit test that caused the mitigation to be disabled now passes.

- There are no remaining occurrences of the `DPDK_TARGET` conditional in
  Stratum.

- The Tofino target (which is in "minimal maintenance" state) is unmodified.
  It compiles, and its unit tests pass, providing some assurance that we
  have maintained backward compatibility with existing code.
