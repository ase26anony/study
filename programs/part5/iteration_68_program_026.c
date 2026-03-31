This script implements all the requirements:

1. **Command-Line Argument Generation**: Tests all target flags (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) with `gcov-tool overlap` command.

2. **Valid Input Data**: Creates a simple C program, compiles it with coverage flags, runs it to generate `.gcda` files, and creates a second `.gcda` file for comparison.

3. **Exhaustive Flag Combination Testing**:
   - Individual flag tests (Tests 1-6)
   - Combined flag tests (Tests 7-10)
   - `-t` flag with valid argument (Test 6, 9, 10)
   - Invalid option `-z` to trigger `default` case (Test 15)

4. **Edge Case and Error Handling**:
   - `-t` without argument (Test 11)
   - `-t` with non-numeric argument (Test 12)
   - Same input file twice (Test 13)
   - Non-existent input files (Test 14)
   - Different threshold values (loop test)
   - Different flag orders

5. **Execution Flow**: Follows the specified flow:
   - Compiles source with coverage
   - Runs program to generate GCDA
   - Creates second GCDA file
   - Systematically invokes `gcov-tool overlap` with various flags
   - Cleans up temporary files

To use this script with coverage-instrumented `gcov-tool`:
