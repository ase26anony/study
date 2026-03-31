**Key features of this test script:**

1. **Comprehensive flag testing**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to cover all switch cases.

2. **Flag combinations**: Tests various combinations like `-lp`, `-pr`, `-lpr`, `-lps`, `-rps`, and `-lprs` to ensure the flag-setting logic works correctly when multiple flags are specified.

3. **Invalid flag testing**: Tests `-x`, `-lx`, and `-Z` to trigger the `default` case and the `fprintf(stderr, "unknown flag")` line.

4. **Real GCOV files**: Creates actual `.gcda` and `.gcno` files by compiling and running a test program with coverage instrumentation.

5. **Multiple execution contexts**: Tests single file, multiple files, wildcards, and different argument patterns.

6. **Error paths**: Tests non-existent files and other error conditions to improve overall coverage.

7. **Clean execution**: Uses a temporary directory and proper cleanup to avoid leaving test files behind.

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation as described:
