This script systematically tests the uncovered reset logic by:

1. **Multiple compilation units with mixed options** (Tests 4, 13): Uses different `-save-temps` and `-dumpdir` options for different files in the same command.

2. **Combine help/version with compilation** (Tests 1, 3, 6, 9, 12, 14): Mixes `--help`, `--version`, and `--target-help` with source files in various orders.

3. **Use `@file` syntax** (Tests 3, 5, 9): Creates response files with dump options and uses them with additional command-line arguments.

4. **Exercise `-save-temps` variants** (Test 10): Tests all `-save-temps` variants (`=cwd`, `=obj`, default).

5. **Driver mode switches** (Tests 2, 7): Chains `-E`, `-S`, `-c` options and uses `-x` language specification.

6. **Environment variables and wrapper** (Test 8): Sets `GCC_EXEC_PREFIX` and `COMPILER_PATH`, and uses a wrapper script.

The script creates temporary files, runs various gcc invocations that stress the driver's state machine, and cleans up. The `2>/dev/null` redirects stderr since many commands will produce warnings/errors due to conflicting options, but that's acceptable as we're testing the driver's internal reset logic, not successful compilation.

To run this test:
