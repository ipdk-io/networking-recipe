// Copyright 2020-present Open Networking Foundation
// Copyright 2022-2023,2025 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef STRATUM_HAL_LIB_TDI_TDI_CONSTANTS_H_
#define STRATUM_HAL_LIB_TDI_TDI_CONSTANTS_H_

#include "absl/time/time.h"
#include "stratum/glue/integral_types.h"

namespace stratum {
namespace hal {
namespace tdi {

// TNA Extern types
constexpr uint32 kTnaExternActionProfileId = 131;
constexpr uint32 kTnaExternActionSelectorId = 132;
constexpr uint32 kTnaExternDirectCounter = 133;

// Built-in table and field names.
constexpr char kMcNodeDevPort[] = "$DEV_PORT";
constexpr char kMcNodeId[] = "$MULTICAST_NODE_ID";
constexpr char kMcNodeL1Xid[] = "$MULTICAST_NODE_L1_XID";
constexpr char kMcNodeL1XidValid[] = "$MULTICAST_NODE_L1_XID_VALID";
constexpr char kMcNodeLagId[] = "$MULTICAST_LAG_ID";
constexpr char kMcReplicationId[] = "$MULTICAST_RID";
constexpr char kMgid[] = "$MGID";
constexpr char kPreMgidTable[] = "$pre.mgid";
constexpr char kPreNodeTable[] = "$pre.node";
constexpr char kRegisterIndex[] = "$REGISTER_INDEX";
constexpr char kMeterIndex[] = "$METER_INDEX";
constexpr char kMeterCirKbps[] = "$METER_SPEC_CIR_KBPS";
constexpr char kMeterCommitedBurstKbits[] = "$METER_SPEC_CBS_KBITS";
constexpr char kMeterPirKbps[] = "$METER_SPEC_PIR_KBPS";
constexpr char kMeterPeakBurstKbits[] = "$METER_SPEC_PBS_KBITS";
constexpr char kMeterCirPps[] = "$METER_SPEC_CIR_PPS";
constexpr char kMeterCommitedBurstPackets[] = "$METER_SPEC_CBS_PKTS";
constexpr char kMeterPirPps[] = "$METER_SPEC_PIR_PPS";
constexpr char kMeterPeakBurstPackets[] = "$METER_SPEC_PBS_PKTS";
constexpr char kCounterIndex[] = "$COUNTER_INDEX";
constexpr char kCounterBytes[] = "$COUNTER_SPEC_BYTES";
constexpr char kCounterPackets[] = "$COUNTER_SPEC_PKTS";
constexpr char kMirrorConfigTable[] = "mirror.cfg";
constexpr char kMatchPriority[] = "$MATCH_PRIORITY";
constexpr char kActionMemberId[] = "$ACTION_MEMBER_ID";
constexpr char kSelectorGroupId[] = "$SELECTOR_GROUP_ID";
constexpr char kActionMemberStatus[] = "$ACTION_MEMBER_STATUS";

// IPsec related consts used in fixed functions
constexpr char kIpsecSadbOffloadId[] = "offload-id";
constexpr char kIpsecSadbDir[] = "direction";
constexpr char kIpsecSadbReqId[] = "req-id";
constexpr char kIpsecSadbSpi[] = "spi";
constexpr char kIpsecSadbSeqNum[] = "ext-seq-num";
constexpr char kIpsecSadbReplayWindow[] = "anti-replay-window-size";
constexpr char kIpsecSadbProtoParams[] = "protocol-parameters";
constexpr char kIpsecSadbMode[] = "mode";
constexpr char kIpsecSadbEspAlgo[] = "encryption-algorithm";
constexpr char kIpsecSadbEspKey[] = "key";
constexpr char kIpsecSadbEspKeylen[] = "key-len";
constexpr char kIpsecSaLtHard[] = "sa-lifetime-hard";
constexpr char kIpsecSaLtSoft[] = "sa-lifetime-soft";
constexpr char kIpsecFetchSpi[] = "rx-spi";

// vport related consts used in fixed functions
constexpr char kGlobalResourceId[] = "global-resource-id";
constexpr char kVsi[] = "vsi";
constexpr char kOperStatus[] = "oper-status";
constexpr char kMacAddress[] = "mac-address";

// TNA specific limits
constexpr uint16 kMaxCloneSessionId = 1015;
constexpr uint16 kMaxMulticastGroupId = 65535;
constexpr uint32 kMaxMulticastNodeId = 0x1000000;
constexpr uint64 kMaxPriority = (1u << 24) - 1;

constexpr absl::Duration kDefaultSyncTimeout = absl::Seconds(1);

// ES2k specific const used in PacketModMeter
// Values for "kEs2kExternPacketModMeter" and "kEs2kExternDirectPacketModMeter"
// has been referred from es2k-specific p4info.proto file,
// P4IDs of the enum prefix for Externs.
constexpr uint32 kEs2kExternPacketModMeter = 133;
constexpr uint32 kEs2kExternDirectPacketModMeter = 134;

// Field name for below mentioned parameters has been referred from compiler
// generated artifact - tdi.json file
constexpr char kEs2kMeterProfileIdKbps[] = "$POLICER_METER_SPEC_PROF_ID";
constexpr char kEs2kMeterCirKbps[] = "$POLICER_METER_SPEC_CIR";
constexpr char kEs2kMeterCommitedBurstKbits[] = "$POLICER_METER_SPEC_CBS";
constexpr char kEs2kMeterPirKbps[] = "$POLICER_METER_SPEC_EIR";
constexpr char kEs2kMeterPeakBurstKbits[] = "$POLICER_METER_SPEC_EBS";
constexpr char kEs2kMeterCirKbpsUnit[] = "$POLICER_METER_SPEC_CIR_UNIT";
constexpr char kEs2kMeterCommitedBurstKbitsUnit[] =
    "$POLICER_METER_SPEC_CBS_UNIT";
constexpr char kEs2kMeterPirKbpsUnit[] = "$POLICER_METER_SPEC_EIR_UNIT";
constexpr char kEs2kMeterPeakBurstKbitsUnit[] = "$POLICER_METER_SPEC_EBS_UNIT";
constexpr char kEs2kMeterGreenCounterBytes[] = "$POLICER_METER_SPEC_GBYTE_CNTR";
constexpr char kEs2kMeterGreenCounterPackets[] =
    "$POLICER_METER_SPEC_GPKT_CNTR";
constexpr char kEs2kMeterYellowCounterBytes[] =
    "$POLICER_METER_SPEC_YBYTE_CNTR";
constexpr char kEs2kMeterYellowCounterPackets[] =
    "$POLICER_METER_SPEC_YPKT_CNTR";
constexpr char kEs2kMeterRedCounterBytes[] = "$POLICER_METER_SPEC_RBYTE_CNTR";
constexpr char kEs2kMeterRedCounterPackets[] = "$POLICER_METER_SPEC_RPKT_CNTR";
constexpr char kEs2kMeterProfileIdKPps[] = "$POLICER_METER_SPEC_PROF_ID";
constexpr char kEs2kMeterIndex[] = "$METER_INDEX";
constexpr char kEs2kMeterCirPps[] = "$POLICER_METER_SPEC_CIR";
constexpr char kEs2kMeterCommitedBurstPackets[] = "$POLICER_METER_SPEC_CBS";
constexpr char kEs2kMeterPirPps[] = "$POLICER_METER_SPEC_EIR";
constexpr char kEs2kMeterPeakBurstPackets[] = "$POLICER_METER_SPEC_EBS";
constexpr char kEs2kMeterCirKPpsUnit[] = "$POLICER_METER_SPEC_CIR_UNIT";
constexpr char kEs2kMeterCommitedBurstPacketsUnit[] =
    "$POLICER_METER_SPEC_CBS_UNIT";
constexpr char kEs2kMeterPirPpsUnit[] = "$POLICER_METER_SPEC_EIR_UNIT";
constexpr char kEs2kMeterPeakBurstPacketsUnit[] =
    "$POLICER_METER_SPEC_EBS_UNIT";

}  // namespace tdi
}  // namespace hal
}  // namespace stratum

#endif  // STRATUM_HAL_LIB_TDI_TDI_CONSTANTS_H_
