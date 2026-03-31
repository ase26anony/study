This script provides comprehensive testing of the uncovered argument parsing logic:

1. **Individual short options**: Tests each case (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
2. **Multiple combinations**: Tests various combinations of options in different orders
3. **Edge cases for `-t`**: Tests with 0.001, 100.0, and 0 as threshold values
4. **Invalid option**: Tests `-x` to trigger the `default:` case and `overlap_usage()`
5. **Complex final command**: Includes a command with all options as requested
6. **Help and version**: Tests overall command-line infrastructure
7. **Error handling**: Tests missing argument for `-t` option

The script creates dummy `.gcda` files if real ones aren't available, ensuring the argument parsing logic is reached even if the actual overlap analysis fails due to invalid data.

To use this script:

1. Build `gcov-tool` with coverage instrumentation:
