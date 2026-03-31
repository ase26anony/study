This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

## Key Features:

1. **Individual Flag Testing**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid GCOV data.

2. **Flag Combinations**: Multiple combinations of flags are tested to ensure the parser handles them correctly.

3. **Invalid Flag**: The `-Z` flag triggers the `default` case and calls `overlap_usage()`.

4. **Multiple Threshold Values**: Tests various floating-point values for `-t` flag (0.5, 1.0, 10.5, 0.0, 100.0, 0.001).

5. **Multiple Profile Data Files**: Generates 3 different execution profiles and uses multiple `.gcda` files in tests.

6. **Verbose Output Handling**: Captures verbose output to files when `-v` flag is used.

7. **Edge Cases**: Tests missing threshold argument and different flag ordering.

## Usage:
