This test script comprehensively covers all the requirements:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) both individually and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with coverage instrumentation (`-fprofile-arcs -ftest-coverage`), runs them to generate `.gcda` files, and uses these as input to `gcov-tool overlap`.

3. **Option-Specific Behavior**:
   - `-v`: Tests verbose mode and checks for verbose output
   - `-f`, `-F`, `-o`, `-h`: Tests boolean flags individually and combined
   - `-t`: Tests with valid float values (0.75, 0.5, etc.) and invalid arguments

4. **Invalid Option Handling**: Tests `-x` to trigger the default case and usage display.

5. **Shell Script Wrapper**: The script handles compilation, execution, multiple invocations, and cleanup.

**Key Features:**
- Uses `trap` for reliable cleanup of temporary files
- Tests edge cases (insufficient arguments, invalid threshold values)
- Provides clear pass/fail feedback for each test
- Documents which specific lines from `gcov-tool.cc` are being tested
- Handles variations in `gcov-tool` output across different versions

To run the test:
