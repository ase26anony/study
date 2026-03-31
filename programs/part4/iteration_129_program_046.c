This script:

1. **Creates a temporary directory** to avoid polluting the current directory
2. **Generates a valid GCOV data file** by:
   - Creating a simple C program
   - Compiling it with `-fprofile-arcs -ftest-coverage`
   - Running it to produce `test.gcda`
3. **Tests all required flag combinations**:
   - `-h` and `-v` (no file needed)
   - `-l`, `-p`, `-r`, `-s` (with `test.gcda`)
   - Combined flags `-l -p -s`
   - Invalid flags `-x` and `-Z` to trigger the `default` case
4. **Includes additional tests** for edge cases:
   - No arguments
   - Flag without file argument
   - Multiple files
5. **Handles output appropriately** by piping stderr to stdout and limiting output with `head`
6. **Cleans up** the temporary directory
7. **Prints a success message** when all tests complete

To run this script:
