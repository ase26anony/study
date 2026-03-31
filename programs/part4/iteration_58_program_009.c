This script systematically tests the reset logic by:

1. **Multiple compilation units with mixed options**: Tests 1, 2, 4, and 10 use different files with varying `-save-temps`, `-dumpdir`, and `-dumpbase` options.

2. **Combine help/version with compilation**: Tests 3 and 5 place `--help` and `--version` options in the middle of command lines, forcing state resets.

3. **Use `@file` syntax**: Tests 3, 6, and 8 use response files to set `at_file_supplied` flag.

4. **Exercise `-save-temps` variants**: Tests 1, 3, 4, 7, and 9 use different `-save-temps` options (`=cwd`, `=obj`, default) with various dump options.

5. **Driver mode switches**: Test 2 chains `-E`, `-S`, and `-c` modes in a single invocation. Test 10 walks through the full pipeline.

6. **Environment variables**: Test 7 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH` to affect driver behavior.

The script creates temporary files and directories, runs the tests, and cleans up. Each test is designed to trigger different paths in the reset logic, particularly focusing on:
- Freeing and reinitializing `dumpdir`, `dumpbase`, `dumpbase_ext`, `outbase`
- Resetting `save_temps_flag` and related boolean flags
- Setting `at_file_supplied` via `@file` syntax
- Changing `spec_machine` and `greatest_status` through mode switches

To run this test, save it as `test_gcc_reset.sh`, make it executable, and run it with a GCC installation in your PATH:
