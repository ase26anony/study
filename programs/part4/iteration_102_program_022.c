This test script provides comprehensive coverage of the `gcov-dump` command-line argument parsing:

1. **Valid single-character flags**: Tests `-h`, `-v`, `-l`, `-p`, `-r`, `-s` individually
2. **Invalid flags**: Tests `-x`, `-z`, and `-xyz` to trigger the `default` case
3. **Combined flags**: Tests `-lp`, `-rl`, `-prs` to exercise the option parsing loop
4. **Edge cases**: Missing filename, non-existent file, no arguments, flag in middle
5. **Different file types**: Tests with both `.gcda` and `.gcno` files
6. **Error handling**: Captures stderr for error messages
7. **Long options**: Tests `--help` if supported

To run the test:
