This script provides comprehensive testing for the uncovered lines in `gcov-tool.cc`:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually and in combination.

2. **Overlap Subcommand with Required Input Files**: Creates two C programs, compiles them with coverage instrumentation, runs them to generate `.gcda` files, and uses these as inputs.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (may not produce visible output but exercises the code path)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with various valid arguments (0.75, 0.5, 1) and invalid argument ("invalid")

4. **Invalid Option Handling**: Tests `-x` to trigger the default case and `overlap_usage()`

5. **Edge Cases**: Tests missing files, single file, and invalid arguments

6. **Cleanup**: Removes all temporary files on exit

To run the test:
