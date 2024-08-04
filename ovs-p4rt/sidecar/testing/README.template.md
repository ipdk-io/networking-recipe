# Using the Unit Test Template

1. Run `unifdef` to create a new source file from the template and
   select the conditionalized code to be kept or discarded.

   For example:

   ```bash
   unifdef unit_test_template.cc -ovlan_pop_table_test.cc \
       -UDIAG_DETAIL -DDUMP_JSON -DSELECT_ACTION
   ```

   unifdef options:

   - `-Dsym` to remove the #ifdef/#endif and keep the contents
   - `-Usym` to remove the #ifdef/#endif and discard the contents
   - `-opath` to specify the output file name

   Preprocessor symbols:

   - `DIAG_DETAIL`
   - `DUMP_JSON`
   - `SELECT_ACTION`

   If you do not specify `-D` or `-U` for a symbol, the conditional
   block will be unchanged in the output file.

3. Rename placeholders in the new source file.

   - Change "PrepareSampleTableEntry" to the name of the function.
   - Change "InitInputInfo" to reflect the name of the info struct.
   - Change "input_info" to the name of the info struct.
   - Change "sample_table" to the name or alias of the table.
   - Change "sample_action" to the name or alias of the action.

4. Run `clang-format` to clean up after the edits.

   ```bash
   clang-format -i vlan_pop_table_test.cc
   ```
