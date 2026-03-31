This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Creates valid GCDA files**: Two C programs are compiled with `-fprofile-arcs -ftest-coverage` and executed to generate `.gcda` files.

2. **Tests each option individually**:
   - `-v` (verbose): Triggers `gcov_set_verbose()`
   - `-f` (function level): Sets `overlap_func_level = 1`
   - `-F` (fullname): Sets `overlap_use_fullname = 1`
   - `-o` (object level): Sets `overlap_obj_level = 1`
   - `-h` (hot only): Sets `overlap_hot_only = 1`
   - `-t` (threshold): Sets `overlap_hot_threshold = atof(optarg)`

3. **Tests option combinations**: Including boolean flags together and with the threshold option.

4. **Triggers the default case**: Using `-x` as an invalid option to call `overlap_usage()`.

5. **Handles edge cases**: Invalid arguments for `-t`, missing arguments, etc.

6. **Provides clear output**: Shows which tests pass/fail and maps them to the specific uncovered lines.

To run the test:
