This script systematically tests the uncovered reset logic by:

1. **Multiple compilation units with mixed options**: Tests 2, 8, and 9 use different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different files in the same invocation.

2. **Combine help/version with compilation**: Tests 1, 5, and 11 mix `--help`, `--version`, and `--target-help` with actual source files.

3. **Use `@file` syntax**: Tests 3, 5, and 7 use response files with various options.

4. **Exercise `-save-temps` and dump options**: Tests 2, 3, 8, and 9 use different combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Driver-specific mode switches**: Test 4 chains `-E`, `-S`, and `-c` modes in one command. Test 15 uses `-x` for language specification.

6. **Environment variables and wrappers**: Test 6 uses `COMPILER_PATH`, and Test 10 uses a wrapper script with `GCC_EXEC_PREFIX`.

The script runs each test independently (with `|| true` to continue on errors) since we're testing the driver's internal state reset, not the success of compilation. The final check verifies the driver is still functional after all tests.

To run this test:
