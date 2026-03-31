This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two slightly different instrumented C programs
2. **Systematically tests each uncovered option** individually (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`)
3. **Tests option combinations** to ensure they work together correctly
4. **Tests boundary values** for the `-t` threshold option (0.0, 1.0, -1.0, 2.5, invalid)
5. **Tests the default case** with an invalid option (`-x`)
6. **Captures all output** to log files for verification
7. **Runs in a temporary directory** and cleans up after itself
8. **Provides clear feedback** about what was tested and the results

To run this test, save it as `run_gcov_tool_test.sh`, make it executable, and execute it:
