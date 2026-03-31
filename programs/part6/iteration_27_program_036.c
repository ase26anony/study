This script provides comprehensive testing of all the uncovered lines:

1. **Individual flags**: Tests each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
2. **Flag combinations**: Tests various combinations as specified
3. **Multiple threshold values**: Tests `-t` with different floating-point values
4. **Multiple input files**: Tests with multiple `.gcda` files
5. **Invalid flag**: Tests `-Z` to trigger the `default` case and `overlap_usage()`
6. **Verbose output**: Captures verbose output to ensure the `-v` flag path is executed

To run this script:
