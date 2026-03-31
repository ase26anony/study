This script provides comprehensive coverage of the switch statement in `gcov-dump.cc`:

1. **Setup**: Creates a temporary directory and minimal C program
2. **Compilation**: Compiles with coverage flags (`-fprofile-arcs -ftest-coverage`)
3. **Profile Generation**: Runs the program to create `.gcda` file
4. **Flag Testing**: Tests each individual flag (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
5. **Combined Flags**: Tests `-l -p -s` together
6. **Invalid Flags**: Tests `-x` and `-Z` to trigger the `default` case
7. **Edge Cases**: Tests with no flags and with multiple files

To run this script:
