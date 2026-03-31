This script provides:

1. **Command-Line Argument Generation**: Tests all required flags (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually and in combinations.

2. **Valid Input Data**: Creates a simple C program, compiles it with coverage flags, runs it with different inputs to generate two distinct `.gcda` files, and also creates a third via merging.

3. **Exhaustive Testing**:
   - Individual flag tests
   - Combined flag tests
   - Tests with `-t` and various numeric arguments
   - Invalid option test (`-z`) to trigger the `default` case and `overlap_usage()`

4. **Edge Cases**:
   - `-t` without argument
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Boundary threshold values
   - Scientific notation

5. **Execution Flow**: Follows the specified flow of compile-run-generate-test-cleanup.

To use this script with a coverage-instrumented `gcov-tool`:
