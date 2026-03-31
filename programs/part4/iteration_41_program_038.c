This script comprehensively tests the uncovered lines by:

1. **Basic initialization**: Help/version flags set `print_help_list`, `print_version`
2. **Verbose flags**: `-v` and `-###` affect `verbose_only_flag`
3. **Dump file handling**: `-save-temps`, `-dumpdir`, `-dumpbase` exercise the allocation and cleanup of `dumpdir`, `dumpbase`, etc.
4. **At-file handling**: `@args.txt` tests `at_file_supplied`
5. **Sysroot/target**: `--sysroot`, `-isysroot`, `-target` affect `target_system_root`, `target_system_root_changed`, `spec_machine`
6. **Linker control**: `-fuse-ld=` sets `use_ld`, `--help=` affects `print_subprocess_help`
7. **Time reporting**: `-ftime-report`, `-fmem-report` affect `report_times_to_file`
8. **Error conditions**: Invalid inputs test `greatest_status` setting
9. **Multiple invocations**: Ensures cleanup and reinitialization between runs
10. **Driver modes**: `-E`, `-S`, `-c`, linking test different execution paths
11. **Complex combinations**: Stress tests the initialization logic

The script uses `2>/dev/null || true` to handle cases where GCC might exit with error (e.g., invalid sysroot paths), which is actually desirable to trigger cleanup paths. Each command runs the GCC driver as a separate process, ensuring full initialization and cleanup cycles.

To run the test:
