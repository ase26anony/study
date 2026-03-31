This script provides comprehensive testing of the uncovered lines in `gcov-tool.cc`:

1. **Creates valid .gcda files** by compiling and running a simple C program with different inputs
2. **Tests each individual flag** (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as specified in the uncovered lines
3. **Tests flag combinations** to ensure they work together
4. **Triggers the default case** (line 553) by using an invalid option `-z` (Test 4a)
5. **Tests edge cases** for the `-t` flag including missing arguments, non-numeric values, and boundary values
6. **Tests error conditions** like non-existent files, same file twice, etc.
7. **Cleans up** all temporary files after execution

To use this script with a coverage-instrumented `gcov-tool`:
