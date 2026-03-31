**Key features of this test script:**

1. **Complete flag coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
2. **Flag combinations**: Tests various combinations like `-lp`, `-pr`, `-lprs`, etc.
3. **Invalid flag testing**: Includes `-x`, `-Z`, and `-lx` to trigger the `default` case
4. **Real GCOV files**: Creates and uses actual `.gcda`/`.gcno` files
5. **Multiple usage scenarios**: Single file, multiple files, wildcards, error cases
6. **Clean execution**: Uses temporary directory and proper cleanup

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
