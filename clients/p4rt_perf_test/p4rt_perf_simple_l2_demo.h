// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef P4RT_PERF_SIMPLE_L2_DEMO_H
#define P4RT_PERF_SIMPLE_L2_DEMO_H

#include "p4/v1/p4runtime.pb.h"
#include "p4rt_perf_session.h"
#include "p4rt_perf_test.h"

void PrepareSimpleL2DemoTableEntry(p4::v1::TableEntry* table_entry,
                                   const SimpleL2DemoMacInfo& mac_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry);

int SimpleL2DemoTest(P4rtSession* session,
                     const ::p4::config::v1::P4Info& p4info,
                     ThreadInfo& t_data);

#endif
