This comprehensive test script:

1. **Creates a valid test program** with loops, conditionals, and function calls to generate meaningful coverage data.

2. **Generates GCOV data files** by compiling with `-fprofile-arcs -ftest-coverage` and running the program twice to create slightly different coverage profiles.

3. **Systematically tests each uncovered option**:
   - `-v` (verbose)
   - `-f` (function level)
   - `-F` (full filename)
   - `-o` (object level)
   - `-h` (hot only)
   - `-t` with various arguments (0.0, 0.5, 1.0, -1.0, 2.5)

4. **Tests combinations** of options to ensure they work together.

5. **Tests edge cases**:
   - Invalid option (`-x`) to trigger the `default` case
   - `-t` without argument (should fail)
   - Multiple options combined

6. **Validates behavior**:
   - Checks that verbose mode produces more output
   - Verifies invalid options trigger usage messages
   - Captures all output for manual inspection

7. **Cleans up automatically** using a temporary directory and trap.

To run this test, make the script executable and execute it:
