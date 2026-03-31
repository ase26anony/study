This script systematically tests all the uncovered lines in the switch statement:

1. **Individual flag tests** (Tests 1-6): Each flag (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually to ensure each case in the switch statement is hit.

2. **Combined flag tests** (Tests 7-10): Tests combinations of flags to ensure they work together correctly.

3. **Edge case tests** (Tests 11-14):
   - `-t` without argument (triggers error handling)
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files

4. **Invalid option tests** (Tests 15-16): Specifically designed to trigger the `default` case and call `overlap_usage()`.

5. **Additional thorough testing** (Tests 17-22): Tests various threshold values and file ordering to ensure robustness.

To use this script:

1. First, ensure you have an instrumented version of `gcov-tool` built with coverage flags:
