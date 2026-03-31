This script comprehensively tests all the uncovered lines in the `gcov-tool.cc` file:

1. **Creates test programs**: Generates two different C programs to create distinct coverage data files for comparison.

2. **Tests each individual option**:
   - `-v` (verbose) - triggers `case 'v'`
   - `-f` (function level) - triggers `case 'f'`
   - `-F` (full filename) - triggers `case 'F'`
   - `-o` (object level) - triggers `case 'o'`
   - `-h` (hot only) - triggers `case 'h'`
   - `-t` (hot threshold) - triggers `case 't'`

3. **Tests option combinations**: Exercises multiple options together to ensure they work correctly in combination.

4. **Tests threshold boundary values**:
   - Valid values: `0.0`, `0.5`, `0.75`, `0.9`, `1.0`
   - Out-of-range values: `-1.0`, `2.5`
   - Invalid format: `invalid`

5. **Tests invalid option**: Uses `-x` to trigger the `default` case and call `overlap_usage()`.

6. **Captures all output**: Each command's output is saved to a log file for verification.

7. **Validates behavior**: The script checks if the verbose option actually produces more output, providing a basic validation that the option is working.

To run this test, save it as `run_gcov_tool_test.sh`, make it executable, and run it:
