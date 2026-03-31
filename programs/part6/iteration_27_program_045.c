This script provides comprehensive coverage of all the requirements:

1. **Comprehensive Flag Combination**: Tests each individual flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) and multiple combinations.

2. **Valid GCOV Data Input**: Creates a C program, compiles it with GCOV instrumentation, and runs it multiple times to generate `.gcda` files.

3. **Overlap Analysis Mode**: All invocations use `gcov-tool overlap` as the subcommand.

4. **Error and Usage Testing**: Includes an invalid `-Z` flag to trigger the `default` case and `overlap_usage()`.

5. **Multiple Input Files**: Uses multiple `.gcda` files from different runs and directories.

6. **Verbose Output Handling**: Captures verbose output to files for verification.

The script also includes:
- Multiple threshold values for `-t` flag (0.5, 1.0, 10.5, 2, 0.75)
- Tests with optimized compilation for different profile patterns
- Proper cleanup of temporary files
- Verification of output generation
- Summary reporting

To run the script:
