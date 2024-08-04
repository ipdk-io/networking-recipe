# Using the unit test template

1. Copy the template file, renaming it to the target file name.

2. Run `unifdef` to select the conditionalized code to be kept
   or discarded.

   unifdef options:

   - `-Dsym` to remove the #ifdef/#endif and keep the contents
   - `-Usym` to remove the #ifdef/#endif and discard the contents
   - `-opath` to specify the output file name

   Preprocessor symbols:

   - `DIAG_DETAIL`
   - `DUMP_JSON`
   - `SELECT_ACTION`

   If you do not specify `-D` or `-U` for a symbol, the conditionals
   and contents for that symbol will be kept.

3. Rename placeholders.

   - Rename "PrepareSampleTableEntry" to the name of the function.
   - Rename "InitInputInfo" to reflect the name of the info struct.
   - Rename "input_info" to the name of the info struct.
   - Rename "sample_table" to the name or alias of the table.
   - Rename "sample_action" to the name or alias of the action.

4. Run clang-format to clean up after the edits.
