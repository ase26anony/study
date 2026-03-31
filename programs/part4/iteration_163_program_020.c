This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to cover each case statement
3. **Tests option combinations** to ensure they work together
4. **Tests boundary values** for the `-t` option (0.0, 0.5, 0.75, 1.0, -1.0, 2.5)
5. **Tests invalid option** (`-x`) to trigger the default case and `overlap_usage()`
6. **Tests edge cases** like missing arguments and non-existent files
7. **Verifies option effects** by comparing output with and without options
8. **Cleans up** all temporary files automatically
9. **Provides clear output** showing which tests passed

To run this test, save it as `test_gcov_tool_overlap.sh`, make it executable, and run it:
