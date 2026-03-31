This script provides comprehensive testing for the uncovered lines:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) both individually and in combination.

2. **Required GCDA Files**: Creates two valid C programs, compiles them with coverage instrumentation, runs them to generate `.gcda` files, and uses these as inputs.

3. **Option-Specific Behavior**:
   - `-v`: Triggers verbose mode
   - `-f`, `-F`, `-o`, `-h`: Sets boolean flags
   - `-t`: Tests with valid floats (0.75, 0.5, 1) and invalid strings

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and call `overlap_usage()`.

5. **Edge Cases**: Tests missing arguments, insufficient files, non-existent files, and invalid threshold values.

To run the test:
