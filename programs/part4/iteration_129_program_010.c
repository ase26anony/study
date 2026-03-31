This script provides comprehensive coverage of the uncovered lines:

1. **Creates a valid GCOV data file** by compiling and running a simple C program with coverage instrumentation.

2. **Tests each individual flag**:
   - `-h`: Prints usage information
   - `-v`: Prints version information
   - `-l`: Dumps GCOV file contents
   - `-p`: Dumps positions
   - `-r`: Dumps raw data
   - `-s`: Dumps stable output

3. **Triggers the default case** with invalid flags `-x` and `-Z`, which will cause the "unknown flag" error message.

4. **Tests combined flags** with `-l -p -s` to ensure multiple internal `flag_*` variables are set correctly.

5. **Includes additional edge cases**:
   - No arguments (should show usage)
   - Multiple input files
   - Flag ordering variations

6. **Handles output appropriately** using `2>&1` to capture both stdout and stderr, and `head` to limit output for readability.

7. **Provides proper cleanup** of temporary files and directories.

To run this test:
