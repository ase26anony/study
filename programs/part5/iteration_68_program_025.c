This comprehensive test script:

1. **Creates a valid C program** with multiple functions and control flow to generate meaningful coverage data
2. **Generates two distinct GCDA files** by running the program with different arguments and using `gcov-tool merge`
3. **Tests all individual flags** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as specified in the uncovered lines
4. **Tests combined flags** to ensure they work together
5. **Tests edge cases** including:
   - Missing argument for `-t`
   - Non-numeric argument for `-t`
   - Same file twice
   - Non-existent files
   - Invalid options (triggering the `default` case and `overlap_usage()`)
   - Boundary values for `-t`
   - Negative values for `-t`
   - Empty files
   - Different flag ordering

To use this script with a coverage-instrumented `gcov-tool`:

1. First build `gcov-tool` with coverage:
