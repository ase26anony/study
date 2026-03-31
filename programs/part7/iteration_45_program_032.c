**Key Features of This Script:**

1. **Complete Flag Coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and numerous combinations to ensure all flag-setting branches are executed.

2. **Invalid Flag Testing**: Includes tests with `-x` and `-Z` to trigger the `default` case and the `fprintf(stderr, ...)` line.

3. **Real GCOV Files**: Creates and compiles actual C programs with coverage instrumentation, then runs them to generate valid `.gcda` and `.gcno` files for testing.

4. **Multiple Usage Scenarios**:
   - Help and version flags (no file arguments)
   - Single file analysis with various flags
   - Multiple file arguments
   - Wildcard expansion
   - Error conditions (missing files, no arguments)

5. **Comprehensive Combinations**: Tests 15 different flag combinations to ensure thorough coverage of the switch statement logic.

**To Use This Script:**

1. First build `gcov-dump` with coverage instrumentation:
