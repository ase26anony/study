This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Systematically tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) that corresponds to the uncovered case statements
3. **Tests option combinations** to ensure they work together correctly
4. **Tests boundary values** for the `-t` threshold option (0.0, 1.0, edge cases)
5. **Tests invalid options** (`-x`) to trigger the `default` case and `overlap_usage()` call
6. **Tests malformed arguments** for `-t` to exercise the `atof()` parsing
7. **Captures all output** to log files for verification
8. **Verifies verbose mode** by comparing output sizes
9. **Cleans up after itself** using a temporary directory

To run this test, save it as `test_gcov_tool_overlap.sh`, make it executable, and run it:
