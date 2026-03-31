This comprehensive test script:

1. **Creates a valid test program** that generates GCOV profile data
2. **Generates the required .gcda files** for overlap analysis
3. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) covering all switch cases
4. **Tests option combinations** to ensure they work together
5. **Tests threshold boundary values** including 0.0, 1.0, and fractional values
6. **Tests invalid options** to trigger the default case and usage function
7. **Tests edge cases** like missing arguments and invalid threshold values
8. **Captures all output** for verification
9. **Cleans up automatically** but preserves the directory for inspection if needed

To run this test, save it as `run_gcov_tool_test.sh`, make it executable, and run it:
