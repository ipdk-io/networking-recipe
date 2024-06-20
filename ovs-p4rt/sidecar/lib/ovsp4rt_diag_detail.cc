// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ovsp4rt_diag_detail.h"

namespace ovs_p4rt {

const char* DiagDetail::getLogTableName() {
  switch (table_id) {
    case LOG_UNKNOWN_TABLE:
      return "UNKNOWN";
    case LOG_L2_FWD_RX_TABLE:
      return "log_l2_fwd_rx_table";
    case LOG_L2_FWD_RX_WITH_TUNNEL_TABLE:
      return "l2_fwd_rx_with_tunnel_table";
    case LOG_L2_FWD_TX_TABLE:
      return "l2_fwd_tx_table";
    case LOG_L2_TO_TUNNEL_V6_TABLE:
      return "l2_to_tunnel_v6_table";
    case LOG_L2_TO_TUNNEL_V4_TABLE:
      return "l2_to_tunnel_v4_table";
    case LOG_L2_FWD_SMAC_TABLE:
      return "l2_fwd_smac_table";
    case LOG_DST_IP_MAC_MAP_TABLE:
      return "dst_ip_mac_map_table";
    case LOG_SRC_IP_MAC_MAP_TABLE:
      return "src_ip_mac_map_table";
    default:
      return "INVALID";
  }
}

}  // namespace ovs_p4rt
