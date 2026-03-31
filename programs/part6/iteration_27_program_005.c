This script provides comprehensive coverage of the uncovered lines in `gcov-tool.cc`:

1. **Individual flag testing**: Tests each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
2. **Flag combinations**: Tests various combinations of flags together
3. **Multiple input files**: Uses 2-4 `.gcda` files in different combinations
4. **Error cases**: Tests invalid flag `-Z` to trigger `overlap_usage()`
5. **Threshold variations**: Tests different floating-point values for `-t` flag
6. **Verbose output**: Captures verbose output to files
7. **Valid GCOV data**: Generates real profile data by compiling and running a test program

To run this script:
