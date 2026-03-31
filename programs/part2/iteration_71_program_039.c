This script provides comprehensive testing of the uncovered lines:

1. **Creates valid GCDA files**: Compiles and runs two C programs with coverage instrumentation to generate `.gcda` files.

2. **Tests each option individually and in combination**:
   - `-v` triggers verbose mode
   - `-f`, `-F`, `-o`, `-h` set boolean flags
   - `-t` with various float values (0.75, 0.5, etc.)
   - Invalid `-t` argument tests error handling

3. **Triggers the default case**: Uses `-x` to trigger the `default:` case and call `overlap_usage()`

4. **Validates behavior**: Checks exit codes and output patterns to ensure options are processed correctly

5. **Handles edge cases**: Tests with missing arguments, single file, and multiple files

To run the test:
