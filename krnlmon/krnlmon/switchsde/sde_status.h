/*
 * Copyright 2021-2024 Intel Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SDE_STATUS_H__
#define __SDE_STATUS_H__

#ifndef SDE_STATUS_DEFINED
/** Specifies an error status code. */
typedef int sde_status_t;
#define SDE_STATUS_DEFINED
#endif

#define SDE_STATUS_VALUES                                                    \
  SDE_STATUS_(SDE_SUCCESS, "Success"),                                       \
      SDE_STATUS_(SDE_NOT_READY, "Not ready"),                               \
      SDE_STATUS_(SDE_NO_SYS_RESOURCES, "No system resources"),              \
      SDE_STATUS_(SDE_INVALID_ARG, "Invalid arguments"),                     \
      SDE_STATUS_(SDE_ALREADY_EXISTS, "Already exists"),                     \
      SDE_STATUS_(SDE_HW_COMM_FAIL, "HW access fails"),                      \
      SDE_STATUS_(SDE_OBJECT_NOT_FOUND, "Object not found"),                 \
      SDE_STATUS_(SDE_MAX_SESSIONS_EXCEEDED, "Max sessions exceeded"),       \
      SDE_STATUS_(SDE_SESSION_NOT_FOUND, "Session not found"),               \
      SDE_STATUS_(SDE_NO_SPACE, "Not enough space"),                         \
      SDE_STATUS_(SDE_EAGAIN,                                                \
                  "Resource temporarily not available, try again later"),    \
      SDE_STATUS_(SDE_INIT_ERROR, "Initialization error"),                   \
      SDE_STATUS_(SDE_TXN_NOT_SUPPORTED, "Not supported in transaction"),    \
      SDE_STATUS_(SDE_TABLE_LOCKED, "Resource held by another session"),     \
      SDE_STATUS_(SDE_IO, "IO error"),                                       \
      SDE_STATUS_(SDE_UNEXPECTED, "Unexpected error"),                       \
      SDE_STATUS_(SDE_ENTRY_REFERENCES_EXIST,                                \
                  "Action data entry is being referenced by match entries"), \
      SDE_STATUS_(SDE_NOT_SUPPORTED, "Operation not supported"),             \
      SDE_STATUS_(SDE_HW_UPDATE_FAILED, "Updating hardware failed"),         \
      SDE_STATUS_(SDE_NO_LEARN_CLIENTS, "No learning clients registered"),   \
      SDE_STATUS_(SDE_IDLE_UPDATE_IN_PROGRESS,                               \
                  "Idle time update state already in progress"),             \
      SDE_STATUS_(SDE_DEVICE_LOCKED, "Device locked"),                       \
      SDE_STATUS_(SDE_INTERNAL_ERROR, "Internal error"),                     \
      SDE_STATUS_(SDE_TABLE_NOT_FOUND, "Table not found"),                   \
      SDE_STATUS_(SDE_IN_USE, "In use"),                                     \
      SDE_STATUS_(SDE_NOT_IMPLEMENTED, "Object not implemented")

/** Defines the status code values. */
enum sde_status_enum {
#define SDE_STATUS_(x, y) x
  SDE_STATUS_VALUES,
  SDE_STS_MAX
#undef SDE_STATUS_
};

/** Returns the error string for a status code. */
extern const char* sde_err_str(sde_status_t sts);

#endif /* __SDE_STATUS_H__ */
