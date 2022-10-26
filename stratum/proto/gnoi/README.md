# gNOI Protobuf files for Stratum

The files should be updated if the Stratum version changes.

You will need to edit the file paths in the *import* statements for the
Protobufs to build correctly.

The following CMake snippet downloads and patches the specified version of
the gNOI Protobuf files. It is equivalent to what the Bazel build does.

## stratum/proto/CMakeLists.txt

```cmake
################################
# Download gNOI Protobuf files #
################################

include(ExternalProject)

# Update to correspond to the version used by Stratum.
# The definitions can be found in stratum/stratum/bazel/deps.bzl.
set(GNOI_COMMIT 437c62e630389aa4547b4f0521d0bca3fb2bf811)
set(GNOI_SHA 77d8c271adc22f94a18a5261c28f209370e87a5e615801a4e7e0d09f06da531f)

# Path to the stratum/proto/gnoi directory.
set(PROTO_GNOI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gnoi)

ExternalProject_Add(fetch-gnoi
  URL https://github.com/openconfig/gnoi/archive/${GNOI_COMMIT}.zip
  URL_HASH SHA256=${GNOI_SHA}
  SOURCE_DIR gnoi-temp
  PATCH_COMMAND
    find . -name *.proto | xargs sed -i -e "s#github.com/openconfig/##g"
  COMMAND
    cp -v cert/cert.proto ${PROTO_GNOI_DIR}/cert
  COMMAND
    cp -v common/common.proto ${PROTO_GNOI_DIR}/common
  COMMAND
    cp -v diag/diag.proto ${PROTO_GNOI_DIR}/diag
  COMMAND
    cp -v file/file.proto ${PROTO_GNOI_DIR}/file
  COMMAND
    cp -v system/system.proto ${PROTO_GNOI_DIR}/system
  COMMAND
    cp -v types/types.proto ${PROTO_GNOI_DIR}/types
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

# The target is only built when explicitly requested.
#     cmake --build build --target fetch-gnoi
set_target_properties(fetch-gnoi PROPERTIES EXCLUDE_FROM_ALL TRUE)
```
