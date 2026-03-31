This script:

1. **Creates a temporary directory** for isolation
2. **Generates a simple C program** and compiles it with coverage instrumentation (`-fprofile-arcs -ftest-coverage`)
3. **Runs the program** to create a valid `.gcda` file
4. **Tests each individual flag** (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
5. **Tests combined flags** (`-l -p -s`)
6. **Triggers the default case** with invalid flags (`-x` and `-Z`)
7. **Handles errors gracefully** using `|| true` for expected failures
8. **Cleans up** the temporary directory

To run this test, save it as `test_gcov_dump.sh` and make it executable:
