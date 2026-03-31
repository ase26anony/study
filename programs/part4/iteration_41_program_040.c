This script systematically tests various aspects of the GCC driver initialization:

1. **Help/Version Flags**: Directly test `print_help_list`, `print_version`, and `print_subprocess_help`
2. **Verbose Flags**: Test `verbose_only_flag` with `-v` and `-###`
3. **Save-temps/Dump Options**: Test `save_temps_flag`, `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase` and their cleanup
4. **At-files**: Test `at_file_supplied` with `@args.txt`
5. **Sysroot/Target**: Test `target_system_root`, `target_system_root_changed`, `spec_machine`
6. **Linker Selection**: Test `use_ld` with `-fuse-ld=*`
7. **Time Reporting**: Test `report_times_to_file` with `-ftime-report`
8. **Error Cases**: Test `greatest_status` with invalid inputs
9. **Multiple Invocations**: Trigger cleanup/reset between different driver calls
10. **Different Stages**: Test `-E`, `-S`, `-c` flags
11. **Driver Modes**: Test both `gcc` and `g++` (affects `is_cpp_driver`)
12. **Environment Variables**: Test initialization with different environments
13. **Edge Cases**: Test dumpdir with trailing slashes/dashes

The script creates a simple C file, uses it for compilation tests, and cleans up afterwards. Each section targets specific variables from the uncovered block, ensuring they get initialized, used, and potentially reset.

To run this test:
