This script provides comprehensive testing for the uncovered lines:

1. **Command-Line Argument Parsing**: Tests each option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates and compiles minimal C programs with coverage instrumentation, runs them to generate `.gcda` files, and uses them as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (may not produce visible output but executes the code path)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with various numeric arguments (0.75, 0.5, 1)

4. **Invalid Option Handling**: Tests `-x` to trigger the default case and call `overlap_usage()`.

5. **Additional Tests**: Includes tests with combined options and multiple `.gcda` files.

To run the test:
