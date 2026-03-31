This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to cover each case statement
3. **Tests option combinations** to ensure they work together correctly
4. **Tests boundary values** for the `-t` threshold option (0.0, 1.0, 0.001, 0.999)
5. **Tests invalid threshold values** (-1.0, 2.5, "invalid") to exercise the `atof` parsing
6. **Tests an invalid option** (`-x`) to trigger the `default` case and `overlap_usage()`
7. **Verifies verbose mode** actually produces more output
8. **Tests with different file combinations** to ensure robustness
9. **Captures all output** to log files for verification
10. **Cleans up automatically** using a temporary directory

To run this test:
