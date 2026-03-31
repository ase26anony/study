This script provides comprehensive testing of the uncovered lines:

1. **Creates valid GCDA files**: Compiles and runs two different C programs to generate `.gcda` files with different coverage profiles.

2. **Tests individual flags**: Each of the target flags (`-v`, `-f`, `-F`, `-o`, `-h`, `-t`) is tested individually.

3. **Tests flag combinations**: Multiple combinations of flags are tested to ensure they work together.

4. **Tests edge cases**:
   - `-t` without argument (triggers error handling)
   - `-t` with non-numeric argument
   - Same file twice
   - Non-existent files
   - Invalid option `-z` (triggers the `default` case calling `overlap_usage()`)

5. **Tests various threshold values**: Different floating-point values for the `-t` flag.

6. **Tests file handling**: Different file orders and invalid numbers of arguments.

To use this script with a coverage-instrumented `gcov-tool`:
