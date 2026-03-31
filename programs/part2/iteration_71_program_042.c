This script comprehensively tests all the uncovered lines by:

1. **Creating valid GCDA files**: Compiles and runs two C programs with coverage instrumentation to generate `.gcda` files.

2. **Testing each option individually and in combination**:
   - `-v` triggers `verbose = true` and `gcov_set_verbose()`
   - `-f` sets `overlap_func_level = 1`
   - `-F` sets `overlap_use_fullname = 1`
   - `-o` sets `overlap_obj_level = 1`
   - `-h` sets `overlap_hot_only = 1`
   - `-t` with argument calls `atof(optarg)` to set `overlap_hot_threshold`

3. **Triggering the default case**: Using `-x` (invalid option) calls `overlap_usage()`

4. **Testing edge cases**:
   - Valid and invalid arguments for `-t`
   - Missing required arguments
   - Insufficient positional arguments
   - Multiple GCDA files

5. **Validating execution**: Each test checks exit codes and output patterns to ensure the code paths are executed.

To run the test:
