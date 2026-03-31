This script systematically tests all the uncovered command-line options in the `gcov-tool overlap` feature:

1. **Individual flag testing**: Each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually
2. **Flag combinations**: Multiple flags are tested together to ensure the parser handles them correctly
3. **Invalid flag**: The `-Z` flag triggers the `default` case and calls `overlap_usage()`
4. **Multiple input files**: Tests with 2-3 `.gcda` files to stress the overlap logic
5. **Different threshold values**: Tests various floating-point values for `-t` flag
6. **Verbose output**: The `-v` flag output is captured to ensure the code path is executed

The script creates a test C program with conditional branches to generate meaningful coverage data, compiles it with GCOV instrumentation, runs it multiple times to generate different profile data sets, and then invokes `gcov-tool overlap` with all the required flag combinations.

To run this test, make the script executable and execute it:
