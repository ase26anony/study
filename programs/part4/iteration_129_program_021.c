This script provides comprehensive coverage of the uncovered switch statement:

1. **Setup**: Creates a temporary directory and a minimal C program
2. **Compilation**: Uses `gcc -fprofile-arcs -ftest-coverage -O0` to create an instrumented binary
3. **Profile Generation**: Runs the program to create `test.gcda`
4. **Individual Flag Tests**: Tests each switch case (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
5. **Default Case**: Tests invalid flags (`-x`, `-Z`) to trigger the `default` branch
6. **Combined Flags**: Tests multiple valid flags together (`-l -p -s`)
7. **Error Handling**: Uses `|| true` to continue even when `gcov-dump` exits with error for invalid flags

To run this test:
