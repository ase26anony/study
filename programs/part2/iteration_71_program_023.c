This script implements all the requirements:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses these as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode and checks for successful execution
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with various numeric arguments (0.75, 0.5, 1)
   - Invalid option `-x`: Triggers the default case to call `overlap_usage()`

4. **Error Handling**: Tests invalid option and missing argument for `-t`.

5. **Cleanup**: Uses trap to clean up temporary files on exit.

To run the test:
