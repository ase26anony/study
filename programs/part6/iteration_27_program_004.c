This script provides comprehensive testing of the uncovered argument parsing logic in `gcov-tool.cc`:

## Key Features:

1. **Individual Flag Testing**: Tests each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) independently
2. **Flag Combinations**: Tests various combinations of flags together
3. **Threshold Variations**: Tests different floating-point values for `-t` flag
4. **Invalid Flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`
5. **Multiple Input Files**: Generates and uses multiple `.gcda` files from different runs
6. **Verbose Output**: Captures output for `-v` flag
7. **Error Cases**: Tests missing arguments and invalid usage

## Execution Flow:

1. Creates a test C program with conditional logic
2. Compiles it with GCOV instrumentation at different optimization levels
3. Executes the program multiple times to generate varied profile data
4. Systematically tests all flag combinations
5. Cleans up temporary files

## To Run:
