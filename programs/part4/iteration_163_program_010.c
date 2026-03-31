This comprehensive test script:

1. **Creates a valid test program** that generates GCOV data when compiled with instrumentation.

2. **Generates two .gcda files** (`base.gcda` and `compare.gcda`) required for the `overlap` subcommand.

3. **Tests each individual option** from the uncovered switch block:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full filename)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` (hot threshold) with various values including boundary cases

4. **Tests option combinations** to ensure they work together correctly.

5. **Tests invalid options** to trigger the `default` case and `overlap_usage()` function.

6. **Captures all output** to log files for verification.

7. **Provides a summary** of test results.

To execute this test:
