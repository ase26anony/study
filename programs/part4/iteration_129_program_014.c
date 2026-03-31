This script provides comprehensive coverage of all the uncovered lines:

1. **Self-contained test environment**: Creates a temporary directory for all test files
2. **Valid GCOV data generation**: 
   - Creates a minimal C program
   - Compiles with `-fprofile-arcs -ftest-coverage`
   - Executes to generate `.gcda` file
3. **Individual flag testing**: Tests each switch case individually (`-h`, `-v`, `-l`, `-p`, `-r`, `-s`)
4. **Combined flags**: Tests `-l -p -s` together
5. **Default case triggering**: Tests invalid flags `-x` and `-Z` to trigger the `default:` branch
6. **Error handling**: Properly handles stderr output for invalid flags
7. **Cleanup**: Removes temporary files (can be disabled for debugging)

To run the test:
