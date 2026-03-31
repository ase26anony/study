This comprehensive test script:

1. **Creates valid GCOV data files** by compiling and running two instrumented C programs with different execution paths.

2. **Systematically tests each uncovered option**:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full filename)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` (hot threshold) with various values

3. **Tests option combinations** to ensure they work together correctly.

4. **Tests boundary values** for the `-t` option including:
   - Minimum (0.0)
   - Maximum (1.0)
   - Very small (0.0001)
   - Out of range negative (-1.0)
   - Out of range positive (2.5)

5. **Triggers the default case** with an invalid option (`-x`) to call `overlap_usage()`.

6. **Handles edge cases** like missing arguments and no arguments.

7. **Captures all output** to log files for verification.

8. **Cleans up** after itself using a temporary directory.

To run this test:
