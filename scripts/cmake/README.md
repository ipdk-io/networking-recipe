# Sample CMake Scripts

This folder contains sample scripts that may be customized and added
to your build environment.

## Preload scripts

- `dpdk.cmake`
- `es2k.cmake`

### About preload scripts

These scripts allow you to specify one or more sets of parameters to be
applied at cmake configuration time.

Each file defines one or more cmake variables to be loaded into the cache
after the command line is processed and before the first CMakeLists.txt
file is read.

To use a sample script, copy it to the top-level folder and make any
changes you wish. You can create as many preload scripts as you want.

Preload scripts are customer-specific and are not stored in the
repository.

### Configure with a preload script

```bash
cmake -B build -C dpdk.cmake [more options]
```

### Override an option

```bash
cmake -B build -C es2k.cmake -DCMAKE_INSTALL_PREFIX=/opt/p4cp
```

## Preset files

- `CMakeUserPresets.json`

### About presets

This file defines user presets that can be used to configure cmake.
It makes use of hidden presets that are defined in `CMakePresets.json`,
which is stored in the repository.

To use it, copy the file to the top-level (networking-recipe) directory
and make any changes you want. Deleting targets you don't plan to use
would be a good place to start.

User presets are customer-specific and are not stored in the repository.
The `.gitignore` file instructs git to ignore the file if it is in the
top-level directory.

Presets were introduced in cmake 3.19. This file uses schema version 4,
which requires cmake 3.23.0 or later.

See the [cmake-presets guide](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
for more information.

### List available presets

```text
frodo@bagend:~/networking-recipe$ cmake --list-presets
Available configure presets:

  "dpdk"         - DPDK target (P4OVS mode)
  "dpdk-ovsp4rt" - DPDK target (OVSP4RT mode)
  "es2k"         - ES2K target (P4OVS mode)
  "es2k-ovsp4rt" - ES2K target (OVSP4RT mode)
  "tofino"       - Tofino target (no OVS)
```

### Configure using a preset

```text
frodo@bagend:~/networking-recipe$ cmake --preset dpdk
Preset CMake variables:

  CMAKE_BUILD_TYPE="RelWithDebInfo"
  CMAKE_EXPORT_COMPILE_COMMANDS:BOOL="TRUE"
  CMAKE_INSTALL_PREFIX:PATH="home/bilbo/work/latest/install"
  DEPEND_INSTALL_DIR:PATH="/opt/deps"
  OVS_INSTALL_DIR:PATH="home/bilbo/work/latest/ovs/install"
  P4OVS_MODE="P4OVS"
  SDE_INSTALL_DIR:PATH="home/bilbo/SDE/install"
  SET_RPATH:BOOL="TRUE"
  TDI_TARGET="dpdk"

-- The C compiler identification is GNU 9.4.0
-- The CXX compiler identification is GNU 9.4.0
    .
    .
-- Configuring done (2.4s)
-- Generating done (0.4s)
-- Build files have been written to: /home/bilbo/networking-recipe/build
```
