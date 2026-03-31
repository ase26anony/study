This script systematically tests the various initialization and cleanup paths:

1. **Help/Version Flags**: Covers `print_help_list`, `print_version`, `print_subprocess_help`
2. **Verbose Flags**: Covers `verbose_only_flag`
3. **Save-temps/Dump Options**: Covers `save_temps_flag`, `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`, and their cleanup via `free()`
4. **At-file Handling**: Covers `at_file_supplied`
5. **Sysroot/Target**: Covers `target_system_root`, `target_system_root_changed`, `target_sysroot_suffix`, `target_sysroot_hdrs_suffix`, `spec_machine`
6. **Linker Options**: Covers `use_ld`
7. **Time Reporting**: Covers `report_times_to_file`
8. **Error Scenarios**: Covers `greatest_status`
9. **Combined Scenarios**: Triggers multiple initialization/cleanup cycles
10. **Compilation Stages**: Exercises different driver modes

The script uses `2>/dev/null || true` to handle cases where GCC might fail (e.g., missing cross-compiler, invalid sysroot) without stopping the script, since we're interested in the initialization/cleanup logic, not successful compilation.

To run this test:
