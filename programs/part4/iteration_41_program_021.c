**Key aspects of this test script:**

1. **Help/Version Flags**: Directly test `print_help_list`, `print_version`, and `print_subprocess_help`
2. **Verbose Flags**: Test `verbose_only_flag` with `-v` and `-###`
3. **Save-temps/Dump Options**: Exercise `save_temps_flag`, `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase` variables
4. **At-file Handling**: Tests `at_file_supplied` with `@args.txt`
5. **Sysroot/Target**: Tests `target_system_root`, `target_system_root_changed`, `spec_machine`
6. **Linker Selection**: Tests `use_ld` with different `-fuse-ld=` options
7. **Time Reporting**: Tests `report_times_to_file` with `-ftime-report`
8. **Different Modes**: Exercises driver through `-E`, `-S`, `-c`, and full compilation
9. **Error Paths**: Triggers error conditions to affect `greatest_status`
10. **Multiple Invocations**: Ensures cleanup/reset between calls
11. **Combinations**: Tests multiple flags together to ensure proper initialization/cleanup

The script creates a simple C file, runs GCC with various flag combinations, and cleans up. Each invocation exercises different parts of the driver initialization/cleanup logic. The error cases (with `2>/dev/null || true`) ensure the script continues even when GCC returns non-zero exit status.

To run this test:
