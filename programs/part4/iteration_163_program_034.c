This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to cover each case in the switch statement
3. **Tests option combinations** to ensure they work together correctly
4. **Tests edge cases** for the `-t` option (0.0, 1.0, 1.5, -0.5)
5. **Tests invalid option** (`-x`) to trigger the `default` case and `overlap_usage()` call
6. **Tests error conditions** (missing argument, non-existent file)
7. **Captures all output** to log files for verification
8. **Runs in a temporary directory** and cleans up automatically
9. **Provides exit code verification** for each test

To run this test, make the script executable and execute it:
