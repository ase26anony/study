This script implements all the requirements:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses these as inputs.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (checks for verbose output)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags in combination
   - `-t`: Tests with valid float arguments (0.75, 0.5, 1)

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and `overlap_usage()`.

5. **Shell Script Wrapper**: The script handles compilation, execution, multiple invocations, and cleanup.

6. **Additional Tests**: Includes tests for combined options and multiple GCDA files.

To run the test:
