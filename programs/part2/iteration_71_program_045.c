This script provides comprehensive testing for the uncovered lines:

1. **Creates valid GCDA files**: Two simple C programs are compiled with `-fprofile-arcs -ftest-coverage` and executed to generate `.gcda` files.

2. **Tests each option individually and in combination**:
   - `-v` triggers `verbose = true` and `gcov_set_verbose()`
   - `-f` sets `overlap_func_level = 1`
   - `-F` sets `overlap_use_fullname = 1`
   - `-o` sets `overlap_obj_level = 1`
   - `-h` sets `overlap_hot_only = 1`
   - `-t` with float argument sets `overlap_hot_threshold = atof(optarg)`

3. **Triggers the default case**: Using `-x` (invalid option) calls `overlap_usage()`

4. **Tests error conditions**: Missing and invalid arguments for `-t`

5. **Validates execution**: Checks exit codes and captures output

To run the test:
