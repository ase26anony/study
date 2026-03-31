This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Creates minimal C programs** with different control flow patterns to generate meaningful coverage data for overlap analysis.

2. **Tests each specific option** mentioned in the uncovered lines:
   - `-v` (verbose) - triggers `gcov_set_verbose()`
   - `-f` (overlap_func_level)
   - `-F` (overlap_use_fullname)
   - `-o` (overlap_obj_level)
   - `-h` (overlap_hot_only)
   - `-t` with various float arguments (0.75, 0.5, 1)

3. **Triggers the default case** with invalid option `-x` to call `overlap_usage()`.

4. **Tests combined options** to ensure they work together.

5. **Tests edge cases**:
   - Multiple `.gcda` files (more than 2)
   - Invalid numeric argument for `-t`
   - Different float formats

6. **Includes proper cleanup** of temporary files.

To run the test:
