// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_STATUS_H_
#define STRATUM_HAL_LIB_TDI_TDI_STATUS_H_

extern "C" {
#include "tdi/common/tdi_defs.h"
}

#include "stratum/glue/status/status.h"
#include "stratum/lib/macros.h"
#include "stratum/public/lib/error.h"

namespace stratum {
namespace hal {
namespace tdi {

// Wrapper object for a tdi_status code from the SDE.
class TdiStatus {
 public:
  explicit TdiStatus(tdi_status_t status) : status_(status) {}
  operator bool() const { return status_ == TDI_SUCCESS; }
  inline tdi_status_t status() const { return status_; }
  inline ErrorCode error_code() const {
    switch (status_) {
      case TDI_SUCCESS:
        return ERR_SUCCESS;
      case TDI_NOT_READY:
        return ERR_NOT_INITIALIZED;
      case TDI_INVALID_ARG:
        return ERR_INVALID_PARAM;
      case TDI_ALREADY_EXISTS:
        return ERR_ENTRY_EXISTS;
      case TDI_NO_SYS_RESOURCES:
      case TDI_MAX_SESSIONS_EXCEEDED:
      case TDI_NO_SPACE:
      case TDI_EAGAIN:
        return ERR_NO_RESOURCE;
      case TDI_ENTRY_REFERENCES_EXIST:
        return ERR_FAILED_PRECONDITION;
      case TDI_TXN_NOT_SUPPORTED:
      case TDI_NOT_SUPPORTED:
        return ERR_OPER_NOT_SUPPORTED;
      case TDI_HW_COMM_FAIL:
      case TDI_HW_UPDATE_FAILED:
        return ERR_HARDWARE_ERROR;
      case TDI_NO_LEARN_CLIENTS:
        return ERR_FEATURE_UNAVAILABLE;
      case TDI_IDLE_UPDATE_IN_PROGRESS:
        return ERR_OPER_STILL_RUNNING;
      case TDI_OBJECT_NOT_FOUND:
      case TDI_TABLE_NOT_FOUND:
        return ERR_ENTRY_NOT_FOUND;
      case TDI_NOT_IMPLEMENTED:
        return ERR_UNIMPLEMENTED;
      case TDI_SESSION_NOT_FOUND:
      case TDI_INIT_ERROR:
      case TDI_TABLE_LOCKED:
      case TDI_IO:
      case TDI_UNEXPECTED:
      case TDI_DEVICE_LOCKED:
      case TDI_INTERNAL_ERROR:
      case TDI_IN_USE:
      default:
        return ERR_INTERNAL;
    }
  }

 private:
  tdi_status_t status_;
};

// A macro to simplify checking and logging the return value of an SDE
// function call.
#define RETURN_IF_TDI_ERROR(expr)                             \
  if (const TdiStatus __ret = TdiStatus(expr)) {              \
  } else /* NOLINT */                                         \
    return MAKE_ERROR(__ret.error_code())                     \
           << "'" << #expr << "' failed with error message: " \
           << FixMessage(tdi_err_str(__ret.status()))

// A macro to simplify creating a new error or appending new info to an
// error based on the return value of an SDE function call. The function that
// invokes the macro does not return to the calling function. The "status"
// parameter must be an object of type ::util::Status.
#define APPEND_STATUS_IF_TDI_ERROR(status, expr)                            \
  if (const TdiStatus __ret = TdiStatus(expr)) {                            \
  } else /* NOLINT */                                                       \
    status =                                                                \
        APPEND_ERROR(!status.ok() ? status                                  \
                                  : ::util::Status(StratumErrorSpace(),     \
                                                   __ret.error_code(), "")) \
            .without_logging()                                              \
        << (status.error_message().empty() ||                               \
                    status.error_message().back() == ' '                    \
                ? ""                                                        \
                : " ")                                                      \
        << "'" << #expr << "' failed with error message: "                  \
        << FixMessage(tdi_err_str(__ret.status()))

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_STATUS_H_
