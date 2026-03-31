This script systematically tests the uncovered reset logic by:

1. **Multiple Compilation Units with Mixed Options**: Tests 2, 4, 6, and 8 compile multiple files with different `-save-temps`, `-dumpdir`, and `-dumpbase` options in a single invocation.

2. **Combine Help/Version with Compilation**: Tests 1, 5, and 10 mix `--help`, `--version`, `--target-help`, and `--help=optimizers` with actual compilation commands.

3. **Use `@file` Syntax**: Tests 3, 5, and 10 use response files (`@opts.txt`, `@args.txt`) to set initial state that gets overridden or reset.

4. **Exercise `-save-temps` and Dump Options**: Tests 2, 3, 4, 6, and 9 use various combinations of `-save-temps`, `-dumpdir`, `-dumpbase`, and `-dumpbase-ext` with different values.

5. **Driver-Specific Mode Switches**: Test 4 chains `-E`, `-S`, and `-c` modes in a single command. Test 8 mixes multiple output modes.

6. **Environment Variables**: Test 7 sets `GCC_EXEC_PREFIX` and `COMPILER_PATH` to affect driver initialization.

The script creates temporary files and directories, runs the GCC driver with complex argument combinations, and cleans up afterward. Each test is designed to trigger the reset logic at different points in the driver's execution flow.

To run this test, save it as `test_gcc_reset.sh`, make it executable, and run it with your GCC build:
