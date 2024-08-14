# Using the Unit Test Template

1. Run `unifdef` to create a new source file from the template and
   select the conditionalized code to be kept or discarded.

   For example:

   ```bash
   unifdef unit_test_template.cc -ovlan_pop_table_test.cc \
       -UDIAG_DETAIL
   ```

   unifdef options:

   - `-Dsym` to remove the #ifdef/#endif and keep the contents
   - `-Usym` to remove the #ifdef/#endif and discard the contents
   - `-opath` to specify the output file name

   Preprocessor symbols:

   - `DIAG_DETAIL` if the function accepts a _detail_ parameter.

   If you do not specify `-D` or `-U` for a symbol, the conditional
   block will be unchanged in the output file.

2. Rename placeholders in the new source file.

   - Change all occurrences of _PrepareTemplateTableEntry_ to the name
     of the function.
   - Change all occurrences of _TemplateTest_ to the name of the test.
   - Change _"template_table"_ to the name or alias of the table.
   - Change _"template_action"_ to the name or alias of the action,
     or delete the _SetUp()_ method if you will be selecting the action
     elsewhere in the test.
   - Optionally change _InitInputInfo_ to something appropriate to the test.
   - Change _LOG_TEMPLATE_TABLE_ to the enum for the table (if
     DIAG_DETAIL is enabled).
   - Change _template_info_ to the name of the info struct.
   - Optionally change _input_info_ to something appropriate to the test.

3. Run `clang-format` to clean up after the edits.

   ```bash
   clang-format -i vlan_pop_table_test.cc
   ```
