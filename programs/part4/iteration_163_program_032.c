This comprehensive test script:

1. **Creates a valid test program** with loops and conditionals to generate meaningful coverage data
2. **Compiles with GCOV instrumentation** using `-fprofile-arcs -ftest-coverage`
3. **Generates two .gcda files** for overlap comparison by running the program twice
4. **Tests each individual option** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) to cover each case in the switch statement
5. **Tests option combinations** to ensure they work together correctly
6. **Tests threshold boundary values** (0.0, 1.0, 0.001, 0.999) and edge cases
7. **Tests invalid cases** including:
   - Invalid option `-x` (triggers default case and `overlap_usage()`)
   - Missing argument for `-t`
   - Invalid threshold values (negative, >1.0, non-numeric)
8. **Captures all output** to log files for verification
9. **Provides a summary** of what was tested

To execute this test:
