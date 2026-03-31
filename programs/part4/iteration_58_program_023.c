This script creates a comprehensive test that:

1. **Multiple Compilation Units with Mixed Options**: Tests 4, 5, and 11 use different `-save-temps`, `-dumpdir`, and `-dumpbase` options for different source files in the same invocation.

2. **Combine Help/Version with Compilation**: Tests 3, 6, and 9 mix `--help`, `--version`, and `--target-help` with actual compilation commands.

3. **Use `@file` Syntax**: Tests 3, 8, 9, and 15 use response files with various combinations of arguments.

4. **Exercise `-save-temps` and Dump Options**: Tests 1, 4, 8, 11, and 14 use different variants of `-save-temps` (`=cwd`, `=obj`) along with `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`.

5. **Driver-Specific Mode Switches**: Tests 2, 7, and 11 use `-E`, `-S`, `-c`, and `-x` options to change processing modes within single invocations.

6. **Environment Variables and Wrappers**: Test 10 uses a wrapper script and environment variables to modify driver behavior.

The script creates temporary files, runs GCC with various argument combinations that should trigger the reset logic, and cleans up afterward. Each test is designed to exercise different paths through the driver's state machine, ensuring that the reset block at lines 11228-11250 gets executed.

To run this test:
