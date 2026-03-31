**Key Features of This Script:**

1. **Complete Flag Coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and various combinations.

2. **Invalid Flag Testing**: Includes `-x` and `-Z` to trigger the `default` case with `fprintf(stderr, "unknown flag...")`.

3. **Real GCOV Files**: Creates and compiles actual C programs to generate valid `.gcda` and `.gcno` files.

4. **Multiple Usage Scenarios**:
   - Help and version flags (no files needed)
   - Single file analysis
   - Multiple file analysis
   - Wildcard expansion
   - Error conditions (non-existent files, no arguments)

5. **Flag Combination Stress**: Tests both separate flags (`-l -p`) and compact forms (`-lp`, `-rps`).

**To Use This Script:**

1. First build `gcov-dump` with coverage instrumentation:
