This script provides comprehensive coverage of all the requirements:

1. **Comprehensive Flag Combination**: Tests each individual flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) and multiple combinations with different threshold values.

2. **Valid GCOV Data Input**: Creates a C program, compiles it with GCOV instrumentation, and runs it multiple times to generate `.gcda` files.

3. **Overlap Analysis Mode**: All tests use the `overlap` subcommand as required.

4. **Error and Usage Testing**: Includes an invalid flag `-Z` test to trigger the `default` case and `overlap_usage()`.

5. **Multiple Input Files**: Generates 4 different `.gcda` files (3 with different arguments, 1 with different optimization level) and tests with varying numbers of input files.

6. **Verbose Output Handling**: Tests `-v` flag individually and in combinations, capturing output to verify execution.

The script also includes:
- Error checking for `gcov-tool` availability
- Color-coded output for better readability
- Cleanup option at the end
- Tests for threshold edge cases
- Flag order variations
- Output redirection for verbose mode

To run the script:
