This script provides comprehensive coverage of the target lines by:

1. **Generating valid GCOV data**: Creates a C program with conditional branches, compiles it with GCOV instrumentation, and runs it multiple times with different inputs to generate distinct `.gcda` files.

2. **Testing all individual flags**: 
   - `-v` (verbose)
   - `-f` (function level overlap)
   - `-F` (use full names)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` with various threshold values (0.5, 1.0, 10.5)

3. **Testing flag combinations**: 
   - `-f -o`
   - `-F -h -t 1.0`
   - `-v -f -F -o -h -t 5.0`
   - `-f -t 0.8 -o`

4. **Testing with multiple input files**: Uses up to 4 different `.gcda` files to stress the overlap merging logic.

5. **Triggering the default case**: Tests with invalid flag `-Z` to trigger `overlap_usage()`.

6. **Additional edge cases**: Tests with very small (0.001) and large (100.0) threshold values, and tests with differently optimized builds.

To run this script:
