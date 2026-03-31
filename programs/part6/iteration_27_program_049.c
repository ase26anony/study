This script provides comprehensive coverage of the target lines in `gcov-tool.cc`:

1. **Individual flag testing**: Each switch case (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually
2. **Flag combinations**: Multiple combinations of flags are tested together
3. **Valid GCOV data**: Creates and compiles a C program, runs it multiple times to generate `.gcda` files
4. **Overlap mode**: All invocations use `gcov-tool overlap` subcommand
5. **Error testing**: Tests invalid flag `-Z` to trigger the `default` case and `overlap_usage()`
6. **Multiple inputs**: Tests with multiple `.gcda` files, wildcards, and directories
7. **Verbose output**: Captures verbose output to files
8. **Different thresholds**: Tests various values for `-t` flag (0.5, 1.0, 10.5, etc.)

To run the script:
