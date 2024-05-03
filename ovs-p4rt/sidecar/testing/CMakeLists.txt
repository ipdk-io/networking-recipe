add_library(ovsp4rt_logging_o OBJECT
  ${SIDECAR_SOURCE_DIR}/common/ovsp4rt_logutils.cc
  ${SIDECAR_SOURCE_DIR}/common/ovsp4rt_logutils.h
  ${SIDECAR_SOURCE_DIR}/lib/ovsp4rt_logging.cc
  ${SIDECAR_SOURCE_DIR}/lib/ovsp4rt_logging.h
)

target_include_directories(ovsp4rt_logging_o PRIVATE
  ${SIDECAR_SOURCE_DIR}
  ${SDE_INSTALL_DIR}/include
)

add_library(ovsp4rt_logging SHARED $<TARGET_OBJECTS:ovsp4rt_logging_o>)

add_executable(logging_test EXCLUDE_FROM_ALL
  logging_test.cc
)

target_include_directories(logging_test PUBLIC
  ${SIDECAR_SOURCE_DIR}
  ${DEPEND_INSTALL_DIR}/include
  ${SDE_INSTALL_DIR}/include
  ${OVS_INCLUDE_DIR}
)

target_link_libraries(logging_test PUBLIC
  ovsp4rt_logging
  absl::strings
  sde::target_sys
)
