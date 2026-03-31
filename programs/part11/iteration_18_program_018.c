This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Command-Line Argument Matrix**: Tests all flags (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually and in various combinations.

2. **Valid Input Data Files**: Creates a C program, compiles it with coverage flags, and generates three distinct `.gcda` files by running the program with different arguments.

3. **Edge Case and Error Testing**:
   - Tests `-t` with integer (1), fractional (0.33), and decimal (0.75) values
   - Tests `-h` combined with `-t` to test interaction
   - Tests flags in different orders (before/after input files, mixed)
   - Triggers the `default:` case with invalid option `-x`

4. **Output and Redirection**: Redirects output to files for successful commands, allows usage message to print for invalid options.

5. **Multi-File Merge Scenario**: Uses three distinct `.gcda` files to ensure overlap analysis routines are invoked.

6. **Script Automation and Cleanup**: Self-contained script that creates test files, runs all tests, and cleans up.

To execute this test:
