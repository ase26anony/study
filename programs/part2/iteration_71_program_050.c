This script implements all the requirements:

1. **Command-Line Argument Parsing**: Tests each short option (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) separately and in combination.

2. **Overlap Subcommand with GCDA Files**: Creates two C programs, compiles them with `-fprofile-arcs -ftest-coverage`, runs them to generate `.gcda` files, and uses these as inputs.

3. **Option-Specific Behavior**:
   - `-v`: Triggers `gcov_set_verbose()`
   - `-f`, `-F`, `-o`, `-h`: Boolean flags tested in combination
   - `-t`: Tested with valid floats (0.75, 0.5) and integer (1), plus invalid non-numeric argument

4. **Invalid Option Handling**: Tests `-x` to trigger the `default:` case and `overlap_usage()`

5. **Shell Script Wrapper**: The script handles compilation, execution, multiple invocations, and cleanup.

**To run the test:**
