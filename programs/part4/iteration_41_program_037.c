This script systematically tests each aspect of the uncovered code:

1. **Help/Version flags** - Exercises `print_help_list`, `print_version`, `print_subprocess_help`
2. **Verbose flags** - Exercises `verbose_only_flag`
3. **Save-temps/dump options** - Exercises `save_temps_flag`, `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`, and their cleanup
4. **At-file handling** - Exercises `at_file_supplied`
5. **Sysroot/target specs** - Exercises `target_system_root`, `target_system_root_changed`, `spec_machine`
6. **Linker selection** - Exercises `use_ld`
7. **Time reporting** - Exercises `report_times_to_file`
8. **Error scenarios** - Exercises `greatest_status`
9. **Combination tests** - Tests multiple variables simultaneously
10. **Sequential invocations** - Forces multiple initialization/cleanup cycles
11. **Different driver modes** - Tests various compilation stages
12. **Environment variables** - Tests additional initialization paths

The script uses `2>/dev/null || true` to handle cases where commands might fail (e.g., invalid sysroot paths, unavailable linkers), which is actually beneficial for testing error cleanup paths. Each command triggers the GCC driver's initialization logic, and the sequence of different invocations ensures that the cleanup/reset code is exercised multiple times.

To run this test:
