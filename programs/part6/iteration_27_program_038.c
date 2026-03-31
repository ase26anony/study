This script provides comprehensive coverage of the uncovered lines by:

1. **Generating valid GCOV data**: Creates a C program, compiles it with GCOV instrumentation, and runs it multiple times with different parameters to generate varied `.gcda` files.

2. **Testing all individual flags**: Each uncovered flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually with valid arguments.

3. **Testing flag combinations**: Multiple combinations of flags are tested together to ensure the parser handles them correctly.

4. **Triggering the default/usage case**: Includes an invalid flag (`-Z`) to trigger the `default` case in the switch statement, which calls `overlap_usage()`.

5. **Using multiple input files**: Tests with 2, 3, and 4 input `.gcda` files to stress the overlap analysis logic.

6. **Testing edge cases**: Includes threshold edge cases (0.0, 100.0) and tests with single files.

7. **Verifying execution**: Captures output and verifies that the usage message was triggered for the error case.

To run this script:
