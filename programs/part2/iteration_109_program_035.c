This comprehensive test script:

1. **Creates valid coverage data files** by compiling and running a simple C program with coverage instrumentation.

2. **Tests all valid flags** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) individually and in combination.

3. **Tests invalid flags** to trigger the `default:` case in the switch statement, including:
   - Single invalid flags (`-x`, `-?`, `-9`)
   - Invalid flags mixed with valid ones (`-l -z`)

4. **Tests edge cases**:
   - No arguments
   - Flags without filename
   - Non-existent files
   - Multiple files
   - Using `--` separator

5. **Tests with different file types** by creating an optimized build with more complex control flow.

6. **Verifies outputs** by checking exit codes and grepping for expected output patterns.

To run this test, save it as `test_gcov_dump.sh`, make it executable, and run it:
