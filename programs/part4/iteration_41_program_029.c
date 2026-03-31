**Key aspects of this test script:**

1. **Multiple initialization cycles**: Each `gcc` invocation runs as a separate process, ensuring the initialization/cleanup block is executed.

2. **Comprehensive flag coverage**:
   - `--version`, `--help`, `--target-help` → `print_version`, `print_help_list`
   - `-v`, `-###` → `verbose_only_flag`
   - `-save-temps`, `-dumpdir`, `-dumpbase` → `save_temps_flag`, `dumpdir`, `dumpbase`, etc.
   - `@args.txt` → `at_file_supplied`
   - `--sysroot`, `-isysroot`, `-target` → `target_system_root`, `spec_machine`
   - `-fuse-ld=*` → `use_ld`
   - `--help=*` → `print_subprocess_help`
   - `-ftime-report`, `-fmem-report` → `report_times_to_file`
   - Error cases → `greatest_status`

3. **Cleanup triggering**: By using flags that allocate resources (like `-dumpdir`) and then running subsequent commands, we trigger the `free()` calls and NULL assignments.

4. **Different execution modes**: The script tests `-E` (preprocess), `-S` (assemble), `-c` (compile), and linking modes.

5. **Error handling**: The `|| true` and `2>/dev/null` ensure the script continues even when commands fail (which is expected for some invalid inputs).

To run this test:
