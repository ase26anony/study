This script creates a comprehensive test that:

1. **Multiple compilation units with mixed options**: Tests 1, 5, 8, and 11 use different files with varying `-save-temps`, `-dumpdir`, and `-dumpbase` options.

2. **Combine help/version with compilation**: Tests 2, 9, and 12 mix `--help`, `--version`, and `--target-help` with actual source files.

3. **Use `@file` syntax**: Tests 3, 4, 9, and 12 use response files to set `at_file_supplied`.

4. **Exercise `-save-temps` and dump variants**: Tests 1, 3, 8, 10 use various combinations of `-save-temps=cwd`, `-save-temps=obj`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Leverage mode switches**: Tests 5 and 11 chain `-E`, `-S`, `-c`, and `-shared` options for different files.

6. **Environment variables and wrapper**: Tests 6, 7, and 12 use `COMPILER_PATH`, `GCC_EXEC_PREFIX`, and a wrapper script.

The script uses `2>/dev/null || true` because many of these combinations will produce errors (which is expected when testing edge cases), but the driver should still execute the reset logic before exiting or continuing.

To run this test:
