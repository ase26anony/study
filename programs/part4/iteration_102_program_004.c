This comprehensive test script:

1. **Creates a minimal C program** with coverage instrumentation that generates both `.gcda` and `.gcno` files.

2. **Tests all individual valid flags** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) with proper file arguments where needed.

3. **Tests invalid flags** (`-x`, `-z`) to trigger the `default` case in the switch statement, capturing the "unknown flag" error message.

4. **Tests combined flags** (`-lp`, `-rl`, `-lprs`) to ensure the option parsing loop processes each character individually.

5. **Tests missing required arguments** to trigger error paths in the argument parsing logic.

6. **Tests edge cases** including:
   - No arguments
   - Long options (`--version`, `--help`) if supported
   - Non-existent files
   - Using `.gcno` files instead of `.gcda`

7. **Redirects output appropriately**:
   - `stdout` to files for valid operations
   - `stderr` to files for error cases
   - Uses `/dev/null` where output isn't needed

8. **Provides clear feedback** about each test's success/failure.

9. **Cleans up** generated files after execution.

To run the test script:
