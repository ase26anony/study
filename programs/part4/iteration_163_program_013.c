This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Systematically tests each uncovered option** individually:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full filename)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` (hot threshold) with various values

3. **Tests option combinations** to ensure they work together correctly
4. **Tests boundary values** for the `-t` option (0.0, 1.0, 0.001, 0.999)
5. **Tests invalid options** to trigger the `default` case and `overlap_usage()` call
6. **Tests edge cases** like missing arguments and out-of-range values
7. **Captures all output** to log files for verification
8. **Runs in a temporary directory** and cleans up after itself
9. **Provides a summary** of what was tested

To execute this test, save it as `run_gcov_tool_test.sh`, make it executable, and run it:
