This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two slightly different instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to cover each case statement
3. **Tests option combinations** to ensure they work together
4. **Tests boundary values** for the `-t` option (0.0, 1.0, -1.0, 2.5)
5. **Tests invalid options** to trigger the `default` case and `overlap_usage()`
6. **Tests edge cases** like missing arguments and different argument orders
7. **Captures all outputs** to log files for verification
8. **Runs in a temporary directory** and cleans up after itself

To execute this test:
