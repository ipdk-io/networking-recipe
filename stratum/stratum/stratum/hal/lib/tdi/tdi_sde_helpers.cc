// Copyright 2019-present Barefoot Networks, Inc.
// Copyright 2022-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Helper functions for use within TdiSdeWrapper.

#include "stratum/hal/lib/tdi/tdi_sde_helpers.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <utility>

#include "absl/strings/str_cat.h"
#include "gflags/gflags.h"
#include "stratum/glue/gtl/stl_util.h"
#include "stratum/hal/lib/tdi/tdi_sde_common.h"
#include "stratum/hal/lib/tdi/tdi_sde_utils.h"
#include "stratum/hal/lib/tdi/tdi_status.h"
#include "stratum/hal/lib/tdi/utils.h"
#include "stratum/lib/macros.h"
#include "stratum/lib/utils.h"

constexpr int MINIMUM_BURST_SIZE = 1;
constexpr int MAXIMUM_BURST_SIZE = 1024;
constexpr int DEFAULT_BURST_SIZE = 20;

// TDI returns OBJECT_NOT_FOUND if it is unable to get any more entries.
// We also check for TDI_NOT_SUPPORTED because that's what the original
// code did.
static inline bool ObjectNotFound(tdi_status_t status) {
  return (status == TDI_OBJECT_NOT_FOUND) || (status == TDI_NOT_SUPPORTED);
}

namespace stratum {
namespace hal {
namespace tdi {
namespace helpers {

::util::StatusOr<std::string> DumpTableMetadata(const ::tdi::Table* table) {
  std::string table_name = table->tableInfoGet()->nameGet().c_str();
  tdi_id_t table_id = table->tableInfoGet()->idGet();
  tdi_table_type_e table_type = table->tableInfoGet()->tableTypeGet();

  return absl::StrCat("table_name: ", table_name, ", table_id: ", table_id,
                      ", table_type: ", table_type);
}

::util::StatusOr<std::string> DumpTableKey(const ::tdi::TableKey* table_key) {
  const ::tdi::Table* table;
  RETURN_IF_TDI_ERROR(table_key->tableGet(&table));
  std::vector<tdi_id_t> key_field_ids;
  key_field_ids = table->tableInfoGet()->keyFieldIdListGet();

  std::string s;
  absl::StrAppend(&s, "tdi_table_key { ");
  for (const auto& field_id : key_field_ids) {
    const ::tdi::KeyFieldInfo* keyFieldInfo =
        table->tableInfoGet()->keyFieldGet(field_id);
    std::string field_name;
    tdi_match_type_core_e key_type;
    size_t field_size;

    RETURN_IF_NULL(keyFieldInfo);
    field_name = keyFieldInfo->nameGet().c_str();
    key_type =
        static_cast<tdi_match_type_core_e>((*keyFieldInfo).matchTypeGet());
    field_size = keyFieldInfo->sizeGet();
    std::string value;

    switch (key_type) {
      case TDI_MATCH_TYPE_EXACT: {
        std::string v(NumBitsToNumBytes(field_size), '\x00');
        const char* valueExact = reinterpret_cast<const char*>(v.data());
        size_t size = reinterpret_cast<size_t>(v.size());

        ::tdi::KeyFieldValueExact<const char*> exactKey(valueExact, size);
        RETURN_IF_TDI_ERROR(table_key->getValue(field_id, &exactKey));
        value = absl::StrCat("0x", StringToHex(v));
        break;
      }

      case TDI_MATCH_TYPE_TERNARY: {
        std::string v(NumBitsToNumBytes(field_size), '\x00');
        std::string m(NumBitsToNumBytes(field_size), '\x00');
        const char* valueTernary = reinterpret_cast<const char*>(v.data());
        const char* maskTernary = reinterpret_cast<const char*>(m.data());
        size_t sizeTernary = reinterpret_cast<size_t>(v.size());
        ::tdi::KeyFieldValueTernary<const char*> ternaryKey(
            valueTernary, maskTernary, sizeTernary);

        RETURN_IF_TDI_ERROR(table_key->getValue(field_id, &ternaryKey));
        value = absl::StrCat("0x", StringToHex(v), " & ", "0x", StringToHex(m));
        break;
      }

      case TDI_MATCH_TYPE_RANGE: {
        std::string l(NumBitsToNumBytes(field_size), '\x00');
        std::string h(NumBitsToNumBytes(field_size), '\x00');
        const char* lowRange = reinterpret_cast<const char*>(l.data());
        const char* highRange = reinterpret_cast<const char*>(h.data());
        size_t sizeRange = reinterpret_cast<size_t>(l.size());
        ::tdi::KeyFieldValueRange<const char*> rangeKey(lowRange, highRange,
                                                        sizeRange);
        RETURN_IF_TDI_ERROR(table_key->getValue(field_id, &rangeKey));
        value = absl::StrCat("0x", StringToHex(l), " - ", "0x", StringToHex(h));
        break;
      }

      case TDI_MATCH_TYPE_LPM: {
        std::string v(NumBitsToNumBytes(field_size), '\x00');
        uint16 prefix_length = 0;
        const char* valueLpm = reinterpret_cast<const char*>(v.data());
        size_t sizeLpm = reinterpret_cast<size_t>(v.size());
        ::tdi::KeyFieldValueLPM<const char*> lpmKey(valueLpm, prefix_length,
                                                    sizeLpm);
        RETURN_IF_TDI_ERROR(table_key->getValue(field_id, &lpmKey));
        value = absl::StrCat("0x", StringToHex(v), "/", prefix_length);
        break;
      }
      default:
        return MAKE_ERROR(ERR_INTERNAL)
               << "Unknown key_type: " << static_cast<int>(key_type) << ".";
    }

    absl::StrAppend(&s, field_name, " { field_id: ", field_id,
                    " key_type: ", static_cast<int>(key_type),
                    " field_size: ", field_size, " value: ", value, " } ");
  }
  absl::StrAppend(&s, "}");
  return s;
}

::util::StatusOr<std::string> DumpTableData(
    const ::tdi::TableData* table_data) {
  const ::tdi::Table* table;
  RETURN_IF_TDI_ERROR(table_data->getParent(&table));

  std::string s;
  absl::StrAppend(&s, "tdi_table_data { ");
  std::vector<tdi_id_t> data_field_ids;

  tdi_id_t action_id = table_data->actionIdGet();
  absl::StrAppend(&s, "action_id: ", action_id, " ");
  data_field_ids = table->tableInfoGet()->dataFieldIdListGet(action_id);

  for (const auto& field_id : data_field_ids) {
    std::string field_name;
    tdi_field_data_type_e data_type;
    size_t field_size;
    bool is_active;
    const ::tdi::DataFieldInfo* dataFieldInfo;
    dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_id, action_id);
    RETURN_IF_NULL(dataFieldInfo);

    field_name = dataFieldInfo->nameGet().c_str();
    data_type = dataFieldInfo->dataTypeGet();
    field_size = dataFieldInfo->sizeGet();
    RETURN_IF_TDI_ERROR(table_data->isActive(field_id, &is_active));

    std::string value;
    switch (data_type) {
      case TDI_FIELD_DATA_TYPE_UINT64: {
        uint64 v;
        RETURN_IF_TDI_ERROR(table_data->getValue(field_id, &v));
        value = std::to_string(v);
        break;
      }
      case TDI_FIELD_DATA_TYPE_BYTE_STREAM: {
        std::string v(NumBitsToNumBytes(field_size), '\x00');
        RETURN_IF_TDI_ERROR(table_data->getValue(
            field_id, v.size(),
            reinterpret_cast<uint8*>(gtl::string_as_array(&v))));
        value = absl::StrCat("0x", StringToHex(v));
        break;
      }
      case TDI_FIELD_DATA_TYPE_INT_ARR: {
        // TODO(max): uint32 seems to be the most common type, but we could
        // differentiate based on field_size, if needed.
        std::vector<uint32_t> v;
        RETURN_IF_TDI_ERROR(table_data->getValue(field_id, &v));
        value = PrintVector(v, ",");
        break;
      }
      case TDI_FIELD_DATA_TYPE_BOOL_ARR: {
        std::vector<bool> bools;
        RETURN_IF_TDI_ERROR(table_data->getValue(field_id, &bools));
        std::vector<uint16> bools_as_ints;
        for (bool b : bools) {
          bools_as_ints.push_back(b);
        }
        value = PrintVector(bools_as_ints, ",");
        break;
      }
      default:
        return MAKE_ERROR(ERR_INTERNAL)
               << "Unknown data_type: " << static_cast<int>(data_type) << ".";
    }

    absl::StrAppend(&s, field_name, " { field_id: ", field_id,
                    " data_type: ", static_cast<int>(data_type),
                    " field_size: ", field_size, " value: ", value,
                    " is_active: ", is_active, " } ");
  }
  absl::StrAppend(&s, "}");

  return s;
}

::util::Status GetFieldExact(const ::tdi::TableKey& table_key,
                             std::string field_name, uint32_t* field_value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  RETURN_IF_TDI_ERROR(table_key.tableGet(&table));
  ::tdi::KeyFieldValueExact<uint64_t> key_field_value(*field_value);
  const ::tdi::KeyFieldInfo* keyFieldInfo =
      table->tableInfoGet()->keyFieldGet(field_name);
  RETURN_IF_NULL(keyFieldInfo);

  field_id = keyFieldInfo->idGet();
  data_type = keyFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_UINT64 ||
            data_type == TDI_FIELD_DATA_TYPE_BYTE_STREAM)
      << "Requested uint64 but field " << field_name << " has type "
      << static_cast<int>(data_type);

  RETURN_IF_TDI_ERROR(table_key.getValue(field_id, &key_field_value));

  *field_value = key_field_value.value_;

  return ::util::OkStatus();
}

::util::Status SetFieldExact(::tdi::TableKey* table_key, std::string field_name,
                             uint64 field_value) {
  ::tdi::KeyFieldValueExact<uint64_t> key_field_value(field_value);
  const ::tdi::Table* table;
  tdi_id_t field_id;
  tdi_field_data_type_e data_type;
  RETURN_IF_TDI_ERROR(table_key->tableGet(&table));
  const ::tdi::KeyFieldInfo* keyFieldInfo =
      table->tableInfoGet()->keyFieldGet(field_name);
  RETURN_IF_NULL(keyFieldInfo);

  field_id = keyFieldInfo->idGet();
  data_type = keyFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_UINT64 ||
            data_type == TDI_FIELD_DATA_TYPE_BYTE_STREAM)
      << "Setting uint64 but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_key->setValue(field_id, key_field_value));
  return ::util::OkStatus();
}

::util::Status SetField(::tdi::TableKey* table_key, std::string field_name,
                        ::tdi::KeyFieldValue value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  RETURN_IF_TDI_ERROR(table_key->tableGet(&table));
  const ::tdi::KeyFieldInfo* keyFieldInfo =
      table->tableInfoGet()->keyFieldGet(field_name);
  RETURN_IF_NULL(keyFieldInfo);

  field_id = keyFieldInfo->idGet();
  data_type = keyFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_UINT64 ||
            data_type == TDI_FIELD_DATA_TYPE_BYTE_STREAM)
      << "Setting uint64 but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_key->setValue(field_id, value));

  return ::util::OkStatus();
}

::util::Status GetField(const ::tdi::TableData& table_data,
                        std::string field_name, uint64* field_value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  const ::tdi::DataFieldInfo* dataFieldInfo;
  RETURN_IF_TDI_ERROR(table_data.getParent(&table));

  tdi_id_t action_id = table_data.actionIdGet();
  dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_name, action_id);
  RETURN_IF_NULL(dataFieldInfo);
  field_id = dataFieldInfo->idGet();
  data_type = dataFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_UINT64 ||
            data_type == TDI_FIELD_DATA_TYPE_BYTE_STREAM)
      << "Requested uint64 but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_data.getValue(field_id, field_value));

  return ::util::OkStatus();
}

::util::Status GetField(const ::tdi::TableData& table_data,
                        std::string field_name, std::string* field_value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  const ::tdi::DataFieldInfo* dataFieldInfo;
  RETURN_IF_TDI_ERROR(table_data.getParent(&table));

  tdi_id_t action_id = table_data.actionIdGet();
  dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_name, action_id);
  RETURN_IF_NULL(dataFieldInfo);
  field_id = dataFieldInfo->idGet();
  data_type = dataFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_STRING)
      << "Requested string but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_data.getValue(field_id, field_value));

  return ::util::OkStatus();
}

::util::Status GetFieldBool(const ::tdi::TableData& table_data,
                            std::string field_name, bool* field_value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  const ::tdi::DataFieldInfo* dataFieldInfo;
  RETURN_IF_TDI_ERROR(table_data.getParent(&table));

  tdi_id_t action_id = table_data.actionIdGet();
  dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_name, action_id);
  RETURN_IF_NULL(dataFieldInfo);
  field_id = dataFieldInfo->idGet();
  data_type = dataFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_BOOL)
      << "Requested bool but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_data.getValue(field_id, field_value));

  return ::util::OkStatus();
}

::util::Status SetField(::tdi::TableData* table_data, std::string field_name,
                        const uint64& value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  const ::tdi::DataFieldInfo* dataFieldInfo;
  RETURN_IF_TDI_ERROR(table_data->getParent(&table));

  tdi_id_t action_id = table_data->actionIdGet();
  dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_name, action_id);
  RETURN_IF_NULL(dataFieldInfo);
  field_id = dataFieldInfo->idGet();
  data_type = dataFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_UINT64 ||
            data_type == TDI_FIELD_DATA_TYPE_BYTE_STREAM)
      << "Setting uint64 but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_data->setValue(field_id, value));

  return ::util::OkStatus();
}

::util::Status SetField(::tdi::TableData* table_data, std::string field_name,
                        const std::string& field_value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  const ::tdi::DataFieldInfo* dataFieldInfo;
  RETURN_IF_TDI_ERROR(table_data->getParent(&table));

  tdi_id_t action_id = table_data->actionIdGet();
  dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_name, action_id);
  RETURN_IF_NULL(dataFieldInfo);
  field_id = dataFieldInfo->idGet();
  data_type = dataFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_STRING)
      << "Setting string but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_data->setValue(field_id, field_value));

  return ::util::OkStatus();
}

::util::Status SetFieldBool(::tdi::TableData* table_data,
                            std::string field_name, const bool& field_value) {
  tdi_id_t field_id;
  const ::tdi::Table* table;
  tdi_field_data_type_e data_type;
  const ::tdi::DataFieldInfo* dataFieldInfo;
  RETURN_IF_TDI_ERROR(table_data->getParent(&table));

  tdi_id_t action_id = table_data->actionIdGet();
  dataFieldInfo = table->tableInfoGet()->dataFieldGet(field_name, action_id);
  RETURN_IF_NULL(dataFieldInfo);
  field_id = dataFieldInfo->idGet();
  data_type = dataFieldInfo->dataTypeGet();

  RET_CHECK(data_type == TDI_FIELD_DATA_TYPE_BOOL)
      << "Setting bool but field " << field_name << " has type "
      << static_cast<int>(data_type);
  RETURN_IF_TDI_ERROR(table_data->setValue(field_id, field_value));

  return ::util::OkStatus();
}

namespace {

/**
 * GetNumberOfEntries - Returns the number of entries in the table.
 */
::util::Status GetNumberOfEntries(std::shared_ptr<::tdi::Session>& tdi_session,
                                  ::tdi::Target& tdi_dev_target,
                                  const ::tdi::Flags& flags,
                                  const ::tdi::Table*& table, uint32& entries) {
  entries = 0;
  // Some types of tables are preallocated and always "full". The SDE
  // does not support querying the utilization of these tables.
  if (IsPreallocatedTable(*table)) {
    size_t table_size;
    RETURN_IF_TDI_ERROR(
        table->sizeGet(*tdi_session, tdi_dev_target, flags, &table_size));
    entries = table_size;
  } else {
    RETURN_IF_TDI_ERROR(
        table->usageGet(*tdi_session, tdi_dev_target, flags, &entries));
  }
  return ::util::OkStatus();
}

/**
 * GetFirstEntry() - Fetches the first entry in the table.
 */
::util::Status GetFirstEntry(
    std::shared_ptr<::tdi::Session>& tdi_session, const ::tdi::Flags& flags,
    const ::tdi::Table*& table,
    std::vector<std::unique_ptr<::tdi::TableKey>>*& table_keys,
    std::vector<std::unique_ptr<::tdi::TableData>>*& table_values) {
  // QUESTION: Why does GetFirstEntry() define its own ::tdi::Target
  // instead of using the one passed to GetAllEntries()?
  const ::tdi::Device* device = nullptr;
  ::tdi::DevMgr::getInstance().deviceGet(0, &device);
  std::unique_ptr<::tdi::Target> dev_tgt;
  device->createTarget(&dev_tgt);

  std::unique_ptr<::tdi::TableKey> table_key;
  std::unique_ptr<::tdi::TableData> table_data;

  RETURN_IF_TDI_ERROR(table->keyAllocate(&table_key));
  RETURN_IF_TDI_ERROR(table->dataAllocate(&table_data));

  auto tdi_status = table->entryGetFirst(*tdi_session, *dev_tgt, flags,
                                         table_key.get(), table_data.get());
  if (ObjectNotFound(tdi_status)) {
    // Table is empty.
    return ::util::OkStatus();
  }
  RETURN_IF_TDI_ERROR(tdi_status);

  table_keys->push_back(std::move(table_key));
  table_values->push_back(std::move(table_data));

  return ::util::OkStatus();
}

/**
 * GetNextEntries() - Fetches the remaining entries from the table.
 * The first entry has already been fetched.
 */
::util::Status GetNextEntries(
    std::shared_ptr<::tdi::Session>& tdi_session, ::tdi::Target& tdi_dev_target,
    const ::tdi::Flags& flags, const ::tdi::Table*& table,
    const uint32 num_entries,
    std::vector<std::unique_ptr<::tdi::TableKey>>*& table_keys,
    std::vector<std::unique_ptr<::tdi::TableData>>*& table_values) {
  // Input and output vectors for GetNextN.
  std::vector<std::unique_ptr<::tdi::TableKey>> keys(num_entries);
  std::vector<std::unique_ptr<::tdi::TableData>> data(num_entries);
  ::tdi::Table::keyDataPairs pairs;

  for (uint32 i = 0; i < num_entries; ++i) {
    RETURN_IF_TDI_ERROR(table->keyAllocate(&keys[i]));
    RETURN_IF_TDI_ERROR(table->dataAllocate(&data[i]));
    pairs.push_back(std::make_pair(keys[i].get(), data[i].get()));
  }

  uint32 actual = 0;
  RETURN_IF_TDI_ERROR(table->entryGetNextN(*tdi_session, tdi_dev_target, flags,
                                           *table_keys->back(), pairs.size(),
                                           &pairs, &actual));

  if (actual) {
    // Append key pointers to the table_keys vector.
    auto keys_end = keys.begin();
    std::advance(keys_end, actual);

    table_keys->insert(table_keys->end(), std::make_move_iterator(keys.begin()),
                       std::make_move_iterator(keys_end));

    // Append data pointers to the table_values vector.
    auto data_end = data.begin();
    std::advance(data_end, actual);

    table_values->insert(table_values->end(),
                         std::make_move_iterator(data.begin()),
                         std::make_move_iterator(data_end));
  }

  return ::util::OkStatus();
}

/**
 * GetExtraEntries() - Fetches table entries until there are no more.
 *
 * Hardware-learned (add-on-miss) entries are not included in the reported
 * table size, so we continue fetching after getting the "remaining" entries.
 *
 * We fetch one entry at a time because pipe_mgr logs error messages if we
 * request multiple entries.
 */
::util::Status GetExtraEntries(
    std::shared_ptr<::tdi::Session>& tdi_session, ::tdi::Target& tdi_dev_target,
    const ::tdi::Flags& flags, const ::tdi::Table*& table,
    std::vector<std::unique_ptr<::tdi::TableKey>>*& table_keys,
    std::vector<std::unique_ptr<::tdi::TableData>>*& table_values) {
  constexpr uint32 NUM_ENTRIES = 1;

  for (;;) {
    std::unique_ptr<::tdi::TableKey> table_key;
    std::unique_ptr<::tdi::TableData> table_data;

    RETURN_IF_TDI_ERROR(table->keyAllocate(&table_key));
    RETURN_IF_TDI_ERROR(table->dataAllocate(&table_data));

    ::tdi::Table::keyDataPairs pairs;
    pairs.push_back(std::make_pair(table_key.get(), table_data.get()));

    uint32 actual = 0;
    RETURN_IF_TDI_ERROR(table->entryGetNextN(*tdi_session, tdi_dev_target,
                                             flags, *table_keys->back(),
                                             NUM_ENTRIES, &pairs, &actual));

    if (actual == 0) break;

    table_keys->push_back(std::move(table_key));
    table_values->push_back(std::move(table_data));
  }
  return ::util::OkStatus();
}

/**
 * GetAllEntriesByCount() - Fetches all the entries in a table by querying
 * the number of entries in the table and then reading that many entries.
 */
::util::Status GetAllEntriesByCount(
    std::shared_ptr<::tdi::Session>& tdi_session, ::tdi::Target& tdi_dev_target,
    const ::tdi::Flags& flags, const ::tdi::Table*& table,
    std::vector<std::unique_ptr<::tdi::TableKey>>*& table_keys,
    std::vector<std::unique_ptr<::tdi::TableData>>*& table_values) {
  uint32 entries = 0;
  RETURN_IF_ERROR(
      GetNumberOfEntries(tdi_session, tdi_dev_target, flags, table, entries));

  RETURN_IF_ERROR(
      GetFirstEntry(tdi_session, flags, table, table_keys, table_values));

  if (table_keys->size() == 0) {
    // Table is empty.
    return ::util::OkStatus();
  }

  if (entries > 1) {
    RETURN_IF_ERROR(GetNextEntries(tdi_session, tdi_dev_target, flags, table,
                                   entries - 1, table_keys, table_values));
  }

  // Add-on-miss entries are not included in the table size, so we
  // need to continue reading until there are no more entries.
  RETURN_IF_ERROR(GetExtraEntries(tdi_session, tdi_dev_target, flags, table,
                                  table_keys, table_values));

  CHECK(table_keys->size() == table_values->size());
  return ::util::OkStatus();
}

}  // namespace

/**
 * GetAllEntries() - Fetches all the entries in a table.
 */
::util::Status GetAllEntries(
    std::shared_ptr<::tdi::Session> tdi_session, ::tdi::Target& tdi_dev_target,
    const ::tdi::Table* table,
    std::vector<std::unique_ptr<::tdi::TableKey>>* table_keys,
    std::vector<std::unique_ptr<::tdi::TableData>>* table_values) {
  // Sanity check.
  RET_CHECK(table_keys) << "table_keys is null";
  RET_CHECK(table_values) << "table_values is null";

  table_keys->resize(0);
  table_values->resize(0);

  const ::tdi::Flags flags(0);

  return GetAllEntriesByCount(tdi_session, tdi_dev_target, flags, table,
                              table_keys, table_values);
}

// TDI does not provide a target-neutral way for us to determine whether a
// table is preallocated, so we provide our own means of detection.
bool IsPreallocatedTable(const ::tdi::Table& table) {
  auto table_type = GetSdeTableType(table);
  return (table_type == TDI_SDE_TABLE_TYPE_COUNTER ||
          table_type == TDI_SDE_TABLE_TYPE_METER);
}

}  // namespace helpers
}  // namespace tdi
}  // namespace hal
}  // namespace stratum
