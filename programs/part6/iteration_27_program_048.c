This script provides comprehensive coverage of all the uncovered lines:

1. **Individual flag testing** (Tests 1-8): Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with different threshold values for `-t`.

2. **Flag combinations** (Tests 9-13): Tests various combinations including `-f -o`, `-F -h -t 1.0`, and all flags together `-v -f -F -o -h -t 5.0`.

3. **Error case** (Test 14): Uses an invalid flag `-Z` to trigger the `default` case in the switch statement, which calls `overlap_usage()`.

4. **Multiple input files**: Tests with 2, 3, and 4 `.gcda` files to ensure the overlap analysis logic is properly exercised.

5. **Different file organizations**: Tests with files in different directories and with absolute paths.

6. **Edge cases** (Tests 18-19): Tests with single file and extreme threshold values.

The script generates a simple C program with conditional branches, compiles it with coverage instrumentation at different optimization levels, runs it with different inputs to generate varied profile data, and then systematically tests all the command-line options for the `gcov-tool overlap` subcommand.

To run this script, make it executable and execute it:
