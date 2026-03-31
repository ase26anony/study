This script provides comprehensive testing of the uncovered lines by:

1. **Creating valid GCOV data files** by compiling and running an instrumented C program
2. **Testing each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately
3. **Testing option combinations** to ensure they work together
4. **Testing boundary values** for the `-t` threshold option
5. **Testing invalid options** to trigger the `default` case and `overlap_usage()`
6. **Capturing all output** to log files for verification
7. **Verifying option effects** by comparing verbose vs non-verbose output

To run this test, save it as `test_gcov_tool_coverage.sh`, make it executable, and run it:
