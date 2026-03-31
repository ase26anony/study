**Key features of this test script:**

1. **Complete flag coverage**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`) to cover all switch cases.

2. **Flag combinations**: Tests various combinations like `-lp`, `-pr`, `-lpr`, `-spr`, `-lpsr` to ensure multiple flag-setting branches execute.

3. **Invalid flag testing**: Tests `-x`, `-Z`, and `-lx` to trigger the `default` case and error message.

4. **Real GCOV data**: Creates and uses actual `.gcda` and `.gcno` files for meaningful tests.

5. **Multiple execution contexts**: Tests single files, multiple files, wildcards, and error conditions.

6. **Error path coverage**: Tests non-existent files, permission errors, and missing arguments.

**To use this script:**

1. First build `gcov-dump` with coverage instrumentation:
