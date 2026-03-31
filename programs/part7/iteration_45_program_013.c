**Key Features of This Test Script:**

1. **Complete Flag Coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and their combinations.

2. **Invalid Flag Testing**: Includes tests with `-x` and `-Z` to trigger the `default` case in the switch statement.

3. **Real GCOV Files**: Creates actual `.gcda` and `.gcno` files by compiling and running a test program.

4. **Usage Scenarios**: Tests single files, multiple files, wildcards, and directory inputs.

5. **Error Conditions**: Tests non-existent files, permission errors, malformed files, and edge cases.

6. **Flag Combinations**: Tests both separate flags (`-l -p`) and combined flags (`-lp`, `-rps`).

**To Use This Script:**

1. First build `gcov-dump` with coverage instrumentation:
