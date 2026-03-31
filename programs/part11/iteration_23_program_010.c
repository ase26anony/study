This script comprehensively tests all the uncovered lines:

1. **Generates valid .gcda files**: Creates two test C programs, compiles them with `-fprofile-arcs -ftest-coverage`, and runs them with different inputs to produce varied coverage data.

2. **Tests all target flags**: 
   - `-v` (verbose): Sets `verbose = true` and calls `gcov_set_verbose()`
   - `-f` (function-level): Sets `overlap_func_level = 1`
   - `-F` (full filenames): Sets `overlap_use_fullname = 1`
   - `-o` (object-level): Sets `overlap_obj_level = 1`
   - `-h` (hot-only): Sets `overlap_hot_only = 1`
   - `-t` (threshold): Sets `overlap_hot_threshold = atof(optarg)` with values 0.5, 0.75, and 1.0

3. **Provides multiple .gcda files**: Generates at least 5 different .gcda files from different runs and passes them to `gcov-tool overlap`.

4. **Triggers the default case**: Tests with invalid option `-z` to trigger the `default:` branch that calls `overlap_usage()`.

5. **Tests various combinations**: Tests all flags together, individual flags, and minimal combinations to ensure all code paths are exercised.

To run this test:
