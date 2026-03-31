This script:

1. **Creates a temporary workspace** to avoid polluting the current directory
2. **Generates a minimal C program** that's easy to compile and run
3. **Compiles with coverage instrumentation** using `-fprofile-arcs -ftest-coverage`
4. **Runs the program** to generate the `.gcda` file that `gcov-dump` needs
5. **Tests each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
6. **Triggers the default case** with invalid flags (`-x` and `-Z`)
7. **Tests flag combinations** including all valid flags together
8. **Includes error handling** - continues even when `gcov-dump` returns an error (for invalid flags)
9. **Cleans up automatically** using a trap to remove temporary files

To run this test:
