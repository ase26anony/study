**Key features of this test script:**

1. **Comprehensive flag testing**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and various combinations.

2. **Invalid flag testing**: Includes tests with `-x` and `-Z` to trigger the `default` case in the switch statement.

3. **Multiple usage scenarios**: Tests single files, multiple files, wildcards, and different argument orders.

4. **Error condition testing**: Tests missing files, no arguments, and other edge cases.

5. **Compact flag syntax**: Tests combined flags like `-lp` and `-rps`.

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
