This comprehensive test script will:

1. **Create a test environment** with temporary directory cleanup
2. **Generate valid GCOV data files** by compiling and running instrumented programs
3. **Test each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) covering all cases in the switch statement
4. **Test option combinations** to ensure they work together
5. **Test boundary values** for the `-t` threshold option (0.0, 1.0, -1.0, 2.5)
6. **Test invalid options** to trigger the `default` case and `overlap_usage()`
7. **Capture all output** to log files for verification
8. **Handle edge cases** like missing arguments and swapped file order

To run this test:
