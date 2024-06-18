// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef OVSP4RT_DIAG_DETAIL_H_
#define OVSP4RT_DIAG_DETAIL_H_

namespace ovs_p4rt {

enum LogTableId {
  LOG_UNKNOWN_TABLE = 0,
  LOG_L2_FWD_RX_TABLE,
  LOG_L2_FWD_TX_TABLE,
  LOG_L2_TO_TUNNEL_V4_TABLE,
  LOG_L2_TO_TUNNEL_V6_TABLE,
  LOG_L2_FWD_SMAC_TABLE,
  LOG_L2_FWD_RX_WITH_TUNNEL_TABLE,
  LOG_DST_IP_MAC_MAP_TABLE,
  LOG_SRC_IP_MAC_MAP_TABLE,
};

struct DiagDetail {
  DiagDetail(LogTableId tbl_id = LOG_UNKNOWN_TABLE) : table_id(tbl_id) {}
  const char* getLogTableName();
  LogTableId table_id;
};

}  // namespace ovs_p4rt

#endif  // OVSP4RT_DIAG_DETAIL_H_
