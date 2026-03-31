This test script provides comprehensive coverage of the uncovered lines:

1. **Valid single-character flags**: Tests each of `-h`, `-v`, `-l`, `-p`, `-r`, `-s` individually
2. **Invalid flags**: Tests `-x`, `-z`, and combinations with invalid flags to trigger the `default` case
3. **Combined flags**: Tests `-lp`, `-rl`, `-prs` to ensure the option parsing loop processes each character
4. **Missing arguments**: Tests `-l` without a filename to trigger error paths
5. **Output redirection**: Redirects stdout/stderr to files and `/dev/null`
6. **Long options**: Tests `--help` and `--version` if supported
7. **Multiple files**: Tests with both `.gcda` and `.gcno` files

To run the test:
