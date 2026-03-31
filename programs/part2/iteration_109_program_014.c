This test script comprehensively covers all the requirements:

1. **Valid flag invocations**: Tests each short option (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) individually and in combination.

2. **Invalid flag invocations**: Tests undefined options (`-x`, `-z`, `-9`, `-?`) to trigger the default case.

3. **File handling**: Creates valid `.gcno` and `.gcda` files by compiling and running a test program with coverage instrumentation.

4. **Edge cases**:
   - No arguments
   - Flags only without filename
   - `--` argument separator
   - Non-existent filename
   - Multiple files
   - Complex coverage data with optimization

5. **Execution flow**: The script creates a temporary directory, generates coverage data, runs all test cases, and cleans up.

To ensure `gcov-dump` itself is instrumented for coverage measurement, you should compile it with:
