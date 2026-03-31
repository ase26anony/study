This script implements all the requirements:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses these as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode (Test 1)
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags (Test 2)
   - `-t`: Tests with valid float arguments (0.75, 0.5, 1) (Tests 3-5)
   - Invalid arguments: Tests missing argument and non-numeric argument for `-t` (Tests 8-9)

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case (Test 6)

5. **Shell Script Wrapper**: The script handles compilation, execution, cleanup, and validates exit codes.

To run the test:
