This script systematically tests all the uncovered switch cases:

1. **Individual flag tests** (lines 534-554):
   - `-v` triggers `verbose = true` and `gcov_set_verbose()`
   - `-f` sets `overlap_func_level = 1`
   - `-F` sets `overlap_use_fullname = 1`
   - `-o` sets `overlap_obj_level = 1`
   - `-h` sets `overlap_hot_only = 1`
   - `-t` with argument calls `atof(optarg)`

2. **Combined flag tests** to ensure multiple options work together.

3. **Error cases** to trigger the `default` case (line 553):
   - Invalid option `-z` should call `overlap_usage()`

4. **Edge cases** for the `-t` flag:
   - Missing argument (should trigger error)
   - Non-numeric argument
   - Boundary values (0.0, 1.0, >1.0, negative)

5. **Valid input data**: Creates real `.gcda` files by compiling and running a simple C program with coverage instrumentation.

To use this script:

1. Ensure `gcov-tool` is built with coverage instrumentation:
