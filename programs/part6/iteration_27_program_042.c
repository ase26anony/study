This script provides comprehensive coverage of the target lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional logic, compiles it with GCOV instrumentation at different optimization levels, and runs it multiple times with different inputs to generate varied `.gcda` files.

2. **Testing all individual flags**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid GCOV data.

3. **Testing flag combinations**: Multiple combinations of flags are tested together to ensure the parser handles them correctly.

4. **Triggering the default case**: Includes a test with invalid flag `-Z` to trigger `overlap_usage()`.

5. **Using multiple input files**: Tests with multiple `.gcda` files from different runs and compilation modes.

6. **Testing various threshold values**: Tests the `-t` flag with different floating-point values including edge cases (0.0, 100.0).

7. **Handling verbose output**: Captures verbose output to files to ensure the `-v` flag path is executed.

To run the script:
