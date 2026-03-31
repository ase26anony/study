**Key features of this test script:**

1. **Flag Combination Stress**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) and multiple combinations (`-lp`, `-pr`, `-lps`, `-rps`, `-lprs`).

2. **Invalid Flag Trigger**: Tests `-x` and `-Z` to hit the `default` case with `fprintf(stderr, "unknown flag '%c'\n", opt)`.

3. **Valid GCOV File Input**: Creates a real C program, compiles it with `-fprofile-arcs -ftest-coverage`, runs it to generate `.gcda` files, and uses them as input.

4. **Execution Context Diversity**:
   - Help and version flags (no file needed)
   - Single file analysis (`-l test.gcda`)
   - Multiple file analysis (`-p test.gcda test.gcno`)
   - Directory scanning with wildcards (`-r *.gcda`)
   - No flags at all

5. **Error Handling**: Tests non-existent files, wrong file types, and no arguments.

6. **Clean Execution**: Uses temporary directory, proper cleanup, and error handling.

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
