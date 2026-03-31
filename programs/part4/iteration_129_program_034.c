This script provides comprehensive coverage of all the requirements:

1. **Self-contained shell script** that creates everything it needs
2. **Covers all individual switch cases** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
3. **Triggers the default case** with invalid flags `-x` and `-Z`
4. **Tests combined flags** with `-l -p -s`
5. **Generates valid GCOV data** by:
   - Creating a simple C program
   - Compiling with `-fprofile-arcs -ftest-coverage`
   - Running the program to generate `.gcda` file
6. **Handles output and errors** appropriately:
   - Uses `2>&1` to capture both stdout and stderr
   - Checks for expected output patterns
   - Doesn't fail on expected errors (invalid flags)
7. **Includes additional edge cases**:
   - No arguments
   - Non-existent file
8. **Provides cleanup** with trap mechanism
9. **Verifies each step** with appropriate error checking

To run the test:
