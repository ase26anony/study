This script comprehensively tests all the uncovered lines in the specified switch statement:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) individually and in combination.

2. **Overlap Subcommand with Required Input Files**: Creates two C programs, compiles them with coverage instrumentation, runs them to generate `.gcda` files, and uses these as inputs.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with valid float arguments (0.5, 0.75, 0.3) and invalid arguments

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and `overlap_usage()`.

5. **Error Cases**: Also tests edge cases like:
   - Invalid argument for `-t` (non-numeric)
   - Missing argument for `-t`
   - Insufficient positional arguments

The script creates minimal C programs that generate meaningful coverage data, ensuring the `gcov-tool overlap` command has actual data to process. Each test checks the exit code to ensure the options are being parsed correctly.

To run this test:
