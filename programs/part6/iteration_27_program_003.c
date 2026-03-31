This script provides comprehensive coverage of the specified uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times with different parameters to generate varied `.gcda` files.

2. **Testing all individual flags**: Tests `-v`, `-f`, `-F`, `-o`, `-h`, and `-t` with different threshold values.

3. **Testing flag combinations**: Tests various combinations including `-f -o`, `-F -h -t 1.0`, and `-v -f -F -o -h -t 5.0`.

4. **Triggering the default case**: Tests an invalid flag `-Z` to trigger `overlap_usage()`.

5. **Using multiple input files**: Tests with 2-3 `.gcda` files simultaneously, including files from differently optimized builds.

6. **Handling verbose output**: Captures verbose output to files to ensure the code path is executed.

7. **Testing edge cases**: Tests with different threshold values and various file combinations.

To run the script:
