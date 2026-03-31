This script systematically tests all the uncovered lines:

1. **Individual flags**: Tests `-h`, `-v`, `-l`, `-p`, `-r`, `-s` separately (lines 111-130)
2. **Flag combinations**: Tests various combinations like `-lp`, `-pr`, `-lprs` to ensure multiple flag-setting branches execute
3. **Invalid flags**: Tests `-x` and `-Z` to trigger the `default` case and `fprintf(stderr, ...)` error message
4. **Different execution contexts**: Tests single files, multiple files, wildcards, and error cases
5. **Valid GCOV file handling**: Creates real `.gcda` and `.gcno` files by compiling and running instrumented programs

To compile and use the instrumented `gcov-dump`:
