This comprehensive test script:

1. **Creates test programs** that generate GCOV coverage data
2. **Generates two different .gcda files** for overlap analysis
3. **Systematically tests each uncovered option**:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full filename)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` with various threshold values (0.5, 0.0, 1.0, 2.5, -1.0, invalid)

4. **Tests option combinations** to ensure they work together
5. **Tests edge cases**:
   - Invalid option (`-x`) to trigger the `default` case and `overlap_usage()`
   - Missing argument for `-t` option
   - Baseline test with no options

6. **Captures all output** to log files for verification
7. **Provides a summary** of test execution

The script ensures that every case in the uncovered switch block (lines 534-554) is exercised:
- Each `case` statement is triggered by its corresponding option
- The `default` case is triggered by an invalid option
- The `atof(optarg)` call is exercised with various numeric and non-numeric inputs
- All internal variables (`verbose`, `overlap_func_level`, etc.) are set

To run this test, ensure `gcov-tool` is in your PATH and was built with GCOV instrumentation. Execute the script with:
