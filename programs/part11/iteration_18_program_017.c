This script comprehensively tests all the requirements:

1. **Command-Line Argument Matrix**: Tests each flag individually (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) and in various combinations.

2. **Valid Input Data Files**: Creates a minimal C program, compiles it with coverage flags, and generates three distinct `.gcda` files by running the program with different arguments.

3. **Edge Case and Error Testing**:
   - Tests `-t` with integer (1), fractional (0.33, 0.75), and edge values (0, 0.001, 1.0, 2.0)
   - Tests `-h` combined with `-t`
   - Tests flag order variations
   - Tests invalid option `-x` to trigger the `default:` case and `overlap_usage()`

4. **Output and Redirection**: Redirects output to files for successful commands and captures stderr for error cases.

5. **Multi-File Merge Scenario**: Uses 2-3 distinct `.gcda` files to exercise merge logic influenced by the flags.

6. **Script Automation and Cleanup**: Self-contained script with cleanup function to remove generated files.

To execute this test:
