This script provides comprehensive coverage of the uncovered switch statement:

1. **Covers all individual cases** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
2. **Triggers the default case** with invalid flags `-x` and `-Z`
3. **Tests combined flags** with `-l -p -s`
4. **Creates valid GCOV input** by compiling and running a test program
5. **Handles errors appropriately** using `|| true` for expected failures
6. **Provides output verification** by showing partial output from each command
7. **Includes cleanup** to remove temporary files

The script will:
- Create a simple C program
- Compile it with GCC coverage flags
- Run it to generate `.gcda` file
- Execute `gcov-dump` with all required flag combinations
- Show that the default case is triggered by invalid flags
- Clean up after itself

To run the script:
