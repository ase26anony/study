**Key Features of This Script:**

1. **Complete Flag Coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to cover all switch cases.

2. **Invalid Flag Testing**: Includes tests with `-x` and `-Z` flags to trigger the `default` case and the `fprintf(stderr, ...)` line.

3. **Flag Combinations**: Tests various combinations like `-lp`, `-rs`, `-lprs` to exercise multiple flag-setting branches.

4. **Real GCOV Files**: Creates and compiles actual C programs to generate valid `.gcda` and `.gcno` files for testing.

5. **Usage Scenarios**: Tests different invocation patterns:
   - Help and version flags (no files needed)
   - Single file analysis
   - Multiple file analysis
   - Wildcard expansion
   - No flags with just a file

6. **Error Paths**: Tests error conditions like non-existent files and wrong file types.

7. **Clean Execution**: Uses temporary directory and proper cleanup.

**To Use This Script:**

1. First build `gcov-dump` with coverage instrumentation:
