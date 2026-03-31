This script provides comprehensive testing of the specific lines in `gcov-tool.cc`:

1. **Command-Line Argument Matrix**: Tests each flag individually (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) and in various combinations.

2. **Valid Input Data Files**: Creates a minimal C program, compiles it with coverage flags, and generates three distinct `.gcda` files by running the program with different arguments.

3. **Edge Case and Error Testing**:
   - Tests `-t` with integer (1), fractional (0.5, 0.33, 0.001), zero (0), and >1 (1.5) values
   - Tests `-h` combined with `-t` (Test 15)
   - Tests flags in different orders (Tests 12, 13)
   - Triggers the `default:` case with invalid option `-x` (Test 16)
   - Tests `-t` without argument (Test 17)

4. **Output Redirection**: All commands redirect output to files to test execution paths.

5. **Multi-File Merge**: Uses 2-3 input `.gcda` files in various tests to exercise overlap analysis.

6. **Script Automation**: Self-contained script with cleanup, compilation, execution, and verification steps.

To run this test, ensure `gcov-tool` is in your PATH (built from GCC source with profiling enabled), then execute:
