This test script provides comprehensive coverage of the uncovered lines:

1. **Valid single-character flags**: Tests `-h`, `-v`, `-l`, `-p`, `-r`, `-s` individually
2. **Invalid flags**: Tests `-x` and `-z` to trigger the `default` case
3. **Combined flags**: Tests `-lp`, `-rl`, `-prs` to exercise the option parsing loop
4. **Missing arguments**: Tests `-l` and `-p` without filenames
5. **Long options**: Tests `--help` and `--version` if supported
6. **Edge cases**: No arguments, multiple files, flag placement variations
7. **Output verification**: Captures stderr for invalid flags to verify error messages

To run the test:
