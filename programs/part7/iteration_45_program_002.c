**Key Features of This Script:**

1. **Complete Flag Coverage**: Tests all individual flags (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and their combinations
2. **Invalid Flag Testing**: Includes `-x`, `-Z`, and `-lx` to trigger the `default` case
3. **Real GCOV Files**: Creates and uses actual `.gcda`/`.gcno` files for meaningful testing
4. **Multiple Usage Scenarios**:
   - Single file analysis
   - Multiple file arguments
   - Wildcard expansion
   - Error conditions (missing files, no arguments)
5. **Output Limiting**: Uses `head` to keep output manageable while still executing all code paths
6. **Error Handling**: Uses `|| true` to continue after expected errors
7. **Cleanup**: Properly cleans up temporary files

**To Use This Script:**

1. First build `gcov-dump` with coverage instrumentation:
