This script comprehensively tests all the uncovered lines in `gcov-tool.cc`:

1. **Generates valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times to generate `.gcda` files.

2. **Tests all individual flags**:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (fullname)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` with various floating-point values (0.5, 1.0, 10.5)

3. **Tests flag combinations**:
   - `-f -o`
   - `-F -h -t 1.0`
   - `-v -f -F -o -h -t 5.0`
   - `-v -f` with mixed profile files

4. **Triggers the default case**: Uses `-Z` (invalid flag) to trigger `overlap_usage()`

5. **Uses multiple input files**: Tests with 1, 2, and 3 `.gcda` files

6. **Tests edge cases**: Very low (0.01) and high (99.9) threshold values

7. **Includes optimized compilation**: Compiles with `-O2` to generate different profile patterns

The script captures all output to files for verification and provides a summary of what was tested. To run it, simply make it executable and execute:
