This comprehensive test script:

1. **Creates a valid test program** with loops and conditionals to generate meaningful coverage data
2. **Generates two .gcda files** for overlap analysis by running the program twice
3. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) from the uncovered switch block
4. **Tests option combinations** to ensure they work together correctly
5. **Tests threshold boundary values** including 0.0, 1.0, negative, and out-of-range values
6. **Tests invalid options** to trigger the `default` case and `overlap_usage()` call
7. **Tests edge cases** like missing arguments, option ordering, and multiple flags
8. **Captures all output** to log files for verification
9. **Provides a summary** of test execution

To run this test, save it as `run_gcov_tool_test.sh`, make it executable, and run it:
