This script provides comprehensive testing for the uncovered lines in `gcov-tool.cc`:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) as specified in the uncovered lines.

2. **Overlap Subcommand with GCDA Files**: Creates and compiles minimal C programs with coverage instrumentation, runs them to generate `.gcda` files, and uses them as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (may trigger `gcov_set_verbose()` internally)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with valid float arguments (0.75, 1.5) and missing arguments

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and call `overlap_usage()`.

5. **Shell Script Wrapper**: The script handles compilation, execution, testing, and cleanup of all temporary files.

To run the test:
