This script provides comprehensive coverage of the specified uncovered lines:

1. **Individual flags**: Tests `-v`, `-f`, `-F`, `-o`, `-h`, and `-t` with various values
2. **Flag combinations**: Tests multiple flags together as specified
3. **Invalid flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`
4. **Multiple input files**: Uses 4 different `.gcda` files from different runs
5. **Valid GCOV data**: Creates a real C program, compiles with instrumentation, and generates profile data
6. **Different scenarios**: Tests edge cases like threshold values 0.0 and 100.0
7. **File combinations**: Tests with different numbers and combinations of input files

To run the script:
