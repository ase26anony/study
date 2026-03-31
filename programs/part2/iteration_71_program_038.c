This script provides comprehensive testing for the uncovered lines:

1. **Creates valid GCDA files**: Two C programs are compiled with `-fprofile-arcs -ftest-coverage` and executed to generate `.gcda` files.

2. **Tests all required options**:
   - `-v` (verbose): Triggers `gcov_set_verbose()` (lines 534-536)
   - `-f` (overlap_func_level): Sets flag to 1 (lines 537-539)
   - `-F` (overlap_use_fullname): Sets flag to 1 (lines 540-542)
   - `-o` (overlap_obj_level): Sets flag to 1 (lines 543-545)
   - `-h` (overlap_hot_only): Sets flag to 1 (lines 546-548)
   - `-t` (overlap_hot_threshold): Calls `atof(optarg)` (lines 549-551)

3. **Triggers the default case**: Uses `-x` to trigger `overlap_usage()` (lines 552-554)

4. **Tests various argument combinations**: Individual flags, combined flags, and different threshold values.

5. **Validates execution**: Checks exit codes and captures output where appropriate.

To run the test:
