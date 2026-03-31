This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual uncovered option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
3. **Tests option combinations** to ensure they work together
4. **Tests threshold boundary values** (0.0, 1.0, edge cases, out-of-range)
5. **Tests invalid options** to trigger the default case
6. **Tests edge cases** like missing arguments and extra arguments
7. **Captures all output** to log files for verification
8. **Cleans up automatically** using a temporary directory

To run this test, save it as `run_gcov_tool_test.sh` and make it executable:
