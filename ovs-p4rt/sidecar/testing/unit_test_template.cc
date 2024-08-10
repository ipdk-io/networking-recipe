// Copyright 2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Unit test for PrepareTemplateTableEntry()

#include <stdint.h>

#include <iostream>
#include <string>

#include "base_table_test.h"
#include "gtest/gtest.h"
#ifdef DIAG_DETAIL
#include "logging/ovsp4rt_diag_detail.h"
#endif
#include "ovsp4rt/ovs-p4rt.h"
#include "ovsp4rt_private.h"

namespace ovsp4rt {

class TemplateTest : public BaseTableTest {
 protected:
  TemplateTest() {}

  void SetUp() { SelectTable("template_table"); }

  void InitAction() { SelectAction("template_action"); }

  void InitInputInfo() {}

  void CheckAction() const {}

  void CheckNoAction() const { ASSERT_FALSE(table_entry.has_action()); }

#ifdef DIAG_DETAIL
  void CheckDetail() const { EXPECT_EQ(detail.table_id, LOG_TEMPLATE_TABLE); }
#endif

  void CheckMatches() const {}

  void CheckTableEntry() const {
    ASSERT_TRUE(HasTable());
    EXPECT_EQ(table_entry.table_id(), TableId());
  }

  struct template_info input_info = {0};
#ifdef DIAG_DETAIL
  DiagDetail detail;
#endif
};

//----------------------------------------------------------------------
// Test cases
//----------------------------------------------------------------------

TEST_F(TemplateTest, remove_entry) {
  // Arrange
  InitInputInfo();

  // Act
  PrepareTemplateTableEntry(&table_entry, input_info, p4info, REMOVE_ENTRY
#ifdef DIAG_DETAIL
                            ,
                            detail
#endif
  );

  // Assert
#ifdef DIAG_DETAIL
  CheckDetail();
#endif
  CheckTableEntry();
  CheckMatches();
  CheckNoAction();
}

TEST_F(TemplateTest, insert_entry) {
  // Arrange
  InitInputInfo();
  InitAction();

  // Act
  PrepareTemplateTableEntry(&table_entry, input_info, p4info, INSERT_ENTRY
#ifdef DIAG_DETAIL
                            ,
                            detail
#endif
  );

  // Assert
  CheckTableEntry();
  CheckAction();
}

}  // namespace ovsp4rt
