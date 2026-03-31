This script provides comprehensive testing for the uncovered lines:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with coverage instrumentation (`-fprofile-arcs -ftest-coverage`), runs them to generate `.gcda` files, and uses these as inputs.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (checks for any output)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags
   - `-t`: Tests with various float values (0.75, 0.0, 0.5, 1.0, etc.)
   - Also tests edge cases like missing arguments and invalid numeric values

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and `overlap_usage()`.

5. **Shell Script Wrapper**: The script handles compilation, execution, multiple invocations, and cleanup.

To run the test:
