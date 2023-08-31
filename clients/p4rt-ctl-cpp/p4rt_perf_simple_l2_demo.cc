// Copyright 2023-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "p4rt_perf_simple_l2_demo.h"
#include "p4rt_perf_util.h"
extern TestParams test_params;
namespace p4rt_perf {

void PrepareSimpleL2DemoTableEntry(p4::v1::TableEntry* table_entry,
                                   const SimpleL2DemoMacInfo& mac_info,
                                   const ::p4::config::v1::P4Info& p4info,
                                   bool insert_entry) {
  table_entry->set_table_id(GetTableId(p4info, "my_control.e_fwd"));
  auto match = table_entry->add_match();
  match->set_field_id(GetMatchFieldId(p4info, "my_control.e_fwd",
                                      "hdrs.mac[vmeta.common.depth].da"));

  std::string mac_addr = CanonicalizeMac(mac_info.dst_mac);
  match->mutable_exact()->set_value(mac_addr);

  auto match1 = table_entry->add_match();
  match1->set_field_id(GetMatchFieldId(p4info, "my_control.e_fwd",
                                       "hdrs.mac[vmeta.common.depth].sa"));

  mac_addr = CanonicalizeMac(mac_info.src_mac);
  match1->mutable_exact()->set_value(mac_addr);

  if (insert_entry) {
    auto table_action = table_entry->mutable_action();
    auto action = table_action->mutable_action();
    action->set_action_id(GetActionId(p4info, "my_control.send"));
    {
      auto param = action->add_params();
      param->set_param_id(GetParamId(p4info, "my_control.send", "port"));
      param->set_value(EncodeByteValue(1, 1));
    }
  }
  return;
}

void SimpleL2DemoTest(p4rt_perf::P4rtSession* session,
                      const ::p4::config::v1::P4Info& p4info, ThreadInfo &t_data) {
  ::p4::v1::TableEntry* table_entry;
  SimpleL2DemoMacInfo mac_info;
  p4::v1::WriteRequest write_request;

  int batch_size = t_data.num_entries;
  uint64_t count = t_data.start + 1;
  for (uint64_t j = 0; j < t_data.num_entries; j++) {
    if (t_data.oper == ADD) {
      table_entry = p4rt_perf::SetupTableEntryToInsert(session, &write_request);
    } else if (t_data.oper == DEL) {
      table_entry = p4rt_perf::SetupTableEntryToDelete(session, &write_request);
    } else {
      std::cout << "Invalid operation" << std::endl;
    }

    auto src_int = count;
    mac_info.src_mac[0] = (src_int >> 40) & 0xFF;
    mac_info.src_mac[1] = (src_int >> 32) & 0xFF;
    mac_info.src_mac[2] = (src_int >> 24) & 0xFF;
    mac_info.src_mac[3] = (src_int >> 16) & 0xFF;
    mac_info.src_mac[4] = (src_int >> 8) & 0xFF;
    mac_info.src_mac[5] = src_int;  // & 0xFF;

    src_int = count + 1;
    mac_info.dst_mac[0] = (src_int >> 40) & 0xFF;
    mac_info.dst_mac[1] = (src_int >> 32) & 0xFF;
    mac_info.dst_mac[2] = (src_int >> 24) & 0xFF;
    mac_info.dst_mac[3] = (src_int >> 16) & 0xFF;
    mac_info.dst_mac[4] = (src_int >> 8) & 0xFF;
    mac_info.dst_mac[5] = src_int & 0xFF;

    PrepareSimpleL2DemoTableEntry(table_entry, mac_info, p4info,
                                  t_data.oper == ADD);
    count++;
  }
  auto sts = p4rt_perf::SendWriteRequest(session, write_request);

  std::cout << "count: " << count - 1 << std::endl;
}

}  // namespace p4rt_perf
