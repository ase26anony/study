This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two different instrumented C programs
2. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately to ensure each case in the switch statement is executed
3. **Tests option combinations** to ensure they work together correctly
4. **Tests boundary values for `-t`** including 0.0, 1.0, and out-of-range values to exercise the `atof` parsing
5. **Tests invalid options** to trigger the `default` case and `overlap_usage()` call
6. **Captures all output** to log files for verification
7. **Verifies verbose mode** by comparing output length with and without `-v`
8. **Cleans up** all temporary files automatically

To run this test, save it as `run_gcov_tool_test.sh`, make it executable, and execute it:
