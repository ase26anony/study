This script provides comprehensive coverage of the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program, compiles it with GCOV instrumentation at different optimization levels, and runs it multiple times to generate `.gcda` files.

2. **Testing all individual flags**: Tests `-v`, `-f`, `-F`, `-o`, `-h`, and `-t` with different threshold values.

3. **Testing flag combinations**: Tests various combinations like `-f -o`, `-F -h -t 1.0`, and the comprehensive `-v -f -F -o -h -t 5.0`.

4. **Triggering the default case**: Tests invalid flag `-Z` to trigger `overlap_usage()`.

5. **Using multiple input files**: Tests with multiple `.gcda` files from different runs.

6. **Handling verbose output**: Captures verbose output to ensure the `-v` flag path is executed.

7. **Testing edge cases**: Tests threshold values like 0.0 and 100.0, and missing threshold arguments.

To run the script:
